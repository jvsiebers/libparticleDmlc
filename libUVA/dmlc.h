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
/* dmlc.h

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
   April 25, 2001: JVS: Add Resolution, nColumns, Origin to dmlc_pdf_type...
   July 30, 2001: PJK: added stuff for fourDC
   August 2, 2001: PJK: added cycle, minLeaf and maxLeaf to dmlc structure
   August 3, 2001: PJK: changed poerator array size
   Sept 18, 2001: JVS: Move tableType to table.h...
   June 4, 2008: JVS: Updates for sun CC compiler
   Jan 18, 2012: JVS: Add #ifndef on to this routine

*/
#ifndef DMLC_H_INCLUDED   /* To eliminate possibility of multiple inclusion of this file */
#define DMLC_H_INCLUDED
/* *********************************************************************** */
#include "typedefs.h" // for definition of Point_2d_float
#include "common.h"   // for TWOD_ARRAY
#include "table.h"    // for table_type
#include "read_write_egs.h"  // for ps_header_type
/* *********************************************************************** */
typedef struct keepType {
   int electron;
   int primOpen;
   int primLeaf;
   int primTip;
   int scatLeaf;
   int scatTip;
}keepTypeStruct;
typedef struct russianRoulette {
   float rrWeight;
   float rrMaxPartWeight;
   int totalParticlesToWrite;
   int numParticlesToPlayRR;
   int numParticlesThatLive;
}russianRouletteType;
/*typedef struct tableType {
   int npts;
   Point_2d_float *points;
   };*/
typedef struct mlcType {   // Structure that contain information about a machines MLC
    float leafWidth; // Period of repeatition (used to be width of leaf) -- temporary to adapt to current code
    float density;    // density in g/cc (MLC_DENSITY)
    float distance;   // distance from target to mlc (MLC_Z)
    float closedOffset; // offset from true closed position (VARIAN_CLOSED)--note this is at the specificationDistance
    float specificationDistance; // distance that MLC leaf thickesses are specified (typically at isocenter = 100 cm
    int   nLeaves;    // number of MLC leaves
    int   nSections;  // Number of sections (typically 2,for even/odd or target/isocenter leaves
    tableType *profile;  // thickness profile for each leaf section
    // Tip Stuff
    float maxTipThickness; // Maximum thickness for tips (TIP_MAX_THICK)
    float tipRadius; // radius of rounded leaf tips (TIP_RAD)
    float tipAngle;  // angle for rounded leaf tip (TIP_ANG)
}mlcTypeStruct;
// defines
#define MAX_FIELD_NUM        2500
#define MAX_LEAF_NUM          100
/* ********************************************************************* */
#ifdef OLD_80_LEAF
#define UVA_21_NUM_LEAF        80
#define UVA_6_NUM_LEAF         80 // need to check
#define HAN_21_NUM_LEAF       120
#define MLC_DENSITY            17.6 // best guess as of 10 Nov 1999
#define ISOCENTRE             100.0
#define MLC_Z                  50.9 // from Varian drawings
#define LEAF_WIDTH              1.0
// leaf thicknesses at various points- got from Radhe via Jeff
#define THICK_1                 2.545
#define POS_THICK_1             0.0137
#define THICK_2                 5.525
#define POS_THICK_2             0.2164
#define THICK_3                 6.130
#define POS_THICK_3             0.7972
#define THICK_4                 5.525
#define POS_THICK_4             0.8653
#define THICK_5                 2.458
#define POS_THICK_5             0.8896
#define THICK_6                 5.03
#define POS_THICK_6             1.0
//#define TIP_WIDTH               0.75
#define MAX_LEAF_VEL            2.0 // cm per sec
#define TIP_RAD                 8.0
#define TIP_ANG                11.3 // angle of tip in degrees
#define TIP_MAX_THICK           6.13  //changed from 5.525 from JVS MCNP input /radiation2/jvs/data/include/photons/cl2100_mlc_leaf.inp . Changed again to max thickness of leaf
#define VARIAN_CLOSED           0.08 // Measured by Mark A
#endif
/* ********************************************************************* */
#ifndef PI
#define PI 3.141592654
#endif
#define COLL_OFF                2.0  // distance to care about beneath the collimators
#define COLL_MAX               20.0  // max collimator size
#define PDF_GRID_WIDTH       1000 // number of points in leaf direction in pdf = 0.4 mm resolution at max field size
#define N_PER_LINE              8
#define MAX_U_BINS            270
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
#define MAX_COMP_BINS         200 // max allowed for Pinnacle
/* *********************************************************************** */
// structure definitions
struct dmlc_type
{
  char  dmlc_file_name[MAX_STR_LEN];

