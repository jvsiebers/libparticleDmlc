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
/* in_out.h
   prototypes and defintions for in_out.c
   routine to tell if inside or outside of a contour
   Modification History:
      Dec 2, 1998: JVS: Prototype #ifndef added
      Jan 25, 1999: JVS: add roi_type
      May 31, 2000: Change function name to in_out_roi
*/
#ifndef IN_OUT_H_INCLUDED
#define IN_OUT_H_INCLUDED

#define IN_CONTOUR  1
#define OUT_CONTOUR 0

#include "roi_type.h"
#include "apertures.h"
#ifdef myCC
// extern "C" 
#endif
int in_out(TwoDContourType *c, float x, float y);

#ifdef myCC
// extern "C" 
#endif
int in_out_roi(roi_curve_type *c, float x, float y);

#endif
