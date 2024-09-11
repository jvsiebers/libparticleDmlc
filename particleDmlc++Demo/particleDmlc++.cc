/// \file
/// \author J V Siebers
///
/*
   Copyright 2000-2003,2007 Virginia Commonwealth University


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
   07-Feb-2007: JVS: 0.23: Break off sub-routines (into particleDmlcClass.cc ) and main (this routine)..., 
                           replace dmlc->psdZPlane with part->z
   08-Feb-2007: JVS: 0.30: particleDmlc++.cc
                           Use the classes for everything
   07-Dec-2007: JVS Making work with updates to MKF source model
   11-Nov-2008: JVS: 0.40: Stand-alone compile, warnings out. 
   21-Feb-2013: JVS: 0.50: Change order of header reading (SourceConfig first) so code will compile on gcc 4.4.6
   21-Dec-2014: JVS: 0.60: Major clean up and use standard constructors for particleDmlcClass
   07-Jan-2015: JVS: 0.61: Getting going for arc treatments too
   14-Jan-2015: JVS: 0.62: Writing iaea phase space in room coordinates for use in dosxyznrc
*/
/* *********************************************************************** */
/* General Definitions for the program                                     */
const float Revision=0.62;
const char *Prog_Name="particleDmlc++";

long PinnWindowID=-1;
#define MAIN
/* *********************************************************************** */
// system libraries
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> /* for fabs */
#include <time.h>
#include <iostream>

#include <sys/syscall.h>
#include <sys/time.h> // for gettimeofday
#include <unistd.h>  // for gettimeofday

#undef min // needed to avoid /usr/local/lib/gcc/x86_64-unknown-linux-gnu/4.2.1/../../../../include/c++/4.2.1/bits/fstream.tcc:612: error: expected unqualified-id before â(â token
#undef max
// Source model libraries...
#ifdef OLD_LIBMKF
#include "SourceConfig.h" // for MKFs source stuff
#else
#include "sm/SourceConfig.h" // for MKFs source stuff
#endif

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
// #include "random.h"
#include "myrandom.h"
//local libraries
#include "particleDmlcClass.h"
#include "printRunTimeInformation.h"

/* ********************************************************************** */
using namespace std;
/* ********************************************************************** */
//
#define DEBUG_DMLC
// #define DEBUG_LEAF
// #define DEBUG_TIP
#define EGSnrc_format 11111
#define IAEA_format   22222
//
/* *********************************************************************** */
 double getDoubleTime(void) {
  struct timeval currentTime;
  if( gettimeofday(&currentTime, NULL) != 0 ) {
    cout << "\n Error: Getting Time"<<endl;  return(0.0);
  }
  //   double timeNow=1.0e-6*currentTime.tv_usec+currentTime.tv_sec;
  //   printf("\n %ld Seconds %ld microSeconds ( %f ) \n", currentTime.tv_sec, currentTime.tv_usec, timeNow);
  return((double) (currentTime.tv_sec + currentTime.tv_usec*1.0e-6) );
}
/* *********************************************************************** */
static  Source::Pointer source;  /// Pointer to the source modules
/* *********************************************************************** */
// Create a class to run thing from 
class  pdmlcRunClass {
  public:
    // Constructor(s)
    pdmlcRunClass(int *argc, char *argv[]);
    // Purpose of this function is to initialize the phase space
    pdmlcRunClass(//float psdZPlane,
	 	  float inputRRWeight,
		  int maxTimeSamples,
		  char *dmlcFileName,
		  //char *psFileName,
		  keepType *inputKeep);
    int pDmlcProcessPhaseSpaceIO();
    // Destructor
    ~pdmlcRunClass() {
       cout << "\n~pDmlcRunClass() \n";
       cout << " Sampled " << count << " particles from phase space file\n";
       delete dmlc;
       delete Phsp;
       // delete keep;
    };
    //
  protected:
    Particle *particle; /// MKFix particle type...kept private since only needed with the class
    //
  private:
    unsigned count; /// A counter of the number of time phsp particles were sampled
    unsigned maxParticlesToRun; /// Maximum particles to run for problem
    char outputFileName[MAX_STR_LEN]; /// Name of the output phase space file
    char psFileName[MAX_STR_LEN];  
    float psdZPlane;
    particleDmlc *dmlc; /// Pointer to the particleDmlc class
    PhspSource* Phsp;   /// Pointer to the phsp source...don't quite know why I need it
  //keepType *keep;     /// For keeping track what types of particles to keep/discard

