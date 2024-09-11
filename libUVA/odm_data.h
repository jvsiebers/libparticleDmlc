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
#ifndef ODM_DATA_H_INCLUDED
#define ODM_DATA_H_INCLUDED

/* 
Structure names: 
	odm_data_str

File name: odm_data.h
*/

/* Description: odm_datastr:  Opening density data structure.

Notes:

	All distances are in cm.

Defined by:	RM

Date(s)/Author(s):	June, 1998 / RM

*/

/* Opening density data structure. There is one instance for each beam */

struct odm_data_str
{
	/* Matrix dimensions */
	int num_x_vals, num_y_vals;
	
	/* Resolution:  dx, dy in cm*/
	Delta_2d_float resolution;

	/* Lower limit:  Position of first element in 
	IEC collimator system (cs) in cm */
	Point_2d_float lower_lim_cs;

	/* Opening density matrix.  Matrices taken together 
	are normalized to unity.  Matrix value multiplied by 
	the weight of the beam gives the "transmitted" MU's 
	required at the point. */

	float *odm;

	float *odm_actual;		/* Needed only for IMRT/DMLC */

	/* ODM for the next iteration.  odm_current = 2 * odm - odm_actual */
	float *odm_current;		/* Needed only for IMRT/DMLC */

	int	num_active_leaves;	/* May not be the same as number of active rows */
					/* Needed only for IMRT/DMLC */

	float 	*x_array;		/* Array of x positions of points on the odm matrix created during init. */
					/* Needed only for IMRT/DMLC */
};

#endif /* ODM_DATA_H_INCLUDED */
