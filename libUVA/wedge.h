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
#ifndef WEDGE_H_INCLUDED
#define	WEDGE_H_INCLUDED

#include "rfs.h" /* for defintion of twod_table */

/* 
Structure names: wedge_str, modulator_factors_str

File name: modulators.h

Description: 

	This file contains data structures for wedges.  

	Wedge is any "two-dimensional" attenuator.  
	Its profile is specified in terms of coordinates of vertices of a closed contour in the y-z 
	plane of the "W" (wedge) coordinate system according to the IEC convention.  In this convention, 
	the wedge in its "zero orientation" with all machine angles at zero degrees, has its thin 
	end pointing toward the gantry.  All possible orientations that a wedge can have for the
	beam delivery system are specified in the wedge data file.  They can be 0, 90, 180, 270
	degrees.  Following the IEC convention, the 90 degree orientation means thin end to the right
	with the viewer facing the gantry, and so on.

	The user specifies the wedge orientation during beam definition and the software rotates data tables
	appropriately.

	Wedges may be dynamic or static.  At present this file addresses static wedges only.

	wedge_str

		Contains the raw wedge data used to compute wedge transmission factors (relative transmitted
		energy fluence) and relative average attenuation coeffecients.

		It also contains info for computing relative fluence distribution and the matrix of relative 
		attenuation coeffecients of the transmitted photons.  

		Other data to compute wedge factors resides in files containing spectra for the specified BDS
		and in the file containing linear attenuation coeffecients for various materials.

		The wedge factors file is created once and for all for the photon energy of the beam delivery 
		system and read at the time of dose calculations.

		Wedge data for all wedges resides in the bds.tbl file.  When bds.tbl is read, data for all wedges 
		is read automatically.  bds.tbl is created with an editor or by an interactive program using 
		the MOTIF interface.  

Notes:

Defined by:		December 21, 92/RM

Date(s)/Author(s):	December 21, 92/RM
			March 3, 1994/RM:  Added depth dependent wedge factors
Modification History:
   9/29/98: JVS: Add path to wedge structure
  10/16/98: JVS: Wedge structure cleaned up.
  Dec 3, 1998: JVS: Adding prototypes
  Dec 8, 1998: JVS: Removing Unused Quantities from Wedge structure
                    Adding wedge factor to Wedge structure

*/

# ifndef TYPEDEFS_H_INCLUDED
#	include <typedefs.h>
# endif

# ifndef RTP_H_INCLUDED
#	include <rtp.h>
# endif

# ifndef BEAM_DATA_ID_H_INCLUDED
#	include <beam_data_id.h>
# endif

# ifndef INTERP_H_INCLUDED
#	include <interp.h>
# endif

#define MAX_WEDGES_PER_BDS 	32
#define MAX_WEDGE_VERTICES 	16
#define MAX_NUM_WEDGE_FACTORS	16

/* Wedge type codes */

#define	STATIC 0
#define	DYNAMIC 1

typedef struct wedge_str
{
    /* char   name[MAX_OBJ_NAME_LEN]; */ /* Wedge name (including wedge angle) */
    char   name[MAX_PATH_LEN]; 
    /* int    angle; */    /* Nominal angle */
    /* char   type;  */    /* DYNAMIC or STATIC */
    /* int   num_orientations; */
    /* int   orientations [4]; */ /* 0, 90, 180, 270 See explanation above */
    /* float  max_field_y;  */  /* Maximum field dimension along the wedge profile */
                           /* for which this wedge is valid (y- axis of the wedge system) */
    /* float  max_field_x; */   /* Maximum field dimension normal to wedge profile. */
    twod_table_type wp;       /* wedge profile data */
    twod_table_type wf;       /* Wedge Factors */ 
} wedge_type;

/* Function prototypes */

/* Included to resolve reference to struct terma_str (DP) */
# ifndef TERMA_H_INCLUDED
#   include <terma.h>
# endif

#ifdef myCC
// extern "C"
#endif
Function_type get_wedge (char *, beam_data_id_type *, char *, wedge_type **);

#ifdef myCC
// extern "C"
#endif
void          free_wdg(void);

#endif   /* WEDGE_H_INCLUDED */
