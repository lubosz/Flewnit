/*
 * RadixSorter.cpp
 *
 *  Created on: Apr 22, 2011
 *      Author: tychi
 */

#include "RadixSorter.h"

#include "Util/HelperFunctions.h"
#include "Simulator/ParallelComputeManager.h"
#include "MPP/OpenCLProgram/RadixSortProgram.h"
#include "Util/Log/Log.h"

#include "URE.h"
#include "Util/Time/FPSCounter.h"
#include "Buffer/PingPongBuffer.h"
#include "MPP/OpenCLProgram/CLProgramManager.h"
#include "MPP/OpenCLProgram/CLKernelArguments.h"

#include "Buffer/Buffer.h"
#include "Buffer/IntermediateResultBuffersManager.h"


//don't know if hardcode is necessary, but numbers deep in source code are even worse ;(
//TODO outsource to XML
#define FLEWNIT_NUM_RADIX_SORT_PASSES 3

#define FLEWNIT_FERMI_NUM_KEY_ELEMENTS_PER_RADIX_COUNTER 4
#define FLEWNIT_NON_FERMI_NUM_KEY_ELEMENTS_PER_RADIX_COUNTER 8

#define FLEWNIT_WARP_SIZE 32


namespace Flewnit
{

RadixSorter::RadixSorter(
		unsigned int numElements,
		unsigned int numBitsPerKey //should be log2ui(numPaxParticles)
									//= 15 for 2^15== 32786
									//= 18 for 2^18==262144
		//implementation dependent stuff, default zero values indicate that optimal
		//(e.g. hardware dependent) values shall be chosen automatically
		//unsigned int numRadicesPerPass,
		//unsigned int numElementsPerRadixCounter
)
:
	mNumElements(numElements),
	mNumBitsPerKey(numBitsPerKey),

	mNumRadixSortPasses(FLEWNIT_NUM_RADIX_SORT_PASSES),
	mNumRadicesPerPass(
		1 <<  //pow2 of..
		(
			//log2 of numRadicesPerPass
			//kinda dumb "metric", but time pressure...
			//numBitsPerKey==14 --> 15 --> 5 ; one radix too much in the last pass, but let's not make is complicated..
			//numBitsPerKey==15 --> 15 --> 5
			//numBitsPerKey==18 --> 18 --> 6
			HelperFunctions::ceilToNextMultiple(numBitsPerKey,FLEWNIT_NUM_RADIX_SORT_PASSES)
			/ FLEWNIT_NUM_RADIX_SORT_PASSES
		)
	),
	mNumElementsPerRadixCounter(
		//nvidia fermit devices have compute capability 2.x
		PARA_COMP_MANAGER->getParallelComputeDeviceInfo().nvidiaDeviceComputeCapability.x >= 2
		? FLEWNIT_FERMI_NUM_KEY_ELEMENTS_PER_RADIX_COUNTER
		: FLEWNIT_NON_FERMI_NUM_KEY_ELEMENTS_PER_RADIX_COUNTER
	),

	//for fermi: 			32768 (32kB out of 48kB available);
	//for GT200 and G80: 	 8192 ( 8kB out of 16kB available);
	mLocalMemForRadixCounters(
		//We cannot use the whole local memory of the OpenCL device;
		//We want a power of two value;
		//--> the trick: sub 1 from localMemorySize, florr it to next lower base2
		//GT200 and G80: floorPow2(16kb-1)=  8 kb
		//fermi		   : floorPow2(48kb-1)= 32 kb
		HelperFunctions::floorToNextPowerOfTwo(
			static_cast<unsigned int>
				( PARA_COMP_MANAGER->getParallelComputeDeviceInfo().localMemorySizeByte -1 )
		)
	),
	//Geforxe GT  435M :  2 -->  2
	//Geforce GTX 280  : 30 --> 32 <-- bottleneck expected, last work group will be alone :(
	//Geforce GTX 570  : 15 --> 16 <-- bottleneck expected, last work group will be alone :(
	//Geforce GTX 580  : 16 --> 16
	mNumComputeUnits_Base2Ceiled(
		HelperFunctions::ceilToNextPowerOfTwo(
			( PARA_COMP_MANAGER->getParallelComputeDeviceInfo().maxComputeUnits )
		)
	),

