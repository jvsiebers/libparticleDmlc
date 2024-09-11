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
/* 2d_table.cpp
	Copyright 1995, LLUMC
	Two dimensional table read and interpolation schemes
	Created by J.V, Siebers, 1994
	Extracted to seperate routines (from model1.cpp) on
	28-Sept-1995
	08-Feb-1996: change from exit to ERROR return
	09-Feb-1996: added eprintf
	15-May-1996: JVS: Change return value to FAIL_SAFE when no value found
     June 4, 2008: JVS: typecast to avoid 64bit compiler warnings
*/
/* ************************************************************************ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "table.h"
#include "2dtable.h"
#include "utilities.h"
/* ************************************************************************ */
// #define MAX_STR_LEN 150 // in utilities.h
/* ************************************************************************ */
int read_2d_table(twod_table_type *tab, char *filename)
{                /* reads in two-dimensional table*/
   FILE *stream;
   char istr[MAX_STR_LEN];
   char tmp_string[MAX_STR_LEN];
   int i,j;
   int sval;
   int len;

   stream = open_file(filename," ","r");
// fopen(filename,"r");
   if(stream == NULL)
   {
      eprintf("\n ERROR: READ_2d_TABLE cannot open stream %s",filename);
      fclose(stream);
      return(ERROR);
   }
   /* read the first line of the file and get the column header data */
   sval = i = 0;
   if(fgets(istr, MAX_STR_LEN, stream) != NULL)
   {
      len = (int) strlen(istr);
      while(isspace(istr[sval]) && sval++ < len ); /* advance out spaces */
      do{
         sscanf(istr+sval,"%s",tmp_string); /* read in the value */
	 if(sscanf(tmp_string,"%f",&tab->y[i])==1)     /* read in the value */
	 i++;            /* only advance if read in valid y value */
	 sval+=(int) strlen(tmp_string);               /* advance past read in value */
	 while(isspace(istr[sval]) && sval++ < len ); /* advance out spaces */
      }while( sval < len && i < MAX_TABLE_ELEMENTS); /* break out at end of line */
      tab->n_yelements = i;
   }
   i = j = sval = 0;
   while(fgets(istr, MAX_STR_LEN, stream) != NULL && i < MAX_TABLE_ELEMENTS)
   {
      sval = 0;
      /* now need to read each element from the table */
      /* first element is the "x" value */
      /* rest of elements are the "data" values */
      len = (int) strlen(istr);
      while(isspace(istr[sval]) && sval++ < len ); /* advance out spaces */
      sscanf(istr+sval,"%s",tmp_string); /* read in the value */
      sscanf(tmp_string,"%f",&tab->x[i]); /* read in the value */
      sval+=(int) strlen(tmp_string);          /* advance past read in value */
      while(isspace(istr[sval]) && sval++ < len ); /* advance out spaces */
      /* Read in the data values */
      for(j=0; (j < tab->n_yelements && sval < len) ; j++)
      {
         while(isspace(istr[sval]) && sval++ < len ); /* advance out spaces */
         if(sval < len)
         {
            if(sscanf(istr+sval,"%s",tmp_string)==1) /* read in the value */
            {
               if(sscanf(tmp_string,"%f",&tab->z[i][j])!=1) /* read in the value */
               {
/*
                   printf("\n ERROR: read_2d_table Failed to read tab->z[%d][%d] from line %s in file %s",
                       i,j,tmp_string,filename);
*/
               }
	       sval+=(int) strlen(tmp_string);          /* advance past read in value */
            }
         }
      }
/*    printf("\n\t %f",tab->x[i]);
      for(k=0; k<tab->n_yelements;k++) printf(" %f ",tab->z[i][k]);
*/
      i++;
   }
   if(i==MAX_TABLE_ELEMENTS)
   {
      eprintf("\n Over ran array length reading file %s",filename); fclose(stream);
      return(ERROR);
   }
   tab->n_xelements = i;
   fclose(stream);

/* printf("\n FINAL TABLE \n \t");
   for(i=0; i<tab->n_yelements;i++) printf(" %f ",tab->y[i]);
   for(j=0; j<tab->n_xelements;j++)
   {
      printf("\n\t %f",tab->x[j]);
      for(k=0; k<tab->n_yelements;k++) printf(" %f ",tab->z[j][k]);
   }
*/

   return(i);
}
/* *********************************************************************** */
float twod_table_value(float x_val, float y_val, twod_table_type *tab)
{                     /* interpolates values on two-dimenstional table */
	int i,j;
	float y1_tmp,y2_tmp;

	if(x_val < tab->x[0] || x_val > tab->x[tab->n_xelements-1])
	{
		putc(7, stdout);
		eprintf("\n ERROR: X Value %f outside of table values: FAIL_SAFE value returned", x_val);
		return(FAIL_SAFE);
	}
	if(y_val < tab->y[0] || y_val > tab->y[tab->n_yelements-1])
	{
		putc(7, stdout);
		eprintf("\n ERROR: Y Value %f outside of table values: FAIL_SAFE value returned", y_val);
		return(FAIL_SAFE);
	}
	locate(tab->x, tab->n_xelements, x_val, &i);
	locate(tab->y, tab->n_yelements, y_val, &j);
	/* now interpolate on the table */
	/* first interpolate on the "y" coordinate */
/*	printf("\n x_val = %f,%d,%d",x_val,i,j);*/
	y1_tmp = interp_table(tab->y,tab->z[i],y_val,j);
	y2_tmp = interp_table(tab->y,tab->z[i+1],y_val,j);
/*	printf("\n %f %f",y1_tmp,y2_tmp);*/

	/* then interpolate on the x coordinate */
/*	printf("Answer = %f",
		interpolate(tab->x[i+1], tab->x[i], x_val,  y2_tmp,  y1_tmp));*/

	return(interpolate(tab->x[i+1], tab->x[i], x_val,  y2_tmp,  y1_tmp));
}
/* *********************************************************************** */
