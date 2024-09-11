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
#ifndef BEAM_DATA_ID_H_INCLUDED
#define BEAM_DATA_ID_H_INCLUDED

/* 
Structure names: beam_data_id_str

File name: beam_data_id.h

Description: 
	This file contains data structures for 

Notes:

Defined by:	RM

Date(s)/Author(s):	Oct 9, 92/RM

Modification History:
   Sept 2, 1998: JVS: typedef for structures too, change machine_room to machine_name
   Aug 31, 2000: JVS: Add in Applicator
*/

/* All character strings terminated with zero */
#define MAX_N_MODALITY 10
#define MAX_N_ENERGY   10
#include "defined_values.h"
typedef struct beam_data_id_str
{
	char	machine_name [MAX_OBJ_NAME_LEN];	/* Room # and machine model, etc. e.g., CL2100C_R245 */

	char	modality [MAX_N_MODALITY];		/* "X", "E", "P", "N", etc. */

	char	energy [MAX_N_ENERGY];			/* 1 to 50 */  
    char    applicator[MAX_OBJ_NAME_LEN];    /* Applicator Name for Electrons, etc */
}beam_data_id_type;

/* Prototype definitions */

/* Function_type  beam_data_id_to_path (struct beam_data_id_str *beam_data_id, char *bds_path ); */
#ifdef myCC
// extern "C"
#endif
int create_bds_path(char *bds_path, beam_data_id_type *beam);

#endif
