// Jan 25, 2012: JVS: Broken off of ctcreate.cc (in mcv/source/mcv) so can use in other routines.

#ifndef _MediaConfigH_Included
#define _MediaConfigH_Included
typedef struct {
   float maxDensity;
   char  mediumName[MAX_STR_LEN];
} EGSMaterialType;

int readMediaConfigFile(int *numMedia, EGSMaterialType **EGSMaterial, char *scanner_file, char *patientDirectory);
int getMediaNumber(int numMaterial,  EGSMaterialType *EGSMaterial, float inputDensity, int *outputMaterialNumber);
int getMediaNumberAndName(int numMaterial,  EGSMaterialType *EGSMaterial, float inputDensity, int *outputMaterialNumber, char* mediaName); 
int reportMediaConfig(int numMedia,  EGSMaterialType *EGSMaterial);
#endif
