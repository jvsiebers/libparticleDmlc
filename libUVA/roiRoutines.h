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
#ifndef ROIROUTINES_H_INCLUDED
#define ROIROUTINES_H_INCLUDED
/* roiRoutines.h

   routines for dealins with ROI....

   Modification History:
      April 25, 2002: JVS: Created
      June 3, 2004: Add checkPinnacleROI

*********************************************************************************** */
#include "case_info.h"
/* ******************************************************************************** */
int checkForPinnacleDensityOverrides(long PinnWindowID, int *nOverride);
int checkForPinnacleDensityOverrides(char *pbeamFileNameStem, int *nOverride);
int saveOverrideCtData(long PinnWindowID, case_info_type *patient);
int checkPinnacleROI(char *roiFileName, char *roiName);
/* ******************************************************************************* */
#endif
