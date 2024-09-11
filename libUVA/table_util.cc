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
	broken off of locate1.cpp, use this and table.cpp
	19-Dec-1995: JVS
	table functions specific to range determinations
	12-Feb-1996: JVS: Add checks after locate to make sure not beyond end of the
							table.  If is beyond end of table, assign to last table value
*/
#include <stdio.h>
#include "utilities.h"
#include "table.h"
/* ************************************************************************* */
float residual_x_value(table_type *tab, float thick, float x_value_in)
{                   /* computes residual x_value after going through */
						  /* given thickness of a material                */
//	 int j;
	 float y_value,x_value;

	/* first must find the initial y_value of the beam  */
	/* first find which element in the table to interpolate after */
	y_value = find_y_value(tab,x_value_in);
	y_value = y_value - thick;
	/* then find the corresponding x_value */
	x_value = find_x_value(tab,y_value);
	return(x_value);
}
/* *********************************************************************** */
float find_x_value(table_type *tab, float y_value)
{
	int j;

	/* locate the element to interpolate after */
	locate(tab->y, tab->n_elements, y_value, &j);
	/* then perform the interpolation  after checking that not beyond */
	if(j>=tab->n_elements-1)
	{
		// printf("\n X Interpolating beyond the end of the table");
		return(tab->x[tab->n_elements-1]);
	}
	return(find_value(tab->y,tab->x,y_value,j));
}
/* *********************************************************************** */
float find_y_value(table_type *tab, float x_value_in)
{
	int j;

	/* locate the element to interpolate after */
	locate(tab->x, tab->n_elements, x_value_in, &j);
	/* then perform the interpolation */
	if(j>=tab->n_elements-1)
	{
		// printf("\n  Y Interpolating beyond the end of the table");
		return(tab->y[tab->n_elements-1]);
	}
	return(find_value(tab->x,tab->y,x_value_in,j));
}
/* *********************************************************************** */
float y_value_thick(table_type *tab, float x_value_in, float e_out)
{                   /* computes the thickness of material to give a  */
						  /* given x_value loss in a material               */
//	 int j;
	 float r_x_value_in, r_e_out;

	/* first must find the initial y_value of the beam  */
	 /* first find which element in the table to interpolate after */
//	locate(tab->x, tab->n_elements, x_value_in, &j);
	 /* then perform the interpolation */
//	r_x_value_in = find_value(tab->x,tab->y,x_value_in,j);
	 r_x_value_in = find_y_value(tab,x_value_in);
	 /* then find the corresponding x_value */
//	locate(tab->x, tab->n_elements, e_out, &j);
//	r_e_out = find_value(tab->x,tab->y,e_out,j);
	r_e_out = find_y_value(tab,e_out);
	return(r_x_value_in - r_e_out);
}
/* *********************************************************************** */
float find_value(float *x, float *y, float xm, int l)
{
	int h;
	h = l+1;
	return(interpolate(x[h], x[l], xm, y[h], y[l]));
}
/* ************************************************************************* */
