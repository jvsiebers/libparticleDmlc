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
/* readPinnacleCurve.cc
   routines to read patient beam information into the patient beam data structure
   Created: 6/9/98: JVS

   Jan 7, 2002: JVS: Cut routines from readBeams.cc...so easier to distribute
                     particleDmlc
   Feb 3, 2015: JVS: Remove closePinnacleContour(TwoDContourType *contour), moved to libpin
*/
/* ****************************************************************** */
//#define DEBUG_CP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> /* for fabs */
#include <ctype.h> // for isspace

#include "utilities.h"
#include "typedefs.h"
#include "read_clif.h"
/* ****************************************************************** */
int readPinnacleRawData(clifObjectType *RawData, int *npts, Point_2d_float **contour)
{ 
   int NumberOfDimensions=0;
   if(   readIntClifStructure(RawData,"NumberOfPoints",npts) != OK 
      || readIntClifStructure(RawData,"NumberOfDimensions",&NumberOfDimensions) != OK )
   {
       printf("\n ERROR: Reading in RawData"); return(FAIL);
   }

   if(NumberOfDimensions != 2)
   {
       printf("\n ERROR: Expected NumberOfDimensions == 2 in readMLC, found %d", NumberOfDimensions); 
       return(FAIL);
   }
   int nArray=0;
   float *Array;
   if( readClifArrayStructure(RawData, "Points", &nArray, &Array) != OK
        || nArray !=  NumberOfDimensions * *npts )
   {
       printf("\n ERROR: Reading RawData Array"); return(FAIL);
   }
   // Allocate Memory for the MLC contour
   Point_2d_float *Values;
   // allocate memory for the verticies
   Values = (Point_2d_float *)calloc( *npts, sizeof(Point_2d_float));
   if(Values == NULL)
   {
     printf("\n ERROR: allocating memory for Point_2d_float");
     return(FAIL);
   }
   for(int iVal=0, iArray=0; iVal< *npts && iArray < nArray; iVal++, iArray+=2)
   {
      Values[iVal].x = Array[iArray];
      Values[iVal].y = Array[iArray+1];
   }
   *contour = Values;
    /* at end, use *contour = value to assign value */
   //   printf("\n Num points = %d",*npts);
   return(OK);
}
/* ******************************************************************* */
int closePinnacleCurve(int *npts, Point_2d_float **curve)
{
   Point_2d_float *value;
   value = *curve;
   // ensure that last point = first (contour closed), if not, close it
   if( value[0].x != value[*npts-1].x ||
       value[0].y != value[*npts-1].y )
   {
     *npts = *npts+1;  /* reallocate memory for additional point */
     value = (Point_2d_float *)realloc((Point_2d_float *) value, 
	    (*npts)*sizeof(Point_2d_float)); 
     value[*npts-1].x = value[0].x; /* assign coordinates to first point */
     value[*npts-1].y = value[0].y;
   }
   *curve = value;
   return(OK) ;
}
/* ************************************************************************** */
int readPinnacleCurve(clifObjectType *Curve, int *npts, Point_2d_float **contour)
{
   // WARNING: This VERSION does NOT close the curve....
   clifObjectType *RawData;
   if(getClifAddress(Curve,"RawData", &RawData)!= OK)
   {
      printf("\n ERROR: Getting Address"); return(FAIL);
   } 
   return(readPinnacleRawData(RawData, npts, contour) );
}
/* ************************************************************************** */ 
