/* egsPhantRoutines.cc
   Created: Feb 23, 2012: JVS: Additional routines used with the PHANT_STRUCT data type
            March 12, 2012: JVS: Add 2 routines
	    May 12, 2013: JVS: Move general routines from egsPhantMassMapping.cpp to here
            May 6, 2015: JVS: Add <cmath> and namespace std for solaris compile
	    June 4, 2015: JVS: cout's were causing havoc with g++ solaris compile...change all to printf

*/
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

//#include <time.h>  // for time for printRunTimeInformation
#include <math.h> // for fabs and roundf
#include <string.h> // for strlen
#include <cmath>

// #include "option.h"
#include "table.h"
#include "utilities.h"
#include "read_scanner.h"
#include "ct_routines.h" 

#include "common.h" // PJK's read_phant routine and PHANT_STRUCT type def....
#include "readWriteEndianFile.h" // For writeBinaryDataToFile and reverse_float_byte_order
#include "egsPhantRoutines.h"

#if defined (__SUNPRO_CC) // so max,min,fabs are defined
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

using namespace std;

/* **************************************** */
int regularizePhantomBoundaries(float *bounds, int nBounds)
{
  /* To ensurs that phantom boundaries are consitent over the phantom....
     VMC++ has 1e-5 tolerance on bounds....When bounds are read from .egs4phant and converted
     the noise (from converting double/float conversion) is sometimes larger than 1e-5, 
     so, this routine tries to regularize the boundaries */

  // Use the limits of the data to determint the bounds
  double deltaBounds = (bounds[nBounds-1]-bounds[0])/(float) (nBounds-1);
  deltaBounds = (double) round(10000.0*deltaBounds)*0.0001; //< was roundf, 
#ifdef DEBUG
  printf("\n Actual delta bounds determined from limits is %.8f", deltaBounds);
#endif
  // Check that bounds are the same for all voxels
  double maxDelta = 0.0;
  for(int iBound=1; iBound<nBounds; iBound++)
  {
    double thisBound = bounds[iBound]-bounds[iBound-1];
    double thisDelta =  fabs(1.00-thisBound/deltaBounds);
    maxDelta = max(maxDelta, thisDelta);
    if( thisDelta >  1.5e-3 ) {
      printf("\n ERROR: Bound variance is greater than tolerance");
      printf("\n\t at point %d, deviation is %g \n", iBound,  thisDelta );
      return(FAIL);
    }
  }
#ifdef DEBUG
  printf("\n  maximum bounds delta is %g ",maxDelta);
#endif
  // Reset the boundaries to use deltabounds
  for(int iBound=0; iBound < nBounds; iBound++) 
  {
    bounds[iBound] =  bounds[0]+(float) iBound*(float) deltaBounds;
  }
  return(OK);
} 
/* ******************************************************* */
int reportOnPhantomDensity(PHANT_STRUCT *phantom)
{
  int nVoxelArray = phantom->x_num*phantom->y_num*phantom->z_num;
  float maxDensity = phantom->densval[0];
  double sumDensity=0.0;
  for(int iArray=0; iArray<nVoxelArray; iArray++)
  {
    maxDensity = max(maxDensity,phantom->densval[iArray]);
    sumDensity+=phantom->densval[iArray];
  }
  printf("\n Maximum density in phantom is %f", maxDensity);
  printf("\n Sum of density in phantom is %f", sumDensity);
  printf("\n Average density in phantom is %f", sumDensity/nVoxelArray);
  return(OK);
}
/* ************************************************************* */
int ensureWaterIsInPhantomDensity(PHANT_STRUCT *phantom)
{
  // To properly compute dose-to-water, VMC++ requires at least one voxel 
  // in the phantom to be "water" (and requires that water be the first
  // material in the ct_ramp.data
  // This routine will check all voxels and count how many voxels have density = 1
  // (Actually, 0.999999  < density < 1.000001 ), where #'s are from 1st entry of ct_ramp.data
  //  If no materials are water, then set first voxel as being water and report
  const float minWaterDensity =  0.999999f;  // Should read from ct_ramp.data
  const float maxWaterDensity =  1.000001f;  // Should read from ct_ramp.data
  //
  int nVoxelArray = phantom->x_num*phantom->y_num*phantom->z_num;
  int numberWaterVoxels = 0;
  for(int iArray=0; iArray<nVoxelArray; iArray++)
  {
    if(  (phantom->densval[iArray] >   minWaterDensity) && 
         (phantom->densval[iArray] <=  maxWaterDensity) )
            numberWaterVoxels++;
  }
  if( 0 == numberWaterVoxels) {
    printf("\n WARNING: No voxels containing water were found in phantom");
    printf("\n\t Assigning first voxel of phantom to have density of 1.00000");
    phantom->densval[0]=1.00000;
  } else {
    printf("\n Phantom contains water in %f percent of the voxels", 100.0*numberWaterVoxels/nVoxelArray);
  }
  return(OK);
}
/* ********************************************************************** */
int TESTwriteVoxelBoundaries(FILE *outputStream, int swab_flag, int nVoxels, float voxelOrigin, float voxelSize, float *bounds)
{
  int nBoundaries = nVoxels+1; // need to write both edges
  // Create floating point array of boundaries
  float *boundary = (float *) calloc(nBoundaries,sizeof(float));
   if(NULL == boundary) {
      printf("\n ERROR: memory allocation"); return(FAIL);
   }
   for(int iVoxel=0; iVoxel<nBoundaries; iVoxel++)
   {
     boundary[iVoxel]=voxelOrigin + (float) iVoxel*voxelSize;
   }
   printf("\n Boundaries: %f  to %f, (nVoxels=%d, voxelSize=%f, voxelOrigin=%f)", boundary[0], boundary[nBoundaries-1],nVoxels, voxelSize, voxelOrigin);
   // write them to the file
  if(OK != writeBinaryDataToFile(outputStream, nBoundaries, boundary, swab_flag) )
   {
     printf("\n ERROR: Writing boundaries to file"); return(FAIL);
   }
  printf("\n ------------------------------------\n");
   for(int i=0; i< nVoxels; i++) {
     if (i%10 == 0) printf ("\n");
     printf("\n %f %f %g", boundary[i], bounds[i], boundary[i]-bounds[i]);
   }
  return(OK);
}
/* ************************************************************ */
int writeVoxelBoundaries(FILE *outputStream, int swab_flag, int nVoxels, float voxelOrigin, float voxelSize)
{
  int nBoundaries = nVoxels+1; // need to write both edges
  // Create floating point array of boundaries
  float *boundary = (float *) calloc(nBoundaries,sizeof(float));
   if(NULL == boundary) {
      printf("\n ERROR: memory allocation"); return(FAIL);
   }
   for(int iVoxel=0; iVoxel<nBoundaries; iVoxel++)
   {
     boundary[iVoxel]=voxelOrigin + (float) iVoxel*voxelSize;
   }
   printf("\n Boundaries: %f  to %f, (nVoxels=%d, voxelSize=%f, voxelOrigin=%f)", boundary[0], boundary[nBoundaries-1],nVoxels, voxelSize, voxelOrigin);
   // write them to the file
  if(OK != writeBinaryDataToFile(outputStream, nBoundaries, boundary, swab_flag) )
   {
     printf("\n ERROR: Writing boundaries to file"); return(FAIL);
   }
  free(boundary);
  return(OK);
}
/* ******************************************************* */
int writeVMCppLittleEndianBinaryDensityFile(char *densityFileName,  PHANT_STRUCT *phantom)
{
   // Check Byte order of current machine, determine if need to swab.....
   int swab_flag = 0;
   switch( (check_byte_order()) )
   {
      case BIG_ENDIAN:
	swab_flag=1;
         break;
      case LITTLE_ENDIAN:
	 break;
      default:
         printf("\n ERROR: Indeterminate Byte Order\n");
         return(FAIL);
   }
   // Open file for writing
   FILE *outputStream= fopen(densityFileName,"wb");
   if (NULL == outputStream) {
     printf("\n ERROR: Cannot open file %s for writing\n",densityFileName); return(FAIL); 
   }
   // Write the header portion which includes
   //  1. nx, ny, nz  (ints)
   //  2. nx+1 xBoundaries (floats)
   //  3. ny+1 yBoundaries (floats)
   //  4. nz+1 zBoundaries (floats)
   //  5. nx*ny*nz density values (floats)

   // Step-by-step process......will use predefined functions that will do the swabbing if necessary
   //
   //  1. nx, ny, nz  (ints)   
   {
      int nVoxels[3];
      if(swab_flag) {
        nVoxels[0] = reverse_int_byte_order(phantom->x_num);
        nVoxels[1] = reverse_int_byte_order(phantom->y_num);
        nVoxels[2] = reverse_int_byte_order(phantom->z_num);
      } else {
        nVoxels[0] = phantom->x_num;
        nVoxels[1] = phantom->y_num;
        nVoxels[2] = phantom->z_num;
      }
      int nWrite = (int) fwrite(nVoxels, sizeof(int), 3,outputStream);
      if(3 != nWrite) {
	printf("\n ERROR: Writing number of voxels"); return(FAIL);
      }
      printf ("\n Wrote nVoxels %d %d %d", nVoxels[0],nVoxels[1],nVoxels[2]);
   }
   //
#ifdef HACK_BOUNDS
   // The following was required when CT create wrote out floating point #'s with roundoff errors...
   // But this does not guarentee using the correct bounds..
   // X
   phantom->x_size = phantom->x_bound[1]-phantom->x_bound[0];
   phantom->x_start = phantom->x_bound[0];
   if(OK != writeVoxelBoundaries(outputStream, swab_flag, phantom->x_num,  phantom->x_start, phantom->x_size))
   {
     printf("\n ERROR: Writing x voxel boundaries to file %s", densityFileName); return(FAIL);
   }
   // Y
   phantom->y_size = phantom->y_bound[1]-phantom->y_bound[0];
   phantom->y_start = phantom->y_bound[0];
   if(OK != writeVoxelBoundaries(outputStream, swab_flag, phantom->y_num,  phantom->y_start, phantom->y_size))
   {
     printf("\n ERROR: Writing y voxel boundaries to file %s", densityFileName); return(FAIL);
   }
   // Z
   phantom->z_size = phantom->z_bound[1]-phantom->z_bound[0];
   phantom->z_start = phantom->z_bound[0];
   if(OK != writeVoxelBoundaries(outputStream, swab_flag, phantom->z_num,  phantom->z_start, phantom->z_size))
   {
     printf("\n ERROR: Writing z voxel boundaries to file %s", densityFileName); return(FAIL);
   }
   printf("\n ====================HACKED BOUNDS USED================================");
#else
   // X
   //  2. nx+1 xBoundaries (floats)
   /* 
    for(int ix=0; ix<phantom->x_num+1; ix++) {
     printf("%f\t", phantom->x_bound[ix]);
     if(0==ix%8) printf ("\n");
   }
   */
   // Regularize the phantom boundaries
   if(OK != regularizePhantomBoundaries(phantom->x_bound, phantom->x_num+1) ) {
     printf("\n ERROR: Regularizing X phantom boundaries");
   }
   if(OK != regularizePhantomBoundaries(phantom->y_bound, phantom->y_num+1) ) {
     printf("\n ERROR: Regularizing Y phantom boundaries");
   }
   if(OK != regularizePhantomBoundaries(phantom->z_bound, phantom->z_num+1) ) {
     printf("\n ERROR: Regularizing Z phantom boundaries");
   }
#ifdef DEBUG
   // Check the regularization
   printf("\n\n -----Check....results now should be regularized......\n");
   if(OK != regularizePhantomBoundaries(phantom->x_bound, phantom->x_num+1) ) {
     printf("\n ERROR: Regularizing X phantom boundaries");
   }
   if(OK != regularizePhantomBoundaries(phantom->y_bound, phantom->y_num+1) ) {
     printf("\n ERROR: Regularizing Y phantom boundaries");
   }
   if(OK != regularizePhantomBoundaries(phantom->z_bound, phantom->z_num+1) ) {
     printf("\n ERROR: Regularizing Z phantom boundaries");
   }
#endif
   // Write the boundaries to file
   if(OK != writeBinaryDataToFile(outputStream, phantom->x_num+1, phantom->x_bound, swab_flag) )
   {
     printf("\n ERROR: Writing to file %s", densityFileName); return(FAIL);
   }
   // Y
   //  3. ny+1 yBoundaries (floats)
   if(OK != writeBinaryDataToFile(outputStream, phantom->y_num+1, phantom->y_bound, swab_flag) )
   {
     printf("\n ERROR: Writing to file %s", densityFileName); return(FAIL);
   } 
   //  4. nz+1 zBoundaries (floats)
   if(OK != writeBinaryDataToFile(outputStream, phantom->z_num+1, phantom->z_bound, swab_flag) )
   {
     printf("\n ERROR: Writing to file %s", densityFileName); return(FAIL);
   }
   // printf("\n Z_old");
#endif
#ifdef DUMP_BOUNDS
   printf ("\n x\n");
   for(int i=0; i< phantom->x_num+1; i++) {
     if (i%10 == 0) printf ("\n");
     printf("\t %f", phantom->x_bound[i]);
   }
   printf ("\n y\n");
   for(int i=0; i< phantom->y_num+1; i++) {
     if (i%10 == 0) printf ("\n");
     printf("\t %f", phantom->y_bound[i]);
   }
   printf ("\n z\n");
   for(int i=0; i< phantom->z_num+1; i++) {
     if (i%10 == 0) printf ("\n");
     printf("\t %f", phantom->z_bound[i]);
   }
#endif
   //  5. nx*ny*nz density values (floats)
   int nVoxelArray = phantom->x_num*phantom->y_num*phantom->z_num;
   if(OK != writeBinaryDataToFile(outputStream, nVoxelArray, phantom->densval, swab_flag) )
   {
     printf("\n ERROR: Writing to file %s", densityFileName); return(FAIL);
   }
   // close the file pointer
   fclose(outputStream);
   return(OK);
}
/* ***************************************************************************** */
/* ******************************************************* */
int getPhantomMaterials(PHANT_STRUCT *sourcePhantom) {
  // read in the media.config file....
  EGSMaterialType *EGSMaterial;
  {
     if(OK != readMediaConfigFile(&sourcePhantom->num_mat, &EGSMaterial, NULL, NULL) ) { 
       printf("\n ERROR: Reading media config file"); return(FAIL);
     }
  }
#ifdef DEBUG
  printf("\n num_mat = %f", sourcePhantom->num_mat);
  reportMediaConfig(sourcePhantom->num_mat, EGSMaterial);
#endif
  printf("\n %s: Assigning the media name for each material.... ",__FUNCTION__);
  for(int iMedia=0; iMedia<sourcePhantom->num_mat; iMedia++) {
    if(strlen(EGSMaterial[iMedia].mediumName) >= MAX_STR_LEN) {
      strncpy(sourcePhantom->med_name[iMedia],EGSMaterial[iMedia].mediumName, MAX_STR_LEN-3);
    } else {
      strcpy(sourcePhantom->med_name[iMedia],EGSMaterial[iMedia].mediumName);
    }
  }
  printf("\n\tAssigning the media for each voxel.... ");
  int nArray=sourcePhantom->x_num*sourcePhantom->y_num*sourcePhantom->z_num;
// Allocate memory
  if(NULL != sourcePhantom->mednum) free(sourcePhantom->mednum);
  sourcePhantom->mednum = (int *) calloc( nArray,sizeof(int));
  if(NULL==sourcePhantom->mednum) {
    printf("\n ERROR: allocating memory for mednum\n"); return(FAIL);
  }
  for(int iArray=0; iArray < nArray; iArray++) {
    float tmpDensity = sourcePhantom->densval[iArray];
    if(tmpDensity > EGSMaterial[sourcePhantom->num_mat-1].maxDensity) {
      sourcePhantom->mednum[iArray] = sourcePhantom->num_mat-1;
    } else {
      int tmpMediaNumber;
      if(OK != getMediaNumber(sourcePhantom->num_mat,  EGSMaterial, tmpDensity, &tmpMediaNumber)) {
	printf("\nERROR: getMediaNumber for sourcePhantom->densval[%d] = %f", iArray, sourcePhantom->densval[iArray]); return(FAIL);
      }
      sourcePhantom->mednum[iArray]=tmpMediaNumber;
    }
    sourcePhantom->mednum[iArray]+=1; // egsnrc uses fortran indexing for numbers, air = material 1
  }
  free(EGSMaterial);
  return(OK);
}
/* *************************************** */
bool regularizeDensity(PHANT_STRUCT *sourcePhantom, float threshold, float newDensity) {
  // Takes all densities < threshold and sets them equal to the new density
  // Useful for regularizing air in a CT scan
  // string functionName="regularizeDensity";
  int nChanged=0;
  float sumChanged=0.0;
  int nArray=sourcePhantom->x_num*sourcePhantom->y_num*sourcePhantom->z_num;
  for(int iArray=0; iArray < nArray; iArray++) {
    if(sourcePhantom->densval[iArray]<threshold) {
      nChanged++;
      sumChanged+=sourcePhantom->densval[iArray];
      sourcePhantom->densval[iArray]=newDensity;
    }
  }
  printf("\n %s changed %d voxels to density %f",__FUNCTION__, nChanged, newDensity);
  if(nChanged) printf("\n\t averageDensity of changed voxels = %f", sumChanged/(float)nChanged);
  return(true);
}
/* ******************************************************* */
int assignEstep(PHANT_STRUCT *phantom, const float eStep){
  printf("\n Assigning estep.... ");
  for(int iMedia=0; iMedia < phantom->num_mat; iMedia++)
    phantom->estep[iMedia]=eStep;
  return(OK);
}
/* **************************************** */
/* ******************************************************* */
int dumpBounds(PHANT_STRUCT *phantom)
{
   printf ("\n x\n");
   for(int i=0; i< phantom->x_num+1; i++) {
     if (i%10 == 0) printf ("\n");
     printf("\t %f", phantom->x_bound[i]);
   }
   printf ("\n y\n");
   for(int i=0; i< phantom->y_num+1; i++) {
     if (i%10 == 0) printf ("\n");
     printf("\t %f", phantom->y_bound[i]);
   }
   printf ("\n z\n");
   for(int i=0; i< phantom->z_num+1; i++) {
     if (i%10 == 0) printf ("\n");
     printf("\t %f", phantom->z_bound[i]);
   }
   return(OK);
}
/* ************************************************ */
/* ********************************************************************** */
int readVMCPhantomFile(PHANT_STRUCT *phantom, char* fileName)
{
   // Check Byte order of current machine, determine if need to swab.....
   int swab_flag = 0;
   switch( (check_byte_order()) )
   {
      case BIG_ENDIAN:
	swab_flag=1;
         break;
      case LITTLE_ENDIAN:
	 break;
      default:
         printf("\n ERROR: Indeterminate Byte Order\n");
         return(FAIL);
   }
   // Open file for writing
   FILE *iStream= fopen(fileName,"rb");
   if (NULL == iStream) {
     printf("\n ERROR: Cannot open file %s for reading\n",fileName); return(FAIL); 
   }
   // Read the header portion which includes
   //  1. nx, ny, nz  (ints)
   //  2. nx+1 xBoundaries (floats)
   //  3. ny+1 yBoundaries (floats)
   //  4. nz+1 zBoundaries (floats)
   //  5. nx*ny*nz density values (floats)

   // Step-by-step process......will use predefined functions that will do the swabbing if necessary
   //
   //  1. nx, ny, nz  (ints)   
   {
      int nVoxels[3];
      int nRead = (int) (fread(nVoxels, sizeof(int), 3,iStream));
      if(3 != nRead) {
	printf("\n ERROR: Reading number of voxels"); return(FAIL);
      }
      if(swab_flag) {
        phantom->x_num = reverse_int_byte_order(nVoxels[0]);
        phantom->y_num = reverse_int_byte_order(nVoxels[1]);
        phantom->z_num = reverse_int_byte_order(nVoxels[2]);
      } else {
        phantom->x_num = nVoxels[0];
        phantom->y_num = nVoxels[1];
        phantom->z_num = nVoxels[2];
      }
      printf ("\n Read nVoxels %d %d %d", nVoxels[0],nVoxels[1],nVoxels[2]);
   }
   // Read in the voxel boundaries
 
   // X
   //  2. nx+1 xBoundaries (floats)
   // Write the boundaries to file
   if(OK != readBinaryDataFromFile(iStream, phantom->x_num+1, phantom->x_bound, swab_flag) )
   {
     printf("\n ERROR: Reading from file %s", fileName); return(FAIL);
   }
   // Y
   //  3. ny+1 yBoundaries (floats)
   if(OK != readBinaryDataFromFile(iStream, phantom->y_num+1, phantom->y_bound, swab_flag) )
   {
     printf("\n ERROR: Reading from file %s", fileName); return(FAIL);
   }
   //  4. nz+1 zBoundaries (floats)
   if(OK != readBinaryDataFromFile(iStream, phantom->z_num+1, phantom->z_bound, swab_flag) )
   {
     printf("\n ERROR: Reading from file %s", fileName); return(FAIL);
   }
   //  5. nx*ny*nz density values (floats)
   int nVoxelArray = phantom->x_num*phantom->y_num*phantom->z_num;
   if(OK != readBinaryDataFromFile(iStream, nVoxelArray, &phantom->densval, swab_flag) )
   {
     printf("\n ERROR: Reading Data From file %s", fileName); return(FAIL);
   }
   // close the file pointer
   fclose(iStream);
   return(OK);
}
/* **************************************************** */
int writeEgsPhantDensitiesAsBinary(char *fileName, PHANT_STRUCT *p)
{
  printf("\nwriteEgsPhantDensitiesAsBinary ");
  if(NULL  == p->densval) {
    printf("\n ERROR: writeEgsPhantDensitiesAsBinary: densval == NULL\n"); return(FAIL);
  }
  FILE *fp;
  fp = fopen(fileName,"wb");
  if(fp == NULL){
     printf("\n ERROR: opening file %s",fileName);
     return(FAIL);
  }
  unsigned int nArray=p->x_num*p->y_num*p->z_num;
  unsigned int nWrite;
  nWrite = (unsigned int) fwrite(p->densval,sizeof(float),nArray,fp);
  fclose(fp);
  if(nWrite != nArray) {
    printf ("\n ERROR: nWrite= %d != %d =nArray ", nWrite, nArray); return(FAIL);
  }
  return(OK);
}
/* ************************************************************* */

