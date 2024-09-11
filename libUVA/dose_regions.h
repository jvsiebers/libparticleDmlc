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
#ifndef DOSE_REGIONS_H_INCLUDED	/* To include this file only once */

#define DOSE_REGIONS_H_INCLUDED

/*
Filename:	dose_regions.h

Description:	
	This file contains data structures symbolic constants and prototype definitions 
	for regions of dose computation.

	Dose regions may be one of the types defined below.  ANAT_VOLUME_REGION may consist of a single anatomic
	structure or may be a SIMPLE combination of anatomic structures combined "linearly" and without
	parenthesis with one of the operators.  For all known applications, this should be sufficient.
	Examples of these combinations are as follows (let A, B, C, .. be given anatomic structures):

	A '&' B			Intersection of A and B, i.e., points lying in both A and B.

	A '|' B			Union of A and B.  Points lying in A or B.

	A '!' B '!' C		Points lying in A but not in B and not in C.  For example outer minus target and rectum.

	These and other operators (including NOOP = '0') are defined in lib.h.

	Dose regions reside in the memory as an array of dose region data structures.  Dose region data structure 
	contains two parts, a part which is common to all types of regions and a part which is a union of data structures
	customized to each type of region.  The former contains region name, region type, whether the region is 
	"on" or not, an array of dose values at each point in the region (per beam or cumulative depending upon 
	the context) and the units in which the points and distances are specified.  

***********************************************************************************************************************

Updates:
        05/25/94...V.L.: Added the object_id and object_valid field for the dose_region.
	02/15/95...L.W.: Added the volume field for the anat_volume region.

***********************************************************************************************************************
	
*/

#include "defined_values.h"
/* Symbolic constants specific to regions of dose computations */

#define POINTS_REGION '1'			/* Region types.  Must never be changed.  New ones may be added at the end */
#define LINE_REGION '2'
#define EQS_PLANE_REGION '3'
#define NON_EQS_PLANE_REGION '4'
#define EQS_VOLUME_REGION '5'
#define NON_EQS_VOLUME_REGION '6'
#define ANAT_VOLUME_REGION '7'

#define MAX_DOSE_REGIONS 32
#define MAX_ANATS_IN_VOLUME 8

#define BYTE_MAP_OFF 0	    /* JSO, if ( region.byte_map [index] == BYTE_MAP_OFF ) */ 

#ifndef	FORTRAN

/* C-specific definitions */

// #include <typedefs.h>
// #include <rtp.h>
// #include <date_time.h>
// #include <img.h>

struct points_region_str
{
	int num_points;

	char **point_names;			/* Array of point names.  Each name is MAX_OBJ_NAME_LEN long */

	Point_3d_float *points;			/* Pointer to any array of point structures */

	float	units;				/* Multiplicative factor to convert coordinates to cm */
};

struct line_region_str
{
	Line_3d_float end_points;

	float	step_size;

	int	num_points;			/* Computed at run time */

	float	units;				/* Multiplicative factor to convert coordinates to cm */
};

struct eqs_plane_region_str
{
	Point_3d_float ul, ur, ll;

	float grid_size;

	int	num_x_points, num_y_points;		/* Calculated at run time */

    /*	struct img_sec_path_str img_sec_path; */		/* Identification of the corresponding image section */

	float	units;					/* Multiplicative factor to convert coordinates to cm */
};

struct non_eqs_plane_region_str
{
	Point_3d_float ul, ur, ll;

	float grid_size;				/* Minimum grid size, i.e., grid size of the final matrix. */

	float 	low_dose_tolerance,
		first_gradient_tolerance,
		second_gradient_tolerance;

	int max_reduction_exponent;			/* Exponent of 2 by which the step size may be reduced */

	int	num_x_points, num_y_points;		/* Calculated at run time */

    /*	struct img_sec_path_str img_sec_path; */		/* Identification of the corresponding image section */

	float	units;					/* Multiplicative factor to convert coordinates to cm */
};

struct eqs_volume_region_str
{
	Point_3d_float min_corner;

	float 	grid_size;

	int	num_x_points, num_y_points, num_z_points;	/* Read from file */

	float	units;					/* Multiplicative factor to convert coordinates to cm */

	float 	margin;

	char 	**anat_names;				/* LWL, XHW */

	char 	**anat_branches;			/* LWL, XHW */

	int     *anat_codes;                            /* LWL, each of them is the same as the anat_code in struct anat_conts_str in anat.str */

	int 	num_anats;

	Byte 	*byte_map;	
};

struct non_eqs_volume_region_str
{
	Point_3d_float min_corner;

	float grid_size;

	int	num_x_points, num_y_points, num_z_points;	/* Read from file */

	float 	low_dose_tolerance,
		first_gradient_tolerance,
		second_gradient_tolerance;

