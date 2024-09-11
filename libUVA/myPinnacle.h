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
#ifndef MYPINNACLE_H_INCLUDED
#define MYPINNACLE_H_INCLUDED
/* myPinnacle.h
   Pinnacle Specific Prototypes for dose computation algorithm

   Modification History:
      Dec 11, 1998: JVS:
      Apr 6, 1999: JVS: Add PinnWindowID here..
      May 15, 2017: JVS: Change readPinnaclePatientBeams
*/
#include "beams.h" /* for definition of beam_type */
#include "dose_region.h"
#ifndef MAIN
extern long PinnWindowID;
#else
long PinnWindowID=-1;
#endif
// int dose_to_pinnacle(char *name, volume_region_type *dose_region, int ibeam, long PinnWindowID);
// The 1st two do not read the patient->dg
int read_pinnacle_patient_beams(char *pbeam_fname, char *Trial_Name, int *n_beams, beam_type **pbeams);
int readPinnaclePatientBeams(char *pbeam_fname, char *Trial_Name, int *n_beams, beam_type **pbeams);
// next one reads the patient->dg
int readPinnaclePatientBeams(case_info_type *patient, int *n_beams, beam_type **pbeams);
#endif