	//( mLocalMemForRadixCounters / ( 4 Byte * mNumRadicesPerPass ) )
    //for fermi architectures, this should be 2^15 Bytes / (2^2 BytesPerCounter * 2^6 radix counter arrays) = 2^7 = 128
    //for GT200 architectures, this should be 2^13 Bytes / (2^2 BytesPerCounter * 2^6 radix counter arrays) = 2^5 =  32
	mNumRadixCountersPerRadixAndWorkGroup(
		mLocalMemForRadixCounters / (4 * mNumRadicesPerPass)
	),

	mNumWorkGroups_TabulationAndReorderPhase(
	  mNumElements  / ( mNumRadixCountersPerRadixAndWorkGroup * mNumElementsPerRadixCounter)
	),


	mRadixSortProgram(0)
{
	assert( HelperFunctions::isPowerOfTwo(mNumElements) );
	assert( (mNumBitsPerKey <= 32 ) && (mNumBitsPerKey >= 1) );

	while(mNumWorkGroups_TabulationAndReorderPhase < 2 * FLEWNIT_WARP_SIZE)
	{
		//we seem to have quite few elements (<64k);
		//global scan phase would have some idle threads; reduce mNumElementsPerRadixCounter to compensate ;)
		if(mNumElementsPerRadixCounter > 1)
		{
			mNumElementsPerRadixCounter /= 2;
			mNumWorkGroups_TabulationAndReorderPhase *= 2;
		}
		else
		{
			LOG<<WARNING_LOG_LEVEL<< "RadixSorter: your element count seems ridiculously low ( "
				<<mNumElements <<" ); Be brave to increase it"
				"to profit from the GPU power;\n";
			break;
		}
	}


	//create after assertions so that the RadixSortProgram class does not have to catch those cases;
	mRadixSortProgram = new RadixSortProgram(this);

}

RadixSorter::~RadixSorter()
{
	//nothing to delete
}


/*
 * input: - keysbuffer: contains numElements uint elements, where the most significant
 * 						(32-numBitsPerKey) bits are zero, to be sorted in ascending order;
 * 		  - oldIndicesBuffer: contains numElements uint elements, their values are irrelevant;
 * output:- keysBuffer: the active of the ping pong buffer contains the sorted keys, the inactive
 * 						contains irrelevant values;
 * 		  - oldIndicesBuffer: the active of the ping pong buffer contains the old indices of the
 * 						now-sorted key array, so that buffers associated to the key array can be
 * 						reordered according to this index buffer:
 * 							reorderedValues[currentIndex] = unorderedValues[oldIndicesBuffer[currentIndex]];
 * 						The inactive buffer contains irrelevant values;
 *
 * */
void RadixSorter::sort(PingPongBuffer* keysBuffer, PingPongBuffer* oldIndicesBuffer)
{
	cl::Event eventToWaitFor;
	EventVector eventVec; //haxx for first pass debugging TODO remove when radix sort works;
	switch (URE_INSTANCE->getFPSCounter()->getTotalRenderedFrames()) {
		case 0:
			eventToWaitFor =
				CLProgramManager::getInstance().getProgram("_initial_updateForce_integrate_calcZIndex.cl")
					->getKernel("kernel_initial_CalcZIndex")->getEventOfLastKernelExecution();
			eventVec.push_back(eventToWaitFor);
			break;
		case 1:
			eventToWaitFor =
				CLProgramManager::getInstance().getProgram("_initial_updateForce_integrate_calcZIndex.cl")
					->getKernel("kernel_initial_updateForce_integrate_calcZIndex")->getEventOfLastKernelExecution();
			//TODO uncomment when finished with debugging the first pass;
			//eventVec.push_back(eventToWaitFor);
			break;
		default:
			eventToWaitFor =
				CLProgramManager::getInstance().getProgram("updateForce_integrate_calcZIndex.cl")
					->getKernel("kernel_updateForce_integrate_calcZIndex")->getEventOfLastKernelExecution();
			//TODO uncomment when finished with debugging the first pass;
			//eventVec.push_back(eventToWaitFor);
			break;
	}

	CLKernel* phase1Kernel = mRadixSortProgram->getKernel("kernel_radixSort_tabulate_localScan_Phase");

	//set key buffer argument, not changing over the passes but via toggling, and toggle is handled
	//CLBufferKernelArgument class automatically
	phase1Kernel->getCLKernelArguments()->getBufferArg("gKeysToSort")->set(keysBuffer);



	CLKernel* phase2Kernel = mRadixSortProgram->getKernel("kernel_radixSort_globalScan_Phase");
	//only intermediate buffers for phase 2, all "fixed", no "changing buffer bindings" to set



	CLKernel* phase3Kernel = mRadixSortProgram->getKernel("kernel_radixSort_reorder_Phase");

	//bind active key buffer for reading
	phase3Kernel->getCLKernelArguments()->getBufferArg("gKeysToSort")->set(keysBuffer,false);
	//inactive for writing
	phase3Kernel->getCLKernelArguments()->getBufferArg("gReorderedKeys")->set(keysBuffer, true);
	//bind active key oldIndicesBuffer for reading
	phase3Kernel->getCLKernelArguments()->getBufferArg("gOldIndices")->set(oldIndicesBuffer, false);
	//inactive for writing
	phase3Kernel->getCLKernelArguments()->getBufferArg("gReorderedOldIndices")->set(oldIndicesBuffer, true);


	//{
		//test first run TODO delete
	phase1Kernel->getCLKernelArguments()->getValueArg<unsigned int>("numPass")->setValue(0);
	phase1Kernel->run( eventVec );
	dumpBuffers("initialRadixSortPhase1Dump",URE_INSTANCE->getFPSCounter()->getTotalRenderedFrames(),
			0, keysBuffer,oldIndicesBuffer);
	//}


//	for(unsigned int currentPass = 0; currentPass < mNumRadixSortPasses; currentPass++)
//	{
//		//--------------------------------------------------------------------------
//		//phase 1
//
//		//only the "numPass" argument - guess what - changes over the passes ;)
//		phase1Kernel->getCLKernelArguments()->getValueArg<unsigned int>("numPass")->setValue(currentPass);
//
//		phase1Kernel->run( EventVector{eventToWaitFor} );
//
//		eventToWaitFor = phase1Kernel->getEventOfLastKernelExecution();
//
//		//--------------------------------------------------------------------------
//		//phase 2
//
//		phase2Kernel->getCLKernelArguments()->getValueArg<unsigned int>("numPass")->setValue(currentPass);
//
//		phase2Kernel->run( EventVector{eventToWaitFor} );
//
//		eventToWaitFor = phase2Kernel->getEventOfLastKernelExecution();
//
//		//--------------------------------------------------------------------------
//		//phase 3
//
//		phase2Kernel->getCLKernelArguments()->getValueArg<unsigned int>("numPass")->setValue(currentPass);
//
//		phase3Kernel->run( EventVector{eventToWaitFor} );
//
//		//make phase 1 weit for phase 3 to finish:
//		eventToWaitFor = phase3Kernel->getEventOfLastKernelExecution();
//
//
//		//##########################################################################
//		//do the buffer toggle
//		keysBuffer->toggleBuffers();
//		oldIndicesBuffer->toggleBuffers();
//
//	}

	//keysBuffer and oldIndicesBuffer should have their sorted resp reordered values in there active component
	//now ;)
}


//internal helper for buffer dump:
uint RadixSorter::getRadix(uint key, uint numPass)
{
	static const int log2NumRadicesPerPass = HelperFunctions::log2ui(mNumRadicesPerPass);
    //shift relevant radix to LSB, mask higher bits
    return ( key >> (numPass * log2NumRadicesPerPass ) ) & (mNumRadicesPerPass -1) ;
}


void RadixSorter::dumpBuffers(String dumpName, unsigned int frameNumber, unsigned int currentRadixPass,
		PingPongBuffer* keysBuffer, PingPongBuffer* oldIndicesBuffer)
{
	static const unsigned int log2NumRadicesPerPass = HelperFunctions::log2ui(mNumRadicesPerPass);

	keysBuffer->readBack(true);
	oldIndicesBuffer->readBack(true);

	IntermediateResultBuffersManager* imrm =
			CLProgramManager::getInstance().getIntermediateResultBuffersManager();

	imrm->getBuffer(0)->readBack(true);
	imrm->getBuffer(1)->readBack(true);
	imrm->getBuffer(2)->readBack(true);
	imrm->getBuffer(3)->readBack(true);


	std::fstream fileStream;
	Path path =
		Path( FLEWNIT_DEFAULT_OPEN_CL_KERNEL_SOURCES_PATH )
		/ String("bufferDumps")
		/
		Path(
			String("bufferDump_")+ dumpName + String("_")+
			HelperFunctions::toString(frameNumber)+String(".txt")
		);


	fileStream.open(
		path.string().c_str(),
		std::ios::out
	);



	unsigned int* locallyScannedRadixCounters =
		reinterpret_cast<unsigned int*>(imrm->getBuffer(0)->getCPUBufferHandle());

	unsigned int* scannedSumsOfLocalRadixCounts =
		reinterpret_cast<unsigned int*>(imrm->getBuffer(1)->getCPUBufferHandle());

	//later
//	unsigned int* partiallyScannedSumsOfGlobalRadixCounts =
//		reinterpret_cast<unsigned int*>(imrm->getBuffer(2)->getCPUBufferHandle());
//
//	unsigned int* sumsOfPartialScansOfSumsOfGlobalRadixCounts =
//		reinterpret_cast<unsigned int*>(imrm->getBuffer(3)->getCPUBufferHandle());

	//to fight the hangups due to huge dump files :(
#define FLEWNIT_MAX_LOCAL_COUNTERS_TO_DUMP 5000


	unsigned int* keys =
		reinterpret_cast<unsigned int*>(keysBuffer->getCPUBufferHandle());

	unsigned int* oldIndices =
		reinterpret_cast<unsigned int*>(oldIndicesBuffer->getCPUBufferHandle());



	unsigned int numTotalRadixCounters = mNumElements/mNumElementsPerRadixCounter;

	fileStream
		<<"Radix sort buffer dump; Current radix pass: "<<currentRadixPass<<";\n\n ";

	fileStream
			<<"scannedSumsOfLocalRadixCounts dump:\n ";

	uint* probeSums = new uint[mNumWorkGroups_TabulationAndReorderPhase];
	uint totalSum=0;
	for(unsigned int globalCounterRunner = 0 ; globalCounterRunner< mNumWorkGroups_TabulationAndReorderPhase; globalCounterRunner++)
	{
		probeSums[globalCounterRunner]=0;
	}

	for(unsigned int globalRadixRunner = 0 ; globalRadixRunner< mNumRadicesPerPass; globalRadixRunner++)
	{
		fileStream
				<<"Current radix: "<<globalRadixRunner<<";\n ";
		for(unsigned int globalCounterRunner = 0 ; globalCounterRunner< mNumWorkGroups_TabulationAndReorderPhase; globalCounterRunner++)
			{
			probeSums[globalCounterRunner] +=
					scannedSumsOfLocalRadixCounts[
					   globalRadixRunner *  mNumRadicesPerPass + globalCounterRunner ];
			totalSum += scannedSumsOfLocalRadixCounts[
			       globalRadixRunner *  mNumRadicesPerPass + globalCounterRunner ];

				fileStream
					<<"el.("<< globalCounterRunner <<"),"
					<<"val("
					<<"("
					<< scannedSumsOfLocalRadixCounts[
					     globalRadixRunner *  mNumRadicesPerPass + globalCounterRunner ]
					<<"), ";
			}
		fileStream <<"\n\n";
	}

	fileStream <<"Probed sums:\n";
	for(unsigned int globalCounterRunner = 0 ; globalCounterRunner< mNumWorkGroups_TabulationAndReorderPhase; globalCounterRunner++)
	{
		fileStream
						<<"tabWorkGroup("<< globalCounterRunner <<"),"
						<<"val("
						<<"("
						<< probeSums[globalCounterRunner]
						<<"), ";
	}


	fileStream
		<<"total sum: "<<totalSum

		<<"\n\n\nFollowing locally scanned radix counters:\n\n";

	totalSum=0;

	for(unsigned int localCounterRunner = 0 ; localCounterRunner< FLEWNIT_MAX_LOCAL_COUNTERS_TO_DUMP; localCounterRunner++)
	//for(unsigned int localCounterRunner = 0 ; localCounterRunner< numTotalRadixCounters; localCounterRunner++)
	{
		fileStream
			<<"local Radix Counter number "<<localCounterRunner<<":\n";

		for(unsigned int elementRunner = 0 ; elementRunner< mNumElementsPerRadixCounter; elementRunner++)
		{
			uint currentElementIndex = mNumElementsPerRadixCounter * localCounterRunner + elementRunner;

			String keyBitString="";
			for(unsigned int bitRunner = 0 ; bitRunner < 32 ; bitRunner++)
			{
				keyBitString.append(
					( (keys[ currentElementIndex ] & (1<< (31-bitRunner) ) ) == 0 )
					?"0":"1"
				);
			}

			uint currentRadix = getRadix(keys[ currentElementIndex ], currentRadixPass);
			String radixBitString="";
			for(unsigned int bitRunner = 0 ; bitRunner < log2NumRadicesPerPass ; bitRunner++)
			{
				radixBitString.append(
					( (keys[ currentElementIndex ] & (1<< ((log2NumRadicesPerPass-1)-bitRunner) ) ) == 0 )
					?"0":"1"
				);
			}


			fileStream
				<<"||Key element number "<<currentElementIndex<<": "

				<<"decimal key value("
					<<keys[ currentElementIndex ]
				<<"), "
				<<"binary key value("
					<<keyBitString
				<<"), "
				<<"decimal current radix value("
					<<currentRadix
				<<"), "
				<<"binary current radix value("
					<<radixBitString
				<<") "
				<<"||\n ";
		}

		fileStream
			<<"### following locally scanned radix counters: ### \n"
			;

		totalSum=0;
		for(unsigned int radixRunner = 0 ; radixRunner< mNumRadicesPerPass; radixRunner++)
		{
			fileStream
				<<"Counter number "<<radixRunner<<":"
				<<"value("
				<<
					locallyScannedRadixCounters[
				  	   //select out of up to 64 counter arrays
					    radixRunner * numTotalRadixCounters +
				  		//grab the element belonging to this sepcific local counter set
				  		localCounterRunner
				  	]
				<<"),\n";

			totalSum+=
				locallyScannedRadixCounters[
			      //select out of up to 64 counter arrays
			      radixRunner * numTotalRadixCounters +
			      //grab the element belonging to this sepcific local counter set
			      localCounterRunner
			    ];
		}

		fileStream
				<<"total sum of local radix counters: "<<totalSum<<";\n";

	}


	fileStream.close();

	//shut down
	assert(0&&"abort on purpose after programmer requested buffer dump :)");
	//URE_INSTANCE->requestMainLoopQuit();
}


}
