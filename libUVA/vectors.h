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
#ifndef VECTORS_DEFS_H_INCLUDED
#define VECTORS_DEFS_H_INCLUDED
/*
vectors_defs.h
This file includes prototypes of fortran routines needed for some C programs.

Modification History:
  Sept. 11, 1998: JVS: prototypes for c function calls=
  April 24, 2001: JVS: Add #ifdefs so can use these from c++ routines as well
*/
#ifdef myCC
// extern "C"
#endif
void vector_mid_point(float *v1, float *v2, float *mid_point);

#ifdef myCC
// extern "C"
#endif
void move_vector(float *v1, float *v2);

#ifdef myCC
// extern "C"
#endif
void vector_linear_combination(
      float  C1,	/* R F FIRST CONSTANT. */
      float *V1,	/* R F FIRST VECTOR ( DIMENSION 3 )*/
      float  C2,	/* R F SECOND CONSTANT.*/
      float *V2,	/* R F SECOND VECTOR.*/
      float *ANSWER);	/* W F ANSWER VECTOR.*/

#ifdef myCC
// extern "C"
#endif
void add_vectors( 
     float *V1,	        /* R F FIRST VECTOR ( DIMENSION 3 )*/
     float *V2,	        /* R F SECOND VECTOR.*/
     float *ANSWER);	/* W F ANSWER VECTOR. */

#ifdef myCC
// extern "C"
#endif
void subtract_vectors(float *v1, 
                      float *v2, 
                    float *answer); 
#ifdef myCC
// extern "C"
#endif 
void vector_magnitude (
	float *V,        /* R F VECTOR */
	float *VECMAG );	 /* W F VECTOR MAGNITUDE */

#ifdef myCC
// extern "C"
#endif
void scalar_product(
	float *V1,	/* R F FIRST VECTOR ( DIMENSION 3 )*/
	float *V2,	/* R F SECOND VECTOR.*/
	float *ANSWER);	/* W F ANSWER (scalar)*/

#ifdef myCC
// extern "C"
#endif
void vector_product(
	float *V1,		/* R F FIRST VECTOR ( DIMENSION 3 ) */
	float *V2,		/* R F SECOND VECTOR.*/
	float *V1XV2);	/* W F ANSWER VECTOR.*/

#ifdef myCC
// extern "C"
#endif
void vector_multiply(
	float CONSTANT,	/* R F */
	float *V1,	/* R F FIRST VECTOR ( DIMENSION 3 ) */
	float *V2);	/* W F ANSWER VECTOR. */

#ifdef myCC
// extern "C"
#endif
void unit_vector(
	float *V,		/* R F INPUT VECTOR.*/
	float *UNIT_V );	/* W F ANSWER VECTOR.*/




/*
Function_type vector_mid_point ( Point_3d_float *v1, Point_3d_float *v2, Point_3d_float *mid_point );

Function_type move_vector ( Point_3d_float *v1, Point_3d_float *v2 );

Function_type vector_linear_combination ( float *c1, Point_3d_float *v1, float *c2, Point_3d_float *v2, Point_3d_float *answer );

Function_type add_vectors ( Point_3d_float *v1, Point_3d_float *v2, Point_3d_float *answer );

Function_type subtract_vectors ( Point_3d_float *v1, Point_3d_float *v2, Point_3d_float *answer );

Function_type vector_magnitude ( Point_3d_float *v, float *vecmag );

Function_type scalar_product ( Point_3d_float *v1, Point_3d_float *v2, float *answer );

Function_type vector_product ( Point_3d_float *v1, Point_3d_float *v2, Point_3d_float *v1xv2 );

Function_type vector_multiply ( float *constant, Point_3d_float *v, Point_3d_float *answer );

Function_type unit_vector ( Point_3d_float *v, Point_3d_float *unit_v );

*/
#endif
