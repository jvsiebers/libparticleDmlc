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
#ifndef RFS_H_INCLUDED
#define RFS_H_INCLUDED

/* 
Structure names: twod_table_type

File name: rfs.str

Description: 

	rfs_str:

		This structure defines RFs (radial factors) data.  Data needed for generating RFs 
		in the form suitable for fast interpolation and for nearest-neighbor lookup are 
		also included in this structure.  Radial factors are assumed to be symmetric around
		the central ray and are tabulated for values of radius starting with 0.
		
Notes:

	All depths are in cm.

	All radii are in cm but in fan-line coordinate system, i.e., they are scaled to the isocenter level.

	RFs are functions of radius and depth.  

	RFs are in floating point fractions normalized to 1 at the central axis for each depth.

	All data structures apply to photons as well as to electrons.

	WE HAVE ASSUMED RADIAL SYMMETRY.  FOR THE MM50, THE DOSE DISTRIBUTION IN THE BUILDUP REGION MAY NOT BE 
	RADIALLY SYMMETRIC.

	We assume that, unlike TMRs, RFs are independent of SSD in the buildup region.

Defined by:	RM

Date(s)/Author(s):	Jan 6, 93/RM

Modification History:
   Oct 5, 1998: JVS: Add typedefs
   Oct 7, 1998: JVS: "rfs_str" type being replaced with twod_table_type
                     This type will then be universal
                     and can be used with other 2d structures like wedges.
   Dec 2, 1998: JVS: Add prototype for free_rfs
   Dec 8, 1998: JVS: Update #includes, finish removing rfs_type
                     get_rfs now has beam_data_id_type passed also
*/
#include "defined_values.h"
#include "beam_data_id.h"
#include "table_types.h"

#define MAX_RFS_TBLS 16
/* #define MAX_NUM_RF_DEPTHS 16 */
/* #define MAX_NUM_RF_RADII 32 */

/* Prototype definitions. */
#ifdef myCC
// extern "C" 
#endif
Function_type get_rfs (char *, beam_data_id_type *, twod_table_type **);
#ifdef myCC
// extern "C" 
#endif
void free_rfs(void);

#endif /* RFS_H_INCLUDED */
