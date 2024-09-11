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
	File Created:
		18-December-1995: Combined ok_check.cpp and some open_file
	Modification History:
		 09-feb-1996: JVS: add pprintf
       07-Nov-1996: JVS: FAIL_SAFE definition changed, NULL=0
       06-Jan-1998: JVS: Add array_read
       07-May-1998: JVS: add myerrno
       11-Sept-1998: JVS: add max, min definitions
      // 28-Oct-1998: PJK: added MAX_NUM_FIELDS for output_path in case_info.h
      03-Dec-1998: JVS: Add #ifndef UTILITIES_H_INCLUDED to ensure single inclusion of the file
                        All comments "c" compliant
       02-Mar-1999: JVS: Add eprintf_mode
       Dec 3, 1999: JVS: Add check_byte_order
       April 24, 2001: JVS: Add cp
       August 24, 2001: JVS: Change OK from 1 to 0...so exit(OK) is unix standard normal...
       Feb 10, 2005: JVS:Add writeLittleEndianBinaryFile()
       April 21, 2005: JVS: Add readBinaryDataFromFile()
       August 13, 2007: JVS: Add fileExists
       Nov 5, 2007: JVS: Remove myerrno from global
       Nov 28, 2007: JVS const char * for g++ 4.2 compatibility
       Jan 23, 2008: JVS: Add readBigEndianFile
       May 20, 2008: JVS: Add free_pbuffer
       June 30, 2008: JVS: Break off jvsDefines.h
       May 11, 2010: JVS: Add includion of printRunTimeInformation.h
       Oct 22, 2015: JVS: Get rid of max, min -- was causing gcc 4.8.2 problems
*/
#ifndef UTILITIES_H_INCLUDED
#define UTILITIES_H_INCLUDED

#include "jvsDefines.h"
#include "printRunTimeInformation.h" 
#include "compilerWorkarounds.h"

/* #define MAX_NUM_FIELDS 100 */
#ifndef N_DATA
#define N_DATA 4096 /* maximum number of datapoints */
#endif
#ifdef MAIN
// int myerrno = OK; // no longer needed here
#else
// extern int myerrno;
#endif
//
#ifdef MAIN
int eprintf_mode = ON;
#else
extern int eprintf_mode;
#endif
#ifndef MAIN
extern          // create global pbuffer
#endif          
char *pbuffer;  // pointer to buffer for output of run info for failures 

/*
#ifndef MAIN          // Removes since not really needed....JVS Nov 21, 2005
extern float Revision;
extern char *Prog_Name;
#endif
*/

#include "endian.h"
// defines required for knowing type
// #define BIG_ENDIAN      1000
// #define LITTLE_ENDIAN  -1000
// #define UNKNOWN_ENDIAN -1

/* #define max and min */
/*
#if !defined( __MINMAX_DEFINED)
#define __MINMAX_DEFINED
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define __max       max
#define __min       min
#endif 
*/
/*__MINMAX_DEFINED */
/* *************************************************************************** */
int advance(char *istr, int *sval, int len);
int check_byte_order(void);
int clean_name(const char *tmp_path, char *opath);
int clean_name(char *);
int copy(const char *SourceFile, const char *DestinationFile);
int eprintf(const char *fmt, ... );
int fileExists(const char *fileName);
bool doesFileExist(const char *fileName);
int ok_check(void);
int ok_checks(const char *string);
FILE *open_file(char *filename,const char*extension, const char *access);
int pprintf(const char *fmt, ... );
bool replaceEnvironmentVariableInString(const char *variableName, char *instring);
int latex_string(char *string, char *nstring);
int changeToLatexString(char *string, char *nstring);
void print_runtime_info(int argc, const char *argv[]);
void allocate_pbuffer(void);
void free_pbuffer(void);
float interpolate(float xh, float xl, float xm, float yh, float yl);
int array_read(FILE *istrm, float *array, int max_array);
int array_read(char *in_string, float *array, int max_array);
int view_errors(void);
// include readWriteEndian.h for these 
// float reverse_float_byte_order(float xold);
// short reverse_short_byte_order(short xold);
//int reverse_int_byte_order(int xold);
//int writeBigEndianBinaryFile(char *doseFileName,  int nDoseArray, float *doseArray);
//int writeLittleEndianBinaryFile(char *doseFileName,  int nDoseArray, float *doseArray);
//int writeBinaryFile(const char *doseFileName, int nDoseArray, float *doseArray, int swab_flag);
//int writeBinaryDataToFile(FILE *outputStream, int nArray, float *array, int swab_flag);
//int readBinaryDataFromFile(FILE *iStream, int nItemsToRead, float **arrayToRead, int swab_flag);
//int readBinaryDataFromFile(FILE *iStream, int nItemsToRead, float *inputArray, int swab_flag);
//int readBigEndianBinaryDataFromFile(char *iFileName, int nVoxels, float **dose);
#endif
