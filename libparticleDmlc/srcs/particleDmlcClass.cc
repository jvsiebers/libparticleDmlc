/** \file 
 simple tag auto added to force generation of simple doxygen comments
**/
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
/* particleDmlc.cc

  does DMLC on a particle by particle basis

Modification History:
   Dec 27, 2000: JVS: Created, with structures based on pjk dmlc program
   Jan 3, 2001: JVS:  ....
   Jan 11, 2001: JVS: "Left" and "Right" replaced with "ALeaves" and "Bleaves"
   Jan 17, 2001: JVS: Have "groove" accounted for in leaves
   Jan 18, 2001: JVS: Very minor bug fix in scatter code.....will increase
                      scatter by tiny amount...(had required both top and bottom
                      thicknesses to be non-zero, now just requires total thick)
   Jan 22, 2001: JVS: Trap for w > 1 (due to round off)
   Jan 25, 2001: JVS: Improve "Impossible" error message to track down 
                      non-repeatable error....
   May 9, 2001: JVS: Isolate fact that some particles (photons) can be in a 
                     zero thickness region in both the upper and lower sections.
                     This is rare.  (For phase space with 60000 original, 23600
                     after run particleDMLC, get 10 events.  Of these 10, 2 project to
                     huge radii in the field.  1/2360 *100 = 0.04% of particles...
                     These should be negligible. Solution would be to break the MLC
                     up into more sections (this would just further reduce the probability)
   June 28, 2001: 0.02: JVS: Add -mlcTable flag to use Varian mlcTable.txt file to look up positions
   July 12, 2001: 0.03: JVS: change casting in scatter.cc and !=0 to >0 for totalThickness here...
   July 17, 2001: JVS: Improve an error message
   August 30, 2001: JVS: Change to random number generator that uses a seed... add -seed flag...
   Nov 8, 2001: JVS: 0.05: Add Compton electrons (very crudely)
   Dec 19, 2001: JVS: 0.06: potentialGroove corrected for fullLeafThicknesses to always be on bottom.
   Feb 5, 2002: JVS: 0.07: add -mcvConfig ${UVA_CONFIG} to set env if it is passed...
   April 10, 2002: JVS: 0.08: Initialize comptonElectrons to DISCARD (since this code is NOT used)
                              Make DEFAULT to use table lookup mlctable.txt, must pass -mlcOffset to use offset
                              initialize scat.weight = 0.0;
   April 12, 2002: JVS: 0.09: Add physicalLeafOffset
   April 22, 2003: JVS: 0.10: Have read configs from $UVA_DATA/dmlc
                              No longer needs to mySetEnv for UVA_CONFIG
   Oct 24, 2003: JVS: 0.11: Works with egsnrc by using firstParticle
    01-Nov-2005: JVS: 0.12: Allow incident particle to have weight other than one...essentially requires that scattered particles produced
                    have weight *= weightFromBefore instead of = weightFromBefore...Note, had to add back grooveDistance and grooveThickness to mlcSection
                    structure......This should be fixed....
   11-Sept-2006: JVS: 0.20: Merged with IBM code...uses new config tables
   05-Oct-2006: JVS: 0.21: Add passing of nTimeSamples, change weightParticle to double,
                           and add advanceRandomStride, so each new particle starts with known part of the random sequence
   05-Feb-2007: JVS: 0.22: Allow passing -rrMaxPartWeight (though not really ever needed)
   07-Feb-2007: JVS: 0.23: Break off sub-routines and main...
   04-Feb-2008: JVS: 0.24: Create constructor that uses argc and argv so will directly parse command line args
   Nov 11, 2008: JVS: Remove warnings for c++ class 
                 "warning: declaration of 'someVariable' shadows a member of 'this'"
                 for routines 
		 checkLeafThickness()
		 convertDmlcCoordinatesToMlcLocation()
		 processMLCElectron()
		 processMLCPhoton()
		 readParticleMLC
		 readMLC
   Feb 22, 2012: JVS: Add in exception handling into constructor
   Dec 17-21, 2014: JVS: Major clean up of the constructor, add initializeParticleDMLC, elimination of replicate code.
                 replace processMLCElectron() and processMLCPhoton() with processMLCParticle()
   May 7, 2015: added d->staticMLC and muIndexNext so will work with static fields
   May 12, 2015: JVS: DynamicArc: when CP[0].fMUs != 0.0, error.
   July 22, 2015: JVS: When no -dml, ReturnFlagFromConstructor = OK
   Nov 12, 2015: JVS  ensure translatToRoom is initialized...
   Dec 31, 2015: JVS: Add initializeRandom -- to ensure random seed is set for particleDmlc
*/
/* *********************************************************************** */
/* *********************************************************************** */
// system libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> // for fabs 
#include <iostream> // for cout
// other libraries
#include "utilities.h"
#include "option.h"
#include "string_util.h"
#include "common.h"
#include "spline_data.h"
#include "read_write_egs.h"
#include "myrandom.h"
//local libraries
#ifdef C_CLASS
#include "particleDmlcClass.h"
#else

#include "particleDmlc.h"

#endif

