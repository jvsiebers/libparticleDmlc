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
#ifndef RAYTRACE_H_INCLUDED	/* To include this file only once */

#define RAYTRACE_H_INCLUDED

/* 
File name: raytrace.h

Description: 

	This file contains data structures, symbolic constants and prototype definitions
	related to raytracing routines.

Defined by:		April 14, 1994, DML, RM

Date(s)/Author(s):	April 14, 1994, DML, RM

Modification History:
   Sept 2, 1998: JVS: add typedefs
   Dec  3, 1998: JVS: grid_snap_box prototype corrected to be type int
   Dec 11, 1998: JVS: Add raytrace_data_type
   may 1, 2000: JVS: Add put_float_on_grid

*/

#ifndef BEAM_H_INCLUDED
#	include "beam.h"
#endif

#ifndef BEAM_DATA_H_INCLUDED
#	include "beam_data.h"
#endif

#ifndef IMG_H_INCLUDED
#	include "img.h"
#endif

#ifndef IMG_INTERP_H_INCLUDED
#	include "img_interp.h"
#endif

typedef struct raytrace_data_str
{
	int cps_offset[3];		/* this offset is used to convert from the PS coords to the CPS coords */
	float a_planes[3], b_planes[3];	/* the corners of the clipped image in the image system */
	int  num_pix_in_1_image;	/* the number of pixels in 1 clipped image */
	int  num_pix_in_1_row;		/* the number of pixels in 1 row of clipped image */
	int min_ps[3], max_ps[3];	/* the corners of the clippedimage in the pixel system */
	struct eqs_1d_ptr_neighb_str neighb; /* this is part of the clipped img struct */
	Byte *bit_map;
	Byte *pix_matrix;
	float	x_pix_size,
		y_pix_size,
		z_pix_size; 
	float	*edrs;			/* For CT numbers from 0 to rang_high * conversion_factor. */
}raytrace_data_type;

/* Function prototypes */
#ifdef myCC
// extern "C"
#endif
Function_type setup_src( beam_data_type *beam_data, Point_3d_float *drr_midpt );
#ifdef myCC
// extern "C"
#endif
Function_type setup_img( int stepsize_copy, clipped_img_type *clipped_img, raytrace_data_type **local_img);
#ifdef myCC
// extern "C"
#endif
Function_type compute_rpl (         Point_3d_float *comp_pt, raytrace_data_type *local_img, float *rpl );
#ifdef myCC
// extern "C"
#endif
Function_type compute_ssd (         Point_3d_float *comp_pt, raytrace_data_type *local_img, float *ssd );
#ifdef myCC
// extern "C"
#endif
Function_type compute_object_path ( Point_3d_float *comp_pt, raytrace_data_type *local_img, 
float *bpl );
#ifdef myCC
// extern "C"
#endif
Function_type conv_ssd (Byte *vox_p, Byte **table_p, float *ssd, raytrace_data_type *local_img );
#ifdef myCC
// extern "C"
#endif
int grid_snap_box (float f, int a, int b );
#ifdef myCC
// extern "C"
#endif
Function_type box_intercept( float *end_pt2, float *intercept, raytrace_data_type *local_img );
#ifdef myCC
// extern "C"
#endif
int box_inf_ray_trav( float *exterior_pt2, float **p0_pp, float **p1_pp, raytrace_data_type *local_img );
#ifdef myCC
// extern "C"
#endif
int fast_box_traverse( float *exterior_pt2, float *intercept, raytrace_data_type *local_img );

#ifdef myCC
// extern "C"
#endif
Function_type identify_rpl_raytrace_data ( raytrace_data_type *raytrace_data );
#ifdef myCC
// extern "C"
#endif
Function_type identify_bolus_raytrace_data ( int num_boluses, float *bolus_densities,
raytrace_data_type **bolus_clipped_imgs );
#ifdef myCC
// extern "C"
#endif
int put_float_on_grid( float input_value );

#endif /* RAYTRACE_H_INCLUDED, to include this file only once */
