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
/* compensator.h
   Structure for reading in a pinnacle compensator and beam modifiers
   Created: Oct. 20, 1999: JVS:
            Nov 4, 1999: JVS: move some info to modifiers.h
            Feb 9, 2000: JVS: Add Interpolation
            Sept 28, 2005: JVS: Add #include "defined_values.h"
          

*/
#ifndef COMPENSATOR_H_INCLUDED
#define COMPENSATOR_H_INCLUDED
#include "defined_values.h" /* for MAX_OBJ_NAME_LEN */
typedef struct
{
  int   IsValid;
  float SourceToCompensatorDistance;
  float Width;
  float Height;
  float Resolution;
  int   nx,ny; /* number of x and y elements */
  int   CompensatorHangsDown;
    int   Interpolation; /* 1 = yes (linear) , 0 = no (step-wise) */
  float OutputFactor;
  float Density;
  char  Type[MAX_OBJ_NAME_LEN];
  char  Name[MAX_OBJ_NAME_LEN];
  char  FillOutsideMode[MAX_OBJ_NAME_LEN];
  char  Fname[MAX_PATH_LEN];
  char filename[MAX_PATH_LEN];
  float *thickness;
}compensator_type;

#endif

