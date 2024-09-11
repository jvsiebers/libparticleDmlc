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
/* 
Structure names: tmrs_str

File name: tmrs.h

Description: 

	tmrs_str:

		This structure defines TMRs and renormalized TMRs data in the form they reside
		in the memory.  TMR data are read from a file and data needed for fast 
		nearest-neighbor lookup and interpolation are generated and are also included 
		in this structure.
Notes:

	All distances are in cm.

	TMRs are in floating point quantities normalized to 1000 at normalization_depth.

	All data structures apply to photons as well as to electrons.

Defined by:	RM

Date(s)/Author(s):	September 1, 92/RM

Modification History:
   Dec 3, 1998: JVS: Add prototype for free and #ifdef around the inclusion
   Dec 8, 1998: JVS: Add beam_data_id_type to get_tmrs and get_tmr_norm so can error check

*/
#ifndef TMRS_H_INCLUDED
#define TMRS_H_INCLUDED

# ifndef INTERP_H_INCLUDED
#	include <interp.h>
# endif

# ifndef BEAM_DATA_ID_H_INCLUDED
#	include <beam_data_id.h>
# endif

#define MAX_TMR_TBLS 16
#define MAX_NUM_TMR_AOPS 16
#define MAX_NUM_TMR_DEPTHS 64
#define MAX_NUM_TMR_SSDS 8

/* Data structure for tmrs read from the file */

typedef struct tmrs_str
{
	struct beam_data_id_str beam_data_id;		/* Needed only for cross-checking */

	char	bds_path [MAX_PATH_LEN];		/* Complete path name of the "beam delivery system", */
							/* /rtp_group/machine_room/modality/energy/scan_pattern */
							/* This is not read but is composed using beam data ID */
							/* structure.  This directory contains various beam data */
							/* tables */

	int	num_aops;

	int	num_depths;

	int	num_ssds;

	float	aops [MAX_NUM_TMR_AOPS];

	float	depths [MAX_NUM_TMR_DEPTHS];

	float	ssds [MAX_NUM_TMR_SSDS];

	float	normalization_depth;

	float	depth_of_maximum;

	float	*tmrs;					/* Pointer to the array containing tmrs stored by */
							/* depth, aop and ssd respectively */

	/* The following data are for creating "equispaced indices and interpolation constants" for fast interpolation */

	float	fine_depth_step;

	float	fine_aop_step;

	float	fine_ssd_step;

	float	depth_extensions [2];				/* For extrapolation outside the range of depths for which */
								/* measured data are available. Extensions are +ve definite. */

	float	aop_extensions [2];				/* For extrapolation outside the range of A/Ps for which */
								/* measured data are available. */

	float	ssd_extensions [2];				/* For extrapolation outside the range of ssds for which */
								/* measured data are available. */

	/* 1d tables of data needed for "fast interpolation" along depth, A/P and SSD axes. */
	/* These tables are obtained by computation.  Usage is explained in an example in */
	/* [lib.interp.test] */

	eqs_1d_interp_tbl_type eqs_1d_interp_tbl_depth;

	eqs_1d_interp_tbl_type eqs_1d_interp_tbl_aop;

	eqs_1d_interp_tbl_type eqs_1d_interp_tbl_ssd;

	int	num_tmrs_per_ssd;				/* Product of number of depths and number of A/Ps */
								/* Needed for fast lookup in the TMR array */

	float	*tmr_norm_factors;				/* These factors ensure that dose at the central axis for an */
								/* open field is the same as the one obtained by using */
								/* measured TMRs.  This array has the same dimension as */
								/* TMR array for a single SSD.  (I.E., these factors are */
								/* independent of SSD.) */
}tmr_type;

/* Prototype definitions */
/*
Function_type get_tmrs (int normalization_mode, char *bds_path, struct tmrs_str **tmr_tbl);
*/
#ifdef myCC
// extern "C" 
#endif
Function_type get_tmrs (char *, beam_data_id_type *, tmr_type **);

#ifdef myCC
// extern "C" 
#endif
Function_type get_tmr_norm_factors (char *, beam_data_id_type *, char *, tmr_type *);

#ifdef myCC
// extern "C" 
#endif
void free_tmr_norm_factors(void);

#ifdef myCC
// extern "C" 
#endif
void free_tmrs(void);

#ifdef myCC
// extern "C" 
#endif
Function_type write_tmr_norm_factors ( char *bds_path, beam_data_id_type *beam_data_id, tmr_type *tmr, char *wedge_name );

#endif
