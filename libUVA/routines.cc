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
/* write_ppm.cc
   routine to write a grayscale portable pixelmap file from an array
   
   Created: 3 Feb 1999: pjk
   Modification History:
            2 Sept 1999: pjk: added integral from jvs code (changed to table type) 
            3 Sept 1999: pjk: added pk_calloc- should easily overload function
            10 Sept 1999: pjk: buggered around with integrate
            1 June 2000: pjk: changed %1d to %3d for writing materials
            27 June 2000: JVS: change readphant to read %3d materials
             9 July 2000: JVS: new readEGS4phantom (very similar to PJK routine)
                               (NOT FINISHED....time constraints.....
            16 July 2000: PJK: changed strncmp to strcmp for media in concat_phant
            15 March 2005: JVS: Format writting in write_phantom so is easier to
                                convert for use with vmc++
            18 July 2005: JVS: Add readPhantomBoundaries....
            17 October 2007: JVS: Split off phantomRoutines.cc
            Feb 23, 2012: JVS: typecast's added to remove compiler warnings
            May 6, 2015: JVS: Add <cmath> and namespace std for solaris compile
*/
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <cmath>
using namespace std;
// #include <sys/syscall.h>

#include "typedefs.h"
#include "defined_values.h"
#include "err_codes.h" // for SUCCESS
#include "dose_regions.h"
#include "dose_region.h"
// jvs headers
#include "utilities.h"
#include "table.h"
#include "spline_data.h"
// pjk headers
#include "common.h"
#include "routines.h"

