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
/* particleDmlc.hs

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
   Feb 2008: JVS: Update for use particleDmlcEPID
   Jan 5, 2010: JVS Delete unused #includes
   Oct 21, 2011: JVS: Add nFoundWithSameIndex to track occurance when 1st segment and 2nd segment both have index==0.0 and random number (fractionalMUs = 0)
   Nov 22, 2011: JVS: Add section of protected functions so derived classes can access them...
   Dec 17-22, 2014: JVS: Major update of constructor and initializations
   Sept 22, 2015: JVS: Initialize CP and transform pointers to NULL per Hui Lin @ RPI
*/

#ifndef _particleDmlcClass_h
#define _particleDmlcClass_h

/* *********************************************************************** */
// system libraries
#include <stdio.h>
#include <stdlib.h>

// other libraries
#include "typedefs.h"
#include "utilities.h"

#include "string_util.h"
#include "common.h"
#include "table.h"

#include "read_write_egs.h"

// these header files ...
#include "read_clif.h"
#include "defined_values.h"
#include "compensator.h"
#include "modifiers.h"
#include "readPinnacle.h"
// ... included to read the compensator

#include "myrandom.h"
#include "typesClass.h" //< contains definitions of keepType, mlcType, dmlc_type, ... things that used to be in dmlc.h --
#include "controlPoint.h"
#include "coordinateTransform.h" // for coordinateTransform

using namespace std;

#define N_MLC_SECTIONS 2

class particleDmlc {
public:
    ~particleDmlc();

    particleDmlc();

    particleDmlc(const int InputSeed, const float InputrrWeight, const char *dmlFileName);

    particleDmlc(const float InputrrWeight, const int maxTimeSamples, const char *dmlFileName);

    particleDmlc(int *argc, char *argv[]); //< Construct based on command line
    bool initializeParticleDmlc(const char *mydmlFileName) {
        return (initializeParticleDmlc(false, mydmlFileName));
    };

    bool initializeParticleDmlc(bool isThisArc, const char *dmlFileName);

    bool initializeParticleDmlc();

    bool parseParticleDmlcArgs(int *argc, char *argv[]);

    // void  InitializeParticleDmlcPointersToNull(){ InitializePointersToNull(); };
    void setRandomSeed(unsigned mySeed);

    int MLCTransport(particle_type &part, particle_type &scat,
                     particle_type &comptonElectron);

    int particleDmlcIsReady() { return (ReturnFlagFromConstructor); }

    bool isUsed() { return (isParticleDmlcUsed); }; // Tells if the dmlc is used or not

    float aLeafXPosition(int iShape, int iLeaf) {
        return (ARR_VAL(dmlc->ALeafBank, iLeaf, iShape));
    };

    float bLeafXPosition(int iShape, int iLeaf) {
        return (ARR_VAL(dmlc->BLeafBank, iLeaf, iShape));
    };

    float leafYPosition(int iLeaf) {
        if (0 == iLeaf || (iLeaf > dmlc->num_leaves / 2)) { return (666); }
        // Simple hard-code for now.  Should search based on leafIndex then use the yEnd's to compute leaf center.
        // The Y=0 is between leaves 30 and 31
        // iLeaf = 1-10 and 51-60 are 1.0 cm leafs
        // iLeaf = 11-50 are 0.5 cm leafs
        if (iLeaf < 11) return (-20.5f + (float) iLeaf);
        if (iLeaf < 51) return (-9.5f + 0.5f * (float) (iLeaf - 11));
        return (+10.5f + (float) (iLeaf - 51));
    };

    int numApertures() { return (dmlc->numApertures); };

    float shapeIndex(int iShape) { return (dmlc->index[iShape]); };

    int nLeavesPerBank() { return (dmlc->num_leaves / 2); };

    void setIsArc(bool setValue) { isArc = setValue; };

    bool getIsArc() { return isArc; };

    void setIsJawTracking(bool setValue) { isJawTracking = setValue; };

    bool getIsJawTracking() { return isJawTracking; };

    bool getIsCPInfoSet() { return isCPInfoSet; };

    float getDmlcIndex(int iIndex) { return dmlc->index[iIndex]; };

    float getDmlcCollimatorAngle(int iIndex) { return dmlc->collimator[iIndex]; };

    void setnCP(int newnCP) { nCP = newnCP; };

    int getnCP() { return nCP; };

    void setDmlcFractionalMU(int index, float fMU) { dmlc->fractionalMUs[index] = fMU; };

    void setDebugLevel(int newLevel) { debugLevel = newLevel; };

    bool selectMLCSections(particle_type *part);

    //bool  selectMLCSections(Point_3d_float *pEnter, Point_3d_float *pExit, Point_3d_float *pCenter,
    //			int *rowIndex, float *fullSectionThickness, int *CurrentLineIndex, particle_type *part);
    bool readControlPointInfoFile(const char *controlPointInfoFile);

