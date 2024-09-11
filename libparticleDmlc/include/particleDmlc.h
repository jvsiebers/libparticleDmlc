/** \file 
 simple tag auto added to force generation of simple doxygen comments
**/
#ifndef particleDmlc_H
#define particleDmlc_H
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
   Nov 2004: IBM: grooveDistance now for each leaf.  Changes for new config files
   October 5, 2006: JVS: Add MaxTimeSamples to mlcType
   November 28, 2007: JVS: Move rest of function prototypes into this file
   Jan 12, 2015: JVS: Remove redundant things defined in typesClass.h
*/
/* *********************************************************************** */
#include "typedefs.h" // for definition of Point_2d_float
#include "common.h"   // for TWOD_ARRAY
#include "table.h"    // for table_type
#include "read_write_egs.h"  // for ps_header_type
#include "read_clif.h" // for clif_types
#include "typesClass.h"
/* *********************************************************************** */
/* ********************************************************************* */
/* *********************************************************************** */
// prototypes
int read_dmlc_file(dmlc_type* dmlc);
int readDmlcFile(dmlc_type* dmlc);
int transport_phase_space(char *fname, mlcType *machineMLC, dmlc_type* d, dmlc_pdf_type *p, table_type *E_mu, table_type *E_mu_Compt, russianRoulette* rr, keepType *keep);
int get_pdf(dmlc_pdf_type *p, dmlc_type* d, mlcType *machineMLC, russianRoulette* rr);
int read_mu_table(char *fname, table_type* E_mu, int LINE, float Density);
int read_mu_water_table(char *fname, table_type* E_mu, int LINE);
//int time_leaf(float *tleaf,int row, float pdf_grid_mid_pos[col],TWOD_ARRAY *leaf, float index_array[MAX_FIELD_NUM]);
float dm_slope(TWOD_ARRAY *leaf, int row, int s, float delta_time);
int trav_leaf(dmlc_pdf_type *p, dmlc_type* d, float pdf_grid_mid_pos[PDF_GRID_WIDTH], float delta_time, float total_time, int row, int s);
float crossings(float intl, float intr, float dtime);
float get_thick(float inval);
int russian_roulette(ps_header_type* ps_hd_out, particle_type* part, FILE *psout, russianRoulette* rr);
int write_part(ps_header_type* h, particle_type* p, FILE *ostrm, russianRoulette* rr);
int tip_thick(dmlc_pdf_type *p, float coll,mlcType *machineMLC);
int scatteredPhotonAndElectron(particle_type* prim, particle_type* scat, particle_type* electron,
                               float thick, float mu, table_type *E_mu, table_type *E_mu_Compt, 
			       float wt_not_int, float psdDistance);
int scatteredPhoton(particle_type* prim, particle_type* scat, 
                float thick, float mu, table_type *E_mu, table_type *E_mu_Compt, 
		    float wt_not_int, float psdDistance);
void error_part(particle_type *p, particle_type *s);
void error_part(particle_type *p);
int tip_exp_thick(dmlc_pdf_type *p, int tiplet, float mu_over_w, float thick, float *act_weight, mlcType *machineMLC );
float int_exp(float thick[NUM_INT+1],float mu);
int samp_tip_thick(dmlc_pdf_type *p, float thickleaf, float *samp_thick,mlcType *machineMLC);
// in scatter
int scattered_photon(ps_header_type* ps_hd_out, particle_type* part, particle_type* scat, FILE *psout, float thick, float mu, table_type *E_mu, table_type *E_mu_Compt, int *count, float wt_not_int, russianRoulette* rr);
int forced_interaction(float thick, float mu_over_w, float *scat_thick );
int sample_KN(particle_type* prim, particle_type* scat);
int rotateByRandomPhi(particle_type* prim, particle_type* scat);
int rotateByRandomPhi(particle_type* prim, particle_type* scat, float ws, int *dir);
int createPdf(dmlc_pdf_type *p, int row);

int readVarianMLCTable(VarianMlcDataType *VarianMLC);
/* *********************************************************************** */
// gatedmlc specific prototypes
int read_dmlc_file(dmlc_type* dmlc, int NumSegments);
int rescale(dmlc_type* d, int NumSegments);
int addcosine(dmlc_type* d, float Amplitude, float Period, float LowLim, float UpLim, float TreatTime, float ScalFac, int numLeaf);
int write_dmlc_file(dmlc_type* d, float Amplitude, float Period);


int readMLC(mlcType *mlc, char *mlcName);
int convertDmlcCoordinatesToMlcLocation(mlcType *machineMLC, dmlc_type *dmlc);
int writeDMLCFile(char *fileName, dmlc_type *dmlc);
int convertDmlcCoordinatesToIsocenterLocation(mlcType *machineMLC, dmlc_type *dmlc);
//
/* From dmlcRoutines.cc */
int projectDMLCtoMLC(mlcType *machineMLC, dmlc_type *dmlc);
float  getMLCThickness( float yValue, float leafWidthAtMLC, mlcType *machineMLC );
/* From readMLC.cc */
/* ******************************************************************* */
FILE *openDMLCConfigFile(char *filename);
int readMLCSection(clifObjectType *mlcSection, tableType *profile);
int readMLC(mlcType *mlc, char *mlcName);
int readPinnacleCurve(clifObjectType *Curve, int *npts, Point_2d_float **contour);
int readParticleMLCSection( clifObjectType *mlcSection, mlcSectionType *section);
int readParticleMLC(mlcType *mlc, char *mlcName);
int checkPhysicalLeafGap(VarianMlcDataType *VarianMLC);

int processMLCPhoton(particle_type *part, particle_type *scat, particle_type *comptonElectron, mlcType *machineMLC, dmlc_type *dmlc,
		       table_type *E_mu, table_type *E_mu_Compt);
int processMLCElectron(particle_type *part,
		       mlcType *machineMLC, dmlc_type *dmlc);

int dealWithIt(char *inputFileName, mlcType *machineMLC, dmlc_type* d, table_type* E_mu, table_type* E_mu_Compt, russianRoulette* rr, keepType* keep);
int createTwoSectionVarianTipTables(mlcType *machineMLC);
int scatteredPhoton(particle_type* prim, particle_type* scat, 
                float thick, float mu, table_type *E_mu, table_type *E_mu_Compt, 
		    float wt_not_int, float psdDistance);
int scatteredPhotonAndElectron(particle_type* prim, particle_type* scat, particle_type* electron,
                float thick, float mu, table_type *E_mu, table_type *E_mu_Compt, 
			       float wt_not_int, float psdDistance);
int debugLeafPositions(int rowIndex, dmlc_type *d);
int checkLeafThickness(mlcType *machineMLC, dmlc_type *dmlc);
// end of particleDmlc.h
/* *********************************************************************** */
#endif
