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
/* max-t.h

   Routines to read pinnacle patient and pinnacle mlc files.
   A .dml file is created from the static mlc files
   Also reads in fluoroscopic data file and adds fluoroscopic 
   motion data onto the dml file.

Created: Aug 26, 2000: PJK

Modification History:
         Dec 20, PJK, moved from utilities to my library  
   June 4, 2008: JVS: Updates for sun CC compiler                       



*/
/* *********************************************************************** */
// stuctures
typedef struct motionType {   // Structure that contain information about a patient's respiration motion
    char  motionFileName[MAX_STR_LEN];
    char  LastName[MAX_STR_LEN];
    char  SourceModality[MAX_STR_LEN];
    char  CreationDate[MAX_STR_LEN];
    char  FirstName[MAX_STR_LEN];
    char  PatientID[MAX_STR_LEN];
    float Magnification;
    float PhaseShift;
    float BreathingPeriod;
    float Orientation; //Direction of motion: must be same as MLC orientation
    float DoseRate;
    tableType *movement; 
}motionTypeStruct;
/* *********************************************************************** */
// prototypes
int readMotionFile(motionType *mtn);


/* *********************************************************************** */