    bool readControlPointInfoFile();

    bool setFractionalMUs();

    bool validateDMLArc();

    // bool isArc;
    bool initializeCoordinateTransform();

    bool validateControlPoints();

    void checkIfArc();

    void checkIfJawTracking();

    keepType *keep;

    void enableTranslateToRoomSystem() { translateToRoomSystem = true; };

    void disableTranslateToRoomSystem() { translateToRoomSystem = false; };

    bool queryTranslateToRoomSystem() { return (translateToRoomSystem); };
protected:
    int writeDMLCFile(char *fileName, dmlc_type *dmlc);

    int checkPhysicalLeafGap(VarianMlcDataType *VarianMLC);

    int readVarianMLCTable(VarianMlcDataType *VarianMLC);

private:
    //
    int debugLevel; // for controlling output messages
    bool isCPInfoSet;      // True if is set, false if not
    bool isParticleDmlcUsed;     // True if used, false if not
    bool isArc;
    bool isJawTracking;
    bool translateToRoomSystem;  // output coordinate system
    string dmlFileName;   // file used by the mlc controller (.dml) file
    //
    char mlcName[MAX_STR_LEN], mu_file_name[MAX_STR_LEN], psFileName[MAX_STR_LEN], outputFileName[MAX_STR_LEN];
    int ReturnFlagFromConstructor;
    int nFoundWithSameIndex;
    //int intSeed
    unsigned nCompton;
    // unsigned seed,
    unsigned nProcessed;
    float coll; // default to max field size
    float maxPrimaryWt, totalPrimaryWt, maxScatWt;
    float comptonElectronWeight, comptonElectronEnergy, dz;
    // keepType* keep;
    dmlc_type *dmlc;
    russianRouletteType *rr;
    mlcType *machineMLC;
    table_type *E_mu, *E_mu_Compt;
    // Variables for MLC section stuff.  A better location might be part of mlcSection type, but this is not clear
    int rowIndex[N_MLC_SECTIONS];
    int currentLineIndex[N_MLC_SECTIONS];
    float fullSectionThickness[N_MLC_SECTIONS];
    Point_3d_float pEnter[N_MLC_SECTIONS], pExit[N_MLC_SECTIONS], pCenter[N_MLC_SECTIONS]; // coordinates of particle when enters, exits, and at center of MLC leaf
    //
    //  For dynamic arcs
    string controlPointFileName;
    int nCP; // number of control points
    controlPoint *CP;
    coordinateTransform *transform;

    //
    //      Original content of particleDmlc.h
    // function prototypes
    int read_dmlc_file(dmlc_type *dmlc);

    int readDmlcFile(); //
    int transport_phase_space(char *fname, mlcType *machineMLC, dmlc_type *d, dmlc_pdf_type *p,
                              table_type *E_mu, table_type *E_mu_Compt, russianRoulette *rr, keepType *keep);

    int get_pdf(dmlc_pdf_type *p, dmlc_type *d, mlcType *machineMLC, russianRoulette *rr);

    int read_mu_table(char *fname, table_type *E_mu, int LINE, float Density);

    int read_mu_water_table(char *fname, table_type *E_mu, int LINE);

    //int time_leaf(float *tleaf,int row, float pdf_grid_mid_pos[col],TWOD_ARRAY *leaf, float index_array[MAX_FIELD_NUM]);
    float dm_slope(TWOD_ARRAY *leaf, int row, int s, float delta_time);

    int trav_leaf(dmlc_pdf_type *p, dmlc_type *d, float pdf_grid_mid_pos[PDF_GRID_WIDTH],
                  float delta_time, float total_time, int row, int s);

    float crossings(float intl, float intr, float dtime);

    float get_thick(float inval);

    // int russian_roulette(ps_header_type* ps_hd_out, particle_type* part, FILE *psout, russianRoulette* rr);
    int russian_roulette(particle_type *part);

    int tip_thick(dmlc_pdf_type *p, float coll, mlcType *machineMLC);

    int
    tip_exp_thick(dmlc_pdf_type *p, int tiplet, float mu_over_w, float thick, float *act_weight, mlcType *machineMLC);

    float int_exp(float thick[NUM_INT + 1], float mu);

    int samp_tip_thick(dmlc_pdf_type *p, float thickleaf, float *samp_thick, mlcType *machineMLC);

    //
    // *********************************************************************** */
    // gatedmlc specific prototypes
    int read_dmlc_file(dmlc_type *dmlc, int NumSegments);

    int rescale(dmlc_type *d, int NumSegments);

    int addcosine(dmlc_type *d, float Amplitude, float Period, float LowLim, float UpLim,
                  float TreatTime, float ScalFac, int numLeaf);

