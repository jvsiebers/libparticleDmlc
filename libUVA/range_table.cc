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
/* range_table.cpp
   routines to compute energy / range / etc. from range-energy tables.
   assumes data to be in table_type structure with x = energy, y=range.
   routies will extrapolate if the data is outside the bounds of the table

   Created: 5/30/97: JVS: routines extracted from modulator_residual_energy
                          and updated to table_type
*/
#include <stdio.h>
#include "table.h"
#include "range_table.h"

float find_energy(table_type *tab, float range)
/* input table has x=energy, y=range             */
{
   float energy;
   table_x_value_extrap(range,&energy,tab);
   return(energy);
}
/* *********************************************************************** */
float find_range(table_type *tab, float energy)
/* input table has x=energy, y=range             */
{
  float range;
  table_y_value_extrap(energy,&range,tab);
  return(range);
}
/* *********************************************************************** */
float range_thick(table_type *tab, float e_in, float e_out)
{                   /* computes the thickness of material to give a  */
						  /* given energy loss in a material               */
                    /* input table has x=energy, y=range             */
   float r_e_in, r_e_out;
   r_e_in  = find_range(tab,e_in );
   r_e_out = find_range(tab,e_out);
	return(r_e_in - r_e_out);
}
/* *********************************************************************** */
float residual_energy(table_type *tab, float thick, float e_in)
{                   /* computes residual energy after going through */
						  /* given thickness of a material                */
   float range,energy;

   range = find_range(tab,e_in); /* find input range */
   range-=thick;               /* subrtact off the thickness of the material */
   energy = find_energy(tab,range); /* find the residual energy */
	return(energy);
}
/* *********************************************************************** */
float equivalent_thickness(float energy, float thickness, float density, table_type *material,
                           table_type *referance)
{
   /* determine the residual range in each material for incident beam */
   float rr_material = find_range(material,energy);
   float rr_water    = find_range(referance,energy);
   /* determine the mass thickness of the material */
   float mthick = thickness*density;
   /* compute the residual energy after going through the material */
   float rr = rr_material - mthick;
   /* compute the residual energy in this material */
   float energy_out = find_energy(material,rr);
   /* find the rr for referance for this energy */
   float rr_water_out = find_range(referance,energy_out);
   /* WET thickness is differance */
   return(rr_water - rr_water_out);
}
/* ************************************************************************** */
