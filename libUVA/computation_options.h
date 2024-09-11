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
Filename:	computation_option.str

Description:	
	This file contains data structures, symbolic constants and prototype definitions 
	for options for dose computation.

	Computation options for each model of dose computation are given in the namelist file 
	/3d/beam_data/computation_options.nml.  There is one namelist data block for each model.
	Options are model dependent.  The data structure for options is a union of options of
	different models.

	Each beam may have a different model of computation.  For example, one beam may use
	the photon pencil beam model while another may use the electron pencil beam model.
	Options data file is opened and all its data are read the first time this routine 
	is called.  Then for each call to this routine, the model dependent data are returned.

Modification History:
   Sept 3, 1998: JVS: Add typdef for computation options

*/

/* Symbolic constants specific to computation_options of computations */

#ifndef COMPUTATION_OPTIONS_H_INCLUDED

#define COMPUTATION_OPTIONS_H_INCLUDED

# ifndef TYPEDEFS_H_INCLUDED
#	include <typedefs.h>
# endif

# ifndef RTP_H_INCLUDED
#	include <rtp.h>
# endif

typedef struct computation_options_str
{
	Logical	src_fn_correction;

	Logical profile_correction;	

	int	ray_trace_step;		/* In units of pixel size */

	char	pb_resolution [2];		/* 'f':  fine, 'm':  medium, or 'c':  coarse postfixed to pb data files */

	/* Eventually we should pick these automatically or reconstruct on the fly depending upon the field size */

	char	pb_size [2];		/* 's':  small, 'm':  medium, or 'l':  large matrices */

}computation_options_type;

/* Prototype definitions */
#ifdef myCC
// extern "C"
#endif
Function_type get_computation_options ( struct computation_options_str **computation_options );

#endif /* COMPUTATION_OPTIONS_H_INCLUDED */