    int write_dmlc_file(dmlc_type *d, float Amplitude, float Period);

    // end gated
    int readMLC(mlcType *mlc); //
    int convertDmlcCoordinatesToMlcLocation(); //
    int convertDmlcCoordinatesToIsocenterLocation(); //
    void reportMUIndexCounters();

    void zeroMUIndexCounters();


    //      End original content of particleDmlc.h    */
    int readParticleMLC(mlcType *mlc, char *mlcName);

    int readParticleMLC(mlcType *mlc); //
    int projectDMLCtoMLC(); //
    // float  getMLCThickness( float yValue, float leafWidthAtMLC, mlcType *machineMLC );
    // int processMLCParticle(particle_type *part, mlcType *machineMLC, dmlc_type *d);
    int processMLCParticle(particle_type *part, particle_type *scat, particle_type *comptonElectron);

    // int processMLCElectron(particle_type *part);// 11/08 mlcType *machineMLC, dmlc_type *dmlc);
    int createTwoSectionVarianTipTables(); // 11/08 mlcType *machineMLC);
    int debugLeafPositions(int rowIndex, dmlc_type *d);

    int checkLeafThickness(); //
    int getLeafIndexAndThickness(float pY, int nIndex, mlcIndexType *mlcIndex, int *leafIndex,
                                 float *fullSectionThickness, int &CurrentLineIndex);

    int getTipThickness(Point_3d_float *pEnterCurrentSection, Point_3d_float *pExitCurrentSection,
                        int beginPosition, float tipXPosition, tableType *tipPosition,
                        float *sectionThickness);

    int OldgetTipThickness(Point_3d_float *pEnter, Point_3d_float *pExit,
                           int beginPosition, float tipXPosition, tableType *tipPosition,
                           float *sectionThickness);

    int getMlcSectionThickness(Point_3d_float *pEnter, Point_3d_float *pExit,
                               Point_3d_float *pCenter, int rowIndex, float fullSectionThickness,
                               mlcSectionType *mlcSection, dmlc_type *d, int muIndex,
                               float fractionalMUs, float tipDistance, float *sectionThickness,
                               int nIndex);

    int forced_interaction(float thick, float mu_over_w, float *scat_thick);

    int scatteredPhotonAndElectron(particle_type *prim, particle_type *scat,
                                   particle_type *electron,
                                   float thick, float mu,
            // 11/08 table_type *E_mu, table_type *E_mu_Compt,
                                   float wt_not_int, float psdDistance);

    int scatteredPhoton(particle_type *prim, particle_type *scat,
                        float thick, float mu,
            // 11/08 table_type *E_mu, table_type *E_mu_Compt,
                        float wt_not_int, float psdDistance);

    void error_part(particle_type *p, particle_type *s);

    void error_part(particle_type *p);

    int sample_KN(particle_type *prim, particle_type *scat);

    int rotateByRandomPhi(particle_type *prim, particle_type *scat);

    int rotateByRandomPhi(particle_type *prim, particle_type *scat, particle_type *electron);

    int rotateByRandomPhi(particle_type *prim, particle_type *scat, float ws, int *dir);

    int createPdf(dmlc_pdf_type *p, int row);
    // int write_part(ps_header_type* h, particle_type* p, FILE *ostrm, russianRoulette* rr);

    int getComptonProducts(particle_type *prim, particle_type *scat, particle_type *electron);

    int readMLCIndexTable(int nLeaves, mlcSectionType *section);

    int readParticleMLCSection(clifObjectType *mlcSection, mlcSectionType *section);

    int readMLCSection(clifObjectType *mlcSection, tableType *profile);

    int printLeafTipSection(mlcSectionType *section);

    int createTipSection(tableType *tip, int nTipSections, float tipRadius,
                         float Angle, float maxTipThick);

    void InitPointers() { InitializeParticleDmlcPointersToNull(); };

    void InitializeParticleDmlcPointersToNull() {
        keep = NULL;
        dmlc = NULL;
        rr = NULL;
        machineMLC = NULL;
        E_mu = NULL;
        E_mu_Compt = NULL;
        CP = NULL;
        transform = NULL;
        mlcName[0] = mu_file_name[0] = psFileName[0] = outputFileName[0] = 0; //< Null terminate all the file name strings
    };

    void InitializePointersToMemory() { InitializeParticleDmlcPointersToMemory(); };

    void InitializeParticleDmlcPointersToMemory() {
        keep = new keepType;
        dmlc = new dmlc_type;
        rr = new russianRoulette;
        machineMLC = new mlcType;
    };

    bool simpleJawTracking(float, int, particle_type *);
    // end of particleDmlc.h
    // *********************************************************************** */
};

// function prototypes for use by all
FILE *openDMLCConfigFile(char *filename);

#endif
//
