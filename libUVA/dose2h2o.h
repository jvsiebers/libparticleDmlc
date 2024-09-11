/*
   Copyright 1999-2007 Virginia Commonwealth University

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

   The proper reference for this work is
   Siebers, J.V., Keall, P.J., Nahum, A.E., and Mohan, R., "Converting absorbed dose to medium 
   to absorbed dose to water for Monte Carlo based photon beam dose calculations", 
   Phys. Med. Biol., 45 p. 983-995 (2000)


   Please contact us if you have any questions 
*/
/* header file for dose2h20.h
   
   pjk 08/01/99 created
   jvs: April 7, 1999: change to MCNP determined SPR values
   pjk: Jan 25, 2000:  increased materials to 20. Will have problems with 
                       high Z numbers
   jvs: July 11, 2000: remove MAX_MATERIALS
   pjk: June 18, 2003: moved to include directory
   hzh: April 7, 2006: added the restriction for re-including the header file
   jvs: Oct 17, 2007: Clean up for general distribution
   jvs: Sept 28, 2009: JVS: add dMaterialTypeType to eliminate sun CC warning
*/
/* *********************************************************************** */


#ifndef _dose2h2o_h_
#define _dose2h2o_h_

// prototypes 
// int read_pinnacle_patient_beams(char *pbeam_fname, char *Trial_Name, int *n_beams, beam_type **pbeams);
//
// #include "common.h" // for PHANT_STRUCT

typedef struct dMaterialType
{
    char doseDirectory[MAX_STR_LEN]; // Directory where the dose will be
    char fileName[MAX_STR_LEN]; // file name
    char doseType[MAX_STR_LEN]; // type of conversion to perform
    int  nDoseGrid; // Number of elements in the dose grid (for conversion)
    float *doseValues; // The dose values
} dMaterialTypeType;
/* Function Prototypes */
int zeroAirDose(dMaterialType *hd, PHANT_STRUCT *phantom);
int convertToDwater(dMaterialType *headinfo, PHANT_STRUCT *, char *beamEnergy, char *config_path);
int readBinaryDoseData(dMaterialType *dmInfo);
int readDwaterConversionFactors(char *config_path, char *beamEnergy, PHANT_STRUCT *phantom,  float *stoppingPowerRatio);

#endif
