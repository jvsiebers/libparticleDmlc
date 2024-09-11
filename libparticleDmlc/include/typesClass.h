/*
   Copyright 2000-2003 Virginia Commonwealth University
             2014: University of Virginia


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
/* particleDmlc.h

   Header file for routines to read .dml files and either convert them to 
   a BEAM/MCNP-type input or process the phase space itself

Created: 9 Nov 1999: PJK

Modification History:
   Dec 8, 1999: JVS: Modify MLC_DENSITY to 17.6
                     (Need to put these parameters in a .config file)
   Jan 20, 2000: PJK: Changed VARIAN_CLOSED to 0.06 as per Arnfield
   August 17, 2000: JVS: add mlcType
   August 28, 2000: copied dmlc_type to dmlcType
   October 2, 2000: PJK: removed PART_MIN_WEIGHT and added russianRoulette
   December 12, 2000: PJK: added structure keepType
   Jan 11, 2001: JVS: Eliminate references to "right" and "left" leaf bank
                      and call them BLeafBank and ALeafBank respectively as
                      Varian does...
   Jan 16, 2001: JVS: Add grooveDistance and grooveThickness....
                      Note: only use 1 for all leaves, and will
                      always be based ( upper && even ) || ( lower && odd )
   May 21, 2001: JVS: Add routines to read VarianMlcDataType...
   August 3, 2001: PJK: Tried to increase MAX_FIELD_NUM- got segmentation fault in particleDMLC. Reset
   August 3, 2001: JVS: get rid of poperator...temp fix, should get rid of MAX_FIELD_NUM...
   April 12, 2002: JVS: Add physicalLeafOffset
   November 05, 2004: IBM: Changed structures mlcSectionType and mlcIndexType
   December 16-18, 2014: JVS: Add #ifdef _typesClass_h on the front end, add compatibility for Arc
   Jan 12, 2015: JVS: Change keepType to class
   Oct 30, 2015: JVS: Convert dmlc_type to class
*/
#ifndef _typesClass_h 
#define _typesClass_h
/* *********************************************************************** */
#include "common.h"   // for TWOD_ARRAY
#include "table.h"    // for table_type
#include "controlPoint.h" // for controlPoint
/* *********************************************************************** */
// defines
#define MAX_FIELD_NUM         4001
#define MAX_LEAF_NUM          100
/* ********************************************************************* */
#ifndef PI
#define PI 3.141592654
#endif
#define COLL_OFF              2.0  // distance to care about beneath the colliamtors
#define PDF_GRID_WIDTH       1000 // number of points in leaf direction in pdf = 0.4 mm resolution at max field size
#define N_PER_LINE              8
#define MAX_SPLIT_TIPS        200 // maximum number of bits to splits tips into
#define NUM_INT                10 // num_int must be even. Number of integration points
                                 // cpu time v. dependent on PDF_GRID_WIDTH

#define LARGE                   1.0E10 //  for infinite slopes
#define TOLERANCE               5.0E-5 // for comparing two values 
//#define PART_MIN_WEIGHT         -1. // RR threshhold <0 means no RR
#define DOUBLE_LIMIT            1.0E-6
#define THICK_FRAC              1.0 // change leaf ip thickness to account for limitations in model re angles
// for reading mu table
#define MU_LINE                 6
#define MU_COMPTON_LINE         2

#define NO_COUNT              781 //for not counting weight 0 particles
#define KEEP                    1 // to keep particles
#define DISCARD                -1 // to discard particles

