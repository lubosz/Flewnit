/*
 * ParticleSceneRepresentation.h
 *
 *  Created on: Apr 21, 2011
 *      Author: tychi
 */

#pragma once

#include "SceneRepresentation.h"

#include "Simulator/SimulatorMetaInfo.h"



#include "UniformGrid.h"



namespace Flewnit
{


//Pure particle buffers; Optionally initialized to invalid particles;
//All buffers have numTotalParticles elements;
//Info: for 256k particles, all particle attribute buffers consume 41* 2^20 Byte = 41 MB;
class ParticleAttributeBuffers
	: public BasicObject
{
	FLEWNIT_BASIC_OBJECT_DECLARATIONS
public:
	ParticleAttributeBuffers(unsigned int numTotalParticles,
			unsigned int invalidObjectID,
			bool initToInvalidObjectID = true);
	virtual ~ParticleAttributeBuffers();
private:
	friend class ParticleSceneRepresentation;
	//after all fluids and rigid bodies have been initialized, they have to be uploaded to the
	//cl device;
	void flushBuffers();
	//usually, all particle attribute data remains on the GPU and don't need to be read back;
	//but for debugging purposes during development, we will need the read-back functionality;
	void readBackBuffers();

	//tracking buffer for fluid objects and rigid bodies to find their belonging particles in the
	//recurrently reordered attribute buffers;
	//used during (at least fluid) rendering as index buffer;
	//mNumTotalParticles elements;
	//no ping pong necessary as no read/write or similar hazard can occur;
	Buffer* mParticleIndexTableBuffer;
	/*
	 * note: the following buffers must all be ping pong buffers, because scattered reading
	 * 		 when reordering after sorting would raise hazards otherwise;
	 * */
	//used to associate a particle with its owning fluid or rigid body object
	//and its particle ID within this object;
	PingPongBuffer* mObjectInfoPiPoBuffer; //uint ping pong buffer

	PingPongBuffer* mZIndicesPiPoBuffer; //uint ping pong buffer
	//the "backtracking" values for reoardering of physical attributes,
	//result of RadixSorter::sort()
	PingPongBuffer* mOldIndicesPiPoBuffer; //uint ping pong buffer
	PingPongBuffer* mPositionsPiPoBuffer; //vec4 ping pong buffer;
	PingPongBuffer* mDensitiesPiPoBuffer;
	PingPongBuffer* mCorrectedVelocitiesPiPoBuffer;
	//the following buffers are needed due to the usage of the velocity verlet integration
	//TODO as this considerably increases bandwidth (also of all things in scattered reordering),
	//we maybe should consider a different integration scheme where no force and only one velocity
	//is sufficient;
	//edit: the big bandwidth eater is the SPH neighbor acquisition (about 1.36 GB!),
	//so copying these buffer maybe is not the biggest memory related issue;
	PingPongBuffer* mPredictedVelocitiesPiPoBuffer;
	PingPongBuffer* mLastStepsAccelerationsPiPoBuffer;


};






class ParticleSceneRepresentation
	:public SceneRepresentation
{
	FLEWNIT_BASIC_OBJECT_DECLARATIONS;

public:

	ParticleSceneRepresentation(
		unsigned int numTotalParticles,
		unsigned int numMaxFluids,
		unsigned int numMaxRigidBodies,
		//be careful with this value: the maximum may be hardware/implementation dependent and hence may be clamped;
		//GT200: max 256; Fermi: max 1024; reason: scarce local memory,
		//hence value reloading for greater particle counts;
		//not implemented a compromise allowing greater RB particle count for lesser performance yet (april 2011)
		unsigned int numMaxParticlesPerRigidBody,
		//should be 1/3* unigrid cell size --> 27 particles/cell-->good
		float voxelSideLengthRepresentedByRigidBodyParticle
	) throw(BufferException);

	virtual ~ParticleSceneRepresentation();


	//{ routines to be called every simulation tick;

	//precondition: mParticleAttributeBuffers->mZIndicesPiPoBuffer has been sorted and
	//				mParticleAttributeBuffers->mOldIndicesPiPoBuffer has been reordered accordingly
	void reorderAttributes();

	//upload mRigidBodyBuffer and mObjectGenericFeaturesBuffer to GPU bevor Simulation tick,
	//as features may have been changed by the user;
	void flushObjectBuffers();
	//to be called after a simulation step:
	// mRigidBodyBuffer is read back and features are applied to managed ParticleRigidBody's;
	void synchronizeRigidBodies();
	//void readbackRigidBodyBuffer();

	//}

	//{getters
	inline unsigned int getNumMaxFluids()const{return mNumMaxFluids; }
	inline unsigned int getNumCurrentFluids()const{return mNumCurrentFluids; }
	inline unsigned int getNumMaxRigidBodies()const{return mNumMaxRigidBodies; }
	inline unsigned int getNumCurrentRigidBodies()const{return mNumCurrentRigidBodies; }

	inline unsigned int getInvalidObjectID()const{return mNumMaxFluids +1; }

	inline ParticleFluid* getFluid(ID fluidId )const
		{ assert(fluidId < mParticleFluids.size()); return mParticleFluids[fluidId];}
	inline ParticleRigidBody* getRigidBody(ID rbId )const
		{ assert(rbId < mParticleRigidBodies.size()); return mParticleRigidBodies[rbId];}

	inline ParticleAttributeBuffers* getParticleAttributeBuffers()const{return mParticleAttributeBuffers;}
	//}

	//{ factory routines

	//factory function; exception if not enough unassigned particles are left in the ParticleAttributeBuffers;
	ParticleFluid* createParticleFluid(
			String name,
			uint numParticles,
			//should be a ParticleLiquidVisualMaterial, but when simulating smoke, it would be a different one..
			//I'm tired of all those base class stubs; Maybe whis will be refactored once..
			VisualMaterial* visMat,
			ParticleFluidMechMat* mechMat) throw(BufferException);

	//factory function; exception if not enough unassigned particles are left in the ParticleAttributeBuffers;
	ParticleRigidBody* createParticleRigidBody(
			String name,
			uint numParticles,
			const AmendedTransform& initialGlobalTransform,
			//visual stuff for default rendering:
			const std::vector<SubObject*>& visualSubobjects,
			//should be a dabug daw material or whatsoever
			VisualMaterial* particleVisualizationMat,
			//Vector4D buffer with at least numParticles elements,
			//created by GeometryTransformer;
			//Will be put as position attribute into VertexBasedGeometry;
			//	This geometry will be put into:
			//	- a SubObject in the visual domain
			//    together with the particleVisualizationMat
			//	  for debug drawing
			//  - a SubObject in the mechanical domain
			//	  together with the mechMat
			//The relativePositionsBuffer is also copied to the associated stride within mRigidBodyRelativePositionsBuffer;
			Buffer* relativePositionsBuffer,
			ParticleFluidMechMat* mechMat) throw(BufferException);

	//} end factory routines

private:

	//internal helper routine for creating the geometry objects for fluid objects;
	VertexBasedGeometry* createGeometryFromAttributeBuffers(unsigned int particleStarIndex, unsigned int particleCount);
	//bind the Buffers of mParticleAttributeBuffers to SimulationPipelineStage::mRenderingResults;
	//called by ParticleSceneRepresentation's constructor;
	void associateParticleAttributeBuffersWithRenderingResults();

	//{ numeric members
	unsigned int mNumTotalParticles;

	unsigned int mNumMaxFluids;
	unsigned int mNumCurrentFluids;

	unsigned int mNumMaxRigidBodies;
	unsigned int mNumCurrentRigidBodies;

	unsigned int mNumMaxParticlesPerRigidBody;
	float mVoxelSideLengthRepresentedByRigidBodyParticle;
	//}


	//{ world object members
	std::vector<ParticleFluid*> mParticleFluids;
	std::vector<ParticleRigidBody*> mParticleRigidBodies;
	//}

	//{ abstracted but particle-represented-object meta info
		//structure of four floats + 1 uint + 3 pad buffer;
		//mNumMaxFluids +  mNumMaxRigidBodies +1 elements;
		//	(+1 because of the invalid object features; mass=0,density=infinite)
		//manage as uint- buffer, as this is the digital system's "most native" data type;
		Buffer* mObjectGenericFeaturesBuffer;

		//structure of mixed type buffer;
		//manage as uint- buffer, as this is the digital system's "most native" data type;
		//mNumMaxRigidBodies elements;
		Buffer* mRigidBodyBuffer;

		Buffer* mRigidBodyRelativePositionsBuffer;
	//}

	ParticleAttributeBuffers* mParticleAttributeBuffers;

	//{
	CLProgram* mCLProgram_reorderAttributes;
	//}


};


}




