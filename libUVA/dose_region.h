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
    May 31, 2000: JVS: Add doseToPinnacle
    Oct 4, 2000: JVS: Change read_pinnacle_dose so does NOT use "Current"....
    Sept 4, 2002: JVS: BigEndian routines....
    Mar 17, 2010: JVS: Add routines so can create all-in-one script to load doses
    April 24, 2010: JVS: all-in-one script calls the per-beam script to avoid pinnacle different name length bug.
    May 12, 2012: JVS: Remove unused PinnWindowID from dose_to_pinn calls
    May 15, 2017: JVS: Break off volume_region.h
*********************************************************************************** */

#ifndef  DOSE_REGION_H_INCLUDED
#define  DOSE_REGION_H_INCLUDED

#include "volume_region.h"
#include "case_info.h" /* for definition of case_info_type */
/* ********************************************************************************* */
bool readDoseMatrix(const char *fileName, volume_region_type *dg);

int read_pinnacle_dose(char *plan_fname, char *trial_name, int dose_grid_id, int ibeam, volume_region_type *dr, float **dose, long PinnWindowID);
int dose_to_pinnacle(char *name, volume_region_type *dose_region, int ibeam); // , long PinnWindowID);
int dose_to_pinn(char *name, char *dosefile, volume_region_type *dose_region, int ibeam); //, long PinnWindowID);
int read_pinnacle_dose_region(char *dose_region_fname, char *trial_name, volume_region_type *dose_grid);
int read_pinnacle_dose_file(char *dose_fname, int dose_grid_size, float **dose);
int read_pin_dose_grid(FILE *fp, volume_region_type *dose_grid);
// #ifdef myCC // commented out myCC on 12/26/2015: JVS
int doseToPinnacle(char *TrialName, char *BeamName, char *dosefile, volume_region_type *dose_region);
int doseToPinnacleWithType(const char *calcType, char *TrialName, char *BeamName, char *doseFile, volume_region_type *dose_region);
int doseToPinnacle(char *scriptFname, char *TrialName, char *BeamName, char *dosefile, volume_region_type *dose_region);

int read_pinnacle_dose(char *TrialName, int ibeam, volume_region_type *dr, float **dose);
// int read_pinnacle_dose_region(volume_region_type *dg); /* gets dose region from Active Pinnacle Session */
int read_pinnacle_dose_region(char *TrialName, volume_region_type *dg);  /* gets dose region from Active Pinnacle Session */
int get_dose_region(char *dose_region_fname, volume_region_type **dose_region,
             case_info_type *patient );
int get_dose_region(char *dose_region_fname, char *trial_name, volume_region_type **dose_region);
int readBigEndianPinnacleDoseFile(char *doseFileName, int DoseGridSize, float **dose);
int writeBigEndianPinnacleDoseFile(char *doseFileName, int nDoseArray, float *doseArray);
int getVoxelVolume(case_info_type *patient);
int createPinnacleLoadDoseScript(const char *scriptFname, const char *TrialName, const char *BeamName, const char *dosefile, const volume_region_type *dose_region);
int writePinnacleDoseRegionToScript(const char *scriptFname, const char *TrialName, const volume_region_type *dose_region, const char *fileMode);
int writePinnacleBeamDoseToScript(const char *scriptFname, const char *TrialName, const char *beamName,
                                  const char *dosefile, const char *fileMode);
int writeLoadAllBeamsPinnacleScript(const char *scriptFileName, const char *trialName, const char *beamName, 
                                    const char *doseFileName, const volume_region_type *doseRegion);
int writeLoadAllBeamsPinnacleScript(const char *scriptFileName, const char *ptFolder, const char *trialName, const char *beamName);
int writeLoadAllBeamsPinnacleScript(const char *beamScriptFileName, const char *compositeScriptFileName);
// #endif

#endif
