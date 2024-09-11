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
/* translation.h
   April 27, 2000: JVS:
                   Describes translation between two coordinate system
                   Used to shift CT data set to have origin of 0,0,0 so
                   the pencil beam code results are predictable
  Aug 13, 2004: Add typedefs.h
  Sept 21, 2009: JVS: Get rid of // extern "C" so compliles well with c++ compilers
*/
#ifndef INCLUDED_TRANSLATION_H
#define INCLUDED_TRANSLATION_H
#include "typedefs.h"
typedef struct{
   float slope;
   float intercept;
} line_type;
typedef struct{
   line_type x;
   line_type y;
   line_type z;
} translation_type;
/* ******************************************************* */
/* #ifdef myCC
// extern "C"
#endif
*/
void translate_point(translation_type *t, Point_3d_float *p);
#endif
