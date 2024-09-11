/*
   Copyright 2000-2003 Virginia Commonwealth University


   Advisory:
1. The authors make no claim of accuracy of the information in these files or the 
   results derived from use of these files.
2. You are not allowed to re-distribute these files or the information contained within.
3. This methods and information contained in these files was interpreted by the authors 
   from various sources.  It is the users sole responsibility to verify the accuracy 
   of these files.  
4. If you find a error within these files, we ask that you to contact the authors 
   and the distributor of these files.
5. We ask that you acknowledge the source of these files in publications that use results
   derived from the input 

   Please contact us if you have any questions 
*/
#ifndef ZERODOSE_ROUTINES_H_INCLUDED
#define ZERODISE_ROUTINES_H_INCLUDED
/* Created: Sept 20, 2000: JVS 
   Modification History:
      Oct 4, 2000: JVS update so never uses current, specify trial!!!
*/
/* Definitions */
#define BITMAP_TYPE char
/* Headers */
#include "utilities.h"
#include "roi_type.h"
#include "in_out.h"
#include "dose_region.h"
/* Prototypes */
int createBitmapMask(BITMAP_TYPE *bitmap, volume_region_type *dr, roi_type *roi);
int applyBitmapMask(BITMAP_TYPE *bitmap, float *dose, int doseGridSize);
int zero_dose_outside(char *TrialName, 
                      char *fileNameStub, 
                      int ibeam, 
  		      volume_region_type *dr,
		      roi_type *roi);
#endif
