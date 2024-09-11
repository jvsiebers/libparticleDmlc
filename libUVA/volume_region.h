/*
   Copyright 2017 University of Virginia


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
/*
    May 15, 2017: JVS: Break off volume_region.h from dose_region.h
*********************************************************************************** */
#ifndef  VOLUME_REGION_H_INCLUDED
#define  VOLUME_REGION_H_INCLUDED


#include "typedefs.h" /* definitions of Point_ */
typedef struct {
    Point_3d_float voxel_size; /* voxel_size */
    Point_3d_float origin;     /* origin */
    Point_3d_int   n;          /* number (dimension) */
}volume_region_type;

#include "volume_region.h"
/* ********************************************************************************* */
#endif