#include "dynamicDelivery.h"
/* ***************************************************************************** */
#ifdef C_CLASS
using namespace std;
/* **************************************************************************** */
particleDmlc::particleDmlc(){
  //< Default Constructor does Nothing Nothing happening here...
  cout << "\n\t"<<__FUNCTION__ << " compiled on " << __DATE__ << endl;
  isParticleDmlcUsed = false;
  isCPInfoSet = false;
  translateToRoomSystem=false;
  InitializeParticleDmlcPointersToNull();
  debugLevel=0;
}
/* **************************************************************************** */
/**** Begin Constructor ****/
particleDmlc::particleDmlc(int *argc, char *argv[]) {
  //< Constructor that reads parameters from the input line
  isCPInfoSet = false;
  isArc=false;
  isJawTracking=false;
  translateToRoomSystem=false;
  ReturnFlagFromConstructor = FAIL;  
  InitializeParticleDmlcPointersToNull();
  debugLevel=0;
  cout << "\n\t"<<__FUNCTION__ << " compiled on " << __DATE__ << endl;
  //
  if( (OK != get_option("-debug",argc,argv,&debugLevel) ) ) {
    cout<<"\n ERROR:"<<__FUNCTION__<<" Reading -debug flag"<<endl;    ReturnFlagFromConstructor = FAIL; 
    throw " ERROR: particleDmlc::particleDmlc(argc,argv) : Reading -debug Flag";
  }
  if(debugLevel) cout << "=====debugLevel = " << debugLevel << "======================" << endl;

  // Read dmlc File name, 
  dmlFileName="";
  {
    if( (OK != get_option("-dml",argc,argv,&dmlFileName) ) ) {
      cout <<"\n ERROR"<<__FUNCTION__<<": Reading -dml flag"<<endl;    ReturnFlagFromConstructor = FAIL; 
      throw " ERROR: particleDmlc::particleDmlc(argc,argv) : Reading -dml Flag";
    }
    if(dmlFileName.empty()) {
      isParticleDmlcUsed = false;
      ReturnFlagFromConstructor = OK;
      return;
    } 
    if(debugLevel > 5) cout << "\tDetected -dml " << dmlFileName << ", reading the dmlc file\n";
  }
  //
  // Check if control point file exists.  If so, is Arc and read it
  controlPointFileName="";
  {
   if( (OK != get_option("-arc",argc,argv,&controlPointFileName) ) ) {
     cout << "\n ERROR:"<<__FUNCTION__<<" Reading -arc flag" << endl;    ReturnFlagFromConstructor = FAIL; 
      throw " ERROR: particleDmlc::particleDmlc(argc,argv) : Reading -arc Flag";
    }
    if(!controlPointFileName.empty()) {
      if(debugLevel > -1) cout << "\tDetected -arc " << controlPointFileName << ", reading the control point information file" << endl;
      isArc = true;
      if(!readControlPointInfoFile()){
    cout << " ERROR:"<<__FUNCTION__<<"::particleDmlc(,)" << endl;
    throw "ERROR: particleDmlc(,) constructor";
      }
    }
  }
  // see if just want to do translation
  if( OK == get_option("-room",argc,argv)){
    translateToRoomSystem=true;
  }
  // Do base initializations
  if(!initializeParticleDmlc()){
    cout << " ERROR: "<<__FUNCTION__<<": initializeParticleDmlc " << endl;
    throw "ERROR: dynamicDelivery(,) constructor";
  }
  if(nCP){
      checkIfArc();
  }
  // do special steps and checks for Arc
  if(isArc) {
    translateToRoomSystem=true;
    if(debugLevel>3) cout << "\t\tsetFractionalMUs()" << flush;
    if(!getIsCPInfoSet() && !setFractionalMUs()) {
      cout << " ERROR:"<<__FUNCTION__<<": setFractionalMUs " << endl;
      throw "ERROR: dynamicDelivery(,) constructor";
    }
    if(!validateDMLArc()) {
      cout << " ERROR: "<<__FUNCTION__<<" control points inconsistent with dml file " << controlPointFileName << endl;
      throw " ERROR: particleDmlc constructor ";
    }
    if(!validateControlPoints()) {
      cout << " ERROR: "<<__FUNCTION__<<" control points are invalid in " << controlPointFileName << endl;
      throw " ERROR: particleDmlc constructor ";
    }
  }
  if(translateToRoomSystem) {
    if(debugLevel>3) cout << "\t\tinitializeCoordinateTransform()"<<flush;
    if(!initializeCoordinateTransform()){
      cout << " ERROR:"<<__FUNCTION__<<" initializing coordinate transform " << endl;
      throw " ERROR: dynamicDelivery(,) constructor";
    }
  } 
  checkIfJawTracking();
  cout << "\t\t isJawTracking " << isJawTracking << endl;
  if(isJawTracking) cout << "\t\tJawTracking is enabled " << endl;
  // overwrite initialized values based on command line args
  if(!parseParticleDmlcArgs(argc,argv)) {
    cout << " ERROR:"<<__FUNCTION__<<" parsing particleDmlcArgs " << endl;
    throw "ERROR: particleDmlc::particleDmlc(int *, char *[])";
  }
  // Indicate all is well....
  if(debugLevel > 1) { cout << __FUNCTION__ << " setting ReturnFlagFromConstructor = OK" << endl;fflush(NULL);}
  ReturnFlagFromConstructor = OK;
}
/* ************************************************************************************************************** */
/* ************************************************************************************************************** */
particleDmlc::particleDmlc(const float InputrrWeight, const int maxTimeSamples, const char *mydmlFileName) {
  ReturnFlagFromConstructor = FAIL;
  InitializeParticleDmlcPointersToNull();
  isCPInfoSet = false;
  isArc=false;
  isJawTracking=false;
  translateToRoomSystem=false;
  debugLevel=0;

  if(!initializeParticleDmlc(mydmlFileName)) {
    cout << " ERROR: initializing particleDmlc" << endl;
    throw "ERROR: initilizing particleDmlc";
  }

  if(InputrrWeight > 0.) rr->rrWeight = InputrrWeight;
  printf("\n RR Parameters:  rrWeight = %f, rrMaxPartWeight = %f", rr->rrWeight, rr->rrMaxPartWeight);
  if(maxTimeSamples > 0) {
     machineMLC->maxTimeSamples = maxTimeSamples;
     printf("\n Setting random stride based on %d timeSamples", machineMLC->maxTimeSamples);
     // experimental testing shows up to 35 randoms sampled in scatter
     // so extend out to 50....
     setRandomStride(machineMLC->maxTimeSamples+50);
  }

  /* This was also in the old code, and at least the error check needs to be in the new code */
  /*
  // get z_plane distance of the phase space file (location where phase space written)
  dmlc->psdZPlane = 45.0; //default for just jaws
  if( (get_option("-z_plane",argc,argv,&dmlc->psdZPlane)!=OK) )
  {
    printf("\n ERROR: getting a plane\n");
    usage(); return(FAIL);
  }
  if (dmlc->psdZPlane > machineMLC->distance)
  {
    printf("\n ERROR: psdZPlane (%f) further downstream than MLC (%f)",dmlc->psdZPlane,machineMLC->distance);
    usage(); return(FAIL);
  }
  */
  ReturnFlagFromConstructor = OK;
  cout << "\nExiting particleDmlcConstructor with return flag = " << ReturnFlagFromConstructor << endl; fflush(stdout);
}
/**** End Constructor ****/
/* ********************************************************************************************************** */
bool particleDmlc::parseParticleDmlcArgs(int *argc, char *argv[])
{
  //
  if(debugLevel>3) cout << "\t particleDmlc::parseParticleDmlcArgs " << flush;
  // Set range-rejection parameters based on passed arguments
  if( OK != get_option("-rrWeight",argc,argv,&rr->rrWeight) || rr->rrWeight <=0.0 ){
    printf("\n ERROR: Reading -rrWeight flag or invalid rr weight\n");  return(false);
  }
  //
  if(OK != get_option("-nTimeSamples", argc, argv,&machineMLC->maxTimeSamples)) {
    printf("\n ERROR: getting -nTimeSamples\n"); return(false);
  }
  // must reset random stride if maxTimeSamples was reset
  setRandomStride(machineMLC->maxTimeSamples+50); ///Random Stride is used allow known random number sequence when skip particles
  //
  /// Load the keep structure which determines particles to keep with defaults
  keep->electron = keep->primOpen = keep->primLeaf = keep->primTip = keep->scatLeaf = keep->scatTip = keep->comptonElectron = DISCARD;
  if( get_option("-comptonElectron",argc,argv)  == OK){
    printf("\n\t Compton Electrons Are Included");
    keep->comptonElectron = KEEP;
  }
  if( get_option("-electron",argc,argv) == OK){
    printf("\n\t Running electrons (and positrons)");
    keep->electron = KEEP;
  }
  if( get_option("-primary",argc,argv)  == OK){
    printf("\n\t Running primary leaf particles");
    keep->primLeaf = KEEP;
  }
  if( get_option("-scatter",argc,argv)  == OK){
    printf("\n\t Running scattered leaf particles");
    keep->scatLeaf = KEEP;
  }
  // if no flags, default is to keep all particles
  if ( (keep->electron == DISCARD) && (keep->primOpen ==  DISCARD) &&
       (keep->primLeaf == DISCARD) && (keep->primTip  ==  DISCARD) &&
       (keep->scatLeaf == DISCARD) && (keep->scatTip  ==  DISCARD) ){
       cout << "\t Running with all particle types " << endl;
       keep->electron = keep->primOpen = keep->primLeaf = keep->primTip = keep->scatLeaf = keep->scatTip = KEEP;
  }
  //
  return(true);
}
/* **************************************************************************** */
bool particleDmlc::initializeParticleDmlc(bool isThisArc, const char *tmpdmlFileName) {

  isArc=isThisArc;
  dmlFileName=tmpdmlFileName; //< Set the internal name of the dml file
  //
  return(initializeParticleDmlc());
}
/* **************************************************************************** */
bool particleDmlc::initializeParticleDmlc(){
  cout << "\n\t"<<__FUNCTION__ << " compiled on " << __DATE__ << endl;
  if(debugLevel>1) cout << "\tparticleDmlc::initializeParticleDmlc() reading " << dmlFileName << endl;
  // Allocate memory for everybody
  InitializeParticleDmlcPointersToMemory();
  // read dml file
  if (OK != readDmlcFile()) {
    cerr << "\n ERROR: particleDmlc:: Reading dml file " << dmlc->dmlc_file_name << endl; return(false);
  }
  //
  // read the config file used for particleDmlc for the MLC being used
  sprintf(mlcName,"%d",dmlc->num_leaves);
  printf("\n\t\t Reading Configuration Data for %s MLC  ", mlcName);
  if(OK != readParticleMLC(machineMLC)){
    printf("\n ERROR: Reading in MLC for machine\n"); return(false);
  } else if( OK != convertDmlcCoordinatesToMlcLocation()){
    printf("\n ERROR: Projecting DMLC to MLC\n"); return(false);
  } else if(OK != createTwoSectionVarianTipTables()){
    printf("\n ERROR: Creating Tip Tables\n"); return(false);
  }
  //
  // cout << __FUNCTION__ << " machineMLC->density = " << machineMLC->density << endl;
  /// Set default range rejection parameters....
  rr->totalParticlesToWrite = 0;
  rr->numParticlesToPlayRR = 0;
  rr->numParticlesThatLive = 0;
  rr->rrMaxPartWeight = 0.10f;
  rr->rrWeight = 1.0f;
  //
  // allow user to set maxTimeSamples (maybe should be in config file?)
  machineMLC->maxTimeSamples = 20; // Set default...Should it be 20 or 100?
  // set random stride based on nTimeSamples, Oct 5, 2006
  // experimental testing shows up to 35 randoms sampled in scatter
  // so extend out to 50....
  setRandomStride(machineMLC->maxTimeSamples+50); ///Random Stride is used to keep particle using same random number every time
  // \todo : Get a better way to get/use random stride.   Currently, probably wasting alot of time with this.
  //
  /// Read in attenuation coefficient data
  {
    /// \todo : Get rid of the PK_CAL and table_type and use a proper c++ class
    // allocate the memory for attenuation coefficient
    E_mu = (table_type *) calloc(1,sizeof(table_type)); //< JVS: replace PK_CAL with what it really dose // PK_CAL(table_type);
    if (E_mu == NULL){
      printf("\n ERROR: memory allocation for E_mu\n"); return(false);
    }
    // read in the table of attenuation coeffients
    if (OK!=read_mu_table(mu_file_name,E_mu,MU_LINE,machineMLC->density)){
      printf("\n ERROR: Getting reading mu\n");return(false);
    }
    // allcate memory for compton
    E_mu_Compt = (table_type *) calloc(1,sizeof(table_type)); //< JVS: replace PK_CAL with what it really dose PK_CAL(table_type);
    if (E_mu_Compt == NULL){
      printf("\n ERROR: memory allocation for E_mu_Compt\n"); return(false);
    }
    // read in the table of attenuation coeffients for Compton Scattering
    if (OK!=read_mu_table(mu_file_name,E_mu_Compt,MU_COMPTON_LINE,machineMLC->density)){
      printf("\n ERROR: Getting reading mu Compt\n");return(false);
    }
  }
  maxPrimaryWt = -1.0;
  maxScatWt    = -1.0;
  totalPrimaryWt = 0.0;
  nProcessed = 0;
  nFoundWithSameIndex = 0;
  comptonElectronEnergy=0.0;
  comptonElectronWeight=0.0;
  nCompton=0;
  isParticleDmlcUsed = true;
  cout << "\n\t" << __FUNCTION__ << " initializing random number generator based on getSeed() (time) " << endl;
  initializeRandom(getSeed()); // default, initialize random number generator to use the time as a seed.
  if(debugLevel>3) cout <<"\t----------------initializeParticleDmlc completed----------------"<<flush;
  return(true);
}
/* ************************************************************************************************************** */
/**** Begin Destructor ****/
particleDmlc::~particleDmlc()
{
  if(!isParticleDmlcUsed) {
    printf("\n ~particleDmlc: particleDMLC was NOT used\n");return;
  }
  // print out a few things before exit and destroy (for now, till verify not done elsewhere)
  printf("\n For RR of %d particles, %d were below the RR weight of %f (RR input (%f) times max rrMaxPartWeight (%f))",rr->totalParticlesToWrite,rr->numParticlesToPlayRR,rr->rrWeight*rr->rrMaxPartWeight,rr->rrWeight,rr->rrMaxPartWeight);

  if (rr->numParticlesToPlayRR>0){
    printf("\n %d of %f survived the RR game",rr->numParticlesThatLive, ((float) rr->numParticlesThatLive)/((float) rr->numParticlesToPlayRR));
    //    printf("\n Scat_count will be erroneous if using RR");
  }
  // information about the number of random numbers samples
  printNRandomsSampled();
  printf("\n the next random number should be");  printf(" %f", randomOne());
  // information about number that bug was avoided for
  printf("\n Number of samples found where index was the same for muIndex and muIndex+1 = %d (representing sampling fractionalMU=0 AND 1st and 2nd segment at 0 MUs", nFoundWithSameIndex);
  //
  if(debugLevel > 2) reportMUIndexCounters();
  if(debugLevel > 2) { cout << "\nDestroying particleDmlc Class...."<< endl; fflush(stdout); }
  if(isArc){
    delete [] CP;
  }
  if(translateToRoomSystem) {
    delete transform;
  }
  if(NULL != E_mu){
    free(E_mu);
    E_mu = NULL;
  }
  if(NULL != E_mu_Compt){
    free(E_mu_Compt);
    E_mu_Compt = NULL;
  }
  if(debugLevel > 2) { cout << "\tClearing memory for the machine MLC sections..." << endl; fflush(stdout);}
  //clear memory
  //  free(machineMLC->profile);
  if(NULL != machineMLC){
    for(int iSection=0; iSection<machineMLC->nSections; iSection++){
      if(NULL != machineMLC->section[iSection].leadingTip.points){
    free(machineMLC->section[iSection].leadingTip.points);
    machineMLC->section[iSection].leadingTip.points = NULL;
      }
      if(NULL != machineMLC->section[iSection].followingTip.points){
    free(machineMLC->section[iSection].followingTip.points);
    machineMLC->section[iSection].followingTip.points = NULL;
      }
      if(NULL != machineMLC->section[iSection].index){
    free(machineMLC->section[iSection].index);
    machineMLC->section[iSection].index = NULL;
      }
    }
    if(NULL != machineMLC->section){
      free(machineMLC->section);
      machineMLC->section = NULL;
    }
    delete machineMLC;
    machineMLC = NULL;
  }
  if(debugLevel > 2) { cout << "\tClearing memory for dmlc" << endl; fflush(stdout); }
  //  delete dmlc;
  if(NULL != dmlc){
    delete dmlc;
    dmlc = NULL;
  }
  if(debugLevel > 2) { cout << "\tClearing memory for rr" << endl; fflush(stdout); }
  if(NULL != rr){
    delete rr;
    rr = NULL;
  }
  if(debugLevel > 2) { cout << "\tClearing memory for keep" << endl; fflush(stdout); }
  if(NULL != keep) delete keep;
  if(debugLevel > 2) { cout << "....completed destroying particleDmlc Class\n"; fflush(stdout); }
}
/**** End Destructor ****/
/* **************************************************************************** */
int particleDmlc::MLCTransport(particle_type &part, particle_type &scat,
                               particle_type &comptonElectron)
{
  nProcessed++;
  if ( part.w > 0.0000000f ) {                // avoid divide by zero and backwards particles...
      scat.weight=comptonElectron.weight=0.0f; // initialize scatter weight to zero...so not hang around from last particle if totalThickness == 0
      scat.firstParticle=0;                    // Scatter particle cannot be a primary!
      comptonElectron.firstParticle = 0;       // Compton electron cannot be a primary!
      //
      // transport the particle
      if (OK != processMLCParticle(&part, &scat, &comptonElectron)) { //
         printf("\n ERROR: Processing Particle %d", nProcessed);
         printf("\t( x %g y %g z %g u %g v %g w %g)", part.x, part.y, part.z, part.u, part.v, part.w);return(FAIL);
      }
  }
  // Apply RussianRoulette to see who survives
  if(part.weight > 0.0f)  // Feb 13, 2007: JVS: Change to > 0 instead of >= 0.0 since don't waste time roulette 0 weight particles since they are not  roulette will just assign their weight to 0 anyway
    if(OK != russian_roulette(&part )) {
      cerr << "ERROR: Performing Russian Roulette\n"; return(FAIL);
    }
  if(scat.weight > 0.0f)
    if(OK != russian_roulette(&scat )) {
      cerr << "ERROR: Performing Russian Roulette\n"; return(FAIL);
    }
  if(comptonElectron.weight > 0.0f)
    if(OK != russian_roulette(&comptonElectron)) {
      cerr << "ERROR: Performing Russian Roulette\n"; return(FAIL);
    }

  advanceRandomToStride(); /// Advance the random number generator till hits the stride length
  /// advanceRandomToStride is done to ensure each source particle uses the same # of random #'s
  /// which will help keep correlations when the same source particles are used for similar 
  /// dmlc files
  return(OK);
}
#endif // C_CLASS
/* ***************************************************************************** */
#ifdef C_CLASS
int particleDmlc::getLeafIndexAndThickness(float pY, int nIndex, mlcIndexType *mlcIndex, int *leafIndex,
#else

int getLeafIndexAndThickness(float pY, int nIndex, mlcIndexType *mlcIndex, int *leafIndex,
#endif
                             float *sectionThickness, int &currentLeafIndex) {
// Feb 24, 2005: IBM: added int &currentLineIndex neccessary for getMlcSectionThickness(...);

    /*  returns leafIndex=-1 if outside of the MLC */
    int tmpIndex = 0;
    // Advance point in structure
    if ((pY < mlcIndex[0].yEnd - 1.0) || // too far before 1st leaf
        (pY > mlcIndex[nIndex - 1].yEnd)) // beyond last leaf
    {
        *fullSectionThickness = 10.0;
        *leafIndex = -1;
        currentLeafIndex = -1;
        return (OK);
    }
    while (mlcIndex[tmpIndex].yEnd < pY && ++tmpIndex < nIndex);
    if (tmpIndex == nIndex) {
        printf("\n ERROR: Cannot find lead index for py=%f", pY);
        printf("\n \t Maximum Leaf Y is %f", mlcIndex[nIndex - 1].yEnd);
        return (FAIL);
    }
    // if(tmpIndex) tmpIndex--;  // so is between i and i+1
//#ifdef DEBUG_DMLC
    if (tmpIndex) {
        if (!((pY <= mlcIndex[tmpIndex].yEnd) && (pY >= mlcIndex[tmpIndex - 1].yEnd))) {
            printf("\n ERROR: Indexing Problem");
            printf("\n \t mlcIndex[i-1] %f pY %f mlcIndex[i] %f", mlcIndex[tmpIndex - 1].yEnd, pY,
                   mlcIndex[tmpIndex].yEnd);
            return (FAIL);
        }
    }
//#endif
    *leafIndex = mlcIndex[tmpIndex].leafIndex;
    *sectionThickness = mlcIndex[tmpIndex].thickness;
    currentLeafIndex = tmpIndex;
    return (OK);
}
/* *************************************************************** */
#ifdef C_CLASS
int particleDmlc::getTipThickness(
#else

int getTipThickness(
#endif
        Point_3d_float *pEnterCurrentSection, Point_3d_float *pExitCurrentSection,
        int beginPosition,
        float tipXPosition,
        tableType *tipPosition,
        float *sectionThickness) {
    // int beginPosition -- tells if started inside or leaf outside of leaf...
    /* tipPosition is an array of the positions on the tips,  these have to be
       offset by the tipXPositions....Note, code could be sped up by saving
       the slopes and intercepts of each segment */
    /* Shift particle by X (part->x - tipXPosition) before find slope
       of the line, then not need to shift tip at all..... */
#ifdef DEBUG_TIP
    printf("\n tipXPosition (%f) pEnterCurrentSection->x (%f) pExitCurrentSection->x (%f)", tipXPosition, pEnterCurrentSection->x, pExitCurrentSection->x);
    printf("\n Relative Positions xEnter (%f) xExit (%f)", pEnterCurrentSection->x-tipXPosition, pExitCurrentSection->x-tipXPosition);
    printf("\n pEnterCurrentSection %f pExit %f tip[0] %f tip[n] %f (%d)", pEnterCurrentSection->z, pExitCurrentSection->z, tipPosition->points[0].y,
       tipPosition->points[(tipPosition->npts)-1].y, tipPosition->npts);
#endif
    // Shift Particle
    //float xEnterRelative = pEnterCurrentSection->x-tipXPosition;
    float xExitRelative = pExitCurrentSection->x - tipXPosition;
    float mParticle = 0.0;    // slope of equation of motion for the particle
    float xIntercept = 0.;    // intercept
    /* Get equation of line for the particle */
    if (pExitCurrentSection->x == pEnterCurrentSection->x) {
        xIntercept = xExitRelative;
    } else {
        mParticle =
                (pExitCurrentSection->z - pEnterCurrentSection->z) / (pExitCurrentSection->x - pEnterCurrentSection->x);
    }
    float bParticle = pExitCurrentSection->z - mParticle * xExitRelative;
#ifdef DEBUG_TIP
    printf("\n Particle: y = %f * x + %f ", mParticle,bParticle);
#endif
    /* Loop over Tip Segment */
    for (int iTip = 1; iTip < tipPosition->npts; iTip++) {
        /* Determine slope and intercept of the tip segment!*/
        float mTip = (tipPosition->points[iTip].y - tipPosition->points[iTip - 1].y) /
                     (tipPosition->points[iTip].x - tipPosition->points[iTip - 1].x);
        float bTip = tipPosition->points[iTip].y - mTip * (tipPosition->points[iTip].x);
#ifdef DEBUG_TIP
        printf("\n TipX: %f %f", tipPosition->points[iTip-1].x, tipPosition->points[iTip].x);
        printf("\n TipY: %f %f", tipPosition->points[iTip-1].y, tipPosition->points[iTip].y);
        printf("\n mTip %f bTip %f", mTip, bTip);
#endif
        /* Determine if intersects this line segment */
        if (mParticle != mTip) { // cannot intersect if slopes are the same, skimming considered a miss!
            if (pExitCurrentSection->x != pEnterCurrentSection->x)
                xIntercept = (bTip - bParticle) / (mParticle - mTip);
            float zIntercept = mTip * xIntercept + bTip;
#ifdef DEBUG_TIP
            printf("\n xIntercept = %f", xIntercept);
            printf("\n zIntercept = %f", zIntercept);
#endif
#define SMALL_FLOAT_LIMIT 0.000005
            if (((tipPosition->points[iTip - 1].y < zIntercept)
                 == (zIntercept < tipPosition->points[iTip].y)) ||
                (  /* Check for particles that are within roundoff error of the tip */
                        (fabs(tipPosition->points[iTip - 1].y - zIntercept) < SMALL_FLOAT_LIMIT) ||
                        (fabs(tipPosition->points[iTip].y - zIntercept) < SMALL_FLOAT_LIMIT))) {  // Intercept Found !!!
                // Determine if thickness is wrt pEnterCurrentSection or pExit
                if (beginPosition == INSIDE_LEAF)
                    *sectionThickness = fabs(zIntercept - pEnterCurrentSection->z);
                else
                    *sectionThickness = fabs(pExitCurrentSection->z - zIntercept);
#ifdef DEBUG_TIP
                printf("\n .....Returning Tip Thickness = %f", *sectionThickness);
#endif
                return (OK);
            }
        }
    }
    /* No intersection found, thickness = 0 (missed tip) */
    *sectionThickness = 0.0;
    return (OK);
}
/* *********************************************************************** */
/* *********************************************************************** */
#ifdef C_CLASS
int particleDmlc::debugLeafPositions(int localRowIndex, dmlc_type *d)
#else

int debugLeafPositions(int localRowIndex, dmlc_type *d)
#endif
{
    FILE *oStrm = fopen("debugLeaves.dat", "w");
    if (oStrm == NULL) {
        printf("\n ERROR: Opening file debugLeaves.dat");
        return (FAIL);
    }
    int muIndex = 0;
    int nTimeSlices = 100;
    if (d->staticMLC) nTimeSlices = 1;
    for (int iTime = 0; iTime < nTimeSlices; iTime++) {
        float fractionalMUs = (float) iTime / (float) nTimeSlices;
        printf("\n FractionalMUs=%f", fractionalMUs);
        while (fractionalMUs > d->fractionalMUs[muIndex] && muIndex < d->numApertures) muIndex++;
        if (muIndex) muIndex--;  // decrement so now is between muIndex and muIndex+1
        //      printf("\n MU(%d) %f  %f  %f", muIndex, dmlc->fractionalMUs[muIndex], fractionalMUs, dmlc->fractionalMUs[muIndex+1]);
        if (muIndex > d->numApertures) {
            printf("\n ERROR: muIndex (%d) exceeded number of shapes %d", muIndex, d->numApertures);
            return (FAIL);
        }
        int muIndexNext = muIndex + 1;
        if (d->staticMLC) muIndexNext = muIndex;
        // NOTE: (ALeafBank) "A" leaves are normally the "Leading" leaves
        float positionLeading = (fractionalMUs - d->fractionalMUs[muIndex]) /
                                (d->fractionalMUs[muIndexNext] - d->fractionalMUs[muIndex]) *
                                (ARR_VAL(d->ALeafBank, localRowIndex, muIndexNext) -
                                 ARR_VAL(d->ALeafBank, localRowIndex, muIndex)) +
                                ARR_VAL(d->ALeafBank, localRowIndex, muIndex);
        float positionFollowing = (fractionalMUs - d->fractionalMUs[muIndex]) /
                                  (d->fractionalMUs[muIndexNext] - d->fractionalMUs[muIndex]) *
                                  (ARR_VAL(d->BLeafBank, localRowIndex, muIndexNext) -
                                   ARR_VAL(d->BLeafBank, localRowIndex, muIndex)) +
                                  ARR_VAL(d->BLeafBank, localRowIndex, muIndex);
        if (positionLeading < positionFollowing) {
            printf("\n ERROR: debugLeafPositions");
            printf("\n\t ERROR: positionLeading (%f) < positionFollowing (%f)", positionLeading, positionFollowing);
            printf("\n\t ERROR: delta = %g", positionLeading - positionFollowing);
            printf("\n\t ERROR: fractionalMUs = %f, muIndex = %d", fractionalMUs, muIndex);
            printf("\n\t A Leaf: %f %f", ARR_VAL(d->ALeafBank, localRowIndex, muIndexNext),
                   ARR_VAL(d->ALeafBank, localRowIndex, muIndexNext));
            printf("\n\t B Leaf: %f %f", ARR_VAL(d->BLeafBank, localRowIndex, muIndexNext),
                   ARR_VAL(d->BLeafBank, localRowIndex, muIndexNext));
            return (FAIL);
        }
        fprintf(oStrm, "%f %f %f\n", fractionalMUs, positionLeading, positionFollowing);
    }
    fclose(oStrm);
    printf("\n debugging data written to debugLeaves.dat");
    return (OK);
}
/* ********************************************************************** */
#ifdef C_CLASS
int particleDmlc::getMlcSectionThickness(Point_3d_float *pEnterCurrentSection,
#else

int getMlcSectionThickness(Point_3d_float *pEnterCurrentSection,
#endif
                           Point_3d_float *pExitCurrentSection,
                           Point_3d_float *pCenterCurrentSection,
                           int rowIndexCurrentSection,
                           float fullThicknessCurrentSection,
                           mlcSectionType *mlcSection,
                           dmlc_type *d,
                           int muIndex,
                           float fractionalMUs,
                           float tipDistance,
                           float *sectionThickness, int nIndex) {
    if (fullThicknessCurrentSection == 0.0) {
        *sectionThickness = 0.0;
        return (OK); // immediate return if thickness = 0;
    }
    int muIndexNext = muIndex + 1;
    if (d->staticMLC) muIndexNext = muIndex;
    // Determine if particle is in the leading leaf, following leaf, or in the "open" area between the leaves.
    // Get Leading Leaf Position
    float segmentMUFraction;
    // 10/21/2011: JVS: Add check of d->fractionalMUs[muIndex], since, somehow, is getting to this point with same index....
    if (!d->staticMLC && d->fractionalMUs[muIndex] == d->fractionalMUs[muIndexNext]) {
        printf("\n ERROR:  particleDmlc::getMlcSectionThickness : d->fractionalMUs[%d] == d->fractionalMUs[%d] = %f",
               muIndex, muIndexNext, d->fractionalMUs[muIndex]);
        return (FAIL);
    }
    if (d->staticMLC) segmentMUFraction = d->fractionalMUs[muIndex]; // should be 1
    else
        segmentMUFraction = (fractionalMUs - d->fractionalMUs[muIndex]) /
                            (d->fractionalMUs[muIndexNext] - d->fractionalMUs[muIndex]);
    // if(!muIndex && fractionalMUs < d->fractionalMUs[0]) segmentMUFraction=0.0f; // Dynamic Arc fMUs for CP[0] -- beam is static at CP[0] till fMU[0] is reached -- false
    float positionLeading = segmentMUFraction *
                            (ARR_VAL(d->ALeafBank, rowIndexCurrentSection, muIndexNext) -
                             ARR_VAL(d->ALeafBank, rowIndexCurrentSection, muIndex)) +
                            ARR_VAL(d->ALeafBank, rowIndexCurrentSection, muIndex);
    float positionFollowing = segmentMUFraction *
                              (ARR_VAL(d->BLeafBank, rowIndexCurrentSection, muIndexNext) -
                               ARR_VAL(d->BLeafBank, rowIndexCurrentSection, muIndex)) +
                              ARR_VAL(d->BLeafBank, rowIndexCurrentSection, muIndex);

//
#ifdef DEBUG_A_BUNCH
    printf("\n fMU %g idx %d %g pl %g pf %g", fractionalMUs, muIndex, d->fractionalMUs[muIndex], positionLeading,positionFollowing);
    printf("\n rowIndex = %d", rowIndex);
    printf("\n fMU %f muIdx %f muIdx+1 %f", fractionalMUs, d->fractionalMUs[muIndex], d->fractionalMUs[muIndexNext]);
    printf(" aL+1 %f aL %f",ARR_VAL( d->ALeafBank, rowIndexCurrentSection, muIndexNext) ,ARR_VAL( d->ALeafBank, rowIndexCurrentSection, muIndex));
    printf(" bL+1 %f bL %f",ARR_VAL( d->BLeafBank, rowIndexCurrentSection, muIndexNext) ,ARR_VAL( d->BLeafBank, rowIndexCurrentSection, muIndex));
#endif
    if (positionLeading < positionFollowing) {
        if (fabs(positionLeading - positionFollowing) < 1.0e-5) { // Feb 16, 2007: Check if just round off
            positionLeading = positionFollowing;  // is just round off, set to be equal and go ahead
        } else { // this is a real error , so we need to deal with it
            printf("\n ERROR: %s:  for leaf %d", __FUNCTION__, rowIndexCurrentSection + 1);
            printf("\n\tERROR: positionLeading < positionFollowing: %f %f", positionLeading, positionFollowing);
            printf("\n\tdelta = %g", positionLeading - positionFollowing);
            printf("\n\tsegmentMUFraction = %g", segmentMUFraction);
            printf("\n\tmuIndex = %d, muIndexNext = %d", muIndex, muIndexNext);
            printf("\n\tfractionalMUs = %f, fMUs[muIndex] = %f, fMUs[muIndexNext] = %f", fractionalMUs,
                   d->fractionalMUs[muIndex], d->fractionalMUs[muIndexNext]);
            printf("\n\tA Leaf: %f->%f", ARR_VAL(d->ALeafBank, rowIndexCurrentSection, muIndex),
                   ARR_VAL(d->ALeafBank, rowIndexCurrentSection, muIndexNext));
            printf("\n\tB Leaf: %f->%f", ARR_VAL(d->BLeafBank, rowIndexCurrentSection, muIndex),
                   ARR_VAL(d->BLeafBank, rowIndexCurrentSection, muIndexNext));
            return (FAIL);
        }
    }
    float leadingSectionThickness = 0.0f;
    float followingSectionThickness = 0.0f;
    // Get the Thickness for this section
    if (pEnterCurrentSection->x > positionLeading) {
        if (pEnterCurrentSection->x < positionLeading + tipDistance) {
            if (getTipThickness(pEnterCurrentSection, pExitCurrentSection, OUTSIDE_LEAF, positionLeading,
                                &mlcSection->leadingTip, &leadingSectionThickness) != OK) {
                printf("\n ERROR: Getting tip thickness");
                return (FAIL);
            }
            // particle sees both leaves
            if (pExitCurrentSection->x < positionFollowing) {
                if (getTipThickness(pEnterCurrentSection, pExitCurrentSection, OUTSIDE_LEAF, positionFollowing,
                                    &mlcSection->followingTip, &followingSectionThickness) != OK) {
                    printf("\n ERROR: Getting tip thickness");
                    return (FAIL);
                }
            }
            *sectionThickness = leadingSectionThickness + followingSectionThickness;
        } else {
            if (pExitCurrentSection->x > positionLeading) {
                *sectionThickness = fullThicknessCurrentSection;
                if ((pCenterCurrentSection->x > positionLeading + mlcSection->index[nIndex].grooveDistance)) // {
                    *sectionThickness -= mlcSection->index[nIndex].grooveThickness;
            } else {
                if (getTipThickness(pEnterCurrentSection, pExitCurrentSection, INSIDE_LEAF, positionLeading,
                                    &mlcSection->leadingTip, &leadingSectionThickness) != OK) {
                    printf("\n ERROR: Getting tip thickness");
                    return (FAIL);
                }
                // Check if hits following leaf too
                if (pExitCurrentSection->x < positionFollowing) {
                    if (getTipThickness(pEnterCurrentSection, pExitCurrentSection, OUTSIDE_LEAF, positionFollowing,
                                        &mlcSection->followingTip, &followingSectionThickness) != OK) {
                        printf("\n ERROR: Getting tip thickness");
                        return (FAIL);
                    }
                }
                *sectionThickness = leadingSectionThickness + followingSectionThickness;
            }
        }
    } else if (pEnterCurrentSection->x < positionFollowing) {
        if (pEnterCurrentSection->x > positionFollowing - tipDistance) {
            if (getTipThickness(pEnterCurrentSection, pExitCurrentSection, OUTSIDE_LEAF, positionFollowing,
                                &mlcSection->followingTip, &followingSectionThickness) != OK) {
                printf("\n ERROR: Getting tip thickness");
                return (FAIL);
            }
            // Check if hits leading leaf too
            if (pExitCurrentSection->x > positionLeading) {
                if (getTipThickness(pEnterCurrentSection, pExitCurrentSection, OUTSIDE_LEAF, positionLeading,
                                    &mlcSection->leadingTip, &leadingSectionThickness) != OK) {
                    printf("\n ERROR: Getting tip thickness");
                    return (FAIL);
                }
            }
            *sectionThickness = leadingSectionThickness + followingSectionThickness;
        } else {
            if (pExitCurrentSection->x < positionFollowing) {
                *sectionThickness = fullThicknessCurrentSection;
                if (pCenterCurrentSection->x < positionFollowing - mlcSection->index[nIndex].grooveDistance)  // {
                    *sectionThickness -= mlcSection->index[nIndex].grooveThickness;
            } else {
                if (getTipThickness(pEnterCurrentSection, pExitCurrentSection, INSIDE_LEAF, positionFollowing,
                                    &mlcSection->followingTip, &followingSectionThickness) != OK) {
                    printf("\n ERROR: Getting tip thickness");
                    return (FAIL);
                }
                // Check if hits following leaf too
                if (pExitCurrentSection->x > positionLeading) {
                    if (getTipThickness(pEnterCurrentSection, pExitCurrentSection, OUTSIDE_LEAF, positionLeading,
                                        &mlcSection->leadingTip, &leadingSectionThickness) != OK) {
                        printf("\n ERROR: Getting tip thickness");
                        return (FAIL);
                    }
                }
                *sectionThickness = leadingSectionThickness + followingSectionThickness;
            }
        }
    } else {
        // positionEnter=OPEN_AREA;//printf("\n Enters In Open Area Between Leaves");
        if (pExitCurrentSection->x <= positionLeading &&
            pExitCurrentSection->x >= positionFollowing) {
            *sectionThickness = 0.0;
        } else {  // Entered in open area, exited in leaf...must figure which tip to use
            if (pExitCurrentSection->x > positionLeading) {
                // NOTE: I ignore traveling through both tips in one section!!!
                if (getTipThickness(pEnterCurrentSection, pExitCurrentSection, OUTSIDE_LEAF, positionLeading,
                                    &mlcSection->leadingTip, sectionThickness) != OK) {
                    printf("\n ERROR: Getting tip thickness");
                    return (FAIL);
                }
            } else if (pExitCurrentSection->x < positionFollowing) {
                // NOTE: I ignore traveling through both tips in one section!!!
                if (getTipThickness(pEnterCurrentSection, pExitCurrentSection, OUTSIDE_LEAF, positionFollowing,
                                    &mlcSection->followingTip, sectionThickness) != OK) {
                    printf("\n ERROR: Getting tip thickness");
                    return (FAIL);
                }
            } else {
                printf("\n ERROR: particleDmlc::getMlcSectionThickness : Reached ?Impossible? State");
                printf("\n Report to developer and debug\n");
                printf("\n FMU %g muIdx %d %g \n postionLeading %g positionFollowing %g", fractionalMUs, muIndex,
                       d->fractionalMUs[muIndex], positionLeading, positionFollowing);
                printf("\n positionLeading-positionFollowing = %g", positionLeading - positionFollowing);
                printf("\n Segment MU Fraction = %g", segmentMUFraction);
                printf("\n pEnterCurrentSection->x = %g, pExitCurrentSection->x = %g", pEnterCurrentSection->x,
                       pExitCurrentSection->x);
                printf("\n pLeading+tip = %g, pFollowing-tip = %g", positionLeading + tipDistance,
                       positionFollowing - tipDistance);
                printf("\n d->fractionalMUs[%d] = %g, d->fractionalMUs[%d] = %g", muIndex, d->fractionalMUs[muIndex],
                       muIndexNext, d->fractionalMUs[muIndexNext]);
                printf("\n aL+1 %f aL %f", ARR_VAL(d->ALeafBank, rowIndexCurrentSection, muIndexNext),
                       ARR_VAL(d->ALeafBank, rowIndexCurrentSection, muIndex));
                printf("\n bL+1 %f bL %f", ARR_VAL(d->BLeafBank, rowIndexCurrentSection, muIndexNext),
                       ARR_VAL(d->BLeafBank, rowIndexCurrentSection, muIndex));
            }
        }
    }
    // Check if Groove is possible
#ifdef DEBUG_FULL
    printf("\n %f %f %f %f %f", pCenterCurrentSection->x, positionLeading, positionFollowing,
       positionLeading+mlcSection->grooveDistance,
       positionFollowing-mlcSection->grooveDistance);
#endif
    /*  Move groove code to only particles that go through the full
        leaf section thickness --Jan 17, 2001: JVS */
    /*
      if( potentialGroove &&  //  ( (Upper && Even) || (Lower && Odd))
      ((pCenterCurrentSection->x > positionLeading   + mlcSection->grooveDistance) ||
      (pCenterCurrentSection->x < positionFollowing - mlcSection->grooveDistance)) &&
      *sectionThickness > mlcSection->grooveThickness )
      *sectionThickness-=mlcSection->grooveThickness;
      */
    // Make sure not overestimating tip thickness...
    *sectionThickness = min(*sectionThickness, fullThicknessCurrentSection);
    return (OK);
}

/* *********************************************************************** */
int particleDmlc::processMLCParticle(particle_type *part, particle_type *scat, particle_type *comptonElectron) //
{
    // Do the things that are required once per particle
#ifdef DEBUG_DMLC_SOURCE
    printf("\n -------New Source Particle x %f y %f", part->x, part->y);
#endif
    // Determine the Attenuation Coefficient for energy of current photon
    float AttenuationCoefficient = 0.0f;
    if (0 == part->charge) { // get charge for photon
        if (part->energy > E_mu->x[0]) {
            AttenuationCoefficient = table_value_extrap(part->energy, E_mu);
        } else {
            printf("\n ERROR: Particle Energy (%g) Below Attenuation Table Minimum (%f)", part->energy, E_mu->x[0]);
            return (FAIL);
        }
    }
    // determine the leaf indicies that the particle hits for each MLC section
    if (!selectMLCSections(part)) {
        printf("\n ERROR: selectMLCSections");
        return (FAIL);
    }
    // assign zero weight and exit of particle is beyond the extent of the MLC
    if (rowIndex[0] == -1 || -1 == rowIndex[1]) {
        // printf("\n Particle Outside of MLC detected");
        part->weight = scat->weight = comptonElectron->weight = 0.0; //
        return (OK);
    }
    //
    // Sample different MU's and determine the leaf thickness at each time...
    // For arc calculation, machineMLC->maxTimeSamples must be 1.
    int muIndex = 0;
    double weightPrimary = 0.0f;
    float TopSectionThickness, BottomSectionThickness;
    float totalThickness = 0.0f;
    float fractionalMUs = -1.0f;
    int maxFixedAngleMUSamples = machineMLC->maxTimeSamples;
    if (dmlc->staticMLC) maxFixedAngleMUSamples = 1; // When nApertures read =1, then only 1 time sample makes sense.
    if (isArc) maxFixedAngleMUSamples = 1;
    int iFixedAngleMUSample = 0;
    while (iFixedAngleMUSample++ < maxFixedAngleMUSamples) {
        //for(int iFixedAngleMUSample=0;iFixedAngleMUSample++ < maxFixedAngleMUSamples, iFixedAngleMUSample++) {
        TopSectionThickness = BottomSectionThickness = -1.0f;  //
        // Randomly Pick the fractional MUs for the determining the leaf positions (and gantry angle for arc)
        fractionalMUs = randomOne();
#ifdef DEBUG_RANDOM
        if(fractionalMUs < 0 || fractionalMUs > 1.0){
          printf("\n ERROR: fractionalMUs not in range [0-1)");
          printf("\t fractionMUs=%f\n", fractionalMUs);
          exit(FAIL);
        }
#endif
        // Determine which index of the dynamic motion this time falls into
        muIndex = 0;
        while (fractionalMUs > dmlc->fractionalMUs[muIndex] && muIndex < dmlc->numApertures) muIndex++;
        if (muIndex) muIndex--;   // decrement so now is between muIndex and muIndexNext
        //      printf("\n MU(%d) %f  %f  %f", muIndex, dmlc->fractionalMUs[muIndex], fractionalMUs, dmlc->fractionalMUs[muIndexNext]);
        int muIndexNext = muIndex + 1;
        if (dmlc->staticMLC) {
            muIndexNext = muIndex;
        } else {
            if (dmlc->fractionalMUs[muIndex] ==
                dmlc->fractionalMUs[muIndexNext]) {  // this does not seem to be an efficient place to do this since
                muIndex++;
                nFoundWithSameIndex++;
            }
        }
        dmlc->nIndexUsed[muIndex] += 1; // for debugging -- so can check weight sampling
        // basic error checking
        if (muIndex > dmlc->numApertures) {
            printf("\n ERROR: muIndex (%d) exceeded number of shapes %d", muIndex, dmlc->numApertures);
            return (FAIL);
        }
        //
        // TEMPORARY: Implement Jaws for JawTracking as 100% efficient Jaws
        if (isJawTracking){
            // weight set to 0 and return if blocked by jaws.
            if( simpleJawTracking(fractionalMUs, muIndex, part)){
                part->weight = scat->weight = comptonElectron->weight = 0.0;
                return(OK);
            }
            // get jaw settings for the current control point
            // determine if falls within the jaws.
        }
        //  find the thicknesses for the 1st and 2nd sections
        if (getMlcSectionThickness(&pEnter[0], &pExit[0], &pCenter[0],
                                   rowIndex[0],
                                   fullSectionThickness[0],
                                   &machineMLC->section[0],
                                   dmlc, muIndex,
                                   fractionalMUs, machineMLC->tipDistance,
                                   &TopSectionThickness, currentLineIndex[0]) != OK) {
            printf("\n ERROR: %s Getting MLC section Thickness for TOP section", __FUNCTION__);
            if (isArc) printf("\t GantryAngle: %f->%f", CP[muIndex].getGantryAngle(), CP[muIndexNext].getGantryAngle());
            return (FAIL);
        }
        // use trick of switching pExit and PEnter so gets tip correct....
        if (getMlcSectionThickness(&pExit[1], &pEnter[1], &pCenter[1],
                                   rowIndex[1],
                                   fullSectionThickness[1],
                                   &machineMLC->section[1],
                                   dmlc, muIndex,
                                   fractionalMUs, machineMLC->tipDistance,
                                   &BottomSectionThickness, currentLineIndex[1]) != OK) {
            printf("\n ERROR: %s Getting MLC section Thickness for BOTTOM section", __FUNCTION__);
            return (FAIL);
        }
        // error checking
        if (TopSectionThickness < 0.0f || BottomSectionThickness < 0.0f) {
            printf("\n ERROR: Invalid SectionThickness Top %f Bottom %f", TopSectionThickness, BottomSectionThickness);
            return (FAIL);
        }
        totalThickness = TopSectionThickness + BottomSectionThickness;
#ifdef DEBUG
        if(totalThickness == 0.0f) printf(".");
#endif
        if (0.0f == totalThickness) {
            weightPrimary += 1.0;
        } else {
            if (0 == part->charge) {  // For electrons, if thickness = 0, then makes it, else, it doesn't
                weightPrimary += exp(
                        -AttenuationCoefficient * (totalThickness) / part->w); // for photon, must attenuate....
            }
        }
    }  // end iFixedAngleMUSample
    //
    if (totalThickness > 0.0f && (0 == part->charge)) {
        // Do the scattered photon.  Need to use initial weight here since sampling one of N..
        // Will do scatter for "last" particle only...
        float pNotInteract = 1.0f - exp(-AttenuationCoefficient * (totalThickness) / part->w);
        float psdDistance = part->z - (machineMLC->section[0].pzExit - TopSectionThickness); // inefficient to calc here
#ifdef DEBUG
        printf("\n PSD : zPlane %f, pzExit %f TopSection %f psdDistance %f", part->z,machineMLC->section[0].pzExit, TopSectionThickness, psdDistance);
#endif
        if (scatteredPhotonAndElectron(part, scat, comptonElectron,
                                       totalThickness, AttenuationCoefficient,
                                       pNotInteract, psdDistance) != OK) {
            printf("\n ERROR: Creating scattered photon");
            return (FAIL);
        }
    }

    // special processes required for arc treatments
    if (isArc) { // rotate the particle by the gantry angle
        // figure out what the gantry angle is via interpolation
        int muIndexNext = muIndex + 1;
        if (dmlc->staticMLC) {
            muIndexNext = muIndex;
            printf("ERROR: Arc and Static MLC not compatible yet in %s", __FUNCTION__);
            return (FAIL);
        } // avoid divide by zero
        float currentGantryAngle = CP[muIndex].getGantryAngle() +
                                   (fractionalMUs - dmlc->fractionalMUs[muIndex]) /
                                   (dmlc->fractionalMUs[muIndexNext] - dmlc->fractionalMUs[muIndex]) *
                                   (CP[muIndexNext].getGantryAngle() - CP[muIndex].getGantryAngle());
        transform->setGantryAngle(currentGantryAngle); // set the gantry angle as the one sampled for this particle
        // transform->updateAcceleratorToPatientMatrix(); // update the translation/rotation matricies -- done in setGantryAngle....
        dmlc->sampledGantryAngle[(int) currentGantryAngle]++;
    }
    if (translateToRoomSystem) {
        //transform->reportAccelToPatientMatrix();
        transform->translateParticleToPatientCoordinateSystem(part);
        if (part->charge) {
            if (scat->weight > 0.0f) transform->translateParticleToPatientCoordinateSystem(scat);
            if (comptonElectron->weight > 0.0f) transform->translateParticleToPatientCoordinateSystem(comptonElectron);
        }
    }

    // Assign average weight for the transmitted particle to be written
    // Note, this assignment must be AFTER scatteredPhotonAndElectron, since the scattered particle weights must be normalized to the
    // weight of the incoming particle....(Nov 11, 2005...allowed input particle to have weight other than 1.0)
    part->weight *=
            (float) weightPrimary / (float) maxFixedAngleMUSamples; // Get the average Weight for the primary particle
#ifdef DEBUG
    if( part->weight == 1.000f ){
      printf("\n x %g y %g  E %g  wt %g", part->x,part->y,part->energy,part->weight);
      // Compute Radius at Isocenter...
      float dz = 100.0- part->z ;
      float xAtIso = part->x + dz*part->u/part->w;
      float yAtIso = part->y + dz*part->v/part->w;
      float rAtIso = sqrt( xAtIso*xAtIso + yAtIso*yAtIso);
      printf(" ISO: x %f y %f r %f", xAtIso, yAtIso, rAtIso);
      for(int iSection=0;iSection<2;iSection++){
        printf ("\n\t index %d y %f", rowIndex[iSection],pCenter[iSection].y );
        printf(" fullSectionThickness = %f",fullSectionThickness[0]);
      }
    }
#endif
    return (OK);
}
/* ************************************************************************** */
bool particleDmlc::simpleJawTracking(float fractionalMUs, int muIndex, particle_type *part){
    bool  blocked_by_jaws = false;
    int muIndexNext = muIndex+1;
    float interpolation_fraction = (fractionalMUs - dmlc->fractionalMUs[muIndex]) /
                                   (dmlc->fractionalMUs[muIndexNext] - dmlc->fractionalMUs[muIndex]);
    // jaw positions need to be projected to the part->z position
    float mag = part->z/100.0f;  // to translate jaw positions to particle position
    float l_Jaw = -1.0f*mag*(
            CP[muIndex].getLeftJawPosition() +
            interpolation_fraction * (CP[muIndexNext].getLeftJawPosition() - CP[muIndex].getLeftJawPosition()));
    float r_Jaw = mag*(
            CP[muIndex].getRightJawPosition() +
            interpolation_fraction * (CP[muIndexNext].getRightJawPosition() - CP[muIndex].getRightJawPosition()));
    float b_Jaw = -1.0f*mag*(
            CP[muIndex].getBottomJawPosition() +
            interpolation_fraction * (CP[muIndexNext].getBottomJawPosition() - CP[muIndex].getBottomJawPosition()));
    float t_Jaw = mag*(
            CP[muIndex].getTopJawPosition() +
            interpolation_fraction * (CP[muIndexNext].getTopJawPosition() - CP[muIndex].getTopJawPosition()));
    // also, need to make sure +/- are correct for jaws (jaws in particle coordinate system)
    // and need to make sure have x and y correctly oriented
    //cout << endl << l_Jaw << "<" << part->x << "<" << r_Jaw << " | " << b_Jaw << "<" << part->y << "<" << t_Jaw << " || " << part->z;

    if( part->x < l_Jaw || part->x > r_Jaw ||
        part->y < b_Jaw || part->y > t_Jaw ) {
        blocked_by_jaws = true;
    }

    // cout << " " << blocked_by_jaws << endl;
    return(blocked_by_jaws);
}
/* ************************************************************************** */
bool particleDmlc::selectMLCSections(particle_type *part) {
    // Point_3d_float *pEnter, Point_3d_float *pExit, Point_3d_float *pCenter,
    //				     int *rowIndex, float *fullSectionThickness, int *currentLineIndex, particle_type *part) {
    if (machineMLC->nSections != 2) { // This error checking should be removed from inner loop
        printf("\n ERROR: This code written specific for 2 section MLC's");
        printf("\n \t %d Sections are not allowed\n", machineMLC->nSections);
        return (FAIL);
    }
    // Top section is 0, bottom section is 0...
    // 2. Determine Leaf Indicies and thicknesses for each section
    for (int iSection = 0; iSection < machineMLC->nSections; iSection++) {
        // Get pCenter (Y) to look up row index and thickness at that location
        pCenter[iSection].x = part->x + part->u / part->w * (machineMLC->section[iSection].pzCenter - part->z);
        pCenter[iSection].y = part->y + part->v / part->w * (machineMLC->section[iSection].pzCenter - part->z);
        // Find out which MLC row we are in....
        if (getLeafIndexAndThickness(pCenter[iSection].y,
                                     machineMLC->section[iSection].nIndex, machineMLC->section[iSection].index,
                                     &rowIndex[iSection], &fullSectionThickness[iSection],
                                     currentLineIndex[iSection]) != OK) {
            printf("\n ERROR: getting leaf index and thickness");
            return (FAIL);
        }
#ifdef DEBUG_DMLC_ROW
        printf (" index %d", rowIndex[iSection] );
#endif
        if (rowIndex[iSection] == -1) {
            // printf("\n Particle Outside of MLC detected");
            part->weight = 0.0f; //
            return (true);
        }
        if (rowIndex[iSection] < 0 || rowIndex[iSection] > machineMLC->nLeaves - 1) {
            printf("\n ERROR: rowIndex of %d is invalid", rowIndex[iSection]);
            return (false);
        }
        // 3. Determine pEnter and pExit for each section...
        pEnter[iSection].x = part->x + part->u / part->w * (machineMLC->section[iSection].pzEnter - part->z);
        // pEnter[iSection].y = part->y + part->v/part->w*(machineMLC->section[iSection].pzEnter - part->z);
        pEnter[iSection].z = machineMLC->section[iSection].pzEnter;
        // Find Point of Exit for MLC Section
        pExit[iSection].x = part->x + part->u / part->w * (machineMLC->section[iSection].pzExit - part->z);
        // pExit[iSection].y = part->y + part->v/part->w*(machineMLC->section[iSection].pzExit - part->z);
        pExit[iSection].z = machineMLC->section[iSection].pzExit;
    }
    // Check if Potentially In a Groove (upper and Even)
    /* Dec 19, 2001: JVS: The rowIndex = leaf Number -1, thus, while in leaf number, the
       groove exists for upperSection+evenLeafRowIndex (odd leaf row number)
       Furthermore, for full thickness leaves, the groove is ALWAYS on the lower edge... */
    return (true);
}
/* ************************************************************************** */
#ifdef C_CLASS
int particleDmlc::checkLeafThickness() // mlcType *machineMLC, dmlc_type *dmlc)
#else

int checkLeafThickness(mlcType *machineMLC, dmlc_type *dmlc)
#endif
{
    FILE *oStrm = fopen("checkLeaf.dat", "w");
    if (oStrm == NULL) exit(-1);
    if (machineMLC->nSections != 2) {
        printf("\n ERROR: This code written specific for 2 section MLC's");
        printf("\n \t %d Sections are not allowed\n", machineMLC->nSections);
    }
    // Top section is 0, bottom section is 0...
    particle_type part;
    part.y = -4.0;
    part.v = 0.0;
    part.u = 0.0;
    part.w = 1.0;
    // Point_3d_float pEnter[2], pExit[2], pCenter[2];
    // 2. Determine Leaf Indicies and thicknesses for each section
    for (float xVar = 4; xVar < 7.5; xVar += 0.001f) {
        part.x = xVar;
        for (int iSection = 0; iSection < machineMLC->nSections; iSection++) {
            // Get pCenter (Y) to look up row index and thickness at that location
            pCenter[iSection].x = part.x;
            pCenter[iSection].y = part.y;
            // Find out which MLC row we are in....
            if (getLeafIndexAndThickness(pCenter[iSection].y,
                                         machineMLC->section[iSection].nIndex, machineMLC->section[iSection].index,
                                         &rowIndex[iSection], &fullSectionThickness[iSection],
                                         currentLineIndex[iSection]) != OK) {
                printf("\n ERROR: getting leaf index and thickness");
                return (FAIL);
            }
            if (rowIndex[iSection] < 0 || rowIndex[iSection] > machineMLC->nLeaves - 1) {
                printf("\n ERROR: rowIndex of %d is invalid", rowIndex[iSection]);
                return (FAIL);
            }
            //      printf("\n iSection %d pCenter %f index %d", iSection, pCenter[iSection].y, rowIndex[iSection]);

            // 3. Determine pEnter and pExit for each section...
            pEnter[iSection].x = part.x;//+1e-6;  part.u = 0 so is just "x"
            pEnter[iSection].z = machineMLC->section[iSection].pzEnter;
            // Find Point of Exit for MLC Section
            pExit[iSection].x = part.x;//-1e-6;
            pExit[iSection].z = machineMLC->section[iSection].pzExit;
        }
        // Sample different MU's and determine the leaf thickness at each time...
        // simple loop for now
        int muIndex;
        float TopSectionThickness, BottomSectionThickness;
        int maxFixedAngleMUSamples = machineMLC->maxTimeSamples;
        if (dmlc->staticMLC) maxFixedAngleMUSamples = 1; // When nApertures read =1, then only 1 time sample makes sense.

        int iFixedAngleMUSample = 0;
        while (iFixedAngleMUSample++ < maxFixedAngleMUSamples) {
            //for(int iFixedAngleMUSample=0;iFixedAngleMUSample++ < maxFixedAngleMUSamples, iFixedAngleMUSample++) {
            TopSectionThickness = BottomSectionThickness = -1.0;
            // Randomly Pick a time
            float fractionalMUs = randomOne();  // get the fractional time
            // Determine which index this time falls into
            muIndex = 0;
            while (fractionalMUs > dmlc->fractionalMUs[muIndex] && muIndex < dmlc->numApertures) muIndex++;
            if (muIndex) muIndex--;  // decrement so now is between muIndex and muIndexNext
            //      printf("\n MU(%d) %f  %f  %f", muIndex, dmlc->fractionalMUs[muIndex], fractionalMUs, dmlc->fractionalMUs[muIndexNext]);
            if (muIndex > dmlc->numApertures) {
                printf("\n ERROR: muIndex (%d) exceeded number of shapes %d", muIndex, dmlc->numApertures);
                return (FAIL);
            }
            if (getMlcSectionThickness(&pEnter[0], &pExit[0], &pCenter[0],
                                       rowIndex[0],
                                       fullSectionThickness[0],
                                       &machineMLC->section[0],
                                       dmlc, muIndex,
                                       fractionalMUs, machineMLC->tipDistance,
                                       &TopSectionThickness, currentLineIndex[0]) != OK) {
                printf("\n ERROR: %s Getting MLC section Thickness for TOP section", __FUNCTION__);
                return (FAIL);
            }
            // use trick of switching pExit and PEnter so gets tip correct....
            if (getMlcSectionThickness(&pExit[1], &pEnter[1], &pCenter[1],
                                       rowIndex[1],
                                       fullSectionThickness[1],
                                       &machineMLC->section[1],
                                       dmlc, muIndex,
                                       fractionalMUs, machineMLC->tipDistance,
                                       &BottomSectionThickness, currentLineIndex[1]) != OK) {
                printf("\n ERROR: %s Getting MLC section Thickness for BOTTOM section", __FUNCTION__);
                return (FAIL);
            }
            if (TopSectionThickness < 0.0f || BottomSectionThickness < 0.0f) {
                printf("\n ERROR: Invalid SectionThickness Top %f Bottom %f", TopSectionThickness,
                       BottomSectionThickness);
                return (FAIL);
            }
        }
        fprintf(oStrm, "\n %f %f %f %f", part.x, TopSectionThickness + BottomSectionThickness, TopSectionThickness,
                BottomSectionThickness);
    }
    fclose(oStrm);

    return (OK);
}

/* ************************************************************************** */
void particleDmlc::zeroMUIndexCounters() {
    for (int iCP = 0; iCP < dmlc->numApertures; iCP++) {
        dmlc->nIndexUsed[iCP] = 0;
    }
    for (int iA = 0; iA < 360; iA++) dmlc->sampledGantryAngle[iA] = 0;
}

/* ********************************************************************** */
void particleDmlc::reportMUIndexCounters() {
    unsigned totalCounts = 0;
    for (int iCP = 0; iCP < dmlc->numApertures; iCP++) {
        totalCounts += dmlc->nIndexUsed[iCP];
    }
    if (totalCounts) {
        printf("\n--perControlPoint---");
        printf("\n\tiCP  \tnCounts\t fCounts\t fMU ");
        for (int iCP = 0; iCP < dmlc->numApertures; iCP++) {
            printf("\n \t%d\t%9u\t%.5f\t%.5f", iCP,
                   dmlc->nIndexUsed[iCP],
                   (float) dmlc->nIndexUsed[iCP] / (float) totalCounts,
                   dmlc->fractionalMUs[iCP]);
        }
    }
    if (isArc) {
        unsigned totalAngleCounts = 0;
        for (int iA = 0; iA < 360; iA++) {
            totalAngleCounts += dmlc->sampledGantryAngle[iA];
        }
        if (totalAngleCounts != totalCounts)
            printf("\n ERROR: unequal counts index %u != %u angle\n", totalCounts, totalAngleCounts);
        printf("\n--perAngle---");
        printf("\n\tAngle\tCounts\t fCounts");
        for (int iA = 0; iA < 360; iA++) {
            if (dmlc->sampledGantryAngle[iA])
                printf("\n \t%d\t%u\t%.5f", iA, dmlc->sampledGantryAngle[iA],
                       (float) dmlc->sampledGantryAngle[iA] / (float) totalAngleCounts);
        }
    }
}

/* *********************************************************************** */
void particleDmlc::setRandomSeed(unsigned mySeed) {
    initializeRandom(mySeed);
}
/* *********************************************************************** */
