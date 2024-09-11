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
#ifndef READ_WRITE_EGS_H_INCLUDED
#define READ_WRITE_EGS_H_INCLUDED
/* read_write_egs.h
   routines to read a particle
               write a particle
               read a header
               write a header
   
   Created: 11 Nov 1999 pjk from JVS read_egs1.cc from JCS code
   Modification History:
      Dec 1, 1999: JVS: Chagne RM to ElectronRestMass (conflicts otherwise)
      Aug 18, 2000: conditional inclusion.
      Dec 17, 2002: add explicit readEGSPhspParticle, writeEGSPhspParticle,readEGSPhspHeader
                    so not confuse with other routines
      Oct 23, 2003: JVS: Add firstParticle ... so can use with BEAMnrc and DOSXYZnrc.
      Dec 2, 2003: JVS: Add writeEGSPhspParticle to be consistant
      Nov 27, 2007: JVS: const char * so g++ 4.2 compatible
      Feb 19, 2008: JVS: Update the comments in the ps_header_type section
      Jan 5, 2010: JVS: Add jvsDefines.h and PARTICLE_TYPE_DEFINED (see libmkf Particle.h)
      July 27, 2012: JVS: 
      Dec 22, 2014: JVS: Add prototype for reportParticle(particle);
                         convert structures to classes
*/
/* *********************************************************************** */
#include "jvsDefines.h" //< for MAX_STR_LEN 
// defines
#define twoToThe29th    536870912 //  512 * 1024 * 1024;
#define twoToThe30th   1073741824 //  1024 * 1024 * 1024; 
#define ElectronRestMass 0.511

/* *********************************************************************** */
// structures
class ps_header_type
{
  public:
  ps_header_type(){
     nParticles = 0;
     nPhotons = 0;
     maximumKineticEnergy = -1.0;;
     minimumElectronKineticEnergy = 1000.0;;
     nOriginalParticles=0.0f;
     negatron_count = positron_count = photon_count = 0;
     mode = (char *) calloc(MAX_STR_LEN,sizeof(char));
     if (NULL == mode) throw " ERROR: ps_header_type,mode ";
     charArray = (char *) calloc(MAX_STR_LEN,sizeof(float));
     if (NULL == charArray) throw " ERROR: ps_header_type,charArray ";
  };
  ~ps_header_type() {
    free(mode);
    free(charArray);
  };
    char      *mode; //< The Mode specifies if Z-last is included in the phsp output
    unsigned  nParticles; //< Number of particles in the file
    unsigned  nPhotons;   //< Number of photons in the file
    float maximumKineticEnergy; //< The maximum Kinetic energy of any particle in the file
    float minimumElectronKineticEnergy; //< The minimum kinetic energy of any particle in the file
    float nOriginalParticles; //< The number of particles simulated to make this file...e.g., the number of electrons on the target
    char *charArray; //< The first record in a phsp file is the header. charArray is dummy buffer 
                                   //< used to make the length of this first record equal to the length of the other records.
                                   //< Thus, phspFileSize = (nParticles+1)*recordSize
  
    unsigned negatron_count; // for error checking, number of electrons
    unsigned positron_count; // for error checking, number of positrons
    unsigned photon_count; // for error checking. number of photons
};

#ifndef PARTICLE_TYPE_DEFINED
#define PARTICLE_TYPE_DEFINED
class particle_type
{ 
 public:
  int   firstParticle;  // if is 1st particle in a history, BEAMnrc / EGSnrc set this to -1
                         // if energy read on read < 0, then is firstParticle, so set =1
                         // else, leave sign alone (so will still work with other codes)
  int   egs4Latch;
  float energy;
  float x;
  float y;
  float z;
  float u;
  float v;
  float w;
  float weight;
  float zlast;
  int   charge;
  void writeReducedPhaseSpace(FILE *oStream);
  //  private: 
};
#endif

class phaseSpaceClass
{
 public:
  phaseSpaceClass();
  ~phaseSpaceClass();
 private:
  ps_header_type header;
  particle_type particle;
};

/* *********************************************************************** */
// prototypes
int assignEGSLatch(particle_type *p);
int reportEGSHeader(ps_header_type *h);
int read_a_particle(ps_header_type* h, particle_type* p, FILE *istrm);
int write_a_particle(ps_header_type* h, particle_type* p, FILE *ostrm);
int read_a_header(ps_header_type* h, FILE *istrm);
int write_a_header(ps_header_type* h, FILE *ostrm);
int readEpidParticle(particle_type* p, FILE *istrm);
int readEGSPhspParticle(ps_header_type *h, particle_type *p, FILE *istrm);
int readEGSPhspHeader(ps_header_type* h, FILE *istrm);
int writeEGSPhspParticle(ps_header_type* h, particle_type* p, FILE *ostrm);
int writeEGSPhspHeader(ps_header_type* h, FILE *ostrm);
int writeEGSPhspParticle(ps_header_type* h, particle_type* p, FILE *ostrm, const char *writeMode);
void reportParticle(const particle_type *p);
/* *********************************************************************** */
#endif
