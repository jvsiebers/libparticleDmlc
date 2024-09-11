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
/* dose_region.h
   Structures and prototypes to read in Dose Region (from Pinnacle Format)
   Created: 6/9/98: JVS
   Modification History:
    Oct 23, 1998: JVS: #incldue typedefs.h
    Jan 26, 1999: JVS: add get_dose_region proto's
    Feb  4, 1999: JVS: add code for reading dose grid from active Pinnacle Session
    April 6, 1999: JVS: Add code for reading dose region from active or inactive session
    April 22, 1999: JVS: Add trial_name to calls for getting dose region....
    June 29, 1999: JVS: Add read_pin_dose_grid and write_mcv_dose_header
    Feb 15, 2011: JVS: Add writeMCVDoseHeader and writeRCFDoseConfig
    April 13, 2020: JVS: Add readBinaryDataFileWithHeader and reportDoseFileStatistics
*********************************************************************************** */

#ifndef  UVA_DOSE_H_INCLUDED
#define  UVA_DOSE_H_INCLUDED

#include "dose_region.h" /* for defintion of volume_region_type */
/* ********************************************************************************* */
int readPinnacleDoseFile(char *doseFileName, volume_region_type *dg, float **dose);
int writePinnacleDoseFile(char *doseFileName, volume_region_type *dg, float *dose);
int write_mcv_dose_header(char *filename, volume_region_type *dg, const char *file_type, float weight);
int read_mcv_dose_header(char *filename, volume_region_type *dg, char *file_type, float *weight);
// int read_pin_dose_grid(FILE *fp, volume_region_type *dose_grid); // this is in dose_region.h
int writeMCVDoseHeader(char *filename, volume_region_type *dg, int fileEndian);
int writeRCFDoseConfig(const char *fileName, const char *directory);
int writeRCFDoseConfig(const char *fileName);
bool readBinaryDataFileWithHeader(char *iFile, volume_region_type *DoseGrid, float **fieldDose);
#endif
