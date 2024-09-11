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
#ifndef DOSE_H_INCLUDED	/* To include this file only once */

#define DOSE_H_INCLUDED

/* 
File name: dose.h

Description: 

	This file contains data structures, symbolic constants and prototype definitions
	related to dose calculation programs.

Defined by:		Jan 12, 93 / RM

Date(s)/Author(s):	Jan 12, 93 / RM
			April 14, 1994 / RM:  Change in setup_beam prototype related to ray trace image 

Modification History:
   Sept 11, 1998: JVS: prototype updates
   Sept 14, 1998: JVS: "
   Sept 22, 1998: JVS: "
   Oct  14, 1998: JVS: change proto for make_fluence_for_open_field
   Dec 7, 1998: JVS: Add create_idd
                     Remove ray_trace from setup_beam
   Dec 15, 1998: JVS: Add wedge_defined to compute_dose,
   Feb 9, 2000: JVS: Add prototype for make_fluence_for_imrt()
   Feb 16, 2000: JVS: Update make_fluence_for_imrt
   Jan 31, 2001: JVS: Add protos for including compensator late so can do fast, 
                      repetative IMRT calcs
*/
/* To resolve some structure definitions (DP) */
#ifndef BEAM_H_INCLUDED
#	include "beam.h"
#endif

#ifndef RAYTRACE_H_INCLUDED
#	include "raytrace.h"
#endif

#ifndef BEAM_DATA_H_INCLUDED
#	include "beam_data.h"
#endif

#ifndef PB_FFTS_H_INCLUDED
#	include "pb_ffts.h"
#endif


/* Prototype definitions */
#ifdef myCC
// extern "C"
#endif
Function_type  compute_aperture_bounds ( struct beam_str *beam, Rect_float *hole_bounds, Rect_float *hole_jaw_bounds_intsct );

#ifdef myCC
// extern "C"
#endif
Function_type setup_beam (
   char *bds_path,
   computation_options_type *computation_options,
   /*   struct raytrace_data_str *rpl_raytrace_data, */
   beam_type *beam,
   beam_data_type *beam_data );

#ifdef myCC
// extern "C"
#endif
Function_type completeIDDComputation(computation_options_type *computation_options,
                         beam_type *beam, 
                         beam_data_type *beam_data);

#ifdef myCC
// extern "C"
#endif
Function_type make_fluence_for_open_field (float jaw_transmission, 
beam_type *beam, struct flu_idd_str *fluence);

#ifdef myCC
// extern "C"
#endif
Function_type  compute_idd ( float inverse_bev_transform [4] [4], struct flu_idd_str *terma, 
struct pb_ffts_str *pb_ffts, struct flu_idd_str **idd );

#ifdef myCC
// extern "C"
#endif
Function_type create_idd(struct computation_options_str *computation_options,
                         beam_type *beam, 
                         beam_data_type *beam_data);

#ifdef myCC
// extern "C"
#endif
Function_type createRtermaWithoutCompensator(computation_options_type *computation_options,
                         beam_type *beam, 
                         beam_data_type *beam_data);

#ifdef myCC
// extern "C"
#endif
Function_type include_rfs ( Logical profile_correction, twod_table_type *rfs_tbl, 
struct flu_idd_str *fluence, struct flu_idd_str **terma );

#ifdef myCC
// extern "C"
#endif
Function_type  compute_dose_at_point ( Point_3d_float *point, clipped_img_type *clipped_img, 
Logical wedge_defined, beam_data_type *beam_data, float *dose );

#ifdef myCC
// extern "C"
#endif
Function_type  	interp_idd ( float x, float y, float depth, struct pb_ffts_str *pb_ffts,
struct flu_idd_str *idd, float *idd_value );

#ifdef myCC
// extern "C"
#endif
Function_type dump_idd ( char *beam_name, char *range, char *modulator_name, 
			 struct flu_idd_str *idd);
#ifdef myCC
// extern "C"
#endif
Function_type make_fluence_from_apertures(
	Rect_float * hole_jaw_bounds_p,
	struct beam_str * beam_p,
	struct beam_data_str * beam_data_p
);


#ifdef myCC
// extern "C"
#endif
Function_type make_fluence_for_imrt( float beam_SAD, compensator_type *c, flu_idd_type *fluence);

#ifdef myCC
// extern "C"
#endif
Function_type setupForImrt( float beam_SAD, compensator_type *comp, flu_idd_type *fluence,
                                     int *fastImrtLookupIndex );

#ifdef myCC
// extern "C"
#endif
Function_type  getIDDandComputeDoseAtPoint ( doseLookupDataType *doseLookup,
                                             beam_data_type *beam_data, 
                                             float *dose );

#ifdef myCC
// extern "C"
#endif
Function_type  computeDoseAtPointWithoutIdd ( Point_3d_float *point, 
                                              clipped_img_type *clipped_img,
                                              Logical wedge_defined,
                                              beam_data_type *beam_data,
                                              doseLookupDataType *doseLookup);

#ifdef myCC
// extern "C"
#endif
Function_type  computeIDD ( 
   struct flu_idd_str *terma, 
   struct pb_ffts_str *pb_ffts,
   struct flu_idd_str **idd );

#ifdef myCC
// extern "C"
#endif
Function_type transformIDDCoordinatesToImageSystem( float inverse_bev_transform [4] [4], 
                                                    struct flu_idd_str *idd );

#endif /* DOSE_H_INCLUDED, to include this file only once */
