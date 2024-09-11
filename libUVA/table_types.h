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
#ifndef TABLE_TYPES_H_INCLUDED
#define TABLE_TYPES_H_INCLUDED
/*
Modification History:
   Dec 8, 1998: JVS: Add type for twod_table_type and table_type  

*/
#include "interp.h" /* for definitions of eqs-1d_interp_tbl_str */

typedef struct {
    char name[MAX_PATH_LEN]; /* name key for identifying data in the structure */
   int num_x;  /* number of x values in a 2d table */
   float *x;   /* table of x values */
   int num_y;  /* number of y values in a 2d table */
   float  *y;  /* table of y values */

   float *data; /* data at the x/y locations in the table */
   float fine_x_step;      /* For fast interpolation along x axis */
   float  x_extensions[2]; /* For extrapolation outside the range of x's for which */
                           /* measured data are available. Extensions are +ve definite */

   float fine_y_step;
   float y_extensions[2];

   /* 1d tables of data needed for "fast interpolation" along depth axes. */
   /* These tables are obtained by computation.  Usage is explained in an example in */
   /* [lib.interp.test] */
   eqs_1d_interp_tbl_type eqs_1d_interp_tbl_x;
   eqs_1d_interp_tbl_type eqs_1d_interp_tbl_y;

} twod_table_type;

/* one-dimensional table */
typedef struct {
   char name[MAX_PATH_LEN]; /* name key for identifying data in the structure */
   int num;  /* number of x values in a 2d table */
   float *x;   /* table of x values */
   float  *y;  /* table of y values */

    /* float fine_x_step; */      /* For fast interpolation along x axis */
    /* float  x_extensions[2]; */ /* For extrapolation outside the range of x's for which */
                           /* measured data are available. Extensions are +ve definite */

   /* 1d tables of data needed for "fast interpolation" along  axes. */
   /* These tables are obtained by computation.  Usage is explained in an example in */
   /* [lib.interp.test] */
    /* eqs_1d_interp_tbl_type eqs_1d_interp_tbl; */

} oned_table_type; 

#endif