#define TOP_SECTION 333
#define BOTTOM_SECTION 222
#define OUTSIDE_LEAF   000
#define INSIDE_LEAF    111
/* ********************************************************************* */
typedef struct
{
  float MagFactor;
  table_type data;
} VarianMlcDataType;
//
// typedef struct keepType{ 
class keepType {
 public:
  keepType(){
    electron = DISCARD;
    primOpen = DISCARD;
    primLeaf = DISCARD;
    primTip  = DISCARD;
    scatLeaf = DISCARD;
    scatTip  = DISCARD;
    comptonElectron = DISCARD;
  };
  ~keepType(){};
   int electron;
   int primOpen;
   int primLeaf;
   int primTip;
   int scatLeaf;
   int scatTip;
   int comptonElectron;
};
//
typedef struct russianRoulette {
   float rrWeight;
   float rrMaxPartWeight;
   int totalParticlesToWrite;
   int numParticlesToPlayRR;
   int numParticlesThatLive;
}russianRouletteType;
//
typedef struct mlcIndexType{
  int   leafIndex; // index of the leaf in the dml file (remember 1 offset)
  float yEnd;    // yCoordinate (at section center) of "end" of the section (thickness exists from [i-1].yEnd to [i].yEnd)
  float thickness; // Thickness of the section
  float grooveDistance; // Distance Groove is from the tip
  float grooveThickness; // "Width" of the groove  
}mlcIndexType;
//
typedef struct mlcSectionType{   // Number of sections that the leaf is broken up into in the z (beam) direction
  char Name[MAX_STR_LEN];
  tableType thicknessProfile;  // tableType is defined in table.h 
  tableType leadingTip;
  tableType followingTip;
  float pzEnter, pzExit, pzCenter;
  int nIndex; 
  mlcIndexType *index;
}mlcSectionType;
//
typedef struct mlcType {   // Structure that contain information about a machines MLC
    float leafWidth; // Period of repeatition (used to be width of leaf) -- temporary to adapt to current code
    float density;    // density in g/cc (MLC_DENSITY)
    float distance;   // distance from target to mlc (MLC_Z)
    float closedOffset; // offset from true closed position (VARIAN_CLOSED)--note this is at the specificationDistance
    float physicalLeafOffset; /* This is PER LEAF.  
      Get physicalLeafOffset from looking at the DYNALOG files and 
      seeing how much leaves cross over when they are specified to be "closed". If leaves cross by 16 "steps" or 0.16 mm, then per leaf this is 0.08 mm.
      Units here are cm, so value would be 0.008 cm  */
    float specificationDistance; // distance that MLC leaf thickesses are specified (typically at isocenter = 100 cm
    int   nLeaves;            // number of MLC leaves
    int   nSections;          // Number of sections (typically 2,for even/odd or target/isocenter leaves
    mlcSectionType *section;  // Sections for new section type
    tableType *profile;       // thickness profile for each leaf section
    // Tip Stuff
    float maxTipThickness; // Maximum thickness for tips (TIP_MAX_THICK)
    float tipRadius; // radius of rounded leaf tips (TIP_RAD)
    float tipAngle;  // angle for rounded leaf tip (TIP_ANG)
    float tipDistance;
    int nTipSections;
    // Stuff for sampling
    int maxTimeSamples; // Number of different time (actually, fractionalMU) samples to determine average leaf transmission from
}mlcType;
//
/* *********************************************************************** */
// structure definitions
//typedef struct dmlc_type
class dmlc_type {
 public:
  // functions
  dmlc_type();
  dmlc_type(const char *fileName);
  ~dmlc_type();
  bool readDmlcFile(const char *fileName);
  bool readDmlcFile();
  bool arcBeam(){return isArc;};
  float ALeaf(int iLeaf, int iCP){ return ARR_VAL(ALeafBank,iLeaf,iCP); };
  float BLeaf(int iLeaf, int iCP){ return ARR_VAL(BLeafBank,iLeaf,iCP); };
  // variables (All public for now since converted from struct) 
  char  dmlc_file_name[MAX_STR_LEN];
  char  file_rev[MAX_STR_LEN];
  char  treatment[MAX_STR_LEN];
  char  last_name[MAX_STR_LEN];
  char  first_name[MAX_STR_LEN];
  char  patient_id[MAX_STR_LEN];
  bool  staticMLC; // true if numApertures = 1, else false.
  int   numApertures;
  int   num_leaves;
  int   field[MAX_FIELD_NUM];
  int   carriage[MAX_FIELD_NUM];
  // int   note[MAX_FIELD_NUM];
  // int   shape[MAX_FIELD_NUM];      // not used currently
  float tolerance;
  float v_coll; // accounts for offset and moving to MLC
  float index[MAX_FIELD_NUM];         // Fractional number of MU's for leaf positions for IMRT, gantryAngle for arc
  float fractionalMUs[MAX_FIELD_NUM];  // Fractional number of MU's for leaf positions-- 
  // char  poperator[MAX_FIELD_NUM][MAX_STR_LEN];
  float collimator[MAX_FIELD_NUM];    // Collimator Angle
  //  float mus[MAX_FIELD_NUM];       // not used currently
  //  float dose_rate[MAX_FIELD_NUM]; // not used currently
  float mag[MAX_FIELD_NUM];           // magnification plane
  unsigned nIndexUsed[MAX_FIELD_NUM]; // for debugging
  unsigned sampledGantryAngle[360];   // for debugging
  float psdZPlane;
  TWOD_ARRAY *ALeafBank;  // pointer to 'left'  leaf bank; A leaves (used to be l_leaves)
  TWOD_ARRAY *BLeafBank;  // pointer to 'right' leaf bank; B leaves (used to be r_leaves)
 private:
  bool isArc;
  bool isCPInfoSet;
  int  nCP;
  controlPoint *CP;
};
//
typedef struct dmlc_pdf_type
{
  TWOD_ARRAY *fracOpen; // fraction of treatment time leaves are open at a point
  TWOD_ARRAY *fracClosed; // fraction of treatment time leaves are fully closed at a point 
  TWOD_ARRAY *fracIntNextClosed; // fraction of treatment time leaves and next set of leaves are closed 
  TWOD_ARRAY *fracIntPrevClosed; // fraction of treatment time leaves and prevoius set of leaves are closed 
  TWOD_ARRAY *fracALeafTips[MAX_SPLIT_TIPS];
  TWOD_ARRAY *fracBLeafTips[MAX_SPLIT_TIPS];
  int numTiplets;
  float tipWidth;
  float thickTip[MAX_SPLIT_TIPS];
  float minPartWeight; // for RR
}dmlc_pdf_type;

#endif
/* *********************************************************************** */
