/// \file
/// \author J V Siebers
///
/* Copyright 2015

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
/* particleDmlc++Demo.cc

  Example of using libparticleDmlc++
  Reads in egsnrc formatted phase space
  Outputs egsnrc formatted phase space _or_ iaea formatted phase space

Modification History:
   June 6, 2015: JVS: V0.01: Created, based on particleDmlc++
*/
/* *********************************************************************** */
/* General Definitions for the program                                     */
const float Revision=0.01;
const char *Prog_Name="particleDmlc++Demo";

#define MAIN
/* *********************************************************************** */
// system libraries
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> // for fabs 
#include <time.h>
#include <iostream>

#include <sys/syscall.h>
#include <sys/time.h> // for gettimeofday
#include <unistd.h>  // for gettimeofday

#undef min // needed to avoid /usr/local/lib/gcc/x86_64-unknown-linux-gnu/4.2.1/../../../../include/c++/4.2.1/bits/fstream.tcc:612: error: expected unqualified-id before â(â token
#undef max

// other libraries
#include "typedefs.h"
#include "utilities.h"   // 108
#include "option.h"
#include "string_util.h"
#include "common.h"
#include "table.h"
#include "spline_data.h"
#include "routines.h"
#include "read_write_egs.h" // 115
#include "myrandom.h"
//local libraries
#include "particleDmlcClass.h"
#include "printRunTimeInformation.h"