  char  file_rev[MAX_STR_LEN];
  char  treatment[MAX_STR_LEN];
  char  last_name[MAX_STR_LEN];
  char  first_name[MAX_STR_LEN];
  char  patient_id[MAX_STR_LEN];
  int   num_shapes;
  int   num_leaves;
  float tolerance;
  float v_coll; /* Maximum Collimator setting in direction of
                   leaf motion....used for creating pdf.. 
                   Currently, this is specified at the MLC plane   
                   accounts for offset and moving to MLC */

  int   field[MAX_FIELD_NUM];
  float index[MAX_FIELD_NUM];
  int   carriage[MAX_FIELD_NUM];
  char  poperator[MAX_FIELD_NUM][MAX_STR_LEN];
  float collimator[MAX_FIELD_NUM];
  TWOD_ARRAY *l_leaves;  // pointer to 'left' leaf bank
  TWOD_ARRAY *r_leaves;  // pointer to 'right' leaf bank
  int note[MAX_FIELD_NUM];
  float mus[MAX_FIELD_NUM];
  float dose_rate[MAX_FIELD_NUM];
  int shape[MAX_FIELD_NUM];
  float mag[MAX_FIELD_NUM];
  float z_plane;
  float cycle;
  int minLeaf;
  int maxLeaf;
}dmlc_typeStruct;

struct dmlcType
{
  char  dmlcFileName[MAX_STR_LEN];

  char  fileRev[MAX_STR_LEN];
  char  treatment[MAX_STR_LEN];
  char  lastName[MAX_STR_LEN];
  char  firstName[MAX_STR_LEN];
  char  patientID[MAX_STR_LEN];
  int   numShapes;
  int   numLeaves;
  float tolerance;
  float vColl; // accounts for offset and moving to MLC

  int   field[MAX_FIELD_NUM];
  float index[MAX_FIELD_NUM];
  int   carriage[MAX_FIELD_NUM];
  char  poperator[MAX_FIELD_NUM][MAX_STR_LEN];
  float collimator[MAX_FIELD_NUM];
  TWOD_ARRAY *lLeaves;  // pointer to 'left' leaf bank
  TWOD_ARRAY *rLeaves;  // pointer to 'right' leaf bank
  int note[MAX_FIELD_NUM];
  float mus[MAX_FIELD_NUM];
  float doseRate[MAX_FIELD_NUM];
  int shape[MAX_FIELD_NUM];
  float mag[MAX_FIELD_NUM];
  float zPlane;
  float cycle;
  int minLeaf;
  int maxLeaf;

}dmlcTypeStruct;

typedef struct dmlc_pdf_type
{
  TWOD_ARRAY *fracOpen; // fraction of treatment time leaves are open at a point
  TWOD_ARRAY *fracOpenU[MAX_U_BINS]; // fraction of treatment time leaves are open at a point for a given u
  TWOD_ARRAY *fracClosed; // fraction of treatment time leaves are fully closed at a point 
  TWOD_ARRAY *fracIntNextClosed; // fraction of treatment time leaves and next set of leaves are closed 
  TWOD_ARRAY *fracIntPrevClosed; // fraction of treatment time leaves and prevoius set of leaves are closed 
  TWOD_ARRAY *fracLeftTips[MAX_SPLIT_TIPS];
  TWOD_ARRAY *fracRightTips[MAX_SPLIT_TIPS];
  int numTiplets;
  float tipWidth;
  float thickTip[MAX_SPLIT_TIPS];
  float minPartWeight; // for RR
  int uDepend;
  int uBins;
  //  Add below so can use with different grid sizes...
  int nColumns;    // Number of columns in each row...
  float Resolution;  // Spacing
  float Origin;      // Origin of the PDF
}dmlc_pdf_typeStuct;

