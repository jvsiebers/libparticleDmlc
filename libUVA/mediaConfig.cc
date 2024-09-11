// Jan 25, 2012: JVS: Broken off of ctcreate.cc (in mcv/source/mcv) so can use in other routines
// June 4, 2015: JVS: get rid of couts as it is causing problems with solaris g++ compile
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#include "utilities.h"

#include "read_clif.h"

#include "mediaConfig.h"
#include "jvsFile.h"
using namespace std;
/* ****************************************************************** */
int readMediaConfigFile(int *numMedia, EGSMaterialType **EGSMaterial, char *scanner_file, char *patientDirectory) {
  // printf("\n in readMediaConfigFile\n");
  const char* configFileName="media.config";
  FILE *tstrm=openConfigFile(configFileName,"r");
  if(NULL == tstrm) {
    printf("\n ERROR: Opening config file %s", configFileName);return(FAIL);
  }
 // read the name of the scanners.db file and media cut_offs
 char clif_name[MAX_STR_LEN];
 strcpy(clif_name,"media_inputs= {");
 if(clif_get_to_line(tstrm,clif_name)!=OK)
 {
    printf("\n ERROR: finding media inputs ={");
    return(FAIL);
 }
 if(NULL != patientDirectory){
   // Get name of Scanner File
   // Default is $PATIENTS/$PINN_INSTITUTION/Scanners.db, but  env variables not passed, so get if from Directory
   // If cannot find that, then use value in the config file  
   sprintf(scanner_file,"%s/../../../Physics/Scanners.db",patientDirectory);
   // Check if file exists
   if(OK != fileExists(scanner_file)) {
      //
     printf("\n WARNING: Cannot find file %s, trying file set in %s", scanner_file, configFileName);
      if(clif_string_read(tstrm,"ScannerFile =",scanner_file)!=OK){
        printf("\n ERROR: Reading scannerfile from media config"); return(FAIL);
      }
      if (OK != fileExists(scanner_file) ) {
	printf("\n ERROR: File %s does not exist",scanner_file); return(FAIL);
      }
   }
 }
 // read in the materials and media
 int num_media =  (int) clif_get_value(tstrm,"Num_Media =");
 *numMedia = num_media;
 char stmp_name[MAX_STR_LEN], stmp_dens[MAX_STR_LEN];

 EGSMaterialType *LocalEGSMaterial; // Now that this is a routine, pass it the type
 LocalEGSMaterial = (EGSMaterialType *) calloc(num_media,sizeof(EGSMaterialType));
 if(EGSMaterial == NULL){
    printf("\n ERROR: Allocating Memory for EGSMaterials"); return(FAIL);
 } 
 for (int iMedia=0;iMedia<num_media;iMedia++){
     sprintf(stmp_name,"Media_%d_Name =",iMedia);
     sprintf(stmp_dens,"Media_%d_MaxDens =",iMedia);
     clif_string_read(tstrm,stmp_name,LocalEGSMaterial[iMedia].mediumName);
     LocalEGSMaterial[iMedia].maxDensity = clif_get_value(tstrm,stmp_dens);
#ifdef DEBUG_CTCREATE
     printf("\nMedia %d Name %s Max Dens %f",iMedia,LocalEGSMaterial[iMedia].mediumName,
          LocalEGSMaterial[iMedia].maxDensity);
#endif
   }
 // error check
 if(checkClifError() != OK){
    printf("\n ERROR: Reading options from media.config"); return(FAIL);
 }
 /* close the clif */
 if(clif_get_to_line(tstrm,"};")!=OK){
   printf("\n ERROR: Closing %s structure",clif_name);return(FAIL); 
 }
 fclose(tstrm);
 // printf("\n exiting readMediaConfigFile\n");
 *EGSMaterial = LocalEGSMaterial;
 return(OK);
}
/* ****************************************************************** */
int getMediaNumber(int numMaterial,  EGSMaterialType *EGSMaterial, float inputDensity, 
    int *outputMaterialNumber) 
{
  int iMedia = 0;
  while(EGSMaterial[iMedia].maxDensity < inputDensity && iMedia < numMaterial) iMedia++;
  if(iMedia >= numMaterial) {
    printf("\nERROR: %s:  density %f  exceeds maximum density of %f", __FUNCTION__, inputDensity, EGSMaterial[numMaterial-1].maxDensity); 
    printf("\n\t numMaterial = %d",numMaterial);
    printf("\n\t EGSMaterial[0].maxDensity = %f  EGSMaterial[%d].maxDensity = %f", EGSMaterial[0].maxDensity,numMaterial-1,EGSMaterial[numMaterial-1].maxDensity);
    return(FAIL);
  }
  *outputMaterialNumber=iMedia;
  return(OK);
}
/* ****************************************************************** */
int getMediaNumberAndName(int numMaterial,  EGSMaterialType *EGSMaterial, float inputDensity, 
                          int *outputMaterialNumber, char* mediaName)
{
  int iMedia = 0;
  while( EGSMaterial[iMedia].maxDensity < inputDensity && iMedia < numMaterial){ 
    // cout << inputDensity << "\t" << iMedia << "\t" << EGSMaterial[iMedia].mediumName << endl;
    iMedia++;
  }
  if(iMedia >= numMaterial) {
    printf("\nERROR: %s:  density %f  exceeds maximum density of %f", __FUNCTION__, inputDensity, EGSMaterial[numMaterial-1].maxDensity); 
    printf("\n\t numMaterial = %d",numMaterial);
    printf("\n\t EGSMaterial[0].maxDensity = %f  EGSMaterial[%d].maxDensity = %f", EGSMaterial[0].maxDensity,numMaterial-1,EGSMaterial[numMaterial-1].maxDensity);
    return(FAIL);
  }
  *outputMaterialNumber=iMedia;
  // Check to see if the string is too long to copy (e.g, unterminated string on read---
//  strcpy(mediaName,"Help");
  if(strlen(EGSMaterial[iMedia].mediumName) > MAX_STR_LEN) {
     strncpy(mediaName,EGSMaterial[iMedia].mediumName, MAX_STR_LEN-3);
  } else {
     strcpy(mediaName,EGSMaterial[iMedia].mediumName);
  }

  return(OK);
}
/* ****************************************************************** */
/* ****************************************************************** */
int reportMediaConfig(int numMedia,  EGSMaterialType *EGSMaterial)
{
  printf("\n\t iMedia \t maxDensity \t mediumName ");
  for(int iMedia=0; iMedia<numMedia; iMedia++) { 
    printf("\n\t%d\t%f\t%s",iMedia, EGSMaterial[iMedia].maxDensity,EGSMaterial[iMedia].mediumName);
  }
  return(OK);
}
/* ****************************************************************** */