//legacy code TODO delete

//friend void ParticleRigidBody::registerToParticleScene();
//friend void ParticleFluid::registerToParticleScene();



//if rb has not a ParticleRigidBodyMechMat, throw exception as this indicates
//that it is not a particleized RigidBody;
//throw exception when particle count > mNumMaxParticlesPerRigidBody;
//multiply the mechanical geometry position buffer with the global transform, copy to appropriate
//stride in position buffers etc.. pp; also setup  correctedVelocities, objectInfos
//and the rest meta info buffers
//void registerRigidBody(ParticleRigidBody* rb) throw(SimulatorException);
//void registerFluid(ParticleFluid* fluid) throw(SimulatorException);


//take all "pure particle buffers" (except predicted velocities, this buffer is only needed for CL integration ),
//assign them to the appropriate semantics attribute binding point (POSITION_SEMANTICS etc.),
//bind mParticleIndexTableBuffer as index buffer with appropriate offset and particle count;
//VertexBasedGeometry* compileAttributeBuffersToGeometryForFluid(ParticleFluid* fluid);


//precondition: UniformGrid::sort() has already been called for this step,
//		 		i.e. z-Index sort has already been performed, so that
//mOldIndicesPiPoBuffer can be used to reorder the attributes;
//void reorderAttributes();
