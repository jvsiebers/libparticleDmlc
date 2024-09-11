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
/* *********************************************************************** */
/* SPLINE.C                                                                */
/*                                                                         */
/*  WRITTEN BY: J. SIEBERS                                                 */
/*              8-23-90                                                    */
/*	Modification History:                                              */
/* 27-Nov-91:  Making an independent program for use with other data       */
/* 05-Dec-91:  Adapting to use with normalization program                  */
/* 02-July-1993: JVS: khi = dat->ndata -1; since offset arrays             */
/* 04-October-1993: JVS: to CPP --                                         */
/* 07-October-1998: JVS: Port to unix                                      */
/* Sept 03, 1999: JVS: Change data_point to spline_data_type               */
/*                                                                         */
/* *********************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "spline_data.h"
#include "utilities.h" /* for OK, FAIL definitions */
/* *********************************************************************** */
/* *********************************************************************** */
/* ***********************************************************************  */
int spline(spline_data_type *dat, float yp1, float ypn)
{
	double  *u;              /* yp1 and ypn are for the end conditions */
	double qn=0.0,un=0.0,sig=0.0,p=0.0;
	int i,n;

	n = dat->n_data;
	u = (double *) malloc((dat->n_data+1)*sizeof(double));
	if(u==NULL)
	{
		printf("ERROR: Memory Allocation Error, u in spline");
		return(FAIL);
	}			       /* the data structure contains ndata and  */
	if(yp1 == -1)               /* the x and y values                     */
	{                           /* the value s ( the first derivative )   */
		dat->y2[0] = 0.0;        /* at the tabulated points                */
		u[0] = 0.0;              /* the array u is used for temporary storage*/
	}
	else
	{
		dat->y2[0] = -0.5;
		u[0] = ( 3.0 / ( dat->x[1] - dat->x[0]) ) *
			( (dat->y[1] - dat->y[0])   /
			  (dat->x[1] - dat->x[0])   - yp1);
	}
	for(i=1; i < dat->n_data - 1; i++)
	{
	  if(dat->x[i+1] == dat->x[i-1] ||
		  dat->x[i+1] == dat->x[i]   ||
		  dat->x[i]   == dat->x[i-1])
	  {
		  printf("ERROR: spline: x-data not monotonically increasing");return(FAIL);
	  }
	  sig  = ( dat->x[i] - dat->x[i-1] ) / ( dat->x[i+1] - dat->x[i-1] );
	  p    = sig * dat->y2[i-1] + 2.0;
	  if(p==0)
	  {
		  printf("ERROR: spline: p==0");return(FAIL);
	  }
	  dat->y2[i] = (sig - 1.0)/p;
	  u[i] = ( 6.0 * ( ( dat->y[i+1] - dat->y[i]   ) /
							 ( dat->x[i+1] - dat->x[i]   ) -
							 ( dat->y[i]   - dat->y[i-1] ) /
							 ( dat->x[i]   - dat->x[i-1] )  )  /
				( dat->x[i+1] - dat->x[i-1] ) - sig * u[i-1] ) / p;
	}
	if(ypn == -1)
	{
		qn = 0.0;
		un = 0.0;
	}
	else
	{
		qn= 0.5;
		un= ( 3.0 / ( dat->x[n-1] - dat->x[n-2] ) ) *
		  ( ypn - ( dat->y[n-1] - dat->y[n-2]) /
					 ( dat->x[n-1] - dat->x[n-2]) );
	}
	dat->y2[n] = ( un - qn * u[n-2]) /( qn * dat->y2[n-2] + 1.0);
	for(i=dat->n_data - 2; i>0; i--)
	{
		dat->y2[i] = dat->y2[i] * dat->y2[i+1] + u[i];
	}
	free(u);
	return(OK);
}
/* ********************************************************************* */
int spline_interp(spline_data_type *dat, double  x2, double  *y2)
{
	int k=0,klo=0,khi=0;
	double h=0.0,a=0.0,b=0.0;
	double line1,line2,line3,line4,line5;

	klo = 0;
	khi = dat->n_data - 1;
	while( (khi - klo) > 1 )
	{
		k = (khi + klo) / 2;
		if( !(dat->x[k] <= x2 ) )
		{
			khi = k;
		}
		else
		{
			klo = k;
		}
	}
	h   =  dat->x[khi] - dat->x[klo];
	if(h == 0)
	{
	  printf("\nERROR: H==0 in spline_interp");
	  printf("\n\t no x-gradient in input data");
	  printf("\n\t x[%d] = %g, x[%d] = %g",klo,dat->x[klo],khi,dat->x[khi]);
	  return(FAIL);
	}
	a = (dat->x[khi] -         x2) / h;
	b = (x2          - dat->x[klo]) / h;
	line1 = a * (double)( dat->y[klo]);
	line2 = b * (double)( dat->y[khi]);
	line3 =	     ( a * ( a*a - 1.0 ) ) * dat->y2[klo];
	line4 =	     ( b * ( b*b - 1.0 ) ) * dat->y2[khi];
	line5 = 	     (h * h) / 6.0;
	*y2 = line1 + line2 + ( line3 + line4 ) * line5;
/*
	*y2 = a * (double)( dat->y[klo]) + b * (double)( dat->y[khi])
		  + ( ( a * ( a*a - 1.0 ) ) * dat->y2[klo]
		  +   ( b * ( b*b - 1.0 ) ) * dat->y2[khi] )
		  * (h * h) / 6.0;
*/
/*
	printf("\n L1 %lf L2 %lf L3 %lf L4 %lf L5 %lf",line1,line2,line3,line4,line5);
	printf("\n x2 %lf y2 %lf",x2,*y2);
*/
	return(OK);
}
/* *************************************************************** */
