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
            26 June 2008: JVS: Clean up output
           11 Nov 2008: JVS: Replace MAX_IM_VAL with MAX_N_BOUNDS 
            29 Jan 2009: JVS: Set x_start, x_size, ...
           12 Jan 2010: JVS/NS/CY : Allow reading in of default EGSphant files as well as VCU egsPhant file (different # digits from mednum)
           16 Feb 2010: JVS : Add converting whole phantom to unit density for ETM testing and conversion to Pinnacle CT format
           17 Feb 2010: JVS: Add routine that can override a single material with a density
	   17 Dec 2010: JVS: Add error checking before write
           18 Feb 2013: JVS: Clean up memory when concat_phant ends.
           11 Nov 2015: JVS: max/min were not defined sinc eupdate of utilities.h, use fmax and fmin
*/
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h> // for memcpy
#include <limits.h> // for definition of USHRT_MAX

#include "typedefs.h"
// jvs headers
#include "utilities.h"
// pjk headers
#include "phantomStructure.h"

#if defined (__SUNPRO_CC) // so max,min,fabs are defined
using namespace std;
#define fmax max
#define fmin min
#if !defined( __MINMAX_DEFINED)
#define __MINMAX_DEFINED
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define __max       max
#define __min       min
#endif 

#endif

bool debugPhantom=false;
/* *********************************************************************** */
/*
int readEGS4phantom(char *fname, EGS4PhantomType *p)
{  
  FILE *fp;
  fp = fopen(fname,"r");
  if(fp == NULL)
  {
     printf("\n ERROR: opening file %s",fname); return(FAIL);
  }
  if (fscanf(fp,"%d",&p->nMaterials) != 1)
  {
    printf("\n ERROR: fscan: nMaterials"); return(FAIL);
  }
  // Allocate memory for things that depend upon number of materials 
  p->materialName = new( char [p->nMaterials][MAX_STR_LEN] );
  p->estep         = new( float[p->nMaterials] );

  for (i=0;i<p->nMaterials;i++)
    if (fscanf(fp,"%s",p->materialNames[i]) != 1)
    {
      printf("\n ERROR: fscan: materialNames");
      return(FAIL);
    }
  for (i=0;i<p->num_mat;i++)
    if (fscanf(fp,"%f",&p->estep[i]) != 1)
    {
      printf("\n ERROR: fscan: estep");
      return(FAIL);
    }

  if (fscanf(fp,"%d%d%d",&p->nVoxels.x,&p->nVoxels.y,&p->nVoxels.z) !=3)
  {
    printf("\n ERROR: fscan: reading in x_num,y_num,z_num");
    return(FAIL);
  }
  /// I am too lazy to finish this routine now....JVS: Jul 9, 2000
  for (i=0;i<p->x_num+1;i++)
    fscanf(fp,"%f",&p->x_bound[i]);
  for (i=0;i<p->y_num+1;i++)
    fscanf(fp,"%f",&p->y_bound[i]);
  for (i=0;i<p->z_num+1;i++)
    fscanf(fp,"%f",&p->z_bound[i]);

  p->mednum = (int *)calloc(p->x_num*p->y_num*p->z_num,sizeof(int));
  if(p->mednum == NULL)
  {
     printf("\n ERROR: Allocating Memory for Int Array");
     printf("\n\t x %d y %d z %d", p->x_num,p->y_num,p->z_num);
     return(FAIL);
  }
  
  p->densval = (float *)calloc(p->x_num*p->y_num*p->z_num,sizeof(float));
  if(p->densval == NULL)
  {
     printf("\n ERROR: Allocating Memory for Float Array");
     return(FAIL);
  }
 
  int nread = 0;
  for (k=0;k<p->z_num;k++) 
      for (j=0;j<p->y_num;j++) 
          for (i=0;i<p->x_num;i++) 
	   {
            if( fscanf(fp,"%3d",&p->mednum[k*p->x_num*p->y_num + j*p->x_num +i])== 1) nread++;
	   }
  if(nread != p->z_num*p->y_num*p->x_num)
  {
    printf("\n ERROR: reading in mednum, read in %d, expected %d", nread,p->z_num*p->y_num*p->x_num);
  }

  nread = 0;
  for (k=0;k<p->z_num;k++) 
      for (j=0;j<p->y_num;j++) 
          for (i=0;i<p->x_num;i++) 
	  {
             if(fscanf(fp,"%f",&p->densval[k*p->x_num*p->y_num + j*p->x_num +i])==1) nread++;
	  }

  if(nread != p->z_num*p->y_num*p->x_num)
  {
    printf("\n ERROR: reading in densval, read in %d, expected %d", nread,p->z_num*p->y_num*p->x_num);
  }

  fclose(fp);
  printf("\n For %s",fname);
  printf("\n\tNumber of voxels %d %d %d",p->x_num,p->y_num,p->z_num);
  printf("\n\tSize of voxels   %f %f %f",p->x_bound[1]-p->x_bound[0],p->y_bound[1]-p->y_bound[0],p->z_bound[1]-p->z_bound[0]);
  printf("\n\tStart of voxels  %f %f %f",p->x_bound[0],p->y_bound[0],p->z_bound[0]);
  return(OK);
} 
*/
/* *********************************************************************** */
int readPhantomBoundaries(FILE *istrm, int nBounds, float *bounds)
{
  double tmpValue; 

  // printf("\nreadPhantomBoundaries \n");
  for(int iBound=0; iBound < nBounds; iBound++) 
  {
    if(1!=fscanf(istrm,"%lf",&tmpValue)) {
      printf("\n ERROR: readPhantomBoundaries for boundary %d", iBound);
    }
    bounds[iBound] = (float) (tmpValue);
    // printf("%lf\t",tmpValue);
    // if(iBound%8 == 0) printf ("\n");
  }
  return(OK);
}
/* *********************************************************************** */
int read_phant(char *fname, PHANT_STRUCT *p)
{
  if (debugPhantom) printf("\n Reading phantom if VCU (%%3d) format (99 media max)");
  return(readEgsPhant(fname,p,"%3d"));
}
/* *********************************************************************** */
int readVCUEgsPhant(char *fname, PHANT_STRUCT *p)
{
  if (debugPhantom) printf("\n Reading phantom if VCU (%%3d) format (99 media max)");
  return(readEgsPhant(fname,p,"%3d"));
}
/* *********************************************************************** */
int readDefaultEgsPhant(char *fname, PHANT_STRUCT *p)
{
  if (debugPhantom) printf("\n Reading phantom if VCU (%%1d) format (9 media max)");
  return(readEgsPhant(fname,p,"%1d"));
}
/* *********************************************************************** */
int readEgsPhant(char *fname, PHANT_STRUCT *p, const char *medFormat)
{
#ifdef DEBUG 
  if (debugPhantom) printf("\n Reading In %s\n",fname);
#endif
  FILE *fp;
  fp = fopen(fname,"r");
  if(fp == NULL)
  {
     printf("\n ERROR: opening file >%s",fname);return(FAIL);
  }

  int i,j,k;
  if (fscanf(fp,"%d",&p->num_mat) != 1)
  {
    printf("\n ERROR: fscan: num_mat"); return(FAIL);
  }
  if(p->num_mat > MAX_MED)
  {
     printf("\n ERROR: %d Exceeds the maximum number of materials (%d)", p->num_mat, MAX_MED);
     return(FAIL);
  }
  if(p->num_mat > 9 && strcmp(medFormat,"%1d") == 0) {
    printf("\n ERROR: readEgsPhant: number of materials = %d but mediaFormat = %s", p->num_mat, medFormat); return(FAIL);
  }

  for (i=0;i<p->num_mat;i++)
    if (fscanf(fp,"%s",p->med_name[i]) != 1)
    {
      printf("\n ERROR: fscan");
      return(FAIL);
    }
  for (i=0;i<p->num_mat;i++)
    if (fscanf(fp,"%f",&p->estep[i]) != 1)
    {
      printf("\n ERROR: fscan");
      return(FAIL);
    }


  if (fscanf(fp,"%d%d%d",&p->x_num,&p->y_num,&p->z_num) !=3)
  {
    printf("\n ERROR: fscan: reading in x_num,y_num,z_num");
    return(FAIL);
  }
  if( p->x_num > MAX_N_BOUNDS ||
      p->y_num > MAX_N_BOUNDS ||
      p->z_num > MAX_N_BOUNDS )
  {
     printf("\n ERROR: %d %d or %d Exceeds Maximum Number of Voxels (%d)", 
         p->x_num, p->y_num, p->z_num , MAX_N_BOUNDS);
     return(FAIL);
  }

  // printf("\n Reading In Bounds\n");
  // Read in the x_bounds
   if(OK != readPhantomBoundaries(fp, p->x_num+1, p->x_bound) ) {
    printf("\n ERROR: Reading phantom x boundaries"); return(FAIL);
    } 
   // Read in the y_bounds
   if(OK != readPhantomBoundaries(fp, p->y_num+1, p->y_bound) ) {
    printf("\n ERROR: Reading phantom y boundaries"); return(FAIL);
    }
   // Read in the z_bounds
   if(OK != readPhantomBoundaries(fp, p->z_num+1, p->z_bound) ) {
    printf("\n ERROR: Reading phantom z boundaries"); return(FAIL);
    }

#ifdef DUMP_BOUNDS
  printf ("\n After readPhantomBoundaries\n");
  for (i=0;i<p->x_num+1;i++) {
    //    fscanf(fp,"%f",&p->x_bound[i]);
    printf("%f\t",p->x_bound[i]);
    if(i%8 == 0) printf ("\n");
  }

  for (i=0;i<p->y_num+1;i++){
    // fscanf(fp,"%f",&p->y_bound[i]);
    printf("%f\t",p->y_bound[i]);
    if(i%8 == 0) printf ("\n");
  }
  for (i=0;i<p->z_num+1;i++){
    // fscanf(fp,"%f",&p->z_bound[i]);
    printf("%f\t",p->z_bound[i]);
    if(i%8 == 0) printf ("\n");
  }
#endif
  p->mednum = (int *)calloc(p->x_num*p->y_num*p->z_num,sizeof(int));
  if(p->mednum == NULL)
  {
     printf("\n ERROR: Allocating Memory for Int Array");
     printf("\n\t x %d y %d z %d", p->x_num,p->y_num,p->z_num);
     return(FAIL);
  }
  
  p->densval = (float *)calloc(p->x_num*p->y_num*p->z_num,sizeof(float));
  if(p->densval == NULL)
  {
     printf("\n ERROR: Allocating Memory for Float Array");
     return(FAIL);
  }
 
  // printf("\n Reading In Medium Numbers\n");
  int nread = 0;
#define KEALL_CODE
#ifdef KEALL_CODE
  // OLD KEALL CODE
  for (k=0;k<p->z_num;k++) 
      for (j=0;j<p->y_num;j++) 
          for (i=0;i<p->x_num;i++) 
	   {
            if( fscanf(fp,medFormat,&p->mednum[k*p->x_num*p->y_num + j*p->x_num +i])== 1) nread++;
	   }
#else
  // New Code that should read in default EGSphant files and VCU EGSphant files 
#define iStringLen 2000
  char iString[iStringLen];
  for (int zIndex=0;zIndex<p->z_num;zIndex++) {
    for (int yIndex=0;yIndex<p->y_num;yIndex++) {
      do{
         if (NULL == fgets(iString, iStringLen, fp)) {  // Get a string from the stream
	   printf("\n ERROR: Null found while reading materials for z_num=%d, y_num=%d \n",zIndex,yIndex); return(FAIL);
         }
      }while( strlen(iString) < (unsigned) p->x_num && yIndex == 0 ); // skips blank lines
      if(strlen(iString) == (unsigned) p->x_num) {
	if (debugPhantom) printf("\n Original EGS Format\n");
          for (int xIndex=0;xIndex<p->x_num;xIndex++) {
            if( sscanf(iString,"%d",&p->mednum[zIndex*p->x_num*p->y_num + yIndex*p->x_num +xIndex])== 1) nread++;
	   }
      } else {
          for (int xIndex=0;xIndex<p->x_num;xIndex++) {
            if( sscanf(iString,"%3d",&p->mednum[zIndex*p->x_num*p->y_num + yIndex*p->x_num +xIndex])== 1) nread++;
	  }
      }
    }
  }
#endif

  if(nread != p->z_num*p->y_num*p->x_num)
  {
    printf("\n ERROR: reading in mednum, read in %d, expected %d", nread,p->z_num*p->y_num*p->x_num);return(FAIL);
  }


  // printf("\n Reading In Density Values\n");
  nread = 0;
  for (k=0;k<p->z_num;k++) 
      for (j=0;j<p->y_num;j++) 
          for (i=0;i<p->x_num;i++) 
	  {
             if(fscanf(fp,"%f",&p->densval[k*p->x_num*p->y_num + j*p->x_num +i])==1) nread++;
	  }

  if(nread != p->z_num*p->y_num*p->x_num)
  {
    printf("\n ERROR: reading in densval, read in %d, expected %d", nread,p->z_num*p->y_num*p->x_num);return(FAIL);
  }

  fclose(fp);
  // 1/29/09 : JVS: _start values were not set!
  p->x_start = p->x_bound[0];
  p->y_start = p->y_bound[0];
  p->z_start = p->z_bound[0];
  // Assuming equal voxel size....
  p->x_size = (p->x_bound[p->x_num]-p->x_bound[0])/(float) p->x_num;
  p->y_size = (p->y_bound[p->y_num]-p->y_bound[0])/(float) p->y_num;
  p->z_size = (p->z_bound[p->z_num]-p->z_bound[0])/(float) p->z_num;
   
  if (debugPhantom) {
    printf("\n Read File %s",fname);
    printf("\n\t nMaterials = %d", p->num_mat);
    printf("\n\t Number of voxels %d %d %d",p->x_num,p->y_num,p->z_num);
    printf("\n\t Size of voxels   %f %f %f",p->x_bound[1]-p->x_bound[0],p->y_bound[1]-p->y_bound[0],p->z_bound[1]-p->z_bound[0]);
    printf("\n\t Start of voxels  %f %f %f",p->x_bound[0],p->y_bound[0],p->z_bound[0]);
    printf("\n\t Start of voxels (start) %f %f %f", p->x_start, p->y_start, p->z_start);
    printf("\n\t Size of voxels  %f %f %f", p->x_size, p->y_size, p->z_size);
  }
  return(OK);
}
/* *********************************************************************** */
int read_egsphantheader(char *fname, PHANT_STRUCT *p)
{ 
#ifdef DEBUG 
  if (debugPhantom) printf("\n Reading In %s\n",fname);
#endif
  FILE *fp;
  fp = fopen(fname,"r");
  if(fp == NULL)
  {
     printf("\n ERROR: opening file >%s",fname);return(FAIL);
  }

  int i;
  if (fscanf(fp,"%d",&p->num_mat) != 1)
  {
    printf("\n ERROR: fscan: num_mat"); return(FAIL);
  }
  if(p->num_mat > MAX_MED)
  {
     printf("\n ERROR: %d Exceeds the maximum number of materials (%d)", p->num_mat, MAX_MED);
     return(FAIL);
  }

  for (i=0;i<p->num_mat;i++)
    if (fscanf(fp,"%s",p->med_name[i]) != 1)
    {
      printf("\n ERROR: fscan");
      return(FAIL);
    }
  for (i=0;i<p->num_mat;i++)
    if (fscanf(fp,"%f",&p->estep[i]) != 1)
    {
      printf("\n ERROR: fscan");
      return(FAIL);
    }


  if (fscanf(fp,"%d%d%d",&p->x_num,&p->y_num,&p->z_num) !=3)
  {
    printf("\n ERROR: fscan: reading in x_num,y_num,z_num");
    return(FAIL);
  }
  if( p->x_num > MAX_N_BOUNDS ||
      p->y_num > MAX_N_BOUNDS ||
      p->z_num > MAX_N_BOUNDS )
  {
     printf("\n ERROR: %d %d or %d Exceeds Maximum Number of Voxels (%d)", 
         p->x_num, p->y_num, p->z_num , MAX_N_BOUNDS);
     return(FAIL);
  }

  // printf("\n Reading In Bounds\n");
  // Read in the x_bounds
   if(OK != readPhantomBoundaries(fp, p->x_num+1, p->x_bound) ) {
    printf("\n ERROR: Reading phantom x boundaries"); return(FAIL);
    } 
   // Read in the y_bounds
   if(OK != readPhantomBoundaries(fp, p->y_num+1, p->y_bound) ) {
    printf("\n ERROR: Reading phantom y boundaries"); return(FAIL);
    }
   // Read in the z_bounds
   if(OK != readPhantomBoundaries(fp, p->z_num+1, p->z_bound) ) {
    printf("\n ERROR: Reading phantom z boundaries"); return(FAIL);
    }
#ifdef DUMP_BOUNDS
  printf ("\n After readPhantomBoundaries\n");
  for (i=0;i<p->x_num+1;i++) {
    //    fscanf(fp,"%f",&p->x_bound[i]);
    printf("%f\t",p->x_bound[i]);
    if(i%8 == 0) printf ("\n");
  }

  for (i=0;i<p->y_num+1;i++){
    // fscanf(fp,"%f",&p->y_bound[i]);
    printf("%f\t",p->y_bound[i]);
    if(i%8 == 0) printf ("\n");
  }
  for (i=0;i<p->z_num+1;i++){
    // fscanf(fp,"%f",&p->z_bound[i]);
    printf("%f\t",p->z_bound[i]);
    if(i%8 == 0) printf ("\n");
  }
#endif

  fclose(fp);
  if (debugPhantom){
    printf("\n Read File %s",fname);
    printf("\n\t nMaterials = %d", p->num_mat);
    printf("\n\t Number of voxels %d %d %d",p->x_num,p->y_num,p->z_num);
    printf("\n\t Size of voxels   %f %f %f",p->x_bound[1]-p->x_bound[0],p->y_bound[1]-p->y_bound[0],p->z_bound[1]-p->z_bound[0]);
    printf("\n\t Start of voxels  %f %f %f",p->x_bound[0],p->y_bound[0],p->z_bound[0]);
  }
  return(OK);
}

int writeVCUEgsPhant(char *fname, PHANT_STRUCT *p)
{
  if(NULL  == p->densval) {
    printf("\n ERROR: writeVCUEgsPhant: densval == NULL\n"); return(FAIL);
  }
  if(NULL  == p->mednum) {
    printf("\n ERROR: writeVCUEgsPhant: mednum == NULL\n"); return(FAIL);
  }
  // printf("\n Memory is good to here--writeVCUEgsPhant\n");
  if (debugPhantom) printf("\n Writing EGS Phant file in VCU (%%3d) format\n");
  return(writeEgsPhant(fname,p,"%3d"));  
}
/* *********************************************************************** */
int writeDefaultEgsPhant(char *fname, PHANT_STRUCT *p)
{
  if (debugPhantom) printf("\n Writing EGS Phant file in Default (%%d) format\n");
  if(p->num_mat > 9) {
    printf("\n ERROR: num_mat = %d, max == 9 for default EGS format\n",p->num_mat);return(FAIL);
  }
  return(writeEgsPhant(fname,p,"%d"));  
}
/* *********************************************************************** */
int write_phant(char *fname, PHANT_STRUCT *p)
{
  if (debugPhantom) printf("\n Writing EGS Phant file in VCU (%%3d) format\n");
  if(NULL  == p->densval) {
    printf("\n ERROR: write_phant: densval == NULL\n"); return(FAIL);
  }
  if(NULL  == p->mednum) {
    printf("\n ERROR: write_phant: mednum == NULL\n"); return(FAIL);
  }
  // printf("\n write_phant:  Memory is good at call to writeEgsPhant\n");
  return(writeEgsPhant(fname,p,"%3d"));  
}
/* *********************************************************************** */
int writeEgsPhant(char *fname, PHANT_STRUCT *p, const char *medFormat)
{
  if(NULL  == p->mednum) {
    printf("\n ERROR: writeEgsPhant: mednum == NULL\n"); return(FAIL);
  }
  if(NULL  == p->densval) {
    printf("\n ERROR: writeEgsPhant: densval == NULL\n"); return(FAIL);
  }
  // Make sure not writing too many items
  if ( MAX_N_BOUNDS <= p->x_num ||
       MAX_N_BOUNDS <= p->y_num ||
       MAX_N_BOUNDS <= p->z_num ) {
    printf("\n ERROR: writeEgsPhant: Number of voxels exceeds maximum");
    printf("\n\t MAX_N_BOUNDS = %d",MAX_N_BOUNDS);
    printf("\n\t p->x_num = %d",p->x_num);
    printf("\n\t p->y_num = %d",p->y_num);
    printf("\n\t p->z_num = %d",p->z_num);
    return(FAIL);
  }
  if (MAX_MED <= p->num_mat) {
    printf("\n ERROR: Number of materials (%d) exceeds maximum (%d)\n", p->num_mat, MAX_MED);  return(FAIL);
  }

  FILE *fp;
  fp = fopen(fname,"w");
  if(fp == NULL)
    {
     printf("\n ERROR: opening file >%s",fname);
     return(FAIL);
    }
#ifdef DEBUG
  else
     if (debugPhantom) printf("\n Opened file %s",fname); 
#endif

  int i,j,k;
  fprintf(fp,"%d\n",p->num_mat);
  for (i=0;i<p->num_mat;i++)
    fprintf(fp,"%s\n",p->med_name[i]);

  for (i=0;i<p->num_mat;i++)
    fprintf(fp,"%g ",p->estep[i]);
  fprintf(fp,"\n");

  fprintf(fp,"%5d%5d%5d",p->x_num,p->y_num,p->z_num);
  // X Bounds
  for (i=0;i<p->x_num+1;i++){
    if(!(i%10)) fprintf(fp,"\n");
    fprintf(fp,"%9.4f ",p->x_bound[i]); // Used to be just %g format
  }
  // fprintf(fp,"\n");
  // Y Bounds
  for (i=0;i<p->y_num+1;i++){
    if(!(i%10)) fprintf(fp,"\n");
    fprintf(fp,"%9.4f ",p->y_bound[i]);
  }
  // fprintf(fp,"\n");
  // Z Bounds
  for (i=0;i<p->z_num+1;i++){
    if(!(i%10)) fprintf(fp,"\n");
    fprintf(fp,"%9.4f ",p->z_bound[i]);
  }
  fprintf(fp,"\n");

  // Medium numbers
  for (k=0;k<p->z_num;k++) {
    for (j=0;j<p->y_num;j++) {
      for (i=0;i<p->x_num;i++) {
	// int index=k*p->x_num*p->y_num + j*p->x_num +i;
        // printf("\nWriting %d\n",index);
	fprintf(fp,medFormat,p->mednum[k*p->x_num*p->y_num + j*p->x_num +i]);
      }
      fprintf(fp,"\n");
    }
    fprintf(fp,"\n");
  }

  // 10/24/2012: JVS: Modify from %g to %.4g, only 4 significant digits
  // --- set to %.3f for ETM testing
  // const char *densityFormat="%.3f ";
  const char *densityFormat="%.4g ";
  // Densities
  for (k=0;k<p->z_num;k++) {
    for (j=0;j<p->y_num;j++) {
      for (i=0;i<p->x_num;i++) {
	fprintf(fp,densityFormat ,p->densval[k*p->x_num*p->y_num + j*p->x_num +i]);
      }
      fprintf(fp,"\n");
    }
    fprintf(fp,"\n");
  }

  fclose(fp);
  if (debugPhantom){
    printf("\n Write File %s\t\tmedFormat=%s\tdensityFormat=%s",fname,medFormat,densityFormat);
    printf("\n\t nMaterials = %d", p->num_mat);
    printf("\n\t Number of voxels %d %d %d",p->x_num,p->y_num,p->z_num);
    printf("\n\t Size of voxels   %f %f %f",p->x_bound[1]-p->x_bound[0],p->y_bound[1]-p->y_bound[0],p->z_bound[1]-p->z_bound[0]);
    printf("\n\t Start of voxels  %f %f %f",p->x_bound[0],p->y_bound[0],p->z_bound[0]);
  }
  fflush(stdout);fflush(stderr);
  return(OK);
}
/* *********************************************************************** */
int concat_phant(PHANT_STRUCT *p)
{
  if (debugPhantom) printf("\n Cleaning unused materials from the phantom file\n");
  // declare and alloc phant structure
  PHANT_STRUCT *tp;  
  tp = (PHANT_STRUCT *)calloc(1,sizeof(PHANT_STRUCT));
  if(tp == NULL){
     printf("\n ERROR: Allocating Memory for Temp Phant");
     return(FAIL);
  }
  // copy phant to temporary file
  memcpy(tp,p,sizeof(PHANT_STRUCT));
  if (memcmp(tp,p,sizeof(PHANT_STRUCT))!=0){
     printf("\n ERROR: copying memory\n");
     return(FAIL);
    }

  tp->mednum = (int *)calloc(p->x_num*p->y_num*p->z_num,sizeof(int));
  if(tp->mednum == NULL){
     printf("\n ERROR: Allocating Memory for Int Array");
     printf("\n\t x %d y %d z %d", p->x_num,p->y_num,p->z_num);
     return(FAIL);
  }
  
  tp->densval = (float *)calloc(p->x_num*p->y_num*p->z_num,sizeof(float));
  if(tp->densval == NULL){
     printf("\n ERROR: Allocating Memory for Float Array");
     return(FAIL);
  }

  // will only copy structure. Need this also 
  int i,j,k;
  for (i=0;i<p->x_num*p->y_num*p->z_num;i++)
    {
     tp->mednum[i] = p->mednum[i];
     tp->densval[i] = p->densval[i];
    }

  // if media don't exist, reduce med_num by one and shuffle all others down
  // always keep air for outside phantom
#ifdef DEBUG
  printf("\n Keeping %s",tp->med_name[0]);
#endif
  int p_count = 0; 
  for (i=1; i<tp->num_mat;i++){
    int no_match = 0;
    for (k=0;k<tp->x_num*tp->y_num*tp->z_num;k++)
      if (tp->mednum[k] == i+1)
	no_match += 1;

    //    int len lena, lenb;
    //lena = (int) strlen(p->med_name[p_count]);  
    //lenb = (int) strlen(tp->med_name[i]);
    //len = MIN(lena,lenb);
    //printf("\n Comparing %s (%d) and %s (%d)",p->med_name[p_count],p_count,tp->med_name[i],i);
    
    //if ((no_match == 0) || (strncmp(p->med_name[p_count],tp->med_name[i],len)==0)) // get rid of material name[i] cos either not there or copy 
    if ((no_match == 0) || (strcmp(p->med_name[p_count],tp->med_name[i])==0)) // get rid of material name[i] cos either not there or copy 
    {
#ifdef DEBUG
      if (no_match == 0)
	printf("\n   No %s found. Removing...",tp->med_name[i]);
      else
	printf("\n   %s and %s are the same. Removing %s",p->med_name[p_count],tp->med_name[i],tp->med_name[i]);
#endif
      p->num_mat -= 1;
      for (j=p_count;j<p->num_mat;j++)
	strcpy(p->med_name[j+1],tp->med_name[j+1+tp->num_mat-p->num_mat]);

      for (k=0;k<tp->x_num*tp->y_num*tp->z_num;k++)
	if (p->mednum[k]>p_count+1)
	  p->mednum[k] -=1;
    }
    else
      p_count += 1;
  }
  free(tp->densval);
  free(tp->mednum);
  free(tp);

  return(OK);
}

/*************************************************************************************
   Author: CYAN
   Date: February 26, 2010
   Purpose: write out a itk dvf header file such that the itk dvf can be read
            by rcf.   
**************************************************************************************/

int write_itkheader(PHANT_STRUCT *p, char* dvfFilePath, char *itkDvfFileName)
{
  char itkDvfHeadName[MAX_STR_LEN];
  strcpy(itkDvfHeadName, dvfFilePath);
  strcat(itkDvfHeadName, itkDvfFileName);
  strcat(itkDvfHeadName, ".mhd");

  char itkDvfBinName[MAX_STR_LEN];
  strcpy(itkDvfBinName, itkDvfFileName);
  strcat(itkDvfBinName, ".img");

  FILE* outputFile = fopen(itkDvfHeadName, "w");
  if( NULL == outputFile ){
    printf("\n ERROR: Failed to open file: %s\n", itkDvfHeadName); return FAIL;
  }

  Point_3d_float voxelSize;
  Point_3d_float origin;
  voxelSize.x = p->x_bound[1] - p->x_bound[0];
  voxelSize.y = p->y_bound[1] - p->y_bound[0];
  voxelSize.z = p->z_bound[1] - p->z_bound[0];

  origin.x = p->x_bound[0];
  origin.y = p->y_bound[0];
  origin.z = p->z_bound[0];

  // check the machine ENDIAN
  int LOCAL_ENDIAN = check_byte_order();

  fprintf(outputFile, "ObjectType = Image\n");
  fprintf(outputFile, "NDims = 3\n");
  fprintf(outputFile, "BinaryData = True\n");

  if(LOCAL_ENDIAN == BIG_ENDIAN){
    fprintf(outputFile, "BinaryDataByteOrderMSB = True\n");
  }else if(LOCAL_ENDIAN == LITTLE_ENDIAN){
    fprintf(outputFile, "BinaryDataByteOrderMSB = False\n");
  }else{
    printf("\n ERROR Unrecognized machine endian.\n");    
  }

  fprintf(outputFile, "TransformMatrix = 1 0 0 0 1 0 0 0 1\n");
  fprintf(outputFile, "Offset = %5.6f  %5.6f  %5.6f\n", origin.x * 10.0, origin.y * 10.0, origin.z * 10.0);
  fprintf(outputFile, "CenterOfRotation = 0 0 0\n");
  fprintf(outputFile, "CenterOfRotation = RPI\n");
  fprintf(outputFile, "ElementSpacing = %5.6f %5.6f %5.6f\n", voxelSize.x * 10.0, voxelSize.y * 10.0, voxelSize.z * 10.0);
  fprintf(outputFile, "DimSize = %d %d %d\n", p->x_num+1, p->y_num+1, p->z_num+1);
  fprintf(outputFile, "ElementType = MET_USHORT\n");
  fprintf(outputFile, "ElementDataFile = %s\n", itkDvfBinName);
  
  fclose(outputFile);

  return OK; 
}
/* ************************************************************************ */
int convertEgsPhantToUnitDensity(PHANT_STRUCT *p)
{
  if (debugPhantom) printf("\n Converting All Voxels of phantom to unit density....");
  for (int k=0;k<p->z_num;k++){
    for (int j=0;j<p->y_num;j++){
      for (int i=0;i<p->x_num;i++){
	   p->densval[k*p->x_num*p->y_num + j*p->x_num +i] = 1.0;
      }
    }
  }
  if (debugPhantom) printf("  ...Done\n");
  return(OK);
}
/* ****************************************************************************** */
int writeEgsPhantAsCT(const char *fileName, PHANT_STRUCT *p) 
{
  if (debugPhantom) printf("\n Writing EGSPHANT as CT to filename %s", fileName);
  size_t nImage=p->x_num*p->y_num*p->z_num;
  // Step 1: Find max density
  float maxDensity=p->densval[0];
  for(size_t iImage=0; iImage<nImage; iImage++)
    maxDensity=(float) fmax(maxDensity,p->densval[iImage]);
  if(maxDensity<=0.0) {
    printf("\n ERROR: maxDensity = %f", maxDensity); return(FAIL);
  }
#define ITYPE short int
  // Allocate memory for the CT image
  ITYPE *image;
  printf("\n Allocating %ld bytes",(long) nImage*sizeof(ITYPE));
  image = (ITYPE *)calloc(nImage,sizeof(ITYPE));
  if(image == NULL){
    printf("\n ERROR: Cannot alloate memory for image\n");return(FAIL);
  }
  // Convert each voxel
  float scaleFactor = USHRT_MAX/maxDensity;
  for(size_t iImage=0; iImage<nImage; iImage++) {
    image[iImage] = (ITYPE) (p->densval[iImage] * scaleFactor);
  }
  // Write out the image
  FILE *ostrm=fopen(fileName,"wb");
  if(NULL == ostrm) {
    printf("\n ERROR: opening %s\n",fileName); return(FAIL);
  }
  size_t nwrite = fwrite(image, sizeof(ITYPE),nImage,ostrm);
  fclose(ostrm);
  if(nImage != nwrite){
    printf("\n ERROR: Num written (%lu) not equal n_image (%lu)\n",(unsigned long) nwrite,(unsigned long) nImage); return(FAIL);
  }

  return(OK);
}
/* ****************************************************************************** */
int copyEGSPhantom(PHANT_STRUCT *phantom1, PHANT_STRUCT *targetPhantom) 
{
  if (debugPhantom) printf("\n copyEGSPhantom .... ");
  // Create values for target phantom
  targetPhantom->num_mat = phantom1->num_mat;
  // printf("\n num_mat: %d, %d", targetPhantom->num_mat, phantom1->num_mat);
  targetPhantom->x_num = phantom1->x_num;
  targetPhantom->y_num = phantom1->y_num;  
  targetPhantom->z_num = phantom1->z_num;
  int nVoxelArray = phantom1->x_num*phantom1->y_num*phantom1->z_num;
  targetPhantom->x_size = phantom1->x_size;
  targetPhantom->y_size = phantom1->y_size;
  targetPhantom->z_size = phantom1->z_size;
  targetPhantom->x_start = phantom1->x_start;
  targetPhantom->y_start = phantom1->y_start;
  targetPhantom->z_start = phantom1->z_start;
  for(int i=0; i<phantom1->num_mat; i++)
    strcpy(targetPhantom->med_name[i],phantom1->med_name[i]);
  targetPhantom->mednum = (int *)calloc(nVoxelArray,sizeof(int));
  if(targetPhantom->mednum == NULL){
     printf("\n ERROR: copyPhantom: Allocating Memory for mednum Array");
     return(FAIL);
  }
  targetPhantom->densval = (float *)calloc(nVoxelArray,sizeof(float));
  if(targetPhantom->densval == NULL)
  {
     printf("\n ERROR: copyPhantom: Allocating Memory for Float densval Array");return(FAIL);
  }
  for(int i=0; i<phantom1->x_num+1;i++) targetPhantom->x_bound[i] = phantom1->x_bound[i];
  for(int i=0; i<phantom1->y_num+1;i++) targetPhantom->y_bound[i] = phantom1->y_bound[i];
  for(int i=0; i<phantom1->z_num+1;i++) targetPhantom->z_bound[i] = phantom1->z_bound[i];

  for(int i=0; i<nVoxelArray; i++) {
    targetPhantom->mednum[i]=phantom1->mednum[i];
    targetPhantom->densval[i]=phantom1->densval[i];
  }  
  if (debugPhantom) printf("...completed\n");
  return(OK);
}
/* ******************************************************* */
int createRatioPhantom(PHANT_STRUCT *phantom1,PHANT_STRUCT *phantom2, PHANT_STRUCT *targetPhantom) 
{
  printf("\n createRatioPhantom .... ");
  if(OK != copyEGSPhantom(phantom1, targetPhantom) ) {
    printf("\n ERROR: createRatioPhantom: copyPhantom\n"); return(FAIL);
  }
  int nVoxelArray = phantom1->x_num*phantom1->y_num*phantom1->z_num;
  for(int i=0; i<nVoxelArray; i++) {
    if(phantom2->densval[i] > 0.0f) {
      targetPhantom->densval[i] = phantom1->densval[i]/phantom2->densval[i];
    }
  } 
  printf(" createRatioPhantom ....  ...completed\n");  
  return(OK);
}
/* ******************************************************* */
int compareEgsPhantProperties(PHANT_STRUCT *phantom1, PHANT_STRUCT *phantom2) 
{
  // QA: Make sure can validly compare these phantoms 
  if(phantom1->x_num != phantom2->x_num ){
    printf("\n ERROR: x_num for the two phantoms is not equal\n");  return(FAIL);}
  if(phantom1->y_num != phantom2->y_num ){   
    printf( "\n ERROR: y_num for the two phantoms is not equal\n");  return(FAIL);}
  if(phantom1->z_num != phantom2->z_num ){  
    printf( "\n ERROR: z_num for the two phantoms is not equal\n");  return(FAIL);} 
   
  if(phantom1->x_start != phantom2->x_start){
    printf( "\n ERROR: x_start for the two phantoms is not equal\n");return(FAIL);} 
  if(phantom1->y_start != phantom2->y_start){
    printf( "\n ERROR: y_start for the two phantoms is not equal\n");return(FAIL);}
  if(phantom1->z_start != phantom2->z_start){
    printf( "\n ERROR: z_start for the two phantoms is not equal\n");return(FAIL);}
  if( phantom1->x_size != phantom2->x_size ){
    printf( "\n ERROR: x_size  for the two phantoms is not equal\n");
    printf("\n  phantom_1 x_size = %f", phantom1->x_size);
    printf("\n  phantom_2 x_size = %f", phantom2->x_size);  
    return(FAIL);
  }
  if( phantom1->y_size != phantom2->y_size ){
    printf( "\n ERROR: y_size  for the two phantoms is not equal\n");return(FAIL);
  }
  if( phantom1->z_size != phantom2->z_size ){
    printf( "\n ERROR: z_size  for the two phantoms is not equal\n");return(FAIL);
  }
  if (debugPhantom) { printf("\n Phantom passed QA ");fflush(NULL); }
  // End of QA
  return(OK);
}
/* ******************************************************* */
int overrideEGSMaterialDensity(const PHANT_STRUCT *phantom, const int materialID, const float newDensity)
{
  if(materialID > phantom->num_mat || materialID <= 0 ) {
    printf("\n ERROR: overrideMaterialDensity: Requesting material %d which is greater than the number of materials in the problem (%d) or less than 1", materialID, phantom->num_mat);
    return(FAIL);
  }
  printf("\n Overriding material %d (%s) with density %f", materialID, phantom->med_name[materialID-1], newDensity);
  int nVoxelArray = phantom->x_num*phantom->y_num*phantom->z_num;
  for(int i=0; i<nVoxelArray; i++) {
    if(phantom->mednum[i] == materialID && phantom->densval[i] < newDensity) {
      phantom->densval[i] = newDensity;
    }
  }
  return(OK);
}
/* ******************************************************* */
int findEGSPhantBoundingBox(const PHANT_STRUCT *p, Point_3d_int *lowerCorner, Point_3d_int *upperCorner )
{
  // JVS: Routine under development -- not finished 2/18/2010
  int materialID = p->mednum[0]; // Assumes 1st voxel is 
  printf("\n Finding bounding box of non-materialID = %d (non-%s) voxels ", materialID, p->med_name[materialID-1]);
  // initialize box to be whole image
  /*  lowerCorner->x=lowerCorner->y=lowerCorner->z = 0; 
  upperCorner->x=p->x_num-1;
  upperCorner->y=p->y_num-1;
  upperCorner->z=p->z_num-1;
  */
  // initialize box to be none of image
  upperCorner->x=upperCorner->y=upperCorner->z = 0; 
  lowerCorner->x=(short)p->x_num;
  lowerCorner->y=(short)p->y_num;
  lowerCorner->z=(short)p->z_num;
  unsigned iVoxel=0; // the voxel ID
  int iX,iY,iZ;
  //  for(iZ=0; iZ < p->z_num; iZ++) {
  for(iZ=0; iZ < p->z_num; iZ++) {
    for(iY=0; iY < p->y_num; iY++) {  
      int tstIndex=0; int found=0;
      for(iX=0; iX < p->x_num; iX++, iVoxel++) {
	// printf("%d",p->mednum[iVoxel]);
        if(p->mednum[iVoxel] == materialID && !found) { 
           tstIndex++;
	} else { found=1; }
      }
      // iVoxel = iX + p->x_num * (iY + (p->y_num * iZ));
      // printf("\n %d %d ", iZ, iY);
      lowerCorner->x = (short) fmin((float) tstIndex,(float) lowerCorner->x); 
    }
    // printf("\n");
  }
  if (debugPhantom) printf("\n\tlowerCorner->x = %d", lowerCorner->x);
  
  return(OK);
}
/* ******************************************************* */

