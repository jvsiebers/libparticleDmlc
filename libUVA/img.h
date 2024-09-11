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
/* img.h
This file contains symbolic constants and prototype definitions for the 

image processing and image acquisition library routines of the 3DRTP system.
*/
#ifndef IMG_H_INCLUDED
#define IMG_H_INCLUDED				/* To prevent multiple inclusions of this file */

 /* Constants */

#ifndef EDR_TBL_SIZE
#define EDR_TBL_SIZE 256
#endif
#define IMG_DC_WATER 100.0			/* Pixel value corresponding to water in the reduced-precision image used */
	
#ifndef Z_PIX_SIZE				/* for dose calculation */
#define Z_PIX_SIZE 0.0				/* If 0, it is set to x_pix_siz */
#endif

/* pixel types - NO_PIX is a hardcopy image, defined by geometry only */
#define 	NO_PIX			0
#define 	FLOAT_PIX		1
#define 	SHORT_PIX		2
#define 	BYTE_PIX		3

#ifndef CLIPPED_IMG_STR_INCLUDED
#include	<clipped_img.h>
#endif

/* Prototype declarations and symbolic constants for clipped image routines */

/* Function mask bits */

#define BIT_MAP_ONLY_BIT 1
#define DOSE_COMP_CLIPPED_IMG_BIT 2

Function_type get_edrs ( float ct_num_scale_factor, float edrs [EDR_TBL_SIZE] );

#endif /* IMG_H_INCLUDED */
