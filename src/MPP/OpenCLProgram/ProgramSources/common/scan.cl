

/*
  Work efficient parallel prefix sum (scan);
  Algoritm completely taken from http://http.developer.nvidia.com/GPUGems3/gpugems3_ch39.html
  
  There is an issue with the padding macro for bank conflict avoidance in the paper; See bankConflictsAvoidance.cl for further info
  
*/

   {% include "common.cl" %}
   {% include "bankConflictsAvoidance.cl" %}
   
  //--------------------- constants used in a scan context; ---------------
  //default: 1; for rigid body update kernel: 9  
  //not working with |default:"1" fillter here as i'm afraid to forget the explicit setting in the updateRigidBodies kernel;
  #define NUM_ARRAYS_TO_SCAN_IN_PARALLEL ( {{ numArraysToScanInParallel }} )
  
  #define SCAN_DATA_TYPE {{ scanDataType }} 
  //--------------------------------------------------------------------------
   


 
  
  
  //-------------------------------------------------------------------------------------
  void scanExclusive_upSweep(
    __local  SCAN_DATA_TYPE * arrayToScan, 
    __local  SCAN_DATA_TYPE * lTotalSum,   //save memory and one barrier by using local mem instead of returning a private value;
    uint numElements, uint workItemOffsetID)
  {
      {% ifnotequal numArraysToScanInParallel "1" %}
       // uint paddedNumElements = PADDED_STRIDE( numElements );
      {% endifnotequal %}
      
      int indexOffset = 1; 
      for (
        int d = (numElements >> 1); //numElements/2
        d > 0;  //TODO try optimization: d >1; this way, the last upsweep iteration is omitted; this one is totally obsolete,
                //because the last element is set to zero before the downsweep anyway
        d >>= 1  // d/=2;
      )    
      {   
        barrier(CLK_LOCAL_MEM_FENCE);  
        if (workItemOffsetID < d)  
        {  
          uint lowerIndex = indexOffset * ( 2 * workItemOffsetID + 1) -1;  
          uint higherIndex = indexOffset * ( 2 * workItemOffsetID + 2) -1;  
          //lowerIndex += CONFLICT_FREE_OFFSET(lowerIndex);  
          //higherIndex += CONFLICT_FREE_OFFSET(higherIndex);  
          uint paddedLowerIndex = CONFLICT_FREE_INDEX(lowerIndex);  
          uint paddedHigherIndex = CONFLICT_FREE_INDEX(higherIndex);
          
          {% ifequal numArraysToScanInParallel "1" %}
          
            arrayToScan[paddedHigherIndex] += arrayToScan[paddedLowerIndex];  
            
          {% endifequal %}
          {% ifnotequal numArraysToScanInParallel "1" %}
          
            //perform the next scan the on each different array:
            //#pragma unroll
            for( uint elemRunner=0; elemRunner < NUM_ARRAYS_TO_SCAN_IN_PARALLEL; elemRunner++)
            {
              //arrayToScan[ elemRunner * paddedNumElements + higherIndex ] += 
              //    arrayToScan[ elemRunner * paddedNumElements + lowerIndex ]; 
              
              arrayToScan[ CONFLICT_FREE_INDEX(elemRunner * numElements + higherIndex ) ] += 
                  arrayToScan[ CONFLICT_FREE_INDEX( elemRunner * numElements + lowerIndex ) ]; 
                   
            }
            
          {% endifnotequal %}
          
           //indexOffset *= 2; 
        }
        //indexOffset <<= 1; //indexOffset *= 2; 
        indexOffset *= 2; 
      }  
      
      
      //note: total sum copy and clear to zero of last element is done here;
      //      reason: save one if() statement + when only the total sum is needed instead of a complete scan,
      //      one can call this routine directly;
      { 
      if (workItemOffsetID == 0) 
        {% ifequal numArraysToScanInParallel "1" %}
        
  //COMMETNED OUT FOR DEBUG
     //     *(lTotalSum) = arrayToScan[ CONFLICT_FREE_INDEX( numElements - 1 ) ];
      *(lTotalSum) =get_local_id(0);    
       
          arrayToScan[ CONFLICT_FREE_INDEX( numElements - 1) ] = 0; // clear the last element
          
        {% endifequal %}
        {% ifnotequal numArraysToScanInParallel "1" %}
            //#pragma unroll
            for( uint elemRunner=0; elemRunner < NUM_ARRAYS_TO_SCAN_IN_PARALLEL; elemRunner++)
            {
              //lTotalSum[elemRunner] = arrayToScan[ elemRunner * paddedNumElements + CONFLICT_FREE_INDEX( numElements - 1 ) ];
              //arrayToScan[ elemRunner * paddedNumElements + CONFLICT_FREE_INDEX(numElements - 1) ] = 0;// clear the last element 
              
//              lTotalSum[elemRunner] = arrayToScan[ CONFLICT_FREE_INDEX( elemRunner * numElements +  numElements - 1 ) ];
              arrayToScan[ CONFLICT_FREE_INDEX( elemRunner * numElements + numElements - 1) ] = 0;// clear the last element 
            }
        {% endifnotequal %}
      } 
      
      
      
  }
  //-------------------------------------------------------------------------------------
  
  //-------------------------------------------------------------------------------------
  void scanExclusive_downSweep(__local  SCAN_DATA_TYPE * arrayToScan, uint numElements, uint workItemOffsetID)
  {
      {% ifnotequal numArraysToScanInParallel "1" %}
        uint paddedNumElements = PADDED_STRIDE( numElements );
      {% endifnotequal %}
      
      int indexOffset = numElements; 
      for (
        int d = 1;
        d < numElements;
        d <<= 1  // d *= 2;
      )    
      {   
        indexOffset >>= 1; //indexOffset /= 2; 
        barrier(CLK_LOCAL_MEM_FENCE);  
        if (workItemOffsetID < d)  
        {  
          uint lowerIndex = indexOffset * ( 2 * workItemOffsetID + 1) -1;  
          uint higherIndex = indexOffset * ( 2 * workItemOffsetID + 2) -1;  
          lowerIndex += CONFLICT_FREE_OFFSET(lowerIndex);  
          higherIndex += CONFLICT_FREE_OFFSET(higherIndex);    
          
          {% ifequal numArraysToScanInParallel "1" %}
             SCAN_DATA_TYPE tempVal = arrayToScan[lowerIndex];
            arrayToScan[lowerIndex] = arrayToScan[higherIndex];  
            arrayToScan[higherIndex] += tempVal;  
          {% endifequal %}
          {% ifnotequal numArraysToScanInParallel "1" %}
            SCAN_DATA_TYPE tempVal;
            //#pragma unroll
            for( uint elemRunner=0; elemRunner < NUM_ARRAYS_TO_SCAN_IN_PARALLEL; elemRunner++)
            {
              tempVal = arrayToScan[elemRunner * paddedNumElements + lowerIndex ];
              arrayToScan[elemRunner * paddedNumElements + lowerIndex ] = arrayToScan[elemRunner * paddedNumElements + higherIndex];  
              arrayToScan[elemRunner * paddedNumElements + higherIndex] += tempVal;  
            }
          {% endifnotequal %}
          
        }
      }  
  }
  //-------------------------------------------------------------------------------------
  
  
  
   //-------------------------------------------------------------------------------------
   //returns the total sum
   void scanExclusive(
      __local  SCAN_DATA_TYPE * arrayToScan, //correctly padded element input/output array
      __local  SCAN_DATA_TYPE * lTotalSum,   //save memory and one barrier by using local mem instead of returning a private value;
      uint numElements, 
      uint workItemOffsetID //usually just get_local_id(0), but when scanning several small arrays within the same work group,
                            //the work item id may be smaller than workItemOffsetID in order to fit the index calculations
     
   )
   {
      //no read in, everything is at the correct place in local memory
      
      scanExclusive_upSweep(arrayToScan, lTotalSum, numElements, workItemOffsetID);
      
      //note: total sum copy and clear to zero of last element is done within upsweep;
      //      reason: save one if() statement + when only the total sum is needed instead of a complete scan,
      //      one can call scanExclusive_upSweep(..) directly;
      // SCAN_DATA_TYPE totalSum = arrayToScan[ CONFLICT_FREE_INDEX( numElements - 1 ) ];
      //if (workItemOffsetID == 0) 
      //{ 
        //(*lTotalSum) = arrayToScan[ CONFLICT_FREE_INDEX( numElements - 1 ) ];
        //arrayToScan[ CONFLICT_FREE_INDEX(numElements - 1) ] = 0;// clear the last element
      //} 
      
      scanExclusive_downSweep(arrayToScan, numElements, workItemOffsetID);
      
      //ensure that the caller can read any value without hazard;
      barrier(CLK_LOCAL_MEM_FENCE);  
      
      
      
        /*
        debug test stuff
             if (workItemOffsetID == 0) 
         
              {       
                  *(lTotalSum) = arrayToScan[ CONFLICT_FREE_INDEX( numElements - 1 ) ];
                }
         barrier(CLK_LOCAL_MEM_FENCE);  
        */
                 

   }
   
   