	int max_reduction_exponent;			/* Exponent of 2 by which the step size may be reduced */

	float	units;					/* Multiplicative factor to convert coordinates to cm */

	float 	margin;

	char 	**anat_names;				/* LWL, XHW */

	char 	**anat_branches;			/* LWL, XHW */

	int     *anat_codes;                            /* LWL, each of them is the same as the anat_code in struct anat_conts_str in anat.str */

	int 	num_anats;

	Byte	*byte_map;
};

struct anat_volume_region_str
{
	char anat_class [MAX_OBJ_NAME_LEN];			/* class for the anats */

	int num_anats;

	char **anat_names;				/* Names of component anatomic structures.  Each is */
							/* MAX_OBJ_NAME_LEN long. */

	char **anat_branches;				/* LWL, XHW */

	int  *anat_codes;                               /* LWL, each of them is the same as the anat_code in struct anat_conts_str in anat.str */

	char *operators [MAX_ANATS_IN_VOLUME];		/* AND, OR, XOR, NOT, ... or NOOP.  Array of pointers to 2 */
							/* byte strings.  The second byte is for null terminator. */

	char pt_placement_model [MAX_OBJ_NAME_LEN];	/* Random, quasi_random, uniform, etc. */

	long seed, orientation_seed;			/* seed for random number generator, must be non-zero */
							/* orientation_seed is for generating orientations of regular grid */

	int num_points_requested;			/* when placing points uniformly, the "num_points_requested" */
							
	int num_points;					/* may be different from the actual "num_points" placed.      */

	int num_points_in_clipped_box;			/* num points in the clipped image frame, some are rejected */

	Point_3d_float *points;

	float volume; 					/* volume for this anat_volume region, added by LW */

	float	units;					/* Multiplicative factor to convert coordinates to cm */

	Logical use_in_evaluation;
};

struct dose_region_str
{
	char name [MAX_OBJ_NAME_LEN];

        int object_id;                                 /* Unique object id number */

        Logical object_valid;                                   /* Validity field */

    /*	struct tm time;	*/					/* Date and time of creation or modification */

	Logical	on;						/* On for calculation or not. */

	char	type [2];					/* Second byte is the NULL terminator. */

	float	*dose_array;					/* Pointer to dose array */

	union
	{
		struct points_region_str points_group;

		struct line_region_str line;

		struct eqs_plane_region_str eqs_plane;

		struct non_eqs_plane_region_str non_eqs_plane;

		struct eqs_volume_region_str eqs_volume;

		struct non_eqs_volume_region_str non_eqs_volume;

		struct anat_volume_region_str anat_volume;

	} data;
};

/* Prototype definitions */

/* added by LWL, 12/4/95, to make each POINTS_REGION contain only one point */
/* source is in get_dose_regions.cxx */
Function_type rearrange_points_region( char *workarea, char *pt_folder, 
					int *num_dose_regions, struct dose_region_str **dose_regions);

Function_type get_dose_regions( char *workarea, char *pt_folder, int *num_dose_regions, struct dose_region_str **dose_regions);

Function_type write_dose_regions( char *workarea, char *pt_folder, int *num_dose_regions, struct dose_region_str **dose_regions);

Function_type get_anat_volume_points ( char *workarea, char *pt_folder, char *region_name, int num_points, 
				       Point_3d_float **anat_volume_points);

Function_type write_anat_volume_points ( char *workarea, char *pt_folder, char *region_name, int num_points, 
				       Point_3d_float **anat_volume_points);

Function_type  place_points (char *workarea, char *pt_folder, char *img_set_name, char *img_name,
        struct dose_region_str *dose_region);

Function_type read_anat_points(char *workarea, char *pt_folder, char *anat_name, int num_points, Point_3d_float **points);

Function_type write_anat_points(char *workarea, char *pt_folder, char *anat_name, int num_points, Point_3d_float *points);

/* added by LWL, 12/4/95, funtion to return total number of points in a given region */
Function_type get_num_points_in_region(struct dose_region_str dose_region, int *num_points_in_region);

/* added by LWL, 12/4/95, function to return volume of a given region */
Function_type get_volume_of_region(struct dose_region_str dose_region, float *volume);

/* added by LWL, 12/11/95, function to sort all the regions according to their region types */ 
Function_type sort_dose_regions_by_type(int num_regions, struct dose_region_str *dose_regions);

/* added by LWL, 5/29/96, functions to read or write one region */ 
/* single_region_io.cxx */
Function_type get_single_region_by_object_id(char *workarea, char *pt_folder, int object_id, struct dose_region_str **region) ;
Function_type write_single_region(char *workarea, char *pt_folder, struct dose_region_str *dose_region) ; 

#endif	/* FORTRAN */

#endif /* DOSE_REGIONS_H_INCLUDED, to include this file only once */
