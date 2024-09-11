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
#ifndef BEV_TRANSFORM_H_INCLUDED
#define BEV_TRANSFORM_H_INCLUDED

#ifdef myCC
// extern "C"
#endif
Function_type multiply_transforms ( float transform_1 [4] [4], float transform_2 [4] [4] );

#ifdef myCC
// extern "C"
#endif
Function_type add_x_rotation ( float angle_radians, float transform [4] [4] );

#ifdef myCC
// extern "C"
#endif
Function_type add_y_rotation ( float angle_radians, float transform [4] [4] );

#ifdef myCC
// extern "C"
#endif
Function_type add_z_rotation ( float angle_radians, float transform [4] [4] );

#ifdef myCC
// extern "C"
#endif
Function_type add_translation ( Point_3d_float *translation_vector, float transform [4] [4] );

#ifdef myCC
// extern "C"
#endif
Function_type subtract_translation ( Point_3d_float *translation_vector, float transform [4] [4] );

#ifdef myCC
// extern "C"
#endif
Function_type setup_inverse_bev_transform (
   Point_3d_float *initial_isoctr_in_img_system,
   struct beam_str *beam,
   float inverse_transform [4] [4] );

#ifdef myCC
// extern "C"
#endif
Function_type transform_points (
   float transform [4] [4], 
   int num_points, 
   Point_3d_float points [], 
   Point_3d_float transformed_points [] );

#ifdef myCC
// extern "C"
#endif
Function_type setup_bev_transform (
   Point_3d_float *initial_isoctr_in_img_system,
   struct beam_str *beam,
   float transform [4] [4] );


#endif
