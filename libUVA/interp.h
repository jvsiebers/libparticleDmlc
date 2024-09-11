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
/* interp.h
This file contains symbolic constants and prototype definitions for the 
library routines of all the table look-up and interpolation functions

Modification History:
   Dec 11, 1998: JVS: Add typedef.
   Jan 10, 2007: JVS: Add in #ifdef myCC in front of many functions so c++ version of pencil beam will compile
*/
#ifndef INTERP_H_INCLUDED

#define INTERP_H_INCLUDED            /* To prevent multiple inclusions of this file */

/* Symbolic Constants */

/* Data structures */

typedef struct eqs_1d_lookup_tbl_str
{
   int   *els;    /* Pointer to the equispaced array of indices.  Each element of the table */
                  /* is set equal to the index of the non-equispaced array for which the */
                  /* for which the value of the independent variable is nearest to the */
                  /* for the element in the equispaced table.  THE FIRST ELEMENT */
                  /* CORRESPONDS TO THE lower_lim (SEE BELOW). */

   int   num_els;        /* Number of elements in the equispaced table */

   float   step;         /* Step size of the equispaced table */

   float   lower_lim,    /* Upper and lower limits in multiples of step size covering the range of */
           upper_lim;    /* values of variable in equispaced table */
}eqs_1d_lookup_tbl_type;

/* Data structure for fast interpolation at equispaced points. See "Fast Interpolation" document and */
/* [lib.interp]make_1d_interp_tbl.c for details */

typedef struct eqs_1d_interp_tbl_str
{
   int   *els_lt; /* Array of indices (i) pointing to the the index of the non-equispaced */
                  /* array for which the for which the value of the independent variable */
                  /* is just less than i * step.    THE FIRST ELEMENT */
                  /* CORRESPONDS TO THE lower_lim (SEE BELOW). */


   float   *fract_dist_lt; /* "Fractional distance (i.e., independent variable)" array.  */
                           /* It is equal to the disance from the position of the equispaced */
                           /* element to the position of the non-equispaced element just less */
                           /* than the equispaced element, divided by the distance between the */
                           /* adjacent non-equispaced elements. Used for computing dependent */
                           /* variable by linear interpolation */

   int   num_els;          /* Number of elements in the equispaced table */

   int   num_non_eqs_els;  /* Number of elements in the original non-equispaced table */

   float   step;           /* Step size of the equispaced table */

   float   lower_lim,      /* Upper and lower limits in multiples of step size covering the range of */
           upper_lim;      /* values of variable in equispaced table */
}eqs_1d_interp_tbl_type;

/* Prototype Declarations */
#ifdef myCC
// extern "C"
#endif
Function_type make_1d_lookup_tbl ( float *non_eqs_array, int num_non_eqs_els, float step, 
float *extensions, eqs_1d_lookup_tbl_type *eqs_1d_lookup_tbl );
#ifdef myCC
// extern "C"
#endif
Function_type make_1d_interp_tbl ( float *non_eqs_array, int num_non_eqs_els, float step,
float *extensions, eqs_1d_interp_tbl_type *eqs_1d_interp_tbl );
#ifdef myCC
// extern "C"
#endif
Function_type free_1d_interp_tbl ( eqs_1d_interp_tbl_type *eqs_1d_interp_tbl );
#ifdef myCC
// extern "C"
#endif
Function_type  trivariate_interp ( float a1, float a2, float a3, int num_non_eqs_els_per_plane, float *values_3d_array, 
 eqs_1d_interp_tbl_type *eqs_1d_interp_tbl_1,  eqs_1d_interp_tbl_type *eqs_1d_interp_tbl_2, 
 eqs_1d_interp_tbl_type *eqs_1d_interp_tbl_3, float *value );
#ifdef myCC
// extern "C"
#endif
Function_type  bivariate_interp ( float a1, float a2, float *values_2d_array, 
 eqs_1d_interp_tbl_type *eqs_1d_interp_tbl_1,  eqs_1d_interp_tbl_type *eqs_1d_interp_tbl_2, float *value );

Function_type  monovariate_interp ( float a, float *values_array,  eqs_1d_interp_tbl_type *eqs_1d_interp_tbl,
float *value );

Function_type   bivariate_eqs_interp ( float a1, float a2, float *values_2d_array,
int num_vals_1, int num_vals_2, float step_1, float step_2, float lower_lim_1, float lower_lim_2, 
float upper_lim_1, float upper_lim_2, float *value );

Function_type     trivariate_mixed_interp ( float a1, float a2, float a3, float *data_array,
int num_vals_1, int num_vals_2, int num_vals_per_plane, float step_1, float step_2, float lower_lim_1, float lower_lim_2, 
float upper_lim_1, float upper_lim_2,  eqs_1d_interp_tbl_type *eqs_1d_interp_tbl, float *value );

/* Qiuwen Wu 10/31/1997 */
#define  NEAREST_NEIGHBOUR 1
#define  LINEAR            2

Function_type finterp_1d(float *fin, int nx, float dx, float x0, float x, int mode, float default_val, float *val);

Function_type finterp_2d(float *fin, int nx, int ny, float dx, float dy, float x0, float y0, 
                         float x, float y, int mode, float default_val, float *val);

Function_type finterp_3d(float *fin, int nx, int ny, int nz, float dx, float dy, float dz, 
                         float x0, float y0, float z0, float x, float y, float z, int mode, 
                         float default_val, float *val);

#endif  /* INTERP_H_INCLUDED */
