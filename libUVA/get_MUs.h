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
/* get_MUs.h
   Header for get_MUs.cc
   Created: Feb 12, 2000: PJK
   Modification History:
      Sept 26, 2000: JVS: Add routines for floating point as this is how Pinnacle
                          has them (IMRT can have floating point MU's!)


*/
/* ******************************************************************** */
// prototypes from libpin/get_MUs.cc
int read_pinn_MUs(char *plan_fname, char *trial_name, int *MUs, long PinnWindowID);
int read_pinn_MUs(char *fname_stem, char *Trial_Name, int *MUs);
int readPinnacleMonitorUnits(char *fname_stem, char *TrialName, float *MonitorUnits);
int readPinnacleMonitorUnits(char *plan_fname, char *trial_name, float *MonitorUnits, long PinnWindowID);
/* ******************************************************************** */
