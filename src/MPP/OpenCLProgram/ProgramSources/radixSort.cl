/*

  (Hopefully) highly optimized radix sort:
  Basic algorithm ideas are taken from http://http.developer.nvidia.com/GPUGems3/gpugems3_ch32.html
  and http://http.developer.nvidia.com/GPUGems3/gpugems3_ch39.html;
  
  Though, there seem to be some mistakes an unnecessary bottle necks (doumentation follows);
  Hence, I had to adopt some elements of the algorithms;

  I don't try to enforce template usage at the moment; It rather confuses than helps me with structuring at the moment; 

  The goal of this thesis is to simulate 256k particles as efficently as possible;
  Due to severe time pressure, the early implementation might contain some "hardcodes" to achieve this
  goal asap; But I try to stay as generic as possible...
*/


 
 
 //target in this thesis: 2^18 = 256 k, both for numParticles and for numUniGridCells
  #define NUM_TOTAL_ELEMENTS ( {{ numTotalElements }} )
  
  //work items for scan, i.e. phase 2! Does NOT apply to tabulate phase one!
  //default: 1024; reason: maximum work group size on fermi architectures   
  #define NUM_SCAN_WORK_ITEMS_PER_WORK_GROUP ({{numScanWorkItemsPerWorkGroup}})
  
  //--------------------------------------------------------------------------------------
  //default: 3 * log2(uniGridNumCellsPerDimension) = 18
  #define NUM_BITS_PER_KEY_TO_SORT ( {{numBitsPerKey}} )
  //goal: sort 6 bits per pass, i.e. we need 2^6=64 radices at once per pass so that we need only three passes for 18 bits
  #define NUM_RADICES_PER_PASS ( {{numRadicesPerPass}} )
  //default: log2(64)=6
  #define NUM_RADICES_PER_PASS ( {{log2NumRadicesPerPass}} )
  #define NUM_RADIX_SORT_PASSES ( NUM_BITS_PER_KEY_TO_SORT / LOG2_NUM_RADICES_PER_PASS )
  //-------------------------------------------------------------------------------------

  //-------------------------------------------------------------------------------------
  //"serialization amount" in order to
  //	- reduce memory needs and/or
  //  - reduce the length of the radix counter arrays to be scanned
  //	- do more work per item for small kernels to compensate kernel invocation/management overhead (doesn't apply to radix sort)
  // default: 4; this way, we need...
  #define NUM_ELEMENTS_PER_RADIX_COUNTER ( {{numElementsPerRadixCounter }} )
  //... "only" 256k / 4 = 64k radix counters per radix ...
  #define NUM_TOTAL_RADIX_COUNTERS_PER_RADIX ( NUM_TOTAL_ELEMENTS / NUM_ELEMENTS_PER_RADIX_COUNTER )
  //...for which we need "only" 64 scan work groups @ 1024 items/WG...
  #define NUM_SCAN_WORK_GROUPS  ( NUM_TOTAL_RADIX_COUNTERS_PER_RADIX / NUM_SCAN_WORK_ITEMS_PER_WORK_GROUP )
  //...which can be performed by a single warp, so that in phase 3, for the "global scan", all but the first 32 items can be skipped
  //-------------------------------------------------------------------------------------                                                                                        
                                                                                                                                                                            
  //-------------------------------------------------------------------------------------
  //hardware dependent memory amounts determine how many radix counters a work group can own
  {% ifequal nvidiaComputeCapabilityMajor 2 %}
    //take 32 kB of 48kB available
    #define LOCAL_MEM_FOR_RADIX_COUNTERS (32768)
  {% endifequal nvidiaComputeCapabilityMajor 2 %}
  {% ifequal nvidiaComputeCapabilityMajor 1 %}
    //take 8kB of 16kB available
    #define LOCAL_MEM_FOR_RADIX_COUNTERS (8192)
  {% endifequal nvidiaComputeCapabilityMajor 1 %}
  //for fermi architectures, this should be 2^15 Bytes / (2^2 BytesPerCounter * 2^6 radix counter arrays) = 2^7 = 128
  #define NUM_RADIX_COUNTERS_PER_RADIX_AND_WORK_GROUP ( LOCAL_MEM_FOR_RADIX_COUNTERS / ( 4 * NUM_RADICES_PER_PASS ) )
  //-------------------------------------------------------------------------------------
  
  //-------------------------------------------------------------------------------------
    //one work item per key element, to that each work item can copy a key to __local memory
    //default for fermi devices: 128 radix counters per radix * 4 elements per radix counter = 512
    #define NUM_TABULATE_WORK_ITEMS_PER_WORK_GROUP (NUM_RADIX_COUNTERS_PER_RADIX_AND_WORK_GROUP * NUM_ELEMENTS_PER_RADIX_COUNTER)
    //default : 2^18/( 2^7 * 2^2) = 2^9 = 512
    #define NUM_TABULATE_WORK_GROUPS ( NUM_TOTAL_ELEMENTS  / (NUM_RADIX_COUNTERS_PER_RADIX_AND_WORK_GROUP * NUM_ELEMENTS_PER_RADIX_COUNTER) )
  //-------------------------------------------------------------------------------------
  
  //mainly dependent from shared memory consumption of a single work item (calculated by by host);
  //trade off between reduction of work group size in order to manage more work groups simultaneous
  //is subject to experimentation for non-memory bound kernels; For memory bound kernels, not more than oe work group
  //at a time fits into the local memory.
  //Minimum (in order to hide memory latency): 256; Target: multiple of 64 (recommended by GPU gems Paper); 
  //As radix sort is highly bound to loacal memory limits, there won't be more than 256 thread 
  //#define NUM_WORK_ITEMS_PER_WORK_GROUP ( {{ numWorkItemsPerWorkGroup }} )
  //#define NUM_TOTAL_WORK_ITEMS ( NUM_TOTAL_ELEMENTS  / NUM_ELEMENTS_PER_WORK_ITEM )
  


  
//-------------------------------------------------------------------------------------
  //bank conflict avoidance; see http://http.developer.nvidia.com/GPUGems3/gpugems3_ch39.html to get the idea,
  //though the formula seems wrong despite the correct explanation
  {% ifequal nvidiaComputeCapabilityMajor 2 %}
    #define NUM_BANKS 32
    #define LOG2_NUMBANKS 5
  {% endifequal nvidiaComputeCapabilityMajor 2 %}
  {% ifequal nvidiaComputeCapabilityMajor 1 %}
    //bank conflict avoidance
    #define NUM_BANKS 16
    #define LOG2_NUMBANKS 4
  {% endifequal nvidiaComputeCapabilityMajor 1 %}
  #define CONFLICT_FREE_OFFSET(n)  ( (n) >> (LOG2_NUMBANKS) )    
//-------------------------------------------------------------------------------------


  //phase one out of three in a radix sort pass
  __kernel __attribute__((reqd_work_group_size(NUM_TABULATE_WORK_ITEMS_PER_WORK_GROUP,1,1))) 
  void tabulateForRadixSort(
    __global uint* gKeysToSort, //NUM_TOTAL_ELEMENTS elements
    __global uint* gRadixCounters, //NUM_TOTAL_WORK_ITEMS * NUM_RADICES_PER_PASS elements;
                                   //is actually an array NUM_RADICES_PER_PASS radix counter arrays;
    __local uint* lRadixCounters,
    uint numPass,
  )
  {
  
  }
  
  
  