    // Function to configure the phase space source, modeled after MKFs SourceConfig.cpp
    Source *ConfigPhspSource();
    bool setupPhspSource();
    int  outputFormat; // choose from IAEA_format, EGSnrc_format
    int inputFormat;
}; // End of the Class Definition
/* *********************************************************************** */
pdmlcRunClass::pdmlcRunClass(int *argc, char *argv[]) {
  // construct the dmlcStructure
  //cout << "------------------------------------------"<<endl;
  //cout << " pdmlcRunClass::pdmlcRunClass(int *, char*)" << endl;
  //cout << "------------------------------------------"<<endl;
  try {
    dmlc = new particleDmlc(argc,argv);
  }
  catch(...) {
    cout << " ERROR: constructing particleDmlc as a part of pdmlcRunClass" << endl; exit(-1);
  }
  //
  //cout << "------------------------------------------"<<endl;
  //cout << " Getting the psFileName and psdZPlane"<<endl;
  //cout << "------------------------------------------"<<endl;
  //
  inputFormat = outputFormat = EGSnrc_format;
  psdZPlane=-1.0;
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
  if( OK == get_option("-oiaea",argc,argv)) {
    outputFormat = IAEA_format;
    cout << "\t outputFormat = IAEA_format" << endl;
  }
  if(dmlc->queryTranslateToRoomSystem()) outputFormat=IAEA_format;
  if(IAEA_format   == outputFormat) cout << "\t outputFormat = IAEA_format" << endl;
  if(EGSnrc_format == outputFormat) cout << "\t outputFormat = EGSnrc_format" << endl;
  if( OK == get_option("-iiaea",argc,argv)) {
    inputFormat = IAEA_format;
    cout << "ERROR: -iiaea is not yet supported" << endl;
    throw "ERROR: -iiaea";
  }
  //cout << "==============================================================================="<<endl;
  if(!setupPhspSource()) {
    cout << " ERROR: ..." << endl;
    throw "ERROR: pdmlcRunClass initialization";
  }
  if( (get_option("-max",argc,argv,&maxParticlesToRun)!=OK) ) {
    cout << "\n ERROR: -max flag  \n";
    throw "ERROR: -max flag";
  }
}
/* *********************************************************************** */
pdmlcRunClass::pdmlcRunClass(// float psdZPlane,
			     float inputRRWeight,
			     int maxTimeSamples,
			     char *dmlcFileName,
			     // char *psFileName,
			     keepType *inputKeep) : count(0) { 
  cout << " pdmlcRunClass::pdmlcRunClass(float,float,int,char *, ..." << endl;
  // Construct the dmlcStructure
  dmlc = new particleDmlc(inputRRWeight, maxTimeSamples, dmlcFileName);
  if(OK != dmlc->particleDmlcIsReady() ) {
    cerr << "ERROR: particleDmlc constructor has failed" << endl; exit(-1);
  } else {
    cout << "particleDMLC is ready to go!!!" << endl;
  }
  
  // IBM moved the keep structure to inside of particleDMLC, but I (JVS) think
  // we may want to keep it external....think about it
  /*
  keep = new keepType;
  if (keep==NULL){
    cout << "\n ERROR: memory allocation for keep structure\n" << endl; 
  }
  // copy contents of keep type to memory keep
  memcpy(keep,inputKeep,sizeof(keepType));
  */
  // cout << "================Need to setup keep type for this instance================="<<endl;
  memcpy(dmlc->keep,inputKeep,sizeof(keepType));
  if(!setupPhspSource() ) {
    cout << " ERROR: ..." << endl;
    throw "ERROR: pdmlcRunClass initialization";
  }
  inputFormat=outputFormat=EGSnrc_format;
  cout << " Constructor Completed pdmlcRunClass::pdmlcRunClass(float,float,int,char *, ..." << endl;
}
/* *********************************************************************** */
bool pdmlcRunClass::setupPhspSource() {		   
  // static Source::Pointer source;
  //cout << "\tpdmlcRunClass::setupPhspSource() " << endl << flush;

  source = ConfigPhspSource();   
  //cout << "\t Getting number of particles to run " << endl; fflush(stdout);
  // set the maximum number of particles to run
  maxParticlesToRun = Phsp->getNInPhsp()-1; // to ensure not read beyond end of phasespace
  //cout << "\t Changing output file extension " << endl;
  // will need the jaws module when we use the full source model
  // Jaws* jaws = new Jaws();
  // create name of output file to write...
  // Prepare the output file
  {
    char *strEnd=strstr(psFileName,"BEAM");
    if(strEnd == NULL || strEnd-psFileName > 256 ){
      cout << "\n WARNING: BEAM not found in " <<  psFileName << endl;
      strcpy(outputFileName,psFileName);
    } else {
      int keepLen=strEnd-psFileName; // Find Characters to Keep
      if(keepLen < 0) {
	cout << "ERROR: keepLen < 0" << endl; return(false);
      }
      strncpy(outputFileName,psFileName,keepLen);
      outputFileName[keepLen]=0; // Null Terminate the String
    }
    strcat(outputFileName,"dmlc.egs4phsp1"); // Put on rest of filename
    cout << "\t Output File Name is "<< outputFileName << endl;
  }
  return(true);
}
/* *********************************************************************** */
Source* pdmlcRunClass::ConfigPhspSource() {
  // Create object for the source and initialize required parameters....
  cout << "\t pdmlcRunClass::ConfigPhspSource() " << endl;
  Phsp = new PhspSource;
  if (!Phsp) {
    cerr << " ERROR: Allocating memory for PhspSource\n"; exit(-1);
  } 
#ifdef DEBUG
  cout << "Assigning phaseSpaceZ to be " << phaseSpaceZ << endl;
#endif
  // Assign required variables for the phspSource
  Phsp->setPhspfile(psFileName);  // The name of the phase space
  Phsp->setZoutput(psdZPlane);
  Phsp->setZphsporigin(psdZPlane); // Z-coordinate of the phase space (sampling plane in accelerator system)
  // Phsp->setParticleweight(1.0); // Only one source...will change when want to run multiple phase spaces simultaneously, but
  // this could require muliple phaseSpaceZ's, and multiple phaseSpaceNames....
  // If that is the case, then better off using "Source" type rather than "PhspSource" type
  // Since "Source" would have full structure for taking care of the above, plus relative weights
  if(!Phsp->readPhaseSpaceHeader()) { // Read in the header information
    cout << "ERROR: readPhaseSpaceHeader" << endl;  exit(-1);
  }
  // Report what is in the header
  Phsp->reportHeader();
  // Compute ratio in phase space file to original number
  // particle_per_MU*=Phsp->getNInPhsp()/Phsp->getNIncidentPhsp();
  // cout << "\tExiting ConfigPhspSource";
  return Phsp;
}
/* *********************************************************************** */
#include "iaea_config.h"
#include "iaea_phsp.h"
/* *********************************************************************** */
bool writeIAEAParticle(IAEA_I32 iaeaSourceID, IAEA_I32 iaeaNewParticle, particle_type *part) {
  // static long iWrite=0;
  // if(0==iWrite++%1000) cout << " " << iWrite << endl;

  IAEA_I32   particleType; //(photon:1 electron:2 positron:3 neutron:4 proton:5 ...)
  switch(part->charge) {
    case 0:
      particleType=1; break;
    case -1:
      particleType=2; break;
    case 1:
      particleType=3; break;
    default:
      return false;
  }
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
int pdmlcRunClass::pDmlcProcessPhaseSpaceIO(){
  // for EGSnrc phsp output
  FILE *oStream = NULL;
  ps_header_type *outputHeader = NULL;
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
    // strncpy(outputHeader->mode,particle->particleHeader.mode,5);
    // strncpy(outputHeader->charArray,particle->particleHeader.dummy,7);
    strncpy(outputHeader->mode,Phsp->getPhspMode() ,5);
    //      strncpy(outputHeader->charArray,particle->particleHeader.dummy,7);
    outputHeader->nParticles = 0;
    outputHeader->nPhotons = 0;
    outputHeader->maximumKineticEnergy = -1.0;
    outputHeader->minimumElectronKineticEnergy = 1.0;
    outputHeader->nOriginalParticles = Phsp->getNIncidentPhsp();
    // Write the EGS header file out so takes place at the top of the file
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
    IAEA_I64 iaeaNumberOfOriginalParticles = (IAEA_I64) Phsp->getNIncidentPhsp();
    iaea_set_total_original_particles(&iaeaSourceID, &iaeaNumberOfOriginalParticles);
    if(iaeaNumberOfOriginalParticles < 0) {
      cout << "\n ERROR: initializing IAEA source...set_total_original_particles " << endl;return(FAIL);
    }
    // additional per-particle things saved
    IAEA_I32 iaeaNExtraInt=0; // was 1, now 0: JVS
    IAEA_I32 iaeaNExtraFloat=0;
    iaea_set_extra_numbers(&iaeaSourceID, &iaeaNExtraFloat,&iaeaNExtraInt);
  } else {
    cout << "\n ERROR: Invalid outputFormat="<<outputFormat<<endl;
    return(FAIL);
  }
  // Declare the particle types that will be used
  // for each particle
  particle_type part, scat, comptonElectron;  // initial particle and scattered particle
  float maxPrimaryWt = -1.0f;
  float maxScatWt = -1.0f;
  float totalPrimaryWt = 0.0f;
  unsigned nProcessed=0;
  float comptonElectronEnergy=0.0f;
  float comptonElectronWeight=0.0f;
  int nCompton=0;
  double timeStart=getDoubleTime();
  //
  particle = new Particle(); /// Storage for the particle acquired from the source model
  //
  cout << "\n Looping for " << maxParticlesToRun << " particles " << endl;
  for(int i=0; i<100; i++){
    if(0==i%10) cout<<"X"; else cout << " ";
  }
  cout << endl << flush;
  unsigned onePercent=(unsigned) (0.01*maxParticlesToRun);
  unsigned myOne=1;
  onePercent=max(onePercent,myOne);
  //
  // Need to add in simplified JawTracking
  //
  for(unsigned iParticle=0; iParticle<maxParticlesToRun; iParticle++) {
    if(0 == iParticle%onePercent ) cout << "."<<flush;
    nProcessed++; 
    // Get a particle from the source
    if (!source.getParticle(particle)) {
      cout << "ERROR: could not create particle " << iParticle << endl; return(FAIL);
    }
    /// Convert between MKF's particle type and the particle type used in the older code
    ConvertParticleType(particle,&part);
    // cout << " Processing particle " << iParticle << " charge = " << part.charge << endl;
    /// Make sure convert particle type also computes the w of the particle !!!!
    /// re-initialize weight for scat and comptonElectron each time so not hang around from last iteration
    scat.weight = comptonElectron.weight = 0.0;
    /// transport the particle
    if(OK != dmlc->MLCTransport(part, scat, comptonElectron) ) {
	  cerr << "ERROR: Tranporting particle through the MLC\n"; return 0;
    }
    // Write surviving particles to the phase space
    if ( part.weight > 0.0f && 
	 ( (dmlc->keep->primLeaf == KEEP && part.charge == 0 ) || (dmlc->keep->electron == KEEP&& part.charge != 0) ) ) {
      maxPrimaryWt    = max(maxPrimaryWt, part.weight);
      totalPrimaryWt += part.weight;
      if(EGSnrc_format == outputFormat) {
	int ret_val = write_a_particle(outputHeader, &part, oStream);
	//       int ret_val = write_a_particle(outputHeader, &part, oStream); // turn off RR
	if ((ret_val != OK) &&  (ret_val != NO_COUNT)) {
	  printf("\n ERROR: Particle %d, Writing primary part",iParticle); return(FAIL);
	} 
      } else if(IAEA_format == outputFormat) {
	if(iParticle<10) cout << " iaea ";
	IAEA_I32 iaeaNewParticle=1; // >0 = newParticle, 0=priorHistory
	if(!writeIAEAParticle(iaeaSourceID, iaeaNewParticle, &part)){
	  cout << " ERROR: writeIAEAParticle " << endl; return(FAIL);
	}
      }
    }
    //
#ifdef DEBUG
    if(part.weight > 0.0f && iParticle<10){
      // reportParticle(&part);
      cout << part.energy << "\t" << part.x << "\t" << part.y << "\t" << part.z
	   << "\t" << part.u << "\t" << part.v << "\t" << part.w << "\t" << part.weight << endl;
    }
#endif
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
    /// Write the final header
    fseek(oStream, 0L, SEEK_SET);
    if (write_a_header(outputHeader, oStream) != OK) {
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
  delete particle;
  return(OK);
}
/* *********************************************************************** */
void usage()
{
    printf("\n %s",Prog_Name);
    printf("\n\t -dml dml file name");
    printf("\n\t -mu mu file name");
    printf("\n\t -ps phase space file name");
    printf("\n\t -z_plane phase space file z plane");
    printf("\n\t -coll max collimator position");
    printf("\n\t -rrWeight particle weight to play RR");
    printf("\n\t (-rrMaxPartWeight: Multiplier for particle weight to set for survivors of RR, 0.1=default)");
    printf("\n\t -electron save electrons (default is all particles)");
    printf("\n\t -primary  primary transmitted particles (default is all particles)");
    printf("\n\t -scatter  save scattered photons from leaves (default is all particles)");
    printf("\n\t -comptonElectron  Save compton electrons (default is OFF)");
    printf("\n\t -mlcTable  uses Varian mlcTable.txt file to determine physical leaf positions (default)");
    printf("\n\t -mlcOffset : uses offset to get physical leaf positions (incorrect)");
    printf("\n\t -mcvConfig UVA_CONFIG  : what to use for UVA_CONFIG....");
    printf("\n\t (-nTimeSamples number of time samples to use...default=20 or 100?"); 
    printf("\n\t (-seed random number seed to use.  Uses time(null) if not specified");
    printf("\n\t (-debug)");
    printf("\n\t (-h) help");
    printf("\n\t (-arc controlPointFileName)");
    printf("\n\t (-room outputiaea phase space in room coordinate system");
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
  //  Allow user to change the random number seed 
  {
    unsigned seed = getSeed();  // Set random seed based on clock
    int intSeed=0;
    if(get_option("-seed",&argc,argv,&intSeed)!=OK){
      printf("\n ERROR: Reading -seed flag\n");
      usage();return(FAIL);
    }
    if(intSeed!=0) seed=(unsigned) intSeed;  // If user set the seed, then use it 
    printf("\n Using Random Number Generator seed of %d", seed);
    initializeRandom(seed);
    //     printf("\n The first random number is %f", randomOne());
  }
  //
  printNRandomsSampled();
  printf("\n The first random number is %f\n", randomOne()); 
  // Construct the class to run
  pdmlcRunClass *setupRun;
  try {
    setupRun = new pdmlcRunClass(&argc,argv);
  }
  catch (...) {
    cout << " ERROR: Creating pdmlcRunClass "; return(FAIL);
  }
  // need to declare the source
  // Could allow different source types
  // Then initialize the source  
  // Need to open the output file and the header


  // cout << "+++++++++++++++++++++Exiting temp++++++++++++++++++"<<endl; 
  //fflush(NULL);
  
  if( OK != setupRun->pDmlcProcessPhaseSpaceIO() ) {
    printf("\n ERROR: pDmlcProccessPhaseSpaceIO\n"); return (FAIL);
  }
  delete setupRun;
  fflush(NULL);
  printf("\n Normal completion of %s\n",Prog_Name);
  return(OK);
} // end of main
/* **************************************************************************** */
