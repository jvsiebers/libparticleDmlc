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
#ifndef MLC_H_INCLUDED

#define MLC_H_INCLUDED

/* 
File name: mlc.h

	This file contains data structures for an mlc, 

Date(s)/Author(s):	May 20 1999, pjk copied from apertures.h

Modification History:


*/
/* #define VERTICES_FLOAT 0 */ /* use this if want to try to use float vertices for apertures */
#include "typedefs.h"

typedef struct mlc_str
{
    TwoDContourType contour;
}mlc_type;

#endif /* MLC_H_INCLUDED to eliminate possibility of multiple inclusion of this file */
