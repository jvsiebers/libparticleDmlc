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
/* Header File for General Utilities for CPP programs
       June 30, 2008: JVS: Broke off from utilities.h
       Feb 3, 2010: JVS: Add qaDoseValues
       Mar 1, 2010: JVS: Add readUnknownEndianBinaryDataFromFile
       May 11, 2010: JVS: Add readBinaryDataFromFile(char * ...)
      Nov 17, 2015: JVS: Add getBinaryFileSize and readUnknownEndianBinaryDataFromFile(char *iFileName,**data)
      April 13, 2020: JVS: Add reportDoseFileStatistics
*/
#ifndef readWriteEndianFile_H_INCLUDED
#define readWriteEndianFile_H_INCLUDED
#include "endian.h"
#include "jvsDefines.h"
/* *************************************************************************** */
float reverse_float_byte_order(float xold);
short reverse_short_byte_order(short xold);
int   reverse_int_byte_order(int xold);
int assignDoseByteOrder(const int nVoxels, float *dose);
long get_nFloatsBinaryFileSize(const char *inputFileName);
int myIsNAN(float test);
int writeBigEndianBinaryFile(const char *doseFileName,  int nDoseArray, float *doseArray);
int writeLittleEndianBinaryFile(const char *doseFileName,  int nDoseArray, float *doseArray);
int writeBinaryFile(const char *doseFileName, int nDoseArray, float *doseArray, int swab_flag);
int writeBinaryDataToFile(FILE *outputStream, int nArray, float *array, int swab_flag);
int readBinaryDataFromFile(FILE *iStream, int nItemsToRead, float **arrayToRead, int swab_flag);
int readBinaryDataFromFile(FILE *iStream, int nItemsToRead, float *inputArray, int swab_flag);
int readBinaryDataFromFile(const char *iFileName, int nItemsToRead, float **arrayToRead, int swab_flag);
int readBigEndianBinaryDataFromFile(const char *iFileName, int nVoxels, float **dose);
int readLittleEndianBinaryDataFromFile(const char *iFileName, int nVoxels, float **dose);
int readUnknownEndianBinaryDataFromFile(const char *iFileName, int nVoxels, float **dose);
long readUnknownEndianBinaryDataFromFile(const char *inputFileName, float **data);
int qaDoseValues(float *doseValue, unsigned int nVoxels);
bool reportDoseFileStatistics(int nVoxels, float *image1);

#endif
