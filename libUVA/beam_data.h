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
#ifndef BEAM_DATA_H_INCLUDED   /* To eliminate possibility of multiple inclusion of this file */
#define BEAM_DATA_H_INCLUDED

/* 
Structure names: beam_data_str

File name: beam_data.h

Description: 

   This file contains data structure (beam_data_str), symbolic constants and prototype definitions 
   for beam data read or computed during or for dose calculations.  

   There is one instance of beam_data_str data structure for each beam.
   Beam data may exist within the program as an array.

Notes:

Defined by:   RM, LB

Date(s)/Author(s):   June 98 /RM

Modification History:
   Sept 2, 1998: JVS: typedef for structures too
   Sept 29, 1998: JVS: Add wdg_tbl
   Dec 14, 1998: JVS: Add Output Factor Table
   May 20, 1999: pjk: added include mlc.h
   Jan 31, 2001: JVS: Add compensatorLookupIndex
   Feb 1, 2001: JVS: Add doseLookupDataType
*/
/* ****************************************************************************** */

# ifndef TYPEDEFS_H_INCLUDED
#   include "typedefs.h"
# endif

# ifndef RTP_H_INCLUDED
#   include "rtp.h"
# endif

# ifndef TMRS_H_INCLUDED
#   include "tmrs.h"
# endif

# ifndef TERMA_H_INCLUDED
#   include "terma.h"
# endif

# ifndef RFS_H_INCLUDED
#   include "rfs.h"
# endif

# ifndef WEDGE_H_INCLUDED
#   include "wedge.h"
# endif

# ifndef COMPUTATION_OPTIONS_H_INCLUDED
#   include "computation_options.h"
# endif

# ifndef APERTURES_H_INCLUDED
#   include "apertures.h"
# endif

# ifndef MLC_H_INCLUDED
#   include "mlc.h"
# endif

# ifndef PB_FFTS_H_INCLUDED
#   include "pb_ffts.h"
# endif

# ifndef SRC_FN_H_INCLUDED
#   include "src_fn.h"
# endif


typedef struct {
   float doseWithoutIDD;
   float x_coll_fan_line;
   float y_coll_fan_line;
   float rpl;
}doseLookupDataType;


typedef struct beam_data_str
{
    /* Logical   setup_done; */
   Point_3d_float isoctr_in_is;            /* In centimeters. Computed by transformation of */
                                           /* initial_isoctr_in_is */
   bds_type *bds_tbl;
   tmr_type *tmr_tbl;
   wedge_type *wdg_tbl;
   twod_table_type *rfs_tbl;

   struct   terma_str *terma_tbl;
   struct   pb_ffts_str *pb_ffts;
   struct   src_fn_str *src_fn;
    /*   struct   flu_idd_str *terma; */
   flu_idd_type *terma;
   flu_idd_type *idd;
    /*   struct   flu_idd_str *idd;   */   /* Idealized 3D dose distribution matrices obtained */
                                   /* by convolution of terma and pencil beams at each */
                                   /* pencil beam depths. */
    int *compensatorLookupIndex; /* Lookup index correlating compensator index to fluence/terma tables */

   Point_3d_float src_in_is;
   Point_3d_float unit_src_to_isoctr_in_is;
   float   inverse_bev_transform [4] [4];/* To transform a point from the collimator system to image system */

   float   aop_at_isoctr;     /* Area/perimeter at isocenter (or at field size definition distance) */
   float   ssd;               /* Along the central ray of the union of all apertues (computed). */
                              /* If no intersection found, it is set to source-isocenter distance. */

   float  output_factor; /* Output Factor For The Beam, Stored Here for to Speed Computation for the Beam */
   doseLookupDataType *doseLookup; /* Lookup table that has fan line coordinates and rpl in it */
}beam_data_type;

#endif /* BEAM_DATA_H_INCLUDED to eliminate possibility of multiple inclusion of this file */
