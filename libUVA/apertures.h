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
#ifndef APERTURES_H_INCLUDED

#define APERTURES_H_INCLUDED

/* 
Comment:  Copied from Jeff's directory on dragon

File name: apertures.h

	This file contains data structures for an aperture, symbolic constants pertaining to apertures and
	prototype declarations for routines to manipulate apertures.

Description: 

	Coordinates of aperture contour are specified in cms in a plane through the isocenter in the collimator 
	system.  For a given beam there can be multiple apertures specified by one instance each of aperture
	data structure (as an array of data structures).

	Each aperture is assigned and internal code (or sequence number) and an "object code".  The object code
	identifies the field shaping device to which the aperture belongs.  Thus, for a block with two apertures,
	each aperture is assigned a new sequence number but the same object code.  The object codes and internal
	sequence numbers begin with 1.

	Each element in the array of apertures may be either the standard cerrobend aperture or one of the other
	types derived from the standard aperture.  The standard cerrobend aperture (called the STD_APERTURE)
	conforms to the planning target volume (or PTV as defined by ICRU) plus a margin for beam penumbra.  

	Note that the PTV includes the gross disease, the microsopic extensions and the uncertainty related to motion
	and setup.  It DOES NOT include the margin for penumbra.

	The STD_APERTURE when converted to MLC shape is called the STD_MLC_APERTURE.

	The aperture without the margin for penumbra (needed for some optimization applications, e.g. ray tracing 
	for intensity modulation) is called the PTV_APERTURE.

	In order to provide a different margin for penumbra inferiorly and superiorly, intermediate volume
	extending the target is constructed by adding a copy of the first and the last contour of the target.
	The aperture enclosing the extended volume is designated the EXTENDED_PTV_APERTURE.  A margin of equal 
	distance is then added to EXTENDED_PTV_APERTURE to obtain the STD_APERTURE and STD_MLC_APERTURE.

	(Margin extension may also be applied to normal anatomic structures to provide greater protection.  The
	margin may be negative to allow limited encroachment of high dose region into the normal structure.)

	The group of related apertures is assigned the same internal code (or sequence number) and object code.

	Note that, normally, the STD_APERTURE is used for dose calculation if there is no STD_MLC_APERTURE, and if
	STD_MLC_APERTURE is present, the STD_APERTURE is ignored by the dose calculation program.  For the 
	aperture(s) to be used for dose calculations, the "use_for_dose_computations" flag is set to TRUE.  
	This flag must be set to FALSE for all other apertures in the same group.

	(Note that it is possible to convert certain groups of disconnected apertures in to shapes defined by MLC, 
	while for others it is not depending upon their positions relative to each other and relative to leaves 
	and their shapes.  For the latter case the beam must be segmented into two of more beams.)

	The internal code is obtained by incrementing the highest current internal code.

Notes:

Defined by:	RM, LB, AJ

Date(s)/Author(s):	Oct 19, 92/RM  March 24, 93/AJ, 
			April 11, 1994:  Add aperture definitions and PTV aperture

Modification History:
   Sept 2, 1998: JVS: pb compatible. NOTE: currently, vertices and bounds are  INTS.  
                      Could/should change to floats.  Float values are loaded as 
                      fvertices and fbounds.  Using VERTICES_FLOAT if want to use
                      float vertices
   Oct 14, 1998: JVS: Add typedef, Change for mcv pencil beam and monte carlo codes
                      Change is that now supports having multiple contours for a single
                      aperture (required for BEAM monte carlo code).  
   Oct 23, 1998: JVS: Remove OBJECT_CODE and use_for_dose_calcs
   Dec 2, 1998: JVS: All comments now c-style (no // comments )
   Oct 21, 1999: JVS: Add "valid" to aperture structure
   May 12, 2000: JVS: Had to change contour_type to 2dContourType to avoid name conflicts
*/
/* #define VERTICES_FLOAT 0 */ /* use this if want to try to use float vertices for apertures */
#include "typedefs.h"
#define AP_VERT_TYPE Point_2d_int

/* Aperture types (see explanation above)*/

#define STD_APERTURE 1
#define STD_MLC_APERTURE 2
#define PTV_APERTURE 3
#define EXTENDED_PTV_APERTURE 4

/* Special types for beam-boluses */
#define BEAM_BOLUS              5
#define FULL_FIELD_BOLUS        6

typedef struct 
{
    int     npts;  /* number of points in the contour */
    Point_2d_float *fvert;   /* float values                    */
    /* Point_2d_int   *ivert;  */  /* int values, fvert * units       */
    /* float units;      */       /* conversion between ints and floats */
}TwoDContourType;


typedef struct aperture_str
{
    int valid;  /* TRUE/FALSE value to determine if aperture is valid */
	Byte type;			     /* STD_APERTURE, STD_MLC_APERTURE, PTV_APERTURE, EXTENDED_PTV_APERTURE */
                                             /* Also: BEAM_BOLUS & FULL_FIELD_BOLUS  (VL, 05/25/94) */
    /* Logical	use_for_dose_computations; */  /* TRUE or FALSE. N.B.: not applicable for BEAM_BOLUS & FULL_FIELD_BOLUS */

	int n_contours;
        TwoDContourType *contour;

    /*        int	num_vertices; */
    /*    	Point_2d_int   *vertices; */ 	      /* int values = fvertices*units*/
    /*	Point_2d_float *fvertices; 	 */     /* float values from pinnacle */
    /*    	float	units;		*/	      /* Pixel size in cm. Multiply vertex coordinates with this */
					      /* to get cm. */

	Logical hole;			      /* TRUE if aperture transmission in the interior of contour is 1. */

	float   hole_transmission;  	      /* so can have tray factor, or multi-level blocks */

	float	transmission;		      /* Which may be for interior or exterior of the aperture contour. */
                                              /*  N.B.: not applicable for BEAM_BOLUS & FULL_FIELD_BOLUS */

        float bolus_edr;		      /*  N.B.: only applicable for BEAM_BOLUS & FULL_FIELD_BOLUS */

        float bolus_thickness;		      /*  N.B.: only applicable for BEAM_BOLUS & FULL_FIELD_BOLUS */

    /*int	object_code;*/		      /* Internal code of the object (block) which the aperture is part of. */

        Rect_int bounds;		      /* Smallest bounding rectangle in pixel units. */
					      /* Created by compute_aperture_bounds. */
        Rect_float fbounds;                   /* float value for bounds */

	Bitvec	bit_map;		      /* Pointer to the bit map. Bitvec is of type pointer. */
					      /* Bits in the hole and bits outside the of the aperture contours */
					      /* are turned on. */

}aperture_type;

#endif /* APERTURES_H_INCLUDED to eliminate possibility of multiple inclusion of this file */