/* *********************************************************************** */
int read_dose(char *fname, DM_STRUCT *dm, PHANT_STRUCT *p)
{  
  FILE *fp;
  fp = fopen(fname,"rb");
  if(fp == NULL)
    {
     printf("\n ERROR: opening file >%s",fname);
     return(FAIL);
    }
  else
     printf("\n Opened file %s",fname); 

  int i,j,k;
  float doseval;
  for (k=0;k<p->z_num;k++)
   for (j=0;j<p->y_num;j++) 
    for (i=0;i<p->x_num;i++) 
    { 
     if (fread(&doseval,sizeof(float),1,fp) != 1) 
     {
       printf("\n ERROR: Couldn't read dose in %s",fname);
       return(FAIL);
     }
     dm->dose[i][j][k] = doseval; 
    } 
 

  fclose(fp);  

  return(OK);
}
/* *********************************************************************** */
int write_dose(char *fname, DM_STRUCT *dm, PHANT_STRUCT *p)
{  
  FILE *fp;
  char fnameext[MAX_STR_LEN];
  sprintf(fnameext,"%s.conv",fname);
  fp = fopen(fnameext,"wb");
  if(fp == NULL)
    {
     printf("\n ERROR: opening file >%s",fnameext);
     return(FAIL);
    }
  else
     printf("\n Opened file %s",fnameext); 

  int i,j,k;
  float doseval;
  for (k=0;k<p->z_num;k++)
   for (j=0;j<p->y_num;j++) 
    for (i=0;i<p->x_num;i++) 
    { 
     if (fwrite(&doseval,sizeof(float),1,fp) != 1) 
     {
       printf("\n ERROR: Couldn't read dose in %s",fnameext);
       return(FAIL);
     }
     dm->dose[i][j][k] = doseval; 
    } 
 

  fclose(fp);  

  return(OK);
}
/* *********************************************************************** */
int sum_dose(DM_STRUCT *dm, PHANT_STRUCT *p)
{
  float sum_dose = 0.0;
  int i,j,k;
  for (k=0;k<p->z_num;k++)
   for (j=0;j<p->y_num;j++)
    for (i=0;i<p->x_num;i++)
      sum_dose += dm->dose[i][j][k];

  printf("\n Sum of dose = %g",sum_dose);
  return(OK);
}
/* *********************************************************************** */
int convolve_dose(DM_STRUCT *dm, PHANT_STRUCT *p, float sx, float sy, float sz)
{  
  DM_STRUCT temp;
  int i,j,k, l, m, n;
  for (k=0;k<p->z_num;k++)
   for (j=0;j<p->y_num;j++)
    for (i=0;i<p->x_num;i++)
      temp.dose[i][j][k] = dm->dose[i][j][k];

  for (k=0;k<p->z_num;k++)
   for (j=0;j<p->y_num;j++)
    for (i=0;i<p->x_num;i++)
      dm->dose[i][j][k] = 0.0;

  // normalise to voxel units
  float sigx, sigy, sigz;
  sigx = sx/p->x_size;
  sigy = sy/p->y_size;
  sigz = sz/p->z_size;

  int limits_x = (int) ceil(NUM_SD*sigx);
  int limits_y = (int) ceil(NUM_SD*sigy);
  int limits_z = (int) ceil(NUM_SD*sigz);
  int range_x = 2*limits_x + 1;
  int range_y = 2*limits_y + 1;
  int range_z = 2*limits_z + 1;

  ARRAY_STRUCT filter_x, filter_y, filter_z;

  printf("limits %d %d %d\n",limits_x, limits_y, limits_z);
  printf("Calculating filters\n");
  if (int_gauss(range_x, &filter_x, limits_x, sigx)!=OK)
  {
    printf("\n ERROR doing int_gauss");
    return(FAIL);
  }
  if (int_gauss(range_y, &filter_y, limits_y, sigy)!=OK)
  {
    printf("\n ERROR doing int_gauss");
    return(FAIL);
  }
  if (int_gauss(range_z, &filter_z, limits_z, sigz)!=OK)
  {
    printf("\n ERROR doing int_gauss");
    return(FAIL);
  }
   
  printf("\nNow doing convolution!\n");
  for (k=0;k<p->z_num;k++)
  {
   printf("Up to slice %d\n",k+1);
   for (j=0;j<p->y_num;j++)
    for (i=0;i<p->x_num;i++)
    {
    for (l=0;l<range_x;l++)
     for (m=0;m<range_y;m++)
      for (n=0;n<range_z;n++)
     {
     if ((0<=i+l-limits_x) && (i+l-limits_x<p->x_num) && (0<=j+m-limits_y)&&(j+m-limits_y<p->y_num) && (0<=k+n-limits_z)&&(k+n-limits_z<p->z_num))
      {
        dm->dose[i][j][k] += filter_x.index[l]*filter_y.index[m]*
                filter_z.index[n]*
	        temp.dose[i+l-limits_x][j+m-limits_y][k+n-limits_z];
      }

     }
   

    }
  }

  return(OK);
}
/* *********************************************************************** */
int convolve_dose(volume_region_type *dr, float sml, float sap, float ssi, float *dose)
{  
  printf("\nIn convolve_dose");
  THREED_ARRAY temp;
  temp.x_count = dr->n.x;
  temp.y_count = dr->n.y;
  temp.z_count = dr->n.z;
  temp.matrix = (float *)calloc(dr->n.x*dr->n.y*dr->n.z,sizeof(float));
  if(temp.matrix == NULL)
  {
     printf("\n ERROR: Allocating Memory for float Array");
     printf("\n\t x %d y %d z %d", dr->n.x,dr->n.y,dr->n.z);
     return(FAIL);
  }

  int i,j,k, l, m, n;
  for (k=0;k<dr->n.z;k++)
   for (j=0;j<dr->n.y;j++)
    for (i=0;i<dr->n.x;i++)
      ARR_VAL3D(&temp,i,j,k) = dose[k*dr->n.x*dr->n.y + j*dr->n.x + i];


  for (k=0;k<dr->n.z;k++)
   for (j=0;j<dr->n.y;j++)
    for (i=0;i<dr->n.x;i++)
      dose[k*dr->n.x*dr->n.y + j*dr->n.x + i] = 0.0;

  // normalise to voxel units
  float sigx, sigy, sigz;
  sigx = sml/dr->voxel_size.x;
  sigy = sap/dr->voxel_size.y;
  sigz = ssi/dr->voxel_size.z;

  int limits_x = (int) ceil(NUM_SD*sigx);
  int limits_y = (int) ceil(NUM_SD*sigy);
  int limits_z = (int) ceil(NUM_SD*sigz);
  int range_x = 2*limits_x + 1;
  int range_y = 2*limits_y + 1;
  int range_z = 2*limits_z + 1;

  printf("\nGetting filters");fflush(stdout);fflush(stderr);

  ARRAY_STRUCT filter_x, filter_y, filter_z;

  printf("limits %d %d %d\n",limits_x, limits_y, limits_z);
  printf("Calculating filters\n");
  if (int_gauss(range_x, &filter_x, limits_x, sigx)!=OK)
  {
    printf("\n ERROR doing int_gauss");
    return(FAIL);
  }
  if (int_gauss(range_y, &filter_y, limits_y, sigy)!=OK)
  {
    printf("\n ERROR doing int_gauss");
    return(FAIL);
  }
  if (int_gauss(range_z, &filter_z, limits_z, sigz)!=OK)
  {
    printf("\n ERROR doing int_gauss");
    return(FAIL);
  }
   
  printf("\nNow doing convolution!\n");
  for (k=0;k<dr->n.z;k++)
  {
   printf("Up to slice %d\n",k+1);
   for (j=0;j<dr->n.y;j++)
    for (i=0;i<dr->n.x;i++)
    {
    for (l=0;l<range_x;l++)
     for (m=0;m<range_y;m++)
      for (n=0;n<range_z;n++)
     {
     if ((0<=i+l-limits_x) && (i+l-limits_x<dr->n.x) && (0<=j+m-limits_y)&&(j+m-limits_y<dr->n.y) && (0<=k+n-limits_z)&&(k+n-limits_z<dr->n.z))
      {
        dose[k*dr->n.x*dr->n.y + j*dr->n.x + i] += filter_x.index[l]*filter_y.index[m]*
                filter_z.index[n]*
	        ARR_VAL3D(&temp,i+l-limits_x,j+m-limits_y,k+n-limits_z );
	//temp.dose[i+l-limits_x][j+m-limits_y][k+n-limits_z];
      }

     }
   

    }
  }


  return(OK);
}
/* *********************************************************************** */
int int_gauss(int range, ARRAY_STRUCT *filter, int limits, float sigma)
{
#define INT_PTS 10
  int i, j;
  float int_pt, iv[INT_PTS], dose_sum;
  dose_sum = 0.0;
  if (range == 1)
  {
     filter->index[0] = 1.0;
     dose_sum = 1.0;
  }
  else
    for (i=0;i<range;i++)
    {
      for (j=0;j<INT_PTS;j++)
      {
        int_pt = (float) (((float) (i-limits)) + ((float) j)*0.125f - 0.5f); 
        iv[j] = (float) (1.0/(sigma*sqrt(2.0*3.141592654))*
			 exp(-int_pt*int_pt/(2.0*sigma*sigma)));

      }
      filter->index[i] = (1.0f/24.0f)*(iv[0]+iv[8]+4.0f*
                               (iv[1]+iv[3]+iv[5]+iv[7]) +
      		          2.0f*(iv[2]+iv[4]+iv[6]));  
      dose_sum += filter->index[i];
      printf("Filter %d = %f\n",i-limits,filter->index[i]);
    }
  printf("Sum of filter = %f (should be 1.0)\n\n",dose_sum);

  return(OK);
}
/* *********************************************************************** */
/* ************************************************************************ */
int gauss(double *sum_rn)
{  
  // sample from Gaussian using stuff jeff told me from lahet
  // sum 12 random numbers- returns a value centred on 0 with sd 1
 *sum_rn = -6.0; // start at -6 so get 0 mean
 for (int rn=0;rn<12;rn++)
     *sum_rn += (double) rand()/ (double) RAND_MAX;

 return(OK);
}
/* *********************************************************************** */
int gauss(float *sum_rn)
{  
  // sample from Gaussian using stuff jeff told me from lahet
  // sum 12 random numbers- retruns a value centred on 0 with sd 1
 *sum_rn = -6.0; // start at -6 so get 0 mean
 for (int rn=0;rn<12;rn++)
     *sum_rn += (float) rand()/ (float) RAND_MAX;

 return(OK);
}
/* *********************************************************************** */
int plotgnu(char *fname, int sleepe, int type)
{  
  // plot file1 and file 2 for up sleep seconds
  char tempname[MAX_STR_LEN];
  sprintf(tempname,"temp.gnu");
  FILE *fp;
  fp = fopen(tempname,"w");
  if(fp == NULL)
    {
     printf("\nERROR: opening gnuplot file >%s",tempname);
     return(FAIL);
    }
  fprintf(fp," set data style linespoints \n");
  if (type == 0)
    fprintf(fp," plot \"%s\",\"%s\" using 1:3:4 with errorbars\n",fname,fname);
  else 
    printf("\nERROR: type for plotgnu not known");

  fprintf(fp," pause %d \n",sleepe); 
  fclose(fp);
  
  char command[2*MAX_STR_LEN];
  sprintf(command,"gnuplot < %s ",tempname);
  //sleep(1);
  if (system(command) < 0){
      printf("\n ERROR: running %s\n",command);
  }

  return(OK);
}
/* *********************************************************************** */
int plotgnups(char *fname, //int sleepe, 
              int type)
{  
  // plot file1 and file 2 for up sleep seconds
  char tempname[MAX_STR_LEN];
  sprintf(tempname,"temp2.gnu");
  FILE *fp;
  fp = fopen(tempname,"w");
  if(fp == NULL)
    {
     printf("\nERROR: opening gnuplot file >%s",tempname);
     return(FAIL);
    }
 fprintf(fp,"set data style linespoints \n");
 fprintf(fp,"set term postscript landscape color\n");
 fprintf(fp,"set output \"%s.ps\" \n",fname);
  if (type == 0)
    fprintf(fp,"plot \"%s\" , \"%s\" using 1:3:4 with errorbars\n",fname,fname);
  else 
    printf("\nERROR: type for plotgnu not known");

 fprintf(fp,"pause 1 \n");
  fclose(fp);
  
  char command[2*MAX_STR_LEN];
  sprintf(command,"gnuplot < %s &",tempname);
  //sleep(1);
    if (system(command) < 0){
        printf("\n ERROR: running %s\n",command);
    }

  return(OK);
}
/* *********************************************************************** */
float integrate(table_type *tab, float lower, float upper)
{
  // check limits of integration
  if( lower < tab->x[0] )
  {
     printf("\n ERROR: %f is below lower limit of data (%f)\n",lower,tab->x[0]);
     exit(FAIL);
  }
  if( upper > tab->x[tab->n_elements-1] )
  {
     printf("\n ERROR: %f is above upper limit of data (%f)\n",upper,tab->x[tab->n_elements-1]);
     exit(FAIL);
  }
  if( upper==lower)
    {
      // limits are same so integral is 0;
      return(0.0);
    }
  else if( upper<lower)
    {
      // something crazy- exit;
      printf("\n ERROR: Integration lower limit > upper limit. Exiting...");
      exit(FAIL);
    }

  /* find lower and upper cuts for the data */
  int lcut = 0;
  while( tab->x[lcut+1] < lower && lcut < tab->n_elements-1) lcut++;
  int ucut = tab->n_elements-1;
  while( tab->x[ucut-1] > upper && ucut > 1) ucut--;
  /* debugging statements */
  //printf("\n lcut: %d, ucut %d", lcut, ucut);
  //printf("\n lower: %f < %f < %f", tab->x[lcut],lower,tab->x[lcut+1]);
  //printf("\n upper: %f < %f < %f", tab->x[ucut-1],upper,tab->x[ucut]);
  /* perform the integral */
  float integral = 0.0;
  float sumdx = 0.0;
  // lower bound
  float lval = table_value_extrap(lower, tab);
  integral += 0.5f*(lval + tab->y[lcut+1])*(tab->x[lcut+1] - lower);
  sumdx += tab->x[lcut+1] - lower;
  //printf("\nlow (%f,%f) high (%f,%f) int %f sum %f",lower,lval,tab->x[lcut+1],tab->y[lcut+1],integral,sumdx);
  // now rest
  for (int li=lcut+1;li<ucut-1;li++)
  {
    integral += 0.5f*(tab->y[li+1]+tab->y[li])*(tab->x[li+1]-tab->x[li]);
    sumdx += tab->x[li+1]-tab->x[li];
    //printf("\nlow (%f,%f) high (%f,%f) intgrl %f sum %f",tab->x[li],tab->y[li],tab->x[li+1],tab->y[li+1],integral,sumdx);
  }
  // upper bound
  float uval = table_value_extrap(upper, tab);
  integral += 0.5f*(tab->y[ucut-1]+uval)*(upper - tab->x[ucut-1]);
  sumdx += upper - tab->x[ucut-1];
  //printf("\nlow (%f,%f) high (%f,%f) int %f sum %f",tab->x[ucut-1],tab->y[ucut-1],upper,uval,integral,sumdx);
  //printf("\n Integral = %g",integral);
  //printf("\n Sum dx = %g", sumdx);
  //printf("\n Ave = %g\n", integral/sumdx);
  return(integral);
}
/* *********************************************************************** */
/* May 16, 2012: JVS: Don't see why these are necessary -- cause compiler warnings so effectively  delete */
#ifdef USE_PEXIT_PJK
void pexit(spline_data_type *sp)
{
  printf("\nCalloc failed for spline_data_type. Exiting...\n");
  exit(FAIL);
} 
/* *********************************************************************** */
void pexit(table_type *tab)
{
  printf("\nCalloc failed for table_type. Exiting...\n");
  exit(FAIL);
}
#endif
/* *********************************************************************** */
float slope(table_type *tab)
{
  float lower = min_x(tab);
  float upper = max_x(tab);
  printf("\nFor slope lower lim = %f, upper lim = %f",lower,upper);
  float slope_val=slope(tab, lower, upper);
  return(slope_val);
}
/* *********************************************************************** */
float slope(table_type *tab, float lower, float upper)
{
  if (tab->n_elements==0)
  {
    printf("\nERROR: n_elements in table = 0! Exiting");
    exit(FAIL);
  }
  float sumx = 0.0;
  float sumxx = 0.0;
  float sumy = 0.0;
  float sumxy = 0.0;
  int count = 0;
  for(int i=0;i<tab->n_elements;i++)
    if (tab->x[i]>=lower && tab->x[i]<=upper)
    {
      sumx += tab->x[i];
      sumxx += tab->x[i]*tab->x[i];
      sumy += tab->y[i];
      sumxy += tab->x[i]*tab->y[i];
      count ++;
    }

  if (count==0)
  {
    printf("\nERROR in slope: no table values within %f and %f. Exiting....",
             lower, upper);
    exit(FAIL);
  }
  float slope_val = (sumxy-sumx*sumy/(float) count)/(sumxx- sumx*sumx/ (float) count);
  return(slope_val);
}
/* *********************************************************************** */
float intercept(table_type *tab)
{
  float lower = min_x(tab);
  float upper = max_x(tab);
  printf("\nFor intercept lower lim = %f, upper lim = %f",lower,upper);
  float intercept_val=intercept(tab, lower, upper);
  return(intercept_val);
}
/* *********************************************************************** */
float intercept(table_type *tab, float lower, float upper)
{
  if (tab->n_elements==0)
  {
    printf("\nERROR: n_elements in table = 0! Exiting");
    exit(FAIL);
  } 
  //printf("\nn_elements %d",tab->n_elements);
  float sumx = 0.0;
  float sumxx = 0.0;
  float sumy = 0.0;
  float sumxy = 0.0;
  int count = 0;
  for(int i=0;i<tab->n_elements;i++)
    if (tab->x[i]>=lower && tab->x[i]<=upper)
    {
      //printf("\n%d X %f Y %f, low %f up %f", i, tab->x[i], tab->y[i], lower, upper);
      sumx += tab->x[i];
      sumxx += tab->x[i]*tab->x[i];
      sumy += tab->y[i];
      sumxy += tab->x[i]*tab->y[i];
      count ++;
    }

  if (count==0)
  {
    printf("\nERROR in intercept: no table values within %f and %f. Exiting....",lower, upper);
    exit(FAIL);
  }
  float slope_val = (sumxy-sumx*sumy/(float) count)/(sumxx- sumx*sumx/(float) count);
  float intercept_val = sumy/(float) count-slope_val*sumx/(float) count;
  //printf("\nsl %f int %f",slope_val, intercept_val);
  return(intercept_val);
}
/* *********************************************************************** */
float min_x(table_type *tab)
{
  float min = 1.E10;
  for (int i=0;i<tab->n_elements;i++)
    if (tab->x[i]<min)
      min = tab->x[i];

  return(min);
}
/* *********************************************************************** */
float max_x(table_type *tab)
{
  float max = -1.E10;
  for (int i=0;i<tab->n_elements;i++)
    if (tab->x[i]>max)
      max = tab->x[i];

  return(max);
}
/* *********************************************************************** */
float average(table_type *tab, float lower, float upper)
{
  // check limits of integration
  if( lower < tab->x[0] )
  {
     printf("\n ERROR: %f is below lower limit of data (%f)\n",lower,tab->x[0]);
     exit(FAIL);
  }
  if( upper > tab->x[tab->n_elements-1] )
  {
     printf("\n ERROR: %f is above upper limit of data (%f)\n",upper,tab->x[tab->n_elements-1]);
     exit(FAIL);
  }
  if( upper==lower)
    {
      // limits are same so integral is 0;
      return(0.0);
    }
  else if( upper<lower)
    {
      // something crazy- exit;
      printf("\n ERROR: Integration lower limit > upper limit. Exiting...");
      exit(FAIL);
    }

  /* find lower and upper cuts for the data */
  int lcut = 0;
  while( tab->x[lcut+1] < lower && lcut < tab->n_elements-1) lcut++;
  int ucut = tab->n_elements-1;
  while( tab->x[ucut-1] > upper && ucut > 1) ucut--;
  /* debugging statements */
  //printf("\n lcut: %d, ucut %d", lcut, ucut);
  //printf("\n lower: %f < %f < %f", tab->x[lcut],lower,tab->x[lcut+1]);
  //printf("\n upper: %f < %f < %f", tab->x[ucut-1],upper,tab->x[ucut]);
  /* perform the integral */
  float integral = 0.0;
  float sumdx = 0.0;
  // lower bound
  float lval = table_value_extrap(lower, tab);
  integral += 0.5f*(lval + tab->y[lcut+1])*(tab->x[lcut+1] - lower);
  sumdx += tab->x[lcut+1] - lower;
  //printf("\nlow (%f,%f) high (%f,%f) int %f sum %f",lower,lval,tab->x[lcut+1],tab->y[lcut+1],integral,sumdx);
  // now rest
  for (int li=lcut+1;li<ucut-1;li++)
  {
    integral += 0.5f*(tab->y[li+1]+tab->y[li])*(tab->x[li+1]-tab->x[li]);
    sumdx += tab->x[li+1]-tab->x[li];
    //printf("\nlow (%f,%f) high (%f,%f) intgrl %f sum %f",tab->x[li],tab->y[li],tab->x[li+1],tab->y[li+1],integral,sumdx);
  }
  // upper bound
  float uval = table_value_extrap(upper, tab);
  integral += 0.5f*(tab->y[ucut-1]+uval)*(upper - tab->x[ucut-1]);
  sumdx += upper - tab->x[ucut-1];
  //printf("\nlow (%f,%f) high (%f,%f) int %f sum %f",tab->x[ucut-1],tab->y[ucut-1],upper,uval,integral,sumdx);
  //printf("\n Integral = %g",integral);
  //printf("\n Sum dx = %g", sumdx);
  //printf("\n Ave = %g\n", integral/sumdx);
  return(integral/sumdx);
}
/* *********************************************************************** */
int createArray(TWOD_ARRAY *array, int row, int col)
{
  array->x_count = row;
  array->y_count = col;
  array->matrix=(float *)calloc(row*col,sizeof(float));
  if (array->matrix == NULL)
  {
    printf("\n ERROR: memory allocation of array"); 
    return(FAIL);
  }
  
  return(OK);
}
/* *********************************************************************** */
