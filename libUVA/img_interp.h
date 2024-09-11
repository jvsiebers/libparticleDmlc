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
#ifndef IMG_INTERP_H_INCLUDED

#define IMG_INTERP_H_INCLUDED

/* Modification History:
   Sept. 14, 1998: JVS: Add **entry to eqs_1d_prt_neighb_str so can know where memory is 
                        and can free it.
   Dec 11, 1998: JVS: Add typedefs
   Feb 9,2000: JVS:  Change order of elements in structure eqs_1d_ptr_neighb_str, entry before z0
*/

typedef struct table_element_str
{
	Byte *img_p;
	float frac;
} table_element_type;


typedef struct eqs_1d_ptr_interp_str
{
	table_element_type *z0_p;		/* a pointer to the element in the table that corresponds to z=0. */
	
	float lower_lim;	/* the z value of the first element in the array */
	float upper_lim;	/* the z value of the  last element in the array */
}eqs_1d_ptr_interp_type;

typedef struct eqs_1d_ptr_neighb_str
{
        Byte **entry;  /* pointer to memory address of first element in the array */
	Byte **z0_pp;		/* a pointer to the element in the table that corresponds to z=0. */
	
	float lower_lim;	/* the z value of the first element in the array */
	float upper_lim;	/* the z value of the  last element in the array */
}eqs_1d_ptr_neighb_type;


#ifdef myCC
// extern "C"
#endif
Function_type make_ptr_tbl_interp ( 	float *unequix, int num_unequix,
					float step, float *extensions,
					eqs_1d_ptr_interp_type *tbl_p,
					Byte *first_vox_p, int num_pix_in_1_image);

#ifdef myCC
// extern "C"
#endif
Function_type make_ptr_tbl_neighb ( 	float *unequix, int num_unequix,
					float step, float *extensions,
					eqs_1d_ptr_neighb_type *tbl_p,
					Byte *first_vox_p, int num_pix_in_1_image);

#endif
