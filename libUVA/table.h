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
#ifndef TABLE_H_INCLUDED
#define TABLE_H_INCLUDED
/* HEADER FILE FOR TABLE ROUTINES 
   20-Aug-1993: J. Siebers 
   Modification History:
   Dec 3, 1998: JVS: add #ifdef around and remove proto for interpolate
   May 10, 2000: PJK: increased MAX_TABLE_ELEMENTS to 2000 for cumm_dvh
   August 16, 2000: JVS: new type tableType, will use dynamic allocation to
                         read the elements
   August 2, 2001: PJK: Increased MAX_TABLE_ELEMENTS again rather than learning to program properly
   March 19, 2002: JVS: Add float tableValueGivenYFindX(float yVal, table_type *tab)
   Nov 21, 2008: JVS: Add int table_x_value_interp(float x_val, float *y_value, table_type *tab)
   Jan 17, 2014: JVS: Add convertTable since I did not have table_type as a class (wish I did)
                      Add locateIndex
   Jan 11, 2017: JVS: Add sortTable since needed it.
   Feb 15, 2020: JVS: Added writeTable
*/
#include "typedefs.h" // for definition of point_2d_float
/* ********************************************************************** */
#define MAX_TABLE_ELEMENTS 20000
/* ********************************************************************** */
typedef struct { float x[MAX_TABLE_ELEMENTS];
		 float y[MAX_TABLE_ELEMENTS];
		 int   n_elements;
	       }table_type;
/*
typedef struct tableType {
   float *x;
   float *y;
   int nElements;
};
*/
typedef struct tableType {  
   int npts;
   Point_2d_float *points;
}tableTypeStruct;

/* *********************************************************************** */
bool sortTable(tableType *tab);
bool convertTable(tableType *tabT, table_type *tab);
bool convertTable(table_type *tab, tableType *tabT);
int locateIndex(const float *array, const int nArray, const float val, const int jStart, const int direction);
void locate(float *x_array, int n_array, float x_val, int *j);
// float interpolate(float xh, float xl, float xm, float yh, float yl);
float interp_table(float *x, float *y, float xm, int l);
int read_data_table(table_type *tab, char *filename);
float table_value(float x_val, table_type *tab);
float table_value_exact(float x_val, table_type *tab);
float find_y_value(table_type *tab, float x_value_in);
float find_x_value(table_type *tab, float y_value);
float y_value_thick(table_type *tab, float x_value_in, float e_out);
float find_value(float *x, float *y, float xm, int l);
float residual_x_value(table_type *tab, float thick, float x_value_in);
int write_data_table(table_type *tab, char *filename);
int writeTable(tableType *tab, char *filename);
int table_y_value(float x_val, float *y_value, table_type *tab);
int table_y_value_extrap(float x_val, float *y_value, table_type *tab);
int table_x_value_extrap(float y_val, float *x_value, table_type *tab);
int table_x_value_interp(float y_val, float *x_value, table_type *tab);
float table_value_extrap(float x_val, table_type *tab);
int clocate(float *array, int n_array, float val, int *j);
float integrate(table_type *tab, float lower, float upper);
int readDataFileToTable(tableType *tab, char *filename);
float tableValueGivenYFindX(float yVal, table_type *tab);
float tableValueGivenYFindXExtrap(float yVal, table_type *tab);
int checkTableError();
#endif
//