/* *********************************************************************** */
// prototypes
int read_dmlc_file(dmlc_type* dmlc);
int transport_phase_space(char *fname, mlcType *machineMLC, dmlc_type* d, dmlc_pdf_type *p, table_type *E_mu, table_type *E_mu_Compt, russianRoulette* rr, keepType *keep);
int read_mu_table(char *fname, table_type* E_mu, int LINE, float Density);
int read_mu_water_table(char *fname, table_type* E_mu, int LINE);
//int time_leaf(float *tleaf,int row, float pdf_grid_mid_pos[col],TWOD_ARRAY *leaf, float index_array[MAX_FIELD_NUM]);
float dm_slope(TWOD_ARRAY *leaf, int row, int s, float delta_time);
int trav_leaf(dmlc_pdf_type *p, dmlc_type* d, float pdf_grid_mid_pos[PDF_GRID_WIDTH], float delta_time, float total_time, int row, int s, mlcType *machineMLC);
float crossings(float intl, float intr, float dtime);
float get_thick(float inval);
int russian_roulette(ps_header_type* ps_hd_out, particle_type* part, FILE *psout, russianRoulette* rr);
int write_part(ps_header_type* h, particle_type* p, FILE *ostrm, russianRoulette* rr);
int tip_thick(dmlc_pdf_type *p,mlcType *machineMLC);
void error_part(particle_type *p, particle_type *s);
void error_part(particle_type *p);
int tip_exp_thick(dmlc_pdf_type *p, int tiplet, float mu_over_w, float thick, float *act_weight, mlcType *machineMLC );
float int_exp(float thick[NUM_INT+1],float mu);
int samp_tip_thick(dmlc_pdf_type *p, float thickleaf, float *samp_thick,mlcType *machineMLC);
// in scatter
int scattered_photon(ps_header_type* ps_hd_out, particle_type* part, particle_type* scat, FILE *psout, float thick, float mu, table_type *E_mu, table_type *E_mu_Compt, int *count, float wt_not_int, float psdDistance, russianRoulette* rr);
int forced_interaction(float thick, float mu_over_w, float *scat_thick );
int sample_KN(particle_type* prim, particle_type* scat);
int rotate(particle_type* prim, particle_type* scat);
int rotate(particle_type* prim, particle_type* scat, float ws, int *dir);
int createPdf(dmlc_pdf_type *p, int row);
float  getMLCThickness( float yValue, float leafWidthAtMLC, mlcType *machineMLC );
int projectDMLCtoMLC(mlcType *machineMLC, dmlc_type *dmlc);

//int get_pdf(dmlc_pdf_type *p, dmlc_type* d, mlcType *machineMLC, russianRoulette* rr);
int get_pdf(dmlc_pdf_type *p, dmlc_type* d, mlcType *machineMLC);
int writePDFtoPPM(dmlc_pdf_type *p);
int getPDFMaxOpenIntensity(dmlc_pdf_type *p, float *maxOpenIntensity);
/* *********************************************************************** */
// gatedmlc specific prototypes
int read_dmlc_file(dmlc_type* dmlc, int NumSegments);
int rescale(dmlc_type* d, int NumSegments);
int addcosine(dmlc_type* d, float Amplitude, float Period, float LowLim, float UpLim, float TreatTime, float ScalFac, int numLeaf);
int write_dmlc_file(dmlc_type* d, float Amplitude, float Period);
int write_dmlc_file(dmlc_type* d);


int readMLC(mlcType *mlc, char *mlcName);
/* *********************************************************************** */
// dmlcT2F specific prototypes
int readSpectrumTable(char *fname, table_type* E_spec);
int calculateIntensity(dmlc_pdf_type *pdf, table_type* E_mu, table_type* E_spec,TWOD_ARRAY *intensity, mlcType *machineMLC);

/* *********************************************************************** */
// fourDC specific prototypes
int updateMLC(dmlc_type* d, FILE *fp, FILE *fpOut, char *commandName);

/* *********************************************************************** */
// realSignal specific prototypes
int writeSignal(FILE *fp, float cycle, float amplitude, float period);

// end of dmlc.h
/* *********************************************************************** */
#endif
