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
/* defined_values.h
   for #defines needed for make_clipped_img

   Created: JVS: 5/27/98

   Modification History:
     6/5/98: JVS: add z_pix_size
     9/2/98: JVS: incorperating into dose comp routines
     April 27, 2000: JVS: Change Z_PIX_SIZE to MAX_Z_PIX_SIZE,
                          code now uses minimum of MAX_Z_PIX_SIZE and x_pix_size, y_pix_size to
                          determine size for intepolation table
     June 1, 2005: JVS: Increase MAX_OBJ_NAME_LEN from 31 to 50..some accelertors have long names

*************************************************************************** */
#ifndef DEFINED_VALUES_H
#define DEFINED_VALUES_H

#define MAX_IMG_SEC_ID_LEN  11 
#define MAX_OBJ_NAME_LEN    50
#define MAX_PAT_ID_LEN      12
#define MAX_COMMENT_LEN     132  
#define MAX_PATH_LEN        132
#define EDR_TBL_SIZE        256
#define BYTE_PIX            3 
#define BIT_MAP_ONLY_BIT    1
#define MAX_MSG_LEN         256
#define MAX_Z_PIX_SIZE 0.1	    /* If 0, it is set to x_pix_siz */

#endif
