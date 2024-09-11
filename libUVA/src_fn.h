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
Structure names: src_fn_str

File name: src_fn.h

Description: 

	This file also contains relevant symbolic constants and prototype definitions.

Notes:


Defined by:	RM

Date(s)/Author(s):	May 93/RM

Modification History:
   Dec 2, 1998: JVS: Add prototype for free_src_fn
   Dec 8, 1998: JVS: Add beam_data_id_type to get_src_fn
*/

#ifndef SRC_FN_H_INCLUDED

#define SRC_FN_H_INCLUDED

# ifndef TYPEDEFS_H_INCLUDED
#	include <typedefs.h>
# endif

# ifndef BEAM_DATA_ID_H_INCLUDED
#	include <beam_data_id.h>
# endif

/* Source function types */

#define GAUSSIAN_SRC 0
#define CONSTANT_SRC 1 

typedef struct src_fn_str
{
	struct	beam_data_id_str beam_data_id;		/* Machine, room, energy, modality, scan pattern */

	char	bds_path [MAX_PATH_LEN];		/* Not read but transferred from beam record. */
							/* Used for checking if data for given src_fn has */
							/* already been read. */

	int	type;					/* GAUSSIAN_SRC, CONSTANT_SRC, ... */

	float	sigma_slope,				/* The rate with which the source function parameters change */
		amplitude_slope,			/* per cm of depth and/or distance from flattening filter */
		sigma_intercept,			/* NOTE THAT AMPLITUDE SPECIFIES THE FRACTION OF FLUENCE ASSUMED */
		amplitude_intercept;			/* TO BE SCATTERED */

	float	effective_mu;				/* Attenuation coeffecient for scattered component in tissue */
}src_fn_type;

/* Prototype definitions */

/* Includes to resolve prototype definitions */
#ifndef PB_FFTS_H_INCLUDED
#	include <pb_ffts.h>
#endif
#ifdef myCC
// extern "C" 
#endif
Function_type get_src_fn (char *,beam_data_id_type *, src_fn_type **);
#ifdef myCC
// extern "C" 
#endif
Function_type include_src_fn_fft ( src_fn_type *src_fn, struct pb_ffts_str *pb_ffts,
struct pb_ffts_str *sh_rng_pb_ffts );
#ifdef myCC
// extern "C" 
#endif
void free_src_fn(void);

#endif	/* SRC_FN_H_INCLUDED */