int qaAndRepairPhantomBoundaries(PHANT_STRUCT *phantom) /// A simple QA function
{
  /// This routine attempts to remove the round-off error that exists in the specification
  /// of the boundary locations for an egsphant file.
  /// It also checks that the phantom is equispaced, and should only be run for equi-spaced phantoms
  /// After further debugging, I don't think the repair part of this routine is actually needed
  /// Values in the .egsphant file are give with 4 digits after the decimal place
  /// Round the starting coordinate to within tolerance

  float fractionalTolerance=0.0015f;
  {  
    float tolerance=fractionalTolerance*phantom->x_size;  // Set small tolerance (0.1%)
    if(phantom->x_bound[0] != phantom->x_start) {
      printf("\n ERROR: first bound in X is not at x_start (%f != %f)", phantom->x_bound[0], phantom->x_start);
      return(FAIL);
    }	 	
    long tempVal=(long) (phantom->x_start*10000);	
    phantom->x_start = phantom->x_bound[0] = (float) tempVal / 10000;
    
    for(int iBound=1; iBound<phantom->x_num; iBound++)
      {
	// check that the distance is within tolerance of the voxel size
	if( fabs(phantom->x_bound[iBound]-phantom->x_bound[iBound-1]-phantom->x_size) > tolerance ) {
	  printf("\n ERROR: phantom boundaries not equispaced in x-direction "); 
	  printf("\n\t %f-%f-%f = %f", phantom->x_bound[iBound],phantom->x_bound[iBound-1],phantom->x_size, 
		 (phantom->x_bound[iBound]-phantom->x_bound[iBound-1]-phantom->x_size)); return(FAIL);
	}
	// do direct assignment to prevent round off errors later
	phantom->x_bound[iBound] = phantom->x_start + ((float) (iBound))*phantom->x_size;
      }
  }
  // do same for y 
  {
    float tolerance=fractionalTolerance*phantom->y_size;  // Set small tolerance (0.1%)
     if(phantom->y_bound[0] != phantom->y_start) {
       printf("\n ERROR: first bound in Y is not at y_start (%f != %f)", phantom->y_bound[0], phantom->y_start);
       return(FAIL);
     }
   	
     long tempVal=(long) (phantom->y_start*10000);	
     phantom->y_start = phantom->y_bound[0] = (float) tempVal / 10000;

     for(int iBound=1; iBound<phantom->y_num; iBound++)
     {
       // check that the distance is within tolerance of the voxel size
       if( fabs(phantom->y_bound[iBound]-phantom->y_bound[iBound-1]-phantom->y_size) > tolerance ) {
	 printf("\n ERROR: phantom boundaries not equispaced in y-direction "); 
	 printf("\n\t %f-%f-%f = %f", phantom->y_bound[iBound],phantom->y_bound[iBound-1],phantom->y_size, 
		(phantom->y_bound[iBound]-phantom->y_bound[iBound-1]-phantom->y_size)); return(FAIL);
       }
       // do direct assignment to prevent round off errors later
       phantom->y_bound[iBound] = phantom->y_start + ((float) (iBound))*phantom->y_size;
     }
  }

  // do same for z
  {
    float tolerance=fractionalTolerance*phantom->z_size;  // Set small tolerance (0.1%)
     if(phantom->z_bound[0] != phantom->z_start) {
       printf("\n ERROR: first bound in Z is not at z_start (%f != %f)", phantom->z_bound[0], phantom->z_start);
       return(FAIL);
     }
   	
     long tempVal=(long) (phantom->z_start*10000);	
     phantom->z_start = phantom->z_bound[0] = (float) tempVal / 10000;

     for(int iBound=1; iBound<phantom->z_num; iBound++)
     {
       // check that the distance is within tolerance of the voxel size
       if( fabs(phantom->z_bound[iBound]-phantom->z_bound[iBound-1]-phantom->z_size) > tolerance ) {
	 printf("\n ERROR: phantom boundaries not equispaced in z-direction "); 
	 printf("\n\t %f-%f-%f = %f", phantom->z_bound[iBound],phantom->z_bound[iBound-1],phantom->z_size, 
		(phantom->z_bound[iBound]-phantom->z_bound[iBound-1]-phantom->z_size)); return(FAIL);
       }
       // do direct assignment to prevent round off errors later
       phantom->z_bound[iBound] = phantom->z_start + ((float) (iBound))*phantom->z_size;
     }
  }
   
  return(OK);
}
/* ******************************************************* */
/* in egsPhantRountines.h
int reportOnPhantomDensity(PHANT_STRUCT *phantom) /// A simple QA function
{
  int nVoxelArray = phantom->x_num*phantom->y_num*phantom->z_num;
  float maxDensity = phantom->densval[0];
  double sumDensity=0.0;
  for(int iArray=0; iArray<nVoxelArray; iArray++)
  {
    maxDensity = max(maxDensity,phantom->densval[iArray]);
    sumDensity+=phantom->densval[iArray];
  }
  // printf("\n x_start = %f", phantom->x_start);
  // printf("\n x_num = %d", phantom->x_num);
  // printf("\n x_size = %f", phantom->x_size);
  printf("\n Maximum density in phantom is %f", maxDensity);
  printf("\n Sum of density in phantom is %f", sumDensity);
  printf("\n Average density in phantom is %f", sumDensity/nVoxelArray);// why do we need the average density?
  fflush(NULL);  // flush all of the streams
  return(OK);
}
*/
/* ************************************************************* */
int copyPhantomZeroDensity(PHANT_STRUCT *source, PHANT_STRUCT *destination)
{
  // \todo : Finish this routine (for everything in PHANT_STRUCT
  destination->x_num = source->x_num;
  destination->y_num = source->y_num;
  destination->z_num = source->z_num;
  destination->num_mat = source->num_mat;
  // Note, there are x_num+1 boundaries in the file.....
  // x_num is the number of voxels, and the boundaries are the voxel edges
  // x_bound[0] = left most boundary
  // x_bound[1] = right boundary of voxel 0 = left boundary of voxel 1
  // x_bound[x_num-1] = left boundary of last voxel
  // x_bound[x_num] = right boundary of the last voxel.
  for(int iBound=0;iBound<source->x_num+1; iBound++){
    destination->x_bound[iBound] = source->x_bound[iBound];}
  // Or...memcpy(destination->x_bound,source->x_bound,MAX_N_BOUNDS*sizeof(float));
  for(int jBound=0;jBound<source->y_num+1; jBound++){
    destination->y_bound[jBound] = source->y_bound[jBound];}
  for(int kBound=0;kBound<source->z_num+1; kBound++){
    destination->z_bound[kBound] = source->z_bound[kBound];}
  
  destination->x_size = source->x_size;
  destination->y_size = source->y_size;
  destination->z_size = source->z_size;
  destination->x_start = source->x_start;
  destination->y_start = source->y_start;
  destination->z_start = source->z_start;
  // must loop over media
  for(int iMedia=0; iMedia<source->num_mat; iMedia++){
     destination->estep[iMedia] = source->estep[iMedia]; 
     strcpy(destination->med_name[iMedia], source->med_name[iMedia]);  
  }
 
 for(int jMedia=0; jMedia<source->num_mat; jMedia++){
     destination->estep[jMedia] = source->estep[jMedia]; 
     strcpy(destination->med_name[jMedia], source->med_name[jMedia]);  
  }
  for(int kMedia=0; kMedia<MAX_MED; kMedia++){
     destination->estep[kMedia] = source->estep[kMedia]; 
     strcpy(destination->med_name[kMedia], source->med_name[kMedia]);  
  }
  // ...and do this for all other parameters
   
  // allocate the memory for the density matrix
  int nTotalVoxels=destination->x_num*destination->y_num*destination->z_num;
  destination->densval = (float *) calloc(nTotalVoxels,sizeof(float));
  if(NULL == destination->densval)
  {
    printf("\n ERROR: copyPhantomZeroDensity reports unable to allocate densval\n"); return(FAIL);
  }
  // allocate the memory for the media numbers
  destination->mednum = (int *) calloc(nTotalVoxels,sizeof(int));
  if(NULL == destination->mednum)
  {
    printf("\n ERROR: copyPhantomZeroDensity reports unable to allocate mednum\n"); return(FAIL);
  }
  return(OK);
}
/* **************************************** */
