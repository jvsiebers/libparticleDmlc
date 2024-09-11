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
/* case_info.h
   Header file for case_info_type
   
   Modification History:
      Jan 26, 1999: JVS: Add #ifndef, change MAX_STR_LEN to MAX_CASE_STR_LEN
      Feb 3, 1999: JVS: Add comments about what is what, rename plan_name to field_name
      April 6, 1999: JVS: Add plan_fname to this list
      May 31, 2000: JVS: Change field_name to PlanName, since that is what it is...
      Sept 14, 2000: JVS: Move field_path to beamInfoType fileNameStub, 
                          Move process_path. to  processFileNameStub
      June 2, 2011: JVS: Add in StartWithDICOM, which is defined in plan.Pinnacle
      march 6, 2012: JVS: ImageDBName = img_header->db_name
      May 11, 2017: JVS: Add writeTrialName
      May 15, 2017: JVS: Add dg to case_info_type
      May 16, 2017: JVS: Add PrescriptionList
      Jan 4, 2018: JVS: Add SetDirectory
*/
#ifndef CASE_INFO_H_INCLUDED
#define CASE_INFO_H_INCLUDED

/* #define MAX_NUM_FIELDS 100 */
#define MAX_CASE_STR_LEN 200

#include "volume_region.h"
#include "read_clif.h"

typedef struct {
   char beamName[MAX_CASE_STR_LEN];
   char fileNameStub[MAX_CASE_STR_LEN];
    /*   char path[MAX_CASE_STR_LEN]; */
}beamInfoType;

/*
typedef struct {
  char Name[MAX_CASE_STR_LEN];
  float RequestedMonitorUnitsPerFraction;
  float PrescriptionDose;
  float NumberOfFractions;
  // ...
}prescriptionType;
*/

typedef struct{
    beamInfoType *beamInfo;
    char PatientName[MAX_CASE_STR_LEN];  /* patient name, i.e. John Jones */
    char PlanFileName[MAX_CASE_STR_LEN];
    char ImageName[MAX_CASE_STR_LEN];    /* including path */
    char ImageDBName[MAX_CASE_STR_LEN];   // DB name of the image -- is img_header->db_name
    char Directory[MAX_CASE_STR_LEN];    /* directory for the plan */
    char PlanName[MAX_CASE_STR_LEN];     /* PlanName */
    char PlanStub[MAX_CASE_STR_LEN];     /* Stub of the Plan Name (i.e. plan) */
    char TrialName[MAX_CASE_STR_LEN];    /* i.e. Trial_1, Trial_2 */
    char writeTrialName[MAX_CASE_STR_LEN];    // trial name for writing the dose files and scripts
    char patient_path[MAX_CASE_STR_LEN];  /* output path for the patient */
    char processFileNameStub[MAX_CASE_STR_LEN];
    // char runPath[MAX_STR_LEN]; // directory where process is running from (avoids PWD, which gets bad path info)
    /*  char field_path[MAX_NUM_FIELDS][MAX_CASE_STR_LEN]; */  /* output path for the beam */
    float voxel_volume; /* input here so don't need to declare volume_region_type
                        in write_dosxyz, and hence the upper files */
    int StartWithDICOM; // if 1, then use coordinate system for images is based off of x_start_dicom and y_start_dicom
    volume_region_type dg;
    clifObjectType *PrescriptionList;  // simpler to save as clif object, then have access to all things needed.
}case_info_type;
/* sample output for case_info_type  
 name: comm2, , 
 image_name: /dsk3/Pinnacle/Patients//dsk3/Pinnacle/Patients//dsk3/Pinnacle/Pati
 directory: /dsk3/Pinnacle/Patients/Institution_1/Mount_0/Patient_22/Plan_7//
 field_name: wedge6
 trial_name: Trial_1
 patient_path: comm2__wedge6_Trial_1
 field_path: Trial_1_l15
 process_path: 
*/

int read_pinnacle_patient(case_info_type *patient);
int readPinnaclePatient(case_info_type *patient);
void setPatientDirectoryFromPlanFileName(case_info_type *patient);
#endif
