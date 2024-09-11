/** \file 
 simple tag auto added to force generation of simple doxygen comments
**/
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
/* range_table.h */
/* JVS: */
/* *********************************************************************** */
float find_energy(table_type *tab, float range)                   ;
float find_range(table_type *tab, float e_in)                     ;
float range_thick(table_type *tab, float e_in, float e_out)       ;
float residual_energy(table_type *tab, float thick, float e_in)   ;
float equivalent_thickness(float energy, float thickness, float density,
                           table_type *material,
                           table_type *referance);
