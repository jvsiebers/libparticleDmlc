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
/* HEADER FILE FOR 2dTABLE ROUTINES */
/* 20-Aug-1993: J. Siebers */
/* ********************************************************************** */
// #define MAX_TABLE_ELEMENTS 100
/* *********************************************************************** */
typedef struct { float x[MAX_TABLE_ELEMENTS];
					  float y[MAX_TABLE_ELEMENTS];
					  /* z contains the data at an x,y pair */
					  float z[MAX_TABLE_ELEMENTS][MAX_TABLE_ELEMENTS];
					  int   n_xelements;
					  int   n_yelements;
					}twod_table_type;
/* *********************************************************************** */
int read_2d_table(twod_table_type *tab, char *filename);
float twod_table_value(float x_val, float y_val, twod_table_type *tab);/* *********************************************************************** */
