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
#ifndef TERMA_H_INCLUDED
#define TERMA_H_INCLUDED

/* 
Structure names: terma_str

File name: terma.str

Description: 

	terma_str:

		This structure defines terma (radial factors) data.  Data needed for generating terma 
		in the form suitable for fast interpolation and for nearest-neighbor lookup are 
		also included in this structure.
		
Notes:

	All depths are in cm.

	All x and y distances are in cm but in fan-line coordinate system, i.e., they are scaled to the isocenter level.

	TERMA values are functions of x, y and depth.  

	TERMA are in floating point fractions normalized to 1 at the central axis for each depth.

	WE HAVE ASSUMED RADIAL SYMMETRY.  FOR THE MM50, THE DOSE DISTRIBUTION IN THE BUILDUP REGION MAY NOT BE 
	RADIALLY SYMMETRIC.

	TERMA may be calculated for open fields as well as for intensity modulated fields.  This is the reason 
	for specifying 	TERMA tables as functions of x and y instead of radius.  For intensity modulated fields,
	beam hardening effects are incorporated into account in the TERMA values.

	TERMA values are equispaced along x and y but not along depth.  (Therefore, a special interpolation
	routine, similar to the one for IDD matrices, will have to be used.)

	For open fields, the maximum field size and number of x and y values are specified for which the TERMA
	values are calculated and tabulated.  Step size and the limits of the rectangle enclosing the TERMA
	matrix are computed and stored.  Only 1/4 of the matrix is stored.  The rest is generated at the 
	time of reading by symmetry.

	For wedged fields, the maximum field size for the wedge and the number of points in the area covered by
	the wedge are used to compute step size so that TERMA matrix rows and columns coincide with boundary
	of the largest wedge field.  Therefore, the MAXIMUM OPEN FIELD SIZE IS ADJUSTED TO ACCOMODATE THIS AND
	WILL NOT BE THE SAME AS THE INPUT MAXIMUM OPEN FIELD SIZE.

	We assume that, unlike TMRs, terma are independent of SSD in the buildup region.

	For electrons ...?

Defined by:	RM

Date(s)/Author(s):	Jan 6, 93/RM
*/

# ifndef BEAM_DATA_ID_H_INCLUDED
#	include <beam_data_id.h>
# endif

# ifndef INTERP_H_INCLUDED
#	include <interp.h>
# endif

#define MAX_TERMA_TBLS 16
#define MAX_NUM_TERMA_DEPTHS 16

struct terma_str
{
	struct beam_data_id_str beam_data_id;		/* Needed only for cross-checking */

	char	bds_path [MAX_PATH_LEN];		/* Complete path name of the "beam delivery system", */
							/* /3d_group/machine_room/modality/energy/scan_pattern */
							/* This is not read but is composed using beam data ID */
							/* structure.  This directory contains various beam data */
							/* tables */

	char	wedge_name [MAX_OBJ_NAME_LEN];

	int	num_depths;

	float	depths [MAX_NUM_TERMA_DEPTHS];

	float	max_field_x,				/* CHANGED FOR WEDGED FIELDS SO THAT INTEGRAL NUMBER OF POINTS */
		max_field_y;				/* LIE WITHIN THE MAXIMUM FIELD SIZE VALID FOR THE WEDGE. */

	int	num_x_points;				/* Must be odd.  Note that for open fields only 1/4th */
							/* TERMA matrix is stored.  For wedges only 1/2 of the matrix */
							/* stored.  For arbitrary intensity modulator, all of the */
							/* matrix is stored */

	int	num_y_points;				/* Must be odd */

	int	num_points_per_plane;			/* Product of num_points_x and num_points_y. */

	float	x_step,					/* Calculated using max_field_x and y_field_x */
		y_step;					/* Note that for wedged fields, the maximum open field dimensions are */
							/* adjusted to improve accuracy of TERMA at points near */
							/* the boundary as discussed above.  For arbitrary */
							/* intensity modulators, maximum field dimensions are */
							/* not adjusted and relatively fine step may have to be used */
							/* to achieve adequate accuracy. */

	
	Rect_float	limits;				/* Coordinates of boundaries of the TERMA matrix.*/
							/* Given in the collimator system. Compute and stored by */
							/* TERMA computation utility. */

	float	*terma;					/* Pointer to the array containing terma stored by */
							/* x, y, and depth respectively.  Array is equispaced */
							/* along x and y */

	/* The following data are for creating "equispaced indices and interpolation constants" for fast interpolation */
	/* along depth axis. */

	float	fine_depth_step;				/* For fast interpolation along depth axis */

	float	depth_extensions [2];				/* For extrapolation outside the range of depths for which */
								/* measured data are available. Extensions are +ve definite */

	/* 1d tables of data needed for "fast interpolation" along depth axes. */
	/* These tables are obtained by computation.  Usage is explained in an example in */
	/* [lib.interp.test] */

	eqs_1d_interp_tbl_type eqs_1d_interp_tbl_depth;
};

/* Prototype definitions. */

#endif /* TERMA_H_INCLUDED */
