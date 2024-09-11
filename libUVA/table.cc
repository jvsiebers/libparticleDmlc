/** \file 
 simple tag auto added to force generation of simple doxygen comments
**/
/*
   Copyright 2000-2005 Virginia Commonwealth University


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
/* 01-November-1991 */
/* 29-October-1992: Locate changed to j_upper >j_lower  */
/* 20-Aug-1993: Making for generic Table -- RENAMED TO TABLE.C */
/* 06-April-1994: adding table_value_exact */
/* 18-Dec-1995: JVS: Adding in changes from locate1.cpp and updating */
/*                    locate1 is now obsolete, replace with this file and
							  range_table.cpp
	 08-Feb-1996: JVS: ERROR return from read_table if in error
	 09-Feb-1996: JVS: Added eprintf
	 15-May-1996: JVS: Return FAIL_SAFE if Not find a match
    29-May-1997: JVS: add table_y_value_extrap -- extrapolates if not find in bounds
    30-May-1997: JVS: Add clocate and table_x_value_extrap
                      clocate should be incorperated into the other routines
                      since it is better than locate since it does error checking
     25-Sept-1997: JVS: remove interpolate
     09-Oct-1997: JVS: make write_data_table output with tab delimter rather than space
     02-March-1999: JVS: Add tableErrno to table_value
     12-May-1999: JVS: Add table_value_extrap
     Jan 22, 2001: JVS: read_data_table not always add .dat anymore...
                        and write_data_table does not add the .dat...
     March 19, 2002: JVS: add float tableValueGivenYFindX(float yVal, table_type *tab)
     August 23, 2005: JVS: fix extrapolation based routines so are correct (and error check)
                           when values are decreasing instead of increasing...
                           Reduce total amount of code by having them call each other
     Nov 5, 2007: JVS: make tableErrno a local global and add checkTableError()
     Nov 21, 2008: JVS: add int table_x_value_interps(float x_val, float *y_value, table_type *tab)
     Feb 20, 2013: JVS: Add ('s in @ line 172 to avoid compiler warning
     Jan 17, 2014: JVS@UVA: clean up formatting, add locateIndex 
     Aug 26, 2014: JVS: fixes to locateIndex, searches when [iLook]=[iLook+1]
     Dec 24, 2015: JVS: Update table_x_value_interp so checks in data[j+1]=val
Jeff Siebers     
routines for locating a value in a array, and interpolating 
to find the x and y values                                  */
/* ********************************************************************** */
/* locate.c
		Routine to locate a value in an array
*/
#include <stdio.h>
#include <stdlib.h>
/* TABLE HEADER FILE */
#define TABLE_MAIN
// #define MAX_STR_LEN 132 // in utilities.h below

