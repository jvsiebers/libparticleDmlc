/** \file 
 simple tag auto added to force generation of simple doxygen comments
**/
/*
   Copyright 2000-2008 Virginia Commonwealth University


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
/*  Routines to read/write Endian-specific files to/from disk
      Modification History:
      June 30, 2008: JVS: Created by cutting off from utilities.cc
      Feb 27, 2009: JVS: Add readLittle
      Feb 3, 2009: JVS: Add qaDoseValues
      Mar 1, 2010: JVS: Add readUnknownEndianBinaryDataFromFile
      May 11, 2010: JVS: Add checkBinaryFileSize
      June 17, 2010: JVS: Check for isnan also when assigning byte order...
      aug 18, 2010: JVS: replaced isnan with myIsNAN since isnan is not uniformly defined in all C++ distributions
      Feb 9, 2012: JVS: imporve error message on output for assignByteOrder
      Feb 16, 2012: JVS: add error checking for NULL pointers before do writes...
      Oct 22, 2015: JVS: update max/min to std::max and std::min from algorithm
                         Definitions from utilities.h were causing compilation problems 
      Nov 17, 2015: JVS: Add getBinaryFileSize and readUnknownEndianBinaryDataFromFile(char *iFileName,**data)
*/
// #include <cmath> // for isnan
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
//#include <utilities.h> // for max min
#include <sys/types.h> // for stat
#include <sys/stat.h>
// #include "jvsDefines.h"
// #include "endian.h"
#include "readWriteEndianFile.h"
#ifdef DEBUG
#include "utilities.h" // for max, min
#endif
/*            */
int myIsNAN(float f) {
  return(f != f);  // nan is the only number that does not equal itself
}
/* ************************************************************************** */
long readUnknownEndianBinaryDataFromFile(const char *inputFileName, float **data) {
   long nVoxels = get_nFloatsBinaryFileSize(inputFileName);
   if(OK != readUnknownEndianBinaryDataFromFile(inputFileName, (int) nVoxels, data) ) {
     printf("\n ERROR: readUnknownEndianBinaryDataFromFile(%s, **data ", inputFileName);
     return(-1);
   }
   return(nVoxels);
}
/* ************************************************************************** */
long get_nFloatsBinaryFileSize(const char *inputFileName) {
  struct stat file0Stat;
  if( stat( inputFileName, &file0Stat) != 0 ){
    printf("\n ERROR: checkBinaryFileSize: Stat'ing input file %s\n",inputFileName);
    perror("STAT:"); return(FAIL);
  }
  /* Determine Number of Voxels */
  return( (long) (file0Stat.st_size/sizeof(float)));
}
/* ************************************************************************** */
int checkBinaryFileSize(const char *inputFileName, const int nExpected) {  
   // Check the file size of the input file
  long nVoxels = get_nFloatsBinaryFileSize(inputFileName);
#ifdef DEBUG
  printf("\n checkBinaryFileSize: Expect %d voxels in the file, found %ld", nExpected,nVoxels); 
#endif
  if(nVoxels != (unsigned int) nExpected) {
    printf("\n ERROR: checkBinaryFileSize: (nVoxels in file = %ld) != (nExpected = %u)",nVoxels, nExpected); 
    printf("\n\tcheckBinaryFileSize: file = %s", inputFileName);
    return(FAIL);
  }
  return(OK);
}
/* ************************************************************************** */
int reverse_int_byte_order(int xold)
{
   int xnew;
   char *pn   = (char *) &xnew;
   char *po   = (char *) &xold;
   pn[0] = po[3];
   pn[1] = po[2];
   pn[2] = po[1];
   pn[3] = po[0];
   return(xnew);   
}
/* *************************************************************************** */
float reverse_float_byte_order(float xold)
{
   float xnew;
   char *pn   = (char *) &xnew;
   char *po   = (char *) &xold;
   pn[0] = po[3];
   pn[1] = po[2];
   pn[2] = po[1];
   pn[3] = po[0];
   return(xnew);
}
/* **************************************************************************** */
short reverse_short_byte_order(short xold)
{
   short xnew;
   char *pn   = (char *) &xnew;
   char *po   = (char *) &xold;
   pn[0] = po[1];
   pn[1] = po[0];
   return(xnew);
}
/* **************************************************************************** */
int check_byte_order()
{
  /* Determine the byte order on this machine */
  float ftest=1.0; /* assign a float to 1.0 */
  char *pf = (char *) &ftest;
  // printf("\n \t %x %x %x %x", pf[0],pf[1],pf[2],pf[3]);
  if(pf[0] == 0 && pf[3] != 0) 
  { 
    //printf("\n INTEL / ALPHA,LINUX \n");
    return(LITTLE_ENDIAN);
  }else if(pf[0] != 0 && pf[3] == 0) 
  { 
    //printf("\n OTHER (SGI,SUN-SOLARIS)\n ");
    return(BIG_ENDIAN);
  } 
  else
  {
        printf("\n ERROR: indeterminate byte order");
        printf("\n \t %x %x %x %x", pf[0],pf[1],pf[2],pf[3]);
        return(UNKNOWN_ENDIAN);
  }
}
/* ************************************************************************************ */
int readBinaryDataFromFile(const char *iFileName, int nItemsToRead, float **arrayToRead, int swab_flag)
{
  if(OK != checkBinaryFileSize(iFileName,nItemsToRead) ) {
    printf("\n ERROR: readBinaryDataFromFile: checkBinaryFileSize failed"); return(FAIL);
  }
  // Open the stream to read from
  // open file and read in the dose 
  FILE *iStream = fopen(iFileName,"r");
  if( iStream == NULL) {
    printf("\n ERROR: readBinaryDataFromFile: Opening Dose File %s",iFileName); return(FAIL);
  }
  return(readBinaryDataFromFile(iStream, nItemsToRead,arrayToRead,swab_flag));
}
/* ************************************************************************************ */
int readBinaryDataFromFile(FILE *iStream, int nItemsToRead, float **arrayToRead, int swab_flag)
{
  // Reads binary data to stream, swab's if requested (1=swab, 0=don't swab)
  // Swab if needed...Put swabbed results in different array so no need to "unswab" when done
  // Allocate memory to read array into
#ifdef DEBUG
  printf("\n readBinaryDataFromFile: Allocating memory before reading ");
#endif
   float *inputArray;
   inputArray = (float *) calloc(nItemsToRead,sizeof(float));
   if(inputArray == NULL) {
      printf("\n ERROR: Allocating memory for inputArray in readBinaryDataFromFile");
      return(FAIL);
   }
   if(OK != readBinaryDataFromFile(iStream, nItemsToRead, inputArray, swab_flag)) {
     printf("\n ERROR: Reading binary data from file"); return(FAIL);
   }
   *arrayToRead = inputArray;
   return(OK);
} 
/* ************************************************************************************ */
int readBinaryDataFromFile(FILE *iStream, int nItemsToRead, float *inputArray, int swab_flag)
{
  // Reads binary data to stream, swab's if requested (1=swab, 0=don't swab)
  // Swab if needed...Put swabbed results in different array so no need to "unswab" when done
  // Allocate memory to read array into
  // Read in the array....
#ifdef DEBUG
   printf("\n readBinaryDataFromFile: Reading %d items of size = %d with swab_flag = %d", nItemsToRead,sizeof(float), swab_flag);
#endif
  int nRead= (int) fread(inputArray,sizeof(float),nItemsToRead, iStream); 
  if(nRead != nItemsToRead) {
    printf("\n ERROR: readBinaryDataFromFile: Number read from file (%d) != nExpected (%d)\n",
	   nRead, nItemsToRead); return(FAIL); 
  }
  // Check if need to swab the data
  if(swab_flag) { // swab if swab_flag != 0
    printf("\n Swabbing bytes as requested ");
    for(int index=0; index<nItemsToRead;index++) {
      inputArray[index] = reverse_float_byte_order( inputArray[index] );
    }
  } 
#ifdef DEBUG
  {  
    float minDose,maxDose;
    maxDose=inputArray[0];
    minDose=maxDose;
    for(int iVoxel=0; iVoxel<nItemsToRead; iVoxel++) {
      if(!myIsNAN(inputArray[iVoxel])) {
        maxDose=std::max(maxDose,inputArray[iVoxel]);
        minDose=std::min(minDose,inputArray[iVoxel]);
      }
    }
    printf("\n minDose = %g, maxDose = %g", minDose, maxDose);
  }
#endif
   return(OK);
} 
/* ***************************************************************************************** */
int assignDoseByteOrder(const int nVoxels, float *dose) 
{
  // Check file to max, min dose 
  float minDose,maxDose;
  float initialMaxDose, initialMinDose;
  float myEpsilon=1.0e-20f;
  maxDose=minDose=dose[0];
  bool validDose=true;
  for( int iVoxel=0; iVoxel<nVoxels; iVoxel++ ) {
    if( myIsNAN( dose[iVoxel] ) ) validDose=false;
    maxDose=std::max(maxDose,dose[iVoxel]);
    minDose=std::min(minDose,dose[iVoxel]);
  }
  double initialRange = maxDose-minDose;
  if( !validDose || minDose < 0.0f || maxDose > 1.0e10 || maxDose < myEpsilon ) {
    validDose=true;
    initialMaxDose=maxDose;
    initialMinDose=minDose;
    printf("\n\t assignDoseByteOrder: Data has byte order different from machine: swabbing byte order");
    maxDose=minDose=reverse_float_byte_order(dose[0]);
    for(int iVoxel=0; iVoxel<nVoxels; iVoxel++) {
      dose[iVoxel] = reverse_float_byte_order( dose[iVoxel] );
      maxDose=std::max(maxDose,dose[iVoxel]);
      minDose=std::min(minDose,dose[iVoxel]);
    }
    if( !validDose || minDose < 0.0f || maxDose > 1.0e10) {
      double finalRange = maxDose-minDose;
      printf("\n ERROR: assignDoseByteOrder: Neither byte order is reasonable...\n"); 
      printf("\n \tinitialByteOrder: max=%g, min=%g", initialMaxDose, initialMinDose);
      printf("\n \tfinalByteOrder: max=%g, min=%g", maxDose, minDose);
      printf("\n \tinitialRange = %g", initialRange);
      printf("\n \tfinalRange   = %g", finalRange);
      if(minDose < 0.0f) printf("\n \tminDose < 0 ");
      if(maxDose > 1.0e10) printf("\n \tmaxDose > 1.0e10 ");
      return(FAIL);
    }
  } else {
    printf("\n\t assignDoseByteOrder: Data has machine native byte-order");
  }
  printf("\n\t assignDoseByteOrder : minDose = %g, maxDose = %g", minDose, maxDose);
  return(OK);
}
/* ***************************************************************************************** */
int readUnknownEndianBinaryDataFromFile(const char *iFileName, int nVoxels, float **dose)
{
  if(OK != checkBinaryFileSize(iFileName,nVoxels) ) {
    printf("\n ERROR: readUnknownEndianBinaryDataFromFile: checkBinaryFileSize failed"); return(FAIL);
  }
  // Open the stream to read from
  // open file and read in the dose 
  FILE *iStream = fopen(iFileName,"r");
  if( iStream == NULL) {
    printf("\n ERROR: Opening Dose File %s",iFileName); return(FAIL);
  }
  int swab_flag=0;
  if(OK != readBinaryDataFromFile(iStream, nVoxels, dose, swab_flag) ){ // read without swabbing
    printf("\n ERROR: readUnknownEndianBinaryDataFromFile :readingBinaryDataFromFile\n"); return(FAIL);
  }
  fclose(iStream);
  if(OK != assignDoseByteOrder(nVoxels, *dose) ) {
    printf("\n ERROR: readUnknownEndianBinaryDataFromFile: assigning byte order "); return(FAIL);
  }
  return(OK);
} 
/* ********************************************************************** */
int readBigEndianBinaryDataFromFile(const char *iFileName, int nVoxels, float **dose)
{
  if(OK != checkBinaryFileSize(iFileName,nVoxels) ) {
    printf("\n ERROR: readBigEndianBinaryDataFromFile: checkBinaryFileSize failed"); return(FAIL);
  }
  // Check to see if should turn on the swab flag
   int swab_flag = 0;
   switch( (check_byte_order()) )
   {
      case BIG_ENDIAN:
         break;
      case LITTLE_ENDIAN:
         swab_flag=1;
	 break;
      default:
         printf("\n ERROR: Indeterminate Byte Order\n");
         return(FAIL);
   }
   // Open the stream to read from
   /* open file and read in the dose */
   FILE *iStream = fopen(iFileName,"r");
   if( iStream == NULL) {
      printf("\n ERROR: Opening Dose File %s",iFileName); return(FAIL);
   }
   if(OK != readBinaryDataFromFile(iStream, nVoxels, dose, swab_flag) ){
     printf("\n ERROR: readBigEndianBinaryDataFromFile :readingBinaryDataFromFile\n"); return(FAIL);
   }
   fclose(iStream);
   return(OK);
} 
/* ********************************************************************** */
int readLittleEndianBinaryDataFromFile(const char *iFileName, int nVoxels, float **dose)
{
  if(OK != checkBinaryFileSize(iFileName,nVoxels) ) {
    printf("\n ERROR: readLittleEndianBinaryDataFromFile: checkBinaryFileSize failed"); return(FAIL);
  }
  // Check to see if should turn on the swab flag
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
   // Open the stream to read from
   /* open file and read in the dose */
   FILE *iStream = fopen(iFileName,"r");
   if( iStream == NULL) {
      printf("\n ERROR: Opening Dose File %s",iFileName); return(FAIL);
   }
   if(OK != readBinaryDataFromFile(iStream, nVoxels, dose, swab_flag) ){
     printf("\n ERROR:readLittleEndianBinaryDataFromFile: readingBinaryDataFromFile\n"); return(FAIL);
   }
   fclose(iStream);
   return(OK);
} 
/* ************************************************************************************ */
int writeBinaryFile(const char *binaryFileName, int nItemsToWrite, float *arrayToWrite, int swab_flag)
{
   FILE *outputStream= fopen(binaryFileName,"wb");
   if (outputStream == NULL) {
     printf("\n ERROR: writeBinaryFile: Cannot open file %s for writing\n",binaryFileName); return(FAIL); 
   }
   if(OK != writeBinaryDataToFile(outputStream, nItemsToWrite, arrayToWrite, swab_flag) )
   {
     printf("\n ERROR: writeBinaryFile: Writing Binary File"); return(FAIL);
   }
   fclose(outputStream);
   return(OK);
}
/* ************************************************************************************ */
int writeBinaryDataToFile(FILE *outputStream, int nItemsToWrite, float *arrayToWrite, int swab_flag)
{
  // Writes binary data to stream, swab's if requested (1=swab, 0=don't swab)
  // Swab if needed...Put swabbed results in different array so no need to "unswab" when done
  // First, check that inputs are reasonable
  if(NULL == arrayToWrite) {
    printf( "\n ERROR: writeBinaryDataToFile: received NULL pointer for arrayToWrite"); 
    printf("\n \t\t nItemsToWrite=%d\tswab_flag=%d ",nItemsToWrite,swab_flag);return(FAIL);
  }
  if(NULL == outputStream) {
    printf("\n ERROR: writeBinaryDataToFile: received NULL pointer for outputStream"); return(FAIL);
  }
  if(nItemsToWrite < 0) {
    printf("\n ERROR: writeBinaryDataToFile: nItemsToWrite=%d < 0 ",nItemsToWrite); return(FAIL);
  }
  // Check if need to swab data
  float *swabbedArray;
  if(swab_flag){ // swab if swab_flag != 0
    // 
    swabbedArray = (float *) calloc(nItemsToWrite,sizeof(float));
    if(swabbedArray == NULL) {
      printf("\n ERROR: writeBinaryDataToFile: Allocating memory for swabbedArray in writeBinaryFile"); return(FAIL);
    }
    for(int index=0; index<nItemsToWrite;index++){
      swabbedArray[index] = reverse_float_byte_order( arrayToWrite[index] );
    }
  } else {
    swabbedArray = arrayToWrite;
  }
  // Write the dose distribution
  int nWrite=(int) fwrite(swabbedArray,sizeof(float),nItemsToWrite, outputStream); 
  if(nWrite != nItemsToWrite) {
    printf("\n ERROR: writeBinaryDataToFile: Wrong number written to file (%d %d)\n", nWrite, nItemsToWrite); return(FAIL); 
  }
  // free swabbedArray if it was allocated here
  if(swab_flag) {
    free(swabbedArray);
  }
  return(OK);
} 
/* ***************************************************************************************************** */
int writeBigEndianBinaryFile(const char *binaryFileName,  int nItemsToWrite, float *arrayToWrite)
{
   // Pinnacle doses always written in BIG_ENDIAN format... Check if need to swab.....
   int swab_flag = 0;
   switch( (check_byte_order()) )
   {
      case BIG_ENDIAN:
         break;
      case LITTLE_ENDIAN:
         swab_flag=1;
	 break;
      default:
         printf("\n ERROR: Indeterminate Byte Order\n");
         return(FAIL);
   }
   if(OK != writeBinaryFile(binaryFileName, nItemsToWrite, arrayToWrite, swab_flag) )
   {
     printf("\n ERROR: writeBigEndianBinaryFile: Writing dose file %s", binaryFileName); return(FAIL);
   }
   return(OK);
} 
/* ***************************************************************************************************** */
int writeLittleEndianBinaryFile(const char *binaryFileName,  int nItemsToWrite, float *arrayToWrite)
{
   // Pinnacle doses always written in BIG_ENDIAN format... Check if need to swab.....
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
   if(OK != writeBinaryFile(binaryFileName, nItemsToWrite, arrayToWrite, swab_flag) )
   {
     printf("\n ERROR: writeLittleEndianBinaryFile: Writing dose file %s", binaryFileName); return(FAIL);
   }
   return(OK);
}
/* ***************************************************************************************************** */
int qaDoseValues(float *doseValue, unsigned int nVoxels) {
  float maxV = doseValue[0];
  float minV = doseValue[0];
  for(unsigned i=0; i<nVoxels; i++){
    maxV = std::max(maxV, doseValue[i]);
    minV = std::min(minV, doseValue[i]);
  }
  float minPermissableDose=0.0;
  float maxPermissableDose=1.0e8;
  printf("\n\t qaDoseValues: minV = %g, maxV = %g", minV, maxV);
  if(minV < minPermissableDose) {
    printf ("\n ERROR: Dose value less than zero detected -- possible byte swapping problem \n"); return(FAIL);
  }
  if(maxV > maxPermissableDose) {
    printf ("\n ERROR: Dose value greater then 1e6 detected -- possible byte swapping problem \n"); return(FAIL);
  }
  printf("\n\t qaDoseValues: doses in range of %g to %g",minPermissableDose,maxPermissableDose);
  return(OK);
}
/* *************************************************************************************************** */
#include <math.h>
/* *********************************************** */
bool reportDoseFileStatistics(int nVoxels, float *image1) {
    printf("\n Base statistics report");
    float maxV = image1[0];
    float minVNonZero = HUGE_VAL;
    double sumPixels = 0.0;
    float minV = image1[0];
    int iMin, iMax;
    iMin = iMax = 0;
    unsigned int nNonZeroVoxels = 0; // Number of non-zero data voxels required for some outputs....(e.g. report)

  for(int i = 0; i < nVoxels; i++) {
        if (image1[i]) nNonZeroVoxels++;
        if (image1[i] > maxV) {
            iMax = i;
            maxV = image1[i];
        }
        //maxV = max(maxV, image1[i]);
        if (image1[i] < minV) {
            iMin = i;
            minV = image1[i];
        }
        //minV = min(minV, image1[i]);
        if (image1[i]) minVNonZero = std::min(minVNonZero, image1[i]);
        sumPixels += image1[i];
    }
    printf("\n\t Maximum Value = %g (index %d)", maxV, iMax);
    printf("\n\t Minimum Value = %g (index %d)", minV, iMin);
    printf("\n\t Minimum Value Non Zero Pixels = %g", minVNonZero);
    printf("\n\t sumPixels = %g", sumPixels);
    if (myIsNAN(maxV) || myIsNAN(minV) || myIsNAN((float) sumPixels)) {
        printf("\n ERROR: NAN detected -- likely an input byte order problem\n"); return (FAIL);
    }
    double averageV = sumPixels / nVoxels;
    double averageNZV = sumPixels / nNonZeroVoxels;
    // Compute standard deviation 

    float sumSquared = 0.0;
    float sumNZSquared = 0.0;
    for(int i = 0; i < nVoxels; i++) {
        sumSquared += (float) pow( image1[i] - averageV, (int) 2);
        if (image1[i]) sumNZSquared += (float) pow( image1[i] - averageNZV, (int) 2);
    }
    float stdDev, stdDevNZ;
    if (nVoxels > 1) {
        stdDev = (float) sqrt(sumSquared / ((float)nVoxels - 1.0f));
        printf("\n\t Average Value = %g", averageV);
        printf("\n\t Standard Dev  = %g", stdDev);
    }
    if (nNonZeroVoxels > 1) {
        stdDevNZ = (float) sqrt(sumNZSquared / ((float) nNonZeroVoxels - 1.0f));
        printf("\n\t nonZero Voxels Average Value = %g", averageNZV);
        printf("\n\t nonZero Voxels Standard Dev  = %g", stdDevNZ);
    }
    printf("\n");
    return true;
}
/* ******************************************************* */
