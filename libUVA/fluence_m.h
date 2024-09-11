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
/*B**************************************************************************

Name : 3d:[jackson.fluence]fluence_m.h 

Function : This file contains structures and function prototypes used in constructing fluence matrices for dose calculation
	   from aperture information and jaw settings

Designer(s) / Date : A.J. 03/24/93

Author(s) / Date(s)) : A.J. 03/24/93

Modification History: 
   Sept 3, 1998: JVS: this_vert changed to AP_VERT_TYPE so can switch between
                      Point_2d_int (default) and Point_2d_float
   Sept 11, 1998: JVS: Add proto for init_fluence
   Sept 18, 1998: JVS: Add proto for sort_vertex_ptr
   Dec 11, 1998: JVS; Add typedefs
*E****************************************************************************/
#ifndef AP_VERT_TYPE
#define AP_VERT_TYPE Point_2d_int
#endif

#define MAX_NUM_APS 20
#define MAX_NUM_OBJECTS 20
#define XJAW ((int ) 'x')
#define YJAW ((int ) 'y')
#define NUM_VERT_IN_JAW 5					/* includes extra point at end (equal to 1st point) */ 
#define NUM_AP_IN_JAW 1
#define OPEN_JAW 20						/* define this in pixel units ! */
#define MAX_NUM_CHAIN_PAIRS 100

#include "typedefs.h"

typedef struct aperture_vertex_str
{
	int object_index ;
	int aperture_index ;
	int vertex_index ;
        AP_VERT_TYPE *this_vert ;			/* pntr to the coordinates of this vertex */
	struct aperture_vertex_str * last_vert ;	/* pntr to the previous vertex in the aperture */
	struct aperture_vertex_str * next_vert ;	/* pntr to the next vertex in the aperture */
}aperture_vertex_type; 


typedef struct ap_str
{
	int ap_num ;
	Logical hole ;
	int num_ap_verts ;					/* number of vertices for each aperture in the object */ 
	aperture_vertex_type * aperture_vertices ;	/* pntr to array of lists of aperture vertices for each aperture */
}ap_type ;

typedef struct object_str
{
	int object_code ;
	float transmission ;
	int num_apertures ;
	ap_type * object_apertures ;
} object_type;		

typedef struct all_objects_str
{
	int num_objects ;
	object_type * objects_p ;
}all_objects_type;

typedef struct sorted_verts_str 
{
	int num_vert_tot ;
	aperture_vertex_type ** sorted_vertices_pp ;
} sorted_verts_type;

typedef struct aperture_chains_str
{
	 aperture_vertex_type ** plus_chains ;
	 aperture_vertex_type ** minus_chains ;
	int num_plus_chains ;
	int num_minus_chains ;
	Logical new_plus_chain ;
	Logical new_minus_chain ;
	int * plus_intsx ;
	int * minus_intsx ;
} aperture_chains_type;

typedef struct object_intsx_str 
{
	float transm ;
	float temp_transm ;
	Logical hole ;
	Logical in_hole ;
	int num_intsx ;
	int curr_intsx ;
	int intsx[2*MAX_NUM_CHAIN_PAIRS] ;
} object_intsx_type;

typedef  struct compt_row_str
{
	int first_row ;
	int num_rows ;
	float * row_fractions ;
} compt_row_type;

typedef struct row_str
{
	 compt_row_type rows ;
} row_type;

typedef struct compt_col_str
{
	int first_col ;
	int last_col ;
	int num_cols ;
	float col_fractions[2] ;
} compt_col_type;

typedef struct col_str
{
	 compt_col_type cols ;
} col_type;

/* ******************************************************************** */
/*                     Function Prototypes                              */
/* ******************************************************************** */
Function_type make_apertures
(
	 beam_type * beam_p,
	 beam_data_type * beam_data_p,
	 all_objects_type * all_objects_p 
) ;

Function_type make_vertex_pntr_list
(
	 all_objects_type * aperture_objects_p,
         sorted_verts_type * sorted_verts_p
) ;
Function_type init_fluence
(
        Rect_float *hole_jaw_bounds_p,
	/*         beam_data_type * beam_data_p */
         pb_ffts_type *pb_ffts_p,
         flu_idd_type *fluence_p 
);
/* ------------------------------- in scan_fluence.c ----------------------- */
Function_type add_transmission_contribution 
(
	Logical first_intsx, 
	Logical last_intsx, 
	float transmission,
	 beam_type * beam_p, 
	 beam_data_type * beam_data_p, 
	 row_type * p_rows, 
	 col_type * p_cols
);
Function_type end_chains
(
	 aperture_vertex_type * p_curr_vert,
         aperture_chains_type * p_chains
);
Function_type fill_rows 
(	
	 all_objects_type * all_objects_p,
	 row_type * p_rows, 
	 beam_type * beam_p, 
	 beam_data_type * beam_data_p, 
	 object_intsx_type * sorted_intsx_p
);
Function_type get_fluence_cols
(
	int this_intsx, 
	 beam_type * beam_p, 
	 beam_data_type * beam_data_p, 
	 col_type * p_cols,
	Logical first_intsx,
	Logical last_intsx
);
Function_type get_fluence_rows
(
	int this_line, 
	 beam_type * beam_p, 
	 beam_data_type * beam_data_p, 
	 row_type * p_rows
);
Function_type get_intersections 
(
	 aperture_chains_type * p_ap_chains,
	int this_line
);
Function_type get_transmission 
(
	int this_object,
	Logical first_intsx,
	Logical last_intsx,
	 object_intsx_type * sorted_intsx_p,
	int num_objects,
	float * transmission_p 	
);
Function_type init_chains
(
         aperture_chains_type * p_chains
);
Function_type init_object_intsx 
(
	 all_objects_type * aperture_objects_p, 
	 object_intsx_type ** sorted_intsx_pp
);
Function_type intersection
(
        Point_2d_int *curr_chain,
        Point_2d_int *next,
        int curr_line,
        int *p_x
);
Function_type put_line_in_fluence 
(
	 all_objects_type * all_object_apertures_p,
	 aperture_chains_type * p_ap_chains, 
	int this_line, 
	 beam_type * beam_p,
	 beam_data_type * beam_data_p,
	 object_intsx_type * sorted_intsx_p
);
Function_type scan_fluence 
(
	 sorted_verts_type * sorted_verts_p, 
	 all_objects_type * aperture_objects_p,
	 beam_type * beam_p,
	 beam_data_type * beam_data_p
);
Function_type sort_intsx_by_x 
(
	 object_intsx_type * p_sorted_object
);
Function_type start_new_chains
(
	 aperture_vertex_type * p_curr_vert,
         aperture_chains_type * p_chains
);
Function_type update_chains
(
	 aperture_vertex_type * p_curr_vert,
         aperture_chains_type * p_chains
);
/* ******************************************************************** */
Function_type sort_vertex_pntrs
(
         sorted_verts_type * sorted_verts_p
);