#include "table.h"
#include "utilities.h"
int tableErrno = OK;
int checkTableError(){return (tableErrno);}
/* list of routines
	locate: -- For a given value x_val, finds the element j such that
      	  	  x_array[j] < x_val < x_array[j+1].
       		  Returns j = 0 if x_val < x_array[0]
       	      	  Returns j = n_array-1 if x_val > x_array[n_array-1];
	interpolate: -- Interpolates a value from x and y values
	interp_table:  -- Interpolates from a table of values
	table_value:   -- Given a table of x,y values and a x_val, returns
	       		the value (y_val) interpolated from the table
	table_value_exact: -- given a table of x,y values and a x_val, returns
       			 the value (y_val) from the table, must match x_val.
	read_data_table:  -- reads in table file values
*/
/* ************************************************************************* */
bool convertTable( tableType *tabT,  table_type *tab) {
  if(NULL==tabT || NULL==tab){
    printf("\n ERROR: convertTable : input not allocated"); return false;
  }
  if(tabT->npts > MAX_TABLE_ELEMENTS) {
    printf("\n ERROR: convertTable : %d > MAX_TABLE_ELEMENTS",tabT->npts); return false;
  }
  for(int iTab=0; iTab<tabT->npts; iTab++){
    tab->x[iTab] = tabT->points[iTab].x;
    tab->y[iTab] = tabT->points[iTab].y;
  }
  tab->n_elements = tabT->npts;
  return true;
}
/* ************************************************************************* */
bool convertTable( table_type *tab,  tableType *tabT) {
  if(NULL==tabT || NULL==tab){
    printf("\n ERROR: convertTable : input not allocated"); return false;
  }
  tabT->npts = tab->n_elements;
  for(int iTab=0; iTab<tabT->npts; iTab++){
     tabT->points[iTab].x=tab->x[iTab];
     tabT->points[iTab].y=tab->y[iTab];
  }
  return true;
}
/* ************************************************************************* */
/* ----------------Given X-value, find the Y -------------------------------*/
/* ************************************************************************* */
int table_y_value(float x_val, float *y_value, table_type *tab)
//   given a table of x,y values, return y corresponding to x...No extrapolation allowed...
//   equivalent to old find_table_value
{
  int j=0;
  if(x_val < tab->x[0] || x_val > tab->x[tab->n_elements-1]){
    printf("\n ERROR: Value %f outside of table_values: FAIL_SAFE returned", x_val);
    printf("\n Min Table: %f \t Max Table: %f",tab->x[0],tab->x[tab->n_elements-1]);
    tableErrno = FAIL_SAFE;
    return(FAIL_SAFE);
  }
  if(clocate(tab->x, tab->n_elements, x_val, &j)!=OK){
    printf("\n ERROR: clocate: values not monotonic");
  }
  *y_value = interp_table(tab->x, tab->y, x_val, j);
  return(OK);
}
/* *********************************************************************** */
float table_value(float x_val, table_type *tab)
//    This version returns the value as a floating point.....
{
  float y_value;
  if( OK != table_y_value( x_val, &y_value, tab) ){
    printf("\n ERROR: table_value");
    return(FAIL_SAFE);
  } 
  return(y_value);
}
/* ************************************************************************* */
int table_y_value_extrap(float x_val, float *y_value, table_type *tab)
//   finds y value at a given X, extrapolates if value not in table range
{
   int j=0;
   if(clocate(tab->x, tab->n_elements, x_val, &j)!=OK){
      printf("\n ERROR: %s: values not monotonic",__FUNCTION__);
   }
   // Error Check to make sure have valid element
   if( j< 0 || j > tab->n_elements-1){
     printf("\n ERROR: %s : array index %d is out of range (0-%d)", __FUNCTION__, j, tab->n_elements-1);
     tableErrno = FAIL; 
     return(FAIL);
   }
   // Make sure element will be in range so can extrapolate
   if( j == tab->n_elements-1) j--;
   //
   *y_value = interp_table(tab->x, tab->y, x_val, j);
   return(OK);
}
/* ************************************************************************* */
float table_value_extrap(float x_val, table_type *tab)
{
  float y_value;
  if(OK != table_y_value_extrap(x_val, &y_value, tab) ){
    printf("\n ERROR: table_value_extrap"); return(FAIL);
  }
  return(y_value);
}
/* ************************************************************************* */
/* ----------------Given Y-value, find the X -------------------------------*/
/* ************************************************************************* */
/* *********************************************************************** */
float tableValueGivenYFindX(float yVal, table_type *tab)
//   given a table of x,y values, return x corresponding to y...does not allow extrapolation
//    equivalent to old find_table_value
{
  int j=0;
  if(yVal < tab->y[0] || yVal > tab->y[tab->n_elements-1]){
    printf("\n ERROR: Value %g outside of table_values: FAIL_SAFE returned", yVal);
    tableErrno = FAIL_SAFE;
    return(FAIL_SAFE);
  }
  if(clocate(tab->y, tab->n_elements, yVal, &j)!=OK){
    printf("\n ERROR: clocate: values not monotonic");
  }
  return(interp_table(tab->y, tab->x, yVal, j));
}
/* ************************************************************************* */
int table_x_value_interp(float y_val, float *x_value, table_type *tab)
//   finds X value at a given Y
//   Does NOT extrapolate
//   Does NOT require y values to be monotonic
//   Gets first value from data
{
   int jIndex=0;
   while( (jIndex < tab->n_elements-1) &&
          ((tab->y[jIndex] < y_val) != (tab->y[jIndex+1] >= y_val)) &&
          (tab->y[jIndex+1] != y_val) ) {
     // printf("\n->%d %f %f yval=%f",jIndex,tab->y[jIndex],tab->y[jIndex+1],y_val);
     jIndex++;
   }
   // Error Check to make sure have valid element
   if( jIndex< 0 || jIndex >= tab->n_elements-1){
     printf("\n ERROR: %s: array index %d is out of range (0-%d)", __FUNCTION__, jIndex, tab->n_elements-1);
     tableErrno = FAIL; 
     return(FAIL);
   }
   // Make sure element will be in range so can extrapolate
   if( jIndex == tab->n_elements-1) jIndex--;
   //
   *x_value = interp_table(tab->y, tab->x, y_val, jIndex);
   return(OK);
}
/* ************************************************************************ */
int table_x_value_extrap(float y_val, float *x_value, table_type *tab)
//   finds x value at a given y, extrapolates if value not in table range
//   REQUIRES A 1-1 CORRESPONDANCE BETWEEN X AND Y VALUES
{
   int j;;

   if(clocate(tab->y,tab->n_elements,y_val,&j)!=OK) {
     printf("\n ERROR: Values in Error Not monotonic"); 
     tableErrno = FAIL;
     return(FAIL);
   }
   // Error Check to make sure have valid element
   if( j< 0 || j > tab->n_elements-1) {
     printf("\n ERROR: %s: array index %d is out of range (0-%d)", __FUNCTION__, j, tab->n_elements-1);
     tableErrno = FAIL; 
     return(FAIL);
   }
   // Make sure element will be in range so can extrapolate
   if( j == tab->n_elements-1) j--;
   //
   *x_value = interp_table(tab->y,tab->x,y_val,j);
   return(OK);
}
/* *********************************************************************** */
float tableValueGivenYFindXExtrap(float yVal, table_type *tab)
{
  float x_value;
  if(OK != table_x_value_extrap(yVal, &x_value, tab) ){
    printf("\n ERROR: tableValueGivenYFindXExtrap"); return(FAIL);
  }
  return(x_value);
}
/* *********************************************************************** */
/* table_value_exact requires exact match of value in table */
float table_value_exact(float x_val, table_type *tab)
{
  int j=0;
  if(x_val < tab->x[0] || x_val > tab->x[tab->n_elements-1]){
    putc(7, stdout);
    printf("\n ERROR: Value %f outside of table values: FAIL_SAFE value returned", x_val);
    tableErrno = FAIL_SAFE;
    return(FAIL_SAFE);
  }
  if(clocate(tab->x, tab->n_elements, x_val, &j)!=OK){
    printf("\n ERROR: clocate: values not monotonic");
  }
  if( tab->x[j] != x_val) {
    printf("\n ERROR: NOT EXACT VALUE OF %f in Table: FAIL_SAFE value returned",x_val);
    putc(7, stdout);
    tableErrno = FAIL_SAFE;
    return(FAIL_SAFE);
  }
  return(tab->y[j]);
}
/* *********************************************************************** */
/* ------------------Routines to find things in arrays --------------------*/
/* *********************************************************************** */
int locateIndex(const float *array, const int nArray, const float val, const int jStart, const int direction)
/// find element in table that val is between j and j+1 starting from jStart 
/// direction of search is +=positive, -=negative, 0=error
/// returns the index 
/// return -1 if value not found
/// Value is between array[j] and array[j+1] independent of if direction is + or -
///
/// Note, on entry, the case can occur where 
///        array[jStart]   < val < array[jStart+1]  
///  and   array[jStart-1] < val < array[jStart]
/// If the direction is +, the correct answer is jStart
/// If the direction is -, the correct answer is jStart-1
/// To ensure this, when direction is -, start the actual search from jLook=jStart-1.
{
  const char *functionName="locateIndex";
  if(NULL == array ) {
    printf ("\n ERROR: %s: passed null pointer ",functionName); return(-1);
  }
  if(jStart >= nArray || jStart < 0 || nArray < 2 ) {
    printf("\n ERROR: %s: invalid input ",functionName); return(-1);
  }
  // printf(" direction = %d, jStart = %d", direction, jStart);
  int jLook=jStart;
  if(direction < 0 && jLook ) jLook--; // See above, to ensure look correctly for 1st point
  if(jLook > nArray-2) jLook=nArray-2; // make sure first element is not off of the array
  if ( (array[jLook] != val) || (array[jLook] == array[jLook+1]) ){  // return immediately if exact match
    if(direction > 0) {
      while( (array[jLook] < val) == (array[jLook+1] < val ) && jLook < nArray-2) {
         jLook++;
      }
    } else if (direction < 0) {
      while( (array[jLook] < val) == (array[jLook+1] < val ) && jLook > 0) {
	jLook--;
      }
    } else {
      printf("\n ERROR: %s: 0 is invalid for direction",functionName); return(-1);
    }
  }
  // Final check, make sure it is between jLook and jLook+1
  if( (array[jLook] < val) == (array[jLook+1] < val ) ) jLook=-1; // Value not found
  return(jLook);
}
/* *********************************************************************** */
int clocate(float *array, int n_array, float val, int *j)
// find element in table that y_value is between j and j+1, report error if
// not single valued function 
{
   int order, j_upper, j_lower, j_midpt;

   order = ( array[j_upper = n_array-1] > array[j_lower = 0]);
   // order = 1 if array[upper] > array[lower]
   // order = 0 if array[upper] < array[lower] 
   /*
   printf("\n order = %d, n_array=%d", order,n_array);
   printf("\n val = %f, array[upper] = %f, array[lower]=%f", val, array[j_upper],array[j_lower]); */

   // check that array is single valued (constant increase or decrease) 
   // also check if value is within array bounds 
   if(order){
      for(int i=0; i<j_upper; i++){
         if(array[i] > array[i+1]) return(FAIL);
      }
      // make sure item is within array limits 
      //if( val >= array[j_upper] ){  
      if( val > array[j_upper] ){  // 2/15/2016: JVS: Modified, just < now.  If ==, then will be processed below
	*j = j_upper;return(OK);}
      // if( val <= array[j_lower] ){ // 2/15/2016: JVS: Modified
      if( val < array[j_lower] ){ 
	*j = j_lower;return(OK);}
   }
   else
   {
      for(int i=0; i<j_upper; i++){
         if(array[i] < array[i+1])return(FAIL);
      }
      // make sure item is within array limits 
      if( val < array[j_upper] ) { *j=j_upper; return(OK);}
      if( val == array[j_upper] ){
	do{
	  j_upper--;
	}while (val == array[j_upper] && (j_upper) > j_lower); // 2/15/2016: JVS: Added
	// printf("\n array[%d]=%g, array[%d]=%g", j_upper-1, array[j_upper-1], j_upper, array[j_upper]);
	*j = j_upper;return(OK);
      }
      if( val > array[j_lower] ){ *j = j_lower;return(OK);}
      if( val == array[j_lower] ){ 
	do{
	   j_lower++;
	}while (val == array[j_lower] && (j_lower) < j_upper); // 2/15/2016: JVS: Added
	*j = j_lower;return(OK);	
      }	
   }

   // find the value if it is within the array */
   while(j_upper > j_lower + 1)  {          // if not found value yet 
     j_midpt = (j_upper + j_lower) >> 1;   // compute a midpoint 
     if( (val < array[j_midpt]) == order ){
       j_upper = j_midpt;
     } else {
       j_lower = j_midpt;
     }
   }
   *j = j_lower;
   return(OK);
}
/* *********************************************************************** */
void locate(float *x_array, int n_array, float x_val, int *j)
{
  int order, j_upper, j_lower, j_midpt;
  
  order = ( x_array[j_upper = n_array-1] > x_array[j_lower = 0]);
  // order = 1 if x[upper] > x[lower], order = 0 if x[upper] < x[lower] 
  // next, make sure item is within array limits 
  if( order && x_val >= x_array[j_upper] ) {
    *j = j_upper;
    return;
  }
  
  while(j_upper > j_lower + 1)   {         // if not found value yet 
    j_midpt = (j_upper + j_lower) >> 1;   // compute a midpoint 
    if( (x_val < x_array[j_midpt]) == order ) {
      j_upper = j_midpt;
    } else {
      j_lower = j_midpt;
    }
  }
  *j = j_lower;
  return;
}
/* *********************************************************************** */
/* float interpolate(float xh, float xl, float xm, float yh, float yl)
{
  // returns ym corresponding to xm
  return(yh - (xh-xm)/(xh-xl)*(yh-yl));
}
*/
/* *********************************************************************** */
float interp_table(float *x, float *y, float xm, int l)
// equivalent to old find_value
{
  int h;
  
  h = l+1;
  return(interpolate(x[h], x[l], xm, y[h], y[l]));
}
/* *********************************************************************** */
int read_data_table(table_type *tab, char *filename)
// reads in a table, call with filename[0]=NULL to prompt for file name
//	else, call with the file name to read the table directly
{
  FILE *stream;
  char in_string[MAX_STR_LEN];
  int i;
  
  stream = fopen(filename, "r");
  if(stream == NULL){
    stream = open_file(filename, "", "r");
    if(NULL == stream) {
      stream = open_file(filename, ".dat", "r");
      if(stream == NULL){
	printf("\n ERROR Opening Table file %s", filename);
	return(ERROR);
      }
    }  
  }	
  i=0;
  
  while(fgets(in_string, MAX_STR_LEN, stream) != NULL && i < MAX_TABLE_ELEMENTS){
    if(sscanf(in_string, "%f %f", &tab->x[i], &tab->y[i]) ==2){
      i++;
    }
  }
  if(i==MAX_TABLE_ELEMENTS){
    printf("\n ERROR: Over ran array length of %d reading table %s", MAX_TABLE_ELEMENTS,filename); 
    fclose(stream);
    return(ERROR);
  }
  tab->n_elements = i;
  fclose(stream);
  return(i);
}
/* *********************************************************************** */
int write_data_table(table_type *tab, char *filename)
// writes a table, call with filename[0]=NULL to prompt for file name
//	else, call with the file name to read the table directly
{
  FILE *stream;
  stream = open_file(filename, "", "w");
  if(stream == NULL){
    printf("\n Error Opening Table file %s", filename);
    return(ERROR);
  }
  int i;
  for(i=0; i<tab->n_elements;i++){
    fprintf(stream,"%f\t%f\n",tab->x[i],tab->y[i]);
  }
  fclose(stream);
  return(i);
}
/* *********************************************************************** */