/* ********************************************************************** */
using namespace std;
/* ********************************************************************** */
#define DEBUG_DMLC
#define EGSnrc_format 11111
#define IAEA_format   22222
/* *********************************************************************** */
 double getDoubleTime(void) {
  struct timeval currentTime;
  if( gettimeofday(&currentTime, NULL) != 0 ) {
    cout << "\n Error: Getting Time"<<endl;  return(0.0);
  }
  return((double) (currentTime.tv_sec + currentTime.tv_usec*1.0e-6) );
}
/* *********************************************************************** */
// Run example in a class since that is the way a program will probably do it
class  pdmlcDemo {
  public:
    // Constructor(s)
    pdmlcDemo(int *argc, char *argv[]);
    pdmlcDemo(float inputRRWeight,
              int maxTimeSamples,
	      char *dmlcFileName,
	      keepType *inputKeep);
    int pDmlcProcessPhaseSpaceIO();
    // Destructor
    ~pdmlcDemo() {
       cout << "\n~pDmlcDemo() \n";
       cout << " Sampled " << count << " particles from phase space file\n";
       delete dmlc;
       // delete keep;
    };
    //
  protected:
      FILE *psFilePointer;
      particle_type particle;
      ps_header_type psHeader; 
    //Particle *particle; /// MKFix particle type...kept private since only needed with the class 
  private:
    unsigned count; /// A counter of the number of time phsp particles were sampled
    unsigned maxParticlesToRun; /// Maximum particles to run for problem
    char outputFileName[MAX_STR_LEN]; /// Name of the output phase space file
    char psFileName[MAX_STR_LEN];  
    float psdZPlane;
    particleDmlc *dmlc; /// Pointer to the particleDmlc class
    bool setupPhspSource();
    int  outputFormat; // choose from IAEA_format, EGSnrc_format
    int inputFormat;
}; // End of the Class Definition
/* *********************************************************************** */
pdmlcDemo::pdmlcDemo(int *argc, char *argv[]) {
  // construct the dmlcStructure
  try {
    dmlc = new particleDmlc(argc,argv);
  }
  catch(...) {
    cout << " ERROR: constructing particleDmlc as a part of pdmlcDemo" << endl; exit(-1);
  }
  //
  inputFormat = outputFormat = EGSnrc_format;
  psdZPlane=-1.0;  // Plane of the incoming phase space (distance from isocenter))
  if( (get_option("-z_plane",argc,argv,&psdZPlane)!=OK) || psdZPlane < 0.0f) {
    cout << "\n ERROR: getting a valid z-plane (must be >0.0) \n";
    throw "ERROR: getting -z_plane";
  }
  psFileName[0]='\0';
  if(  (get_option("-ps",argc,argv,psFileName)!=OK) 
     || psFileName[0] == '\0' ) {
    cout << "\n ERROR: Reading -ps flag"<<endl; 
    throw "ERROR: Reading -ps flag";
  }
  // Allow for outputting to iaea phase space (required if want to write for arcs)
  if( OK == get_option("-oiaea",argc,argv)) {
    outputFormat = IAEA_format;
    cout << "\t outputFormat = IAEA_format" << endl;
  }
  if(dmlc->queryTranslateToRoomSystem()) outputFormat=IAEA_format;  // force iaea format if room coordinate system
  if(IAEA_format   == outputFormat) cout << "\t outputFormat = IAEA_format" << endl;
  if(EGSnrc_format == outputFormat) cout << "\t outputFormat = EGSnrc_format" << endl;
  if( OK == get_option("-iiaea",argc,argv)) {
    inputFormat = IAEA_format;
    cout << "ERROR: -iiaea is not yet supported" << endl;
    throw "ERROR: -iiaea";
  }
  //  Prepare for using the source
  if(!setupPhspSource()) {
    cout << " ERROR: ..." << endl;
    throw "ERROR: pdmlcDemo initialization";
  }
  if( (get_option("-max",argc,argv,&maxParticlesToRun)!=OK) ) {
    cout << "\n ERROR: -max flag  \n";
    throw "ERROR: -max flag";
  }
}
/* *********************************************************************** */
pdmlcDemo::pdmlcDemo(float inputRRWeight,
	             int maxTimeSamples,
	             char *dmlcFileName,
		     keepType *inputKeep) : count(0) { 
  cout << " pdmlcDemo::pdmlcDemo(float,float,int,char *, ..." << endl;
  // Construct the dmlcStructure
  dmlc = new particleDmlc(inputRRWeight, maxTimeSamples, dmlcFileName);
  if(OK != dmlc->particleDmlcIsReady() ) {
    cerr << "ERROR: particleDmlc constructor has failed" << endl; exit(-1);
  } else {
    cout << "particleDMLC is ready to go!!!" << endl;
  }
  // demo of copying something out of 
  memcpy(dmlc->keep,inputKeep,sizeof(keepType));
  if(!setupPhspSource() ) {
    cout << " ERROR: ..." << endl;
    throw "ERROR: pdmlcDemo initialization";
  }
  inputFormat=outputFormat=EGSnrc_format;
  cout << " Constructor Completed pdmlcDemo::pdmlcDemo(float,float,int,char *, ..." << endl;
}
/* *********************************************************************** */
bool pdmlcDemo::setupPhspSource() {	
   psFilePointer = fopen(psFileName, "rb");
   if(psFilePointer==NULL){
      printf("\n ERROR: %s opening input file %s \n",__FUNCTION__, psFileName);return(false);
   }
   if(OK != read_a_header(&psHeader, psFilePointer)){
     printf("\n ERROR: Reading Header from file"); return(false);
   }
   maxParticlesToRun = psHeader.nParticles; // to ensure not read beyond end of phasespace
   // Set name of output file name = inputName.dmlc.egsphsp1
   strcpy(outputFileName,psFileName);
   strcat(outputFileName,".dmlc.egsphsp1"); // Put on rest of filename
   cout << "\t Output File Name is "<< outputFileName << endl;
  return(true);
}
/* *********************************************************************** */
#include "iaea_config.h"
#include "iaea_phsp.h"
/* *********************************************************************** */
bool writeIAEAParticle(IAEA_I32 iaeaSourceID, IAEA_I32 iaeaNewParticle, particle_type *part) {
  IAEA_I32   particleType; //(photon:1 electron:2 positron:3 neutron:4 proton:5 ...)
  switch(part->charge) {  // ensure we have a valid particle type
    case 0:
      particleType=1; break;
    case -1:
      particleType=2; break;
    case 1:
      particleType=3; break;
    default:
      return false;
  }
  // translate to IAEA particle type
  IAEA_Float kineticEnergy   = part->energy;  // kinetic energy in MeV 
  IAEA_Float wt              = part->weight;  // statistical weight 
  IAEA_Float x               = part->x;
  IAEA_Float y               = part->y;
  IAEA_Float z               = part->z;  // position in cartesian coordinates
  IAEA_Float u               = part->u;
  IAEA_Float v               = part->v;
  IAEA_Float w               = part->w;  // direction in cartesian coordinates
  IAEA_Float extraFloat = 0.0f;
  static IAEA_I32   extraInt      = 0;
  iaea_write_particle(&iaeaSourceID, &iaeaNewParticle, &particleType, &kineticEnergy, &wt, 
		      &x, &y, &z,
		      &u, &v, &w,
		      &extraFloat,&extraInt);
  extraInt++;
  if(iaeaNewParticle<0){
    cout << "\n ERROR: writeIAEAParticle " << endl; return false;
  }
  return true;
}
/* *********************************************************************** */
int pdmlcDemo::pDmlcProcessPhaseSpaceIO(){
  // for EGSnrc phsp output
  FILE *oStream;
  ps_header_type *outputHeader;
  // for IAEA phsp output
  IAEA_I32 iaeaSourceID, iaeaResult, iaeaAccess;
  //------------

  if(EGSnrc_format==outputFormat){
    //   Open the output stream
    if( (oStream = fopen(outputFileName,"wb") ) == NULL ) {
      cout << "\n ERROR: can't open file "<< outputFileName << endl; return(FAIL);
    }
    // Initialize the EGS header information for the outgoing phase space
    outputHeader = new ps_header_type;
    if (outputHeader==NULL) {
      cout<<"\n ERROR: memory allocation for phase space header (out)"<<endl; return(FAIL);
    }
    strncpy(outputHeader->mode,psHeader.mode,5);
    outputHeader->nParticles = 0;
    outputHeader->nPhotons   = 0;
    outputHeader->maximumKineticEnergy = -1.0;
    outputHeader->minimumElectronKineticEnergy = 1.0;
    outputHeader->nOriginalParticles = psHeader.nOriginalParticles;
    // Write the EGS header file out so takes place at the top of the file
    printf(" Writing empty .egsphsp1 header\t (expect 0 particles written)");
    if (OK != write_a_header(outputHeader, oStream) ) {
      cout<<"\n ERROR: Writing header for output  phase space "<<endl; return(FAIL);
    } 
  } else if(IAEA_format == outputFormat) {
    // initialize the source
    iaeaAccess=2; // 1=read,2=write,3=append
    int fileNameLength = strlen(outputFileName);
    iaea_new_source(&iaeaSourceID, outputFileName,   
		    &iaeaAccess, &iaeaResult,fileNameLength);
    if(iaeaResult<0){
      cout << "\n ERROR: initializing IAEA source " << endl;return(FAIL);
    }
    // set one-time variables
    // set values which are constant for all particles -- none
    //IAEA_I32 iaeaVariableIndex; IAEA_Float iaeaVariableValue;
    //iaea_set_constant_variable(iaeaSourceID, iaeaVariableIndex, iaeaVariableValue);                         
    //
    IAEA_I64 iaeaNumberOfOriginalParticles = (IAEA_I64) psHeader.nOriginalParticles;
    iaea_set_total_original_particles(&iaeaSourceID, &iaeaNumberOfOriginalParticles);
    if(iaeaNumberOfOriginalParticles < 0) {
      cout << "\n ERROR: initializing IAEA source...set_total_original_particles " << endl;return(FAIL);
    }
    // additional per-particle things saved
    IAEA_I32 iaeaNExtraInt=0; // was 1, now 0: JVS
    IAEA_I32 iaeaNExtraFloat=0;
    iaea_set_extra_numbers(&iaeaSourceID, &iaeaNExtraFloat,&iaeaNExtraInt);
    //
  } else {
    cout << "\n ERROR: Invalid outputFormat="<<outputFormat<<endl;
    return(FAIL);
  }
  // Declare the particle types that will be used
  // for each particle
  particle_type part, scat, comptonElectron;  // initial particle and scattered particle
  float maxPrimaryWt   = -1.0f;
  float maxScatWt      = -1.0f;
  float totalPrimaryWt = 0.0f;
  unsigned nProcessed=0;
  float comptonElectronEnergy=0.0f;
  float comptonElectronWeight=0.0f;
  int nCompton=0;
  double timeStart=getDoubleTime();
  //
  cout << "\n Looping for " << maxParticlesToRun << " particles " << endl;
  for(int i=0; i<100; i++){// create markers so can see how progress is going.  One mark per 10%
    if(0==i%10) cout<<"X"; else cout << " ";
  }
  cout << endl << flush;
  unsigned onePercent=(unsigned) (0.01*maxParticlesToRun);
  onePercent=max(onePercent,(unsigned) 1);
  for(unsigned iParticle=0; iParticle<maxParticlesToRun; iParticle++) {
    if(0 == iParticle%onePercent ) cout << "."<<flush;
    nProcessed++; 
    // Get a particle from the source
    if(OK != readEGSPhspParticle(&psHeader,&part,psFilePointer)) {
      cout << "ERROR: could not create particle " << iParticle << endl; return(FAIL);
    }
    /// 
    /// re-initialize weight for scat and comptonElectron each time so not hang around from last iteration
    scat.weight = comptonElectron.weight = 0.0;
    /// transport the particle
    if(OK != dmlc->MLCTransport(part, scat, comptonElectron) ) {
	  cerr << "ERROR: Transporting particle through the MLC\n"; return 0;
    }
    // Write surviving particles to the phase space
    if ( part.weight > 0.0f && 
	 ( (dmlc->keep->primLeaf == KEEP && part.charge == 0 ) || (dmlc->keep->electron == KEEP&& part.charge != 0) ) ) {
      maxPrimaryWt    = max(maxPrimaryWt, part.weight);
      totalPrimaryWt += part.weight;
      if(EGSnrc_format == outputFormat) {
	int ret_val = write_a_particle(outputHeader, &part, oStream);
	if ((ret_val != OK) &&  (ret_val != NO_COUNT)) {
	  printf("\n ERROR: Particle %d, Writing primary part",iParticle); return(FAIL);
	} 
      } else if(IAEA_format == outputFormat) {
	// if(iParticle<10) cout << " iaea ";
	IAEA_I32 iaeaNewParticle=1; // >0 = newParticle, 0=priorHistory
	if(!writeIAEAParticle(iaeaSourceID, iaeaNewParticle, &part)){
	  cout << " ERROR: writeIAEAParticle " << endl; return(FAIL);
	}
      }
    }
    //
    // Write scatter particle for primary photons...
    if ( ( 0 == part.charge 
	   && KEEP == dmlc->keep->scatLeaf  ) 
           && scat.weight > 0.0f ) {
      //      printf("\n Scatter with weight %g", scat.weight);
      maxScatWt = max(maxScatWt, scat.weight);
      if(EGSnrc_format == outputFormat) {
	int ret_val = write_a_particle(outputHeader, &scat, oStream);
	if ((ret_val != OK) &&  (ret_val != NO_COUNT)) {
	  printf("\n ERROR: Particle %d, Writing scattered part",iParticle); return(FAIL);
	}
      } else if(IAEA_format == outputFormat) {
	IAEA_I32 iaeaNewParticle=0; // >0 = newParticle, 0=priorHistory
	if(!writeIAEAParticle(iaeaSourceID, iaeaNewParticle, &scat)){
	  cout << " ERROR: writeIAEAParticle " << endl; return(FAIL);
	}
      }
    }
    // Write Compton Electrons.......
    if ( (part.charge == 0 && dmlc->keep->comptonElectron == KEEP) && comptonElectron.weight > 0.0f ) {
      nCompton++;
      comptonElectronEnergy += comptonElectron.energy;
      comptonElectronWeight += comptonElectron.weight;
      if(EGSnrc_format == outputFormat) {
	int ret_val = write_a_particle(outputHeader, &comptonElectron, oStream);
	if ((ret_val!=OK) &&  (ret_val!=NO_COUNT)) {
	  printf("\n ERROR: Particle %d, Writing scattered part",iParticle); return(FAIL);
	}
      } else if(IAEA_format == outputFormat) {
	IAEA_I32 iaeaNewParticle=0; // >0 = newParticle, 0=priorHistory
	if(!writeIAEAParticle(iaeaSourceID, iaeaNewParticle, &comptonElectron)){
	  cout << " ERROR: writeIAEAParticle " << endl; return(FAIL);
	}
      }
    }
    // Go to next particle          
  }
  if(EGSnrc_format == outputFormat) {
    printf("\n====================After transport through MLC=========================\n");
    fseek(oStream, 0L, SEEK_SET);
    if (OK != write_a_header(outputHeader, oStream)) {
      printf("\n ERROR: Writing phase space header");return(FAIL);
    } 
    // Close the files 
    fclose(oStream);
    delete outputHeader;
  }else if(IAEA_format == outputFormat) {
    iaea_destroy_source(&iaeaSourceID, &iaeaResult);
    if(iaeaResult<0) {
      cout << " ERROR: closing IAEA source, saving final header" << endl; return(FAIL);
    }
  }
  /// Write out some final statistics of potential interest
  double runTime=getDoubleTime()-timeStart;
  count = nProcessed;
  printf("\n Completed Processing %d Particles in %f seconds", nProcessed, runTime);
  if( runTime > 0 ) printf("\n %f Particles/Second", nProcessed/runTime);
  printf("\n MaxPrimaryWt = %g", maxPrimaryWt);
  printf("\n MaxScatWt = %g", maxScatWt);
  printf("\n TotalPrimaryWt = %g", totalPrimaryWt);
  if( nCompton ) {
     printf("\n %d Compton Electrons, Average Energy = %g, Average Weight = %g", 
        nCompton, comptonElectronEnergy/nCompton, comptonElectronWeight/nCompton);
  }
  return(OK);
}
/* *********************************************************************** */
void usage()
{
    printf("\n %s",Prog_Name);
    printf("\n\t -dml dml file name");
    printf("\n\t -mu mu file name");
    printf("\n\t -ps input phase space file name");
    printf("\n\t -z_plane phase space file z plane");
    printf("\n\t -coll max collimator position");
    printf("\n\t -rrWeight particle weight to play RR");
    printf("\n\t (-rrMaxPartWeight: Multiplier for particle weight to set for survivors of RR, 0.1=default)");
    printf("\n\t -electron save electrons (default is all particles)");
    printf("\n\t -primary  primary transmitted particles (default is all particles)");
    printf("\n\t -scatter  save scattered photons from leaves (default is all particles)");
    printf("\n\t -comptonElectron  Save compton electrons (default is OFF)");
    printf("\n\t -mlcTable  uses Varian mlcTable.txt file to determine physical leaf positions (default)");
    //printf("\n\t -mlcOffset : uses offset to get physical leaf positions (incorrect)");
    printf("\n\t -mcvConfig UVA_CONFIG  : what to use for UVA_CONFIG....");
    printf("\n\t (-nTimeSamples number of time samples to use...default=20 or 100?"); 
    printf("\n\t (-seed random number seed to use.  Uses time(null) if not specified");
    printf("\n\t (-debug)");
    printf("\n\t (-h) help");
    printf("\n\t (-arc controlPointFileName)");
    printf("\n\t (-room output iaea phase space in room coordinate system (for arcs)");
    printf("\n");
}
/* *********************************************************************** */
/* *********************************************************************** */
int main(int argc, char *argv[])
{
  printRunTimeInformation(argc, argv);
  // Set the UVA_CONFIG environment if desired 
  if( OK != mySetEnv("-mcvConfig","UVA_CONFIG",&argc, argv) ){
      printf("\n ERROR: getting/setting mySetEnv"); usage(); return(FAIL);
  }
  if(OK == get_option("-h", &argc,argv) || 
     OK == get_option("-help", &argc,argv) ||
     OK == get_option("-?", &argc,argv) ){
    usage(); return(OK);
  }
  //
  {
    unsigned seed = getSeed();  // Set random seed based on clock
    initializeRandom(seed);
  }
  // Construct the class to run
  pdmlcDemo *pdmlcRun;
  try {
    pdmlcRun = new pdmlcDemo(&argc,argv);
  }
  catch (...) {
    cout << " ERROR: Creating pdmlcDemo "; return(FAIL);
  }
  if( OK != pdmlcRun->pDmlcProcessPhaseSpaceIO() ) {
    printf("\n ERROR: pDmlcProccessPhaseSpaceIO\n"); return (FAIL);
  }
  delete pdmlcRun;
  fflush(NULL);
  printf("\n Normal completion of %s\n",Prog_Name);
  return(OK);
} // end of main
/* **************************************************************************** */
