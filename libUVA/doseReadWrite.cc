/** \file
    \author Various
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
/* doseReadWrite.cc
   Modification History:
     June 11, 2008: JVS: Created by breaking off from libpin/dose_regions.cc
                         These codes require NO PinnComm  
     June 30, 2008: JVS: Make it fully stand-alone so it does NOT require libjvs
     May 16, 2012: JVS: change several int's to size_t to remove CC compiler warnings for 64 bit

*********************************************************************************** */
#include <stdio.h>
#include <stdlib.h>      // for calloc
// #include <string.h>      // for strcat and strcpy
// #include "string_util.h" // for get_value
#include "utilities.h"   // for error routines
// #include "typedefs.h"
// #include "dose_region.h" // for definition of dose regions structure and function prototypes
// #include "read_clif.h"
// #include "readPinnacle.h" // for prototypes XDR
#include "readWriteEndianFile.h" // Mar 31, 2010: JVS : Headers now here

/* ********************************************************************************* */
/* ********************************************************************************* */
int writeBigEndianPinnacleDoseFile(char *doseFileName,  int nDoseArray, float *doseArray)
{
   return(writeBigEndianBinaryFile((const char *) doseFileName,nDoseArray, doseArray));
} 
/* ***************************************************************** */
int read_pinnacle_dose_file(char *DoseFileName, int DoseGridSize, float **dose)
{
   /* allocate memory for the dose */
   float *ldose;
#ifdef DEBUG
   printf("\n Allocating Dose Grid Size of %d", DoseGridSize);
#endif
   ldose = (float *) calloc(DoseGridSize, sizeof(float));
   if( ldose == NULL)
   {
      eprintf("\n ERROR: Allocating memory"); return(FAIL);
   }
   /* open file and read in the dose */
   FILE *dstrm = fopen(DoseFileName,"r");
   if( dstrm == NULL)
   {
      eprintf("\n ERROR: Opening Dose File %s",DoseFileName); return(FAIL);
   }
   size_t sread = fread(ldose, sizeof(float), DoseGridSize, dstrm);
   fclose(dstrm);
   if(sread != (size_t) DoseGridSize)
   {
      eprintf("\n ERROR: Reading in binary file %s", DoseFileName);
      eprintf("\n\t nread %d! = n_expected %d",sread, DoseGridSize); 
      return(FAIL);
   }
   *dose = ldose;
   return(OK);
}
/* ********************************************************************************* */
/* ********************************************************************************* */
int readBigEndianPinnacleDoseFile(char *DoseFileName, int DoseGridSize, float **dose)
{
#ifdef DEBUG
  printf("\n readBigEndianPinnacleDoseFile  %s",DoseFileName);
#endif
  if(read_pinnacle_dose_file(DoseFileName,DoseGridSize,dose) != OK)
  {
    eprintf("\n ERROR: Reading pinnacle dose file"); return(FAIL);
  }
  /* Check if need to swab */
   int swab_flag = 0;
   switch( (check_byte_order()) )
   {
      case BIG_ENDIAN:
         break;
      case LITTLE_ENDIAN:
         swab_flag=1;
	 break;
      default:
         eprintf("\n ERROR: Indeterminate Byte Order\n");
         return(FAIL);
   }
   if(swab_flag) {
#ifdef DEBUG
     printf("\t Byte swapping");
#endif
     float *ldose;
     ldose = *dose;
     for(int index=0; index<DoseGridSize; index++) {
          ldose[index] = reverse_float_byte_order( ldose[index] );
     }
   }
   return(OK);
}
/* ********************************************************************************* */
