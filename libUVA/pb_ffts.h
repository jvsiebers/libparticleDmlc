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
#ifndef PB_FFTS_H_INCLUDED
#define PB_FFTS_H_INCLUDED

/* 
Structure names: reduced_pb_ffts_str, pb_ffts_str

File name: pb_ffts.h

Description: 

	pb_ffts_str:

		This data structure define ffts of pencil beam data in the form produced by the FFT routines.

		Parameters to be used for dose calculation by fast interpolation are also included.

		Note that the each dimension of the reconstructed short range FFT matrix may be larger by a factor of 2, 4, .. 
		than the original FFT matrix depending on the field size (or the dimension of rectangle bounding all 
		the apertures, 	whichever is smaller).  Therefore, there may be multiple instances of each of the
		long and short range rectilinear pencil beam FFTs data for different field sizes.

	fl_idd_str:

		This data structure defines data structure for fluence and idealized dose distribution data.

Notes:

	All depths are in cm.

Defined by:	RM, AJ

Date(s)/Author(s):	Sept 28, 92/RM, Jan 13, 93/AJ

Modification History:
   Oct 4, 1998: JVS: Add typedefs
   Dec 2, 1998: JVS: Update prototypes
   Dec 8, 1998: JVS: Add beam_data_id_type to read_pb_ffts so can error check

*/

# ifndef INTERP_H_INCLUDED
#	include <interp.h>
# endif
#include "beam_data_id.h"

#define REDUCTION_BASE 2

#define MAX_NUM_PB_DEPTHS 16
#define MAX_UP_LIM_FSS 10					/* Dimensions of resampling info table */

#define MAX_PB_FFTS_DATASETS 16

typedef struct pb_ffts_str
{
	char	pb_ffts_namelist_file [MAX_PATH_LEN];		/* Complete path name of the "beam delivery system", */
							/* /3d_group/machine_room/modality/energy */
							/* This is not read but is composed using beam data ID */
							/* structure.  This directory contains various beam data */
							/* tables */

	int	num_depths;

	float	depths [MAX_NUM_PB_DEPTHS];

	eqs_1d_interp_tbl_type depth_interp_tbl;	/* Data structure containing data for fast interpolation */
							/* along depth. This data structure is constructed by */
							/* make_eqs_1d_interp_tbl function. */

	int 	num_vals_x, 
		num_vals_y;

	float 	resolution;				/* Along x and y axes. */

	float	fine_depth_step;

	float	depth_extension [2];

	float	*pb_ffts;				/* The array containing the reconstructed fft matrix */
							/* stored in the original format. */
}pb_ffts_type;

typedef struct flu_idd_str
{
	int	num_depths;

	float	depths [MAX_NUM_PB_DEPTHS];

	int 	num_vals_x, 
		num_vals_y;

	float 	resolution;				/* Along x and y axes. */

	Point_2d_float 	lower_lim;			/* Position along x and y axes of the first point */
							/* of the fluence matrix in cm in the isocenter plane. */
							/* Coordinates are in collimator system. */

	Point_2d_float 	upper_lim;			/* Position along x and y axes of the last point. */
							/* Must be = lower limit + (num_values - 1) * resolution. */
							/* Coordinates are in collimator system. */

	float	*data_array;				/* The array containing the fluence (2 or 3D) or IDD (3D) matrix */

	Point_3d_float	lower_lim_in_is,		/* These and the unit vectors below are needed only for the */
			upper_lim_in_is;		/* IDD matrix for dose calculations. */

	Point_3d_float	unit_x_in_is, 			/* Unit vectors along x and y direction of the matrix in IMG system */
			unit_y_in_is;			/* Computed using inverse BEV transforms. */

	int	num_vals_per_plane;			/* Product of num_vals_x and num_vals_y. */
}flu_idd_type;


#ifdef myCC
// extern "C"
#endif
void free_pb_ffts(void);

#ifdef myCC
// extern "C"
#endif
Function_type read_pb_ffts ( char *bds_path, 
                             beam_data_id_type *beam_data_id,
                             char *pb_resolution, 
                             char *pb_size, 
                             pb_ffts_type **pb_ffts );

#endif /* PB_FFTS_H_INCLUDED */
