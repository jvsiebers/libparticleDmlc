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
/* rpm.h

   Header file for routines to read .rpm files

Created: 1 August 2001: RG/PJK

Modification History:
         22 Jan 2002: Added header and Ver 1.4 information. Moved to libpjk
         13 March 2002: Added rotationTime to rpm structure
         28 August 2002: PJK: Added readMidRPM as file format changed again

*/
/* *********************************************************************** */
// defines
#define MIN_WIDTH 0.05

/* *********************************************************************** */
// includes
#include "ct_routines.h"
/* *********************************************************************** */
// structure definitions

struct rpmType
{
    int   nBins; // number of bins to sort
    char  imageInFileName[MAX_STR_LEN]; // Pinnacle format image file read in
    char  imageOutFileName[MAX_STR_LEN]; // Pinnacle format image file written out
    float timePerSlice;
    float rotationTime;
    int   phaseFlag;
    int   equiBinFlag;

    char  rpmFileName[MAX_STR_LEN];
    float scaleRPM;
    int   randStart;
    float perp;

    float version;
    float exhExtremeLimit;   // max pos value used to provide scale, baseline
    float inhExtremeLimit;   // min pos value
    int   simTypeFlag;        // 0 for CT simulation, 1 for Fluoro simulation
    char  gateTypeFlag[MAX_STR_LEN];      // TRUE for Phase Gate, FALSE for Amp Gate ??
    float periodicDevnThres;  // Threshold to detect devn from periodic breathing (def 20)
    float upperGatingThres;   // upper gating threshold
    float lowerGatingThres;   // lower gating threshold
    int   ROIBoxWidth;         // width of ROI box in Image Window ( In pixels)
    int   ROIBoxHeight;      // height of ROI box in Image Window ( In pixels)
    int   ROIBoxLeftPos;    // upper left column position of ROI Box in image window
    int   ROIBoxUpPos;    // upper Row position of ROI Box in image window
    int   RecLength;      // recording length specified by user
    int   BuffstartIndex; // index of starting sample in circular buffer
    int   BuffendIndex;  // index of ending sample in circular buffer
    int   AudInstrFlag;   // enabled = 0, disabled = 1
    int   AudInstrMode;   // 1 = Manual, 2 = Auto
    float InhTimeManual;  // breath in instruction time in manual mode
    float InhTimeAuto;    // breath in insatruction time in auto mode
    float ExhTimeManual;  // breath out instruction time in manual mode
    float ExhTimeAuto;    // breath out insatruction time in auto mode
    int   VidInstrFlag;   // enabled = 0, disabled = 1
    int   VidInstrMode;  // 1 = Manual , 2 = Auto
    float VidMotionRange;  // displayed hash mark region range in cm, manual mode
    float VidIncrMotionRange; // incremental motion range in cm, auto mode
        
    table_type *position;
    table_type *phase;
    table_type *statusFlag;
    table_type *frameNumber;
    table_type *xRayOnFlag;   // 0 is on, 1 is off
    float xRayStart;

};

/* *********************************************************************** */
// prototypes
int readRPMfile(rpmType* rpm);
int readMidRPMfile(rpmType* rpm);
int readOldRPMfile(rpmType* rpm);
int overlay(dmlc_type* d, rpmType* rpm);
int overlayMotion(dmlc_type* d, rpmType* rpm);
int repeatRPM(float dmlcTime, rpmType* rpm);
int write4Dppm(image_header_type *img_header, ITYPE *image, char *fileName);
int getTimePositionPhase(image_header_type *img_header, rpmType *rpm, table_type *imagePosition, table_type *imagePhase);
int sortImages(int *imageSet, table_type *tab, rpmType *rpm, int *imageSetnBins);
int moveImageXY(image_header_type *img_header, ITYPE *image, ITYPE *imageOut, float moveX, float moveY);

// end of rpm.h
/* *********************************************************************** */
