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
/* read_write_egs.cc
   routines to read a particle
               write a particle
               read a header
               write a header
   
   Created: 11 Nov 1999 pjk from JVS read_egs1.cc from JCS code
   Modification History:
      17 July 2001: JVS: Update some error messages
      12 Nov  2001: JVS: Add check in writing particle (if positron/negatron/charge)
      10 April 2001: JVS: Add in error checking in read_a_header
      Dec 17, 2002: add explicit readEGSPhspParticle, writeEGSPhspParticle,readEGSPhspHeader
                    so not confuse with other routines
      Oct 23, 2003: Add firstParticle so will work with BEAMnrc and DOSXYZnrc who set
                    particle energy negative if first in a history....
      Dec 2, 2003: JVS: Add writeEGSPhspHeader to be consistant
     March 7, 2006: JVS: write_a_particle bug fix to do with firstParticle energy (got wrong max energy in the header)
     Nov 1, 2006: JVS: Overload the write functions so can write MODE0 and MODE2 particles, also add ability to read MODE0 files
     Nov 27, 2007: JVS; const char * so g++ 4.2 compatible
     Oct 26, 2010: SW: both "11" and "10" will be detected as electron, to eliminate "fail to read latch" error
     Feb 23, 2012: JVS: typecast's added to remove compiler warnings
     Jul 26, 2012: JVS: Add assignEGSLatch
     Jul 27, 2012: JVS: rewind stream before writing header so make sure is at top of file.
     Nov 8, 2012: JVS: Add reportParticle
     May 6, 2015: JVS: Add <cmath> and namespace std for solaris compile
     July 26, 2016: JVS: writeReducedPhaseSpaceParticle did not properly assign radius for positrons.  Now it does

*/
/* *********************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <limits.h> // for INT_MAX
#include <math.h>
#include <string.h>
#include <cmath>
using namespace std;
#include "option.h"
#include "utilities.h" 
#include "read_write_egs.h" 
// #define DEBUG
/* *********************************************************************** */
int reportEGSHeader(ps_header_type *h) {
  printf("\nHeader Status");
  printf("\n\t mode = %s", h->mode);
  printf("\n\t nParticles = %u", h->nParticles);
  if(h->nParticles) {
   printf("\n\t nPhotons  = %u\t (%.3f%%)", h->nPhotons,100.0f*(float)h->nPhotons/(float)h->nParticles);
   printf("\n\t nElectrons  = %u\t (%.3f%%)", h->nParticles-h->nPhotons,  100.0f*((float)h->nParticles-(float)h->nPhotons)/(float)h->nParticles);
  }
  printf("\n\t maximumKineticEnergy = %f", h->maximumKineticEnergy);
  printf("\n\t minimumElectronKineticEnergy  = %f", h->minimumElectronKineticEnergy);
  printf("\n\t nOriginalParticles  = %f", h->nOriginalParticles);
  return(OK);
}
/* *********************************************************************** */
void reportParticle(const particle_type *p) {
  printf("\n %d\t%.3g\t%+.3g\t%+.3g\t%+.3g\t%+.3g\t%+.3g\t%+.3g\t%.3g\t%d", p->charge, p->energy,p->x,p->y,p->z,p->u,p->v,p->w,p->weight,p->egs4Latch);
}
/* *********************************************************************** */
int readEGSPhspParticle(ps_header_type *h, particle_type *p, FILE *istrm)
{
  if(OK != read_a_particle(h,p,istrm) )  return(FAIL);
  p->w = (float) sqrt(1.0f - (p->u*p->u + p->v*p->v) );
//  printf("\n %.3f %.3f %.3f %.3f %.3f %.3f",p->energy,p->x,p->y,p->u,p->v,p->weight);
  return(OK);
}
int readEGSPhspHeader(ps_header_type* h, FILE *istrm)
{
  return(read_a_header(h, istrm));
}
int writeEGSPhspParticle(ps_header_type* h, particle_type* p, FILE *ostrm)
{
  return(write_a_particle(h, p,ostrm));
}

/// Paul's original write_a_particle routine
int write_a_particle(ps_header_type* h, particle_type* p, FILE *ostrm) 
{
  return(writeEGSPhspParticle(h, p,ostrm,"MODE2"));
}
int writeEGSPhspHeader(ps_header_type* h, FILE *ostrm)
{
  return(write_a_header(h, ostrm));
}
/* ******************************************************************* */
int read_a_particle(ps_header_type* h, particle_type* p, FILE *istrm)
{
  float floatArray[20];
  if( fread(&p->egs4Latch, sizeof(int),   1, istrm) != 1)
  {
    fprintf(stderr, "\n ERROR: read_a_particle:  Failed to read Latch\n");
    return (FAIL);;
  }
  /* For Mode0 read in 6 floats, for MODE2, read in 7 floats */
  int nFloats=0;
  if (strncmp(h->mode, "MODE0", 5) == 0) { nFloats=6; }
  if (strncmp(h->mode, "MODE2", 5) == 0) { nFloats=7; }
  if (nFloats < 5) {
    fprintf(stderr, "\n ERROR: read_a_particle: Invalid mode (%s) \n",h->mode); return(FAIL);
  }
  if( fread(floatArray, sizeof(float), nFloats, istrm) != (unsigned) nFloats)
  {
     fprintf(stderr, "\n ERROR: Failed to read Data\n");
     return (FAIL);
  }
  p->energy = floatArray[0];
  if( p->energy < 0 )  // egsnrc has negative energy for 1st particle in a history
  {
      p->firstParticle=1;
      p->energy*=-1;
  }
  else
  {
      p->firstParticle=0;
  }
  p->x = floatArray[1];
  p->y = floatArray[2];
  p->u = floatArray[3];
  p->v = floatArray[4];
  p->weight = floatArray[5];
  if( nFloats==7) p->zlast = floatArray[6];
  /* Determine the charge of the particle: */
  /* particleIsPositron = (egs4Latch % twoToThe29th) / twoToThe28th;
     particleIsNegatron = (egs4Latch % twoToThe30th) / twoToThe29th; */
  int particleIsNegatron = ( ( (p->egs4Latch/twoToThe30th) % 2) == 1);
  int particleIsPositron = ( ( (p->egs4Latch/twoToThe29th) % 2) == 1);
  if (particleIsPositron && particleIsNegatron)  {
    //10/26/2010: SW: this will be determined as electron
    // fprintf(stderr, "ERROR: Particle with egs4Latch = %d is both a "
    //                "positron and a negatron.",  p->egs4Latch);
    //fprintf(stderr, "\t Energy = %f, X = %f", p->energy, p->x);
    //      return (FAIL);
    fprintf(stderr, "WARNING: Particle with egs4Latch = %d is both a "
                    "positron and a negatron.  Assigning to be a negatron",  p->egs4Latch);
    particleIsNegatron = 1;
    particleIsPositron = 0;
  }
  if(particleIsNegatron) {
    h->negatron_count++;
    p->charge = -1;
  }
  else if(particleIsPositron) {
    h->positron_count++;
    p->charge = 1;
  }
  else{
    h->photon_count++;
    p->charge = 0;
  }
#ifdef DEBUG
  reportParticle(p);
  printf("\t read");
#endif
  //printf("\n Read a particle");
  return(OK);
}
/* *********************************************************************** */
/// Overload of function so can tell what mode to write particle...So can convert between
/// MODE2 and MODE0 files
int writeEGSPhspParticle(ps_header_type* h, particle_type* p, FILE *ostrm, const char *writeMode)
{
  //printf("\n DEBUG in write_part");
#ifdef DEBUG
  reportParticle(p);
  printf("\t write");
#endif
  float floatArray[20];
  if( fwrite(&p->egs4Latch, sizeof(int),   1, ostrm) != 1){
    fprintf(stderr, "\n ERROR: writeEGSPhspParticle: Failed to write Latch\n");return (FAIL);;
  }
  //printf("\n DEBUG wrote latch");
  // March 7, 2006: Need to use energyToStore instead of p->energy so 
  // will get the max energy correct in the header (And, so particle will still have
  // valid energy)
  float energyToStore = p->energy;
  if(p->firstParticle) { //< egsNRC uses negative to indicate first particle
    energyToStore*=-1;
  }
  floatArray[0] = energyToStore;
  floatArray[1] = p->x;
  floatArray[2] = p->y;
  floatArray[3] = p->u;
  floatArray[4] = p->v;
  floatArray[5] = p->weight;
  floatArray[6] = p->zlast;
  /* For Mode0 read in 6 floats, for MODE2, read in 7 floats */
  int nFloats=0;
  if (strncmp(writeMode, "MODE0", 5) == 0) { nFloats=6; }
  if (strncmp(writeMode, "MODE2", 5) == 0) { nFloats=7; }
  if (nFloats < 5) {
    fprintf(stderr, "\n ERROR: Invalid mode in read_a_particle\n"); return(FAIL);
  }
  if( fwrite(floatArray, sizeof(float), nFloats, ostrm) != (unsigned) nFloats){
     fprintf(stderr, "\n ERROR: writeEGSPhspParticle: Failed to write phsp data\n");return (FAIL);
  }
  /* Determine the charge of the particle: */
  /* particleIsPositron = (egs4Latch % twoToThe29th) / twoToThe28th;
     particleIsNegatron = (egs4Latch % twoToThe30th) / twoToThe29th; */
  int particleIsNegatron = ( ( (p->egs4Latch/twoToThe30th) % 2) == 1);
  int particleIsPositron = ( ( (p->egs4Latch/twoToThe29th) % 2) == 1);
  if (particleIsPositron && particleIsNegatron) 
  {
    // 10/26/2010: SW: this is determined as electron
    //fprintf(stderr, "ERROR: Particle with egs4Latch = %d is both a "
    //                "positron and a negatron.",  p->egs4Latch);
    //fprintf(stderr, "\t Energy = %f, X = %f", p->energy, p->x);
    //return (FAIL);
    particleIsNegatron = 1;
    particleIsPositron = 0;
  }
  if( ( ( p->charge ==  0) &&  (particleIsNegatron   || particleIsPositron) ) ||
      ( ( p->charge == -1) &&  !particleIsNegatron ) ||
      ( ( p->charge == +1) &&  !particleIsPositron ) ){
    fprintf(stderr,"\n ERROR : Charge = %d, but positron= %d and negatron = %d", 
            p->charge, particleIsPositron, particleIsNegatron);
    fprintf(stderr," egs4Latch = %d", p->egs4Latch ); 
    return(FAIL);
  }

  //printf("\n DEBUG starting head stuff");
  // update header structure
  h->nParticles++;
  if (p->energy>h->maximumKineticEnergy)
    h->maximumKineticEnergy = p->energy;

  if (!(particleIsNegatron) && !(particleIsPositron))
    h->nPhotons++;

  if ((particleIsNegatron) &&   p->energy-ElectronRestMass<h->minimumElectronKineticEnergy )
    h->minimumElectronKineticEnergy = (float) (p->energy-ElectronRestMass);

  // printf("\n Wrote a particle");

  return(OK);
}
/* *********************************************************************** */
/// Read in EGS / EGSnrc phase space format
int read_a_header(ps_header_type* h, FILE *istrm)
{
  // read mode of the file (MODE0 and MODE2 are valid)
  if(fread(h->mode, sizeof(char), 5, istrm)!=5)
  {
    printf("\n ERROR: reading mode from header"); return(FAIL);
  }
  h->mode[5] = '\0';
  /// require to match a MODE0 or MODE2 file
  if ( !(strncmp(h->mode, "MODE0", 5) == 0 || strncmp(h->mode, "MODE2", 5) == 0 )) {
      fprintf(stderr, "ERROR:read_a_header: %s is an unrecognized MODE", h->mode);
      return(FAIL);
  }

  int intArray[10];
  if(fread(intArray, sizeof(int), 2, istrm)!=2)
  {
    printf("\n ERROR: reading intArray from header"); return(FAIL);
  }
  h->nParticles = intArray[0];
  h->nPhotons   = intArray[1];
  printf ("\n %u Particles reported in Phase Space", h->nParticles);
  printf ("\n %u Photons", h->nPhotons);
    
  float floatArray[20];
  if (fread(floatArray, sizeof(float), 3, istrm)!=3)
  {
    printf("\n ERROR: reading floatArray from header"); return(FAIL);
  }
  unsigned int nPad=0;
  if (strncmp(h->mode, "MODE0", 5) == 0) { nPad=3; }
  if (strncmp(h->mode, "MODE2", 5) == 0) { nPad=7; }
  if (fread(h->charArray, sizeof(char), nPad, istrm)!=nPad)  /* padding =7 for Mode2, 3 for Mode0*/
  {
    printf("\n ERROR: reading mode from header"); return(FAIL);
  }
  h->charArray[7] = '\0';
  h->maximumKineticEnergy         = floatArray[0];
  h->minimumElectronKineticEnergy = floatArray[1];
  h->nOriginalParticles           = floatArray[2];
  printf("\nMax ke = %f\tMin ke = %f\tn Orig particles = %f (char %s)",
        h->maximumKineticEnergy,h->minimumElectronKineticEnergy,h->nOriginalParticles,h->charArray);


  // printf("\n Read a header\n");
  return(OK);
}
/* *********************************************************************** */
int write_a_header(ps_header_type* h, FILE *ostrm)
{
  rewind(ostrm); // make sure we are at the beginning of the file
  // write mode of the file

  printf ("\n Writing mode=%s phase space", h->mode);

  fwrite(h->mode, sizeof(char), 5, ostrm);

  /// require to match a MODE0 or MODE2 file

  if ( !(strncmp(h->mode, "MODE0", 5) == 0 || strncmp(h->mode, "MODE2", 5) == 0 )){
      fprintf(stderr, "ERROR:write_a_header:  %s is unrecognized MODE", h->mode);
      return(FAIL);
  }
  fwrite(&h->nParticles, sizeof(int), 1, ostrm);
  fwrite(&h->nPhotons, sizeof(int), 1, ostrm);
  printf ("\n %u Particles written in Phase Space", h->nParticles);
  printf ("\n %u Photons", h->nPhotons);
    
  float floatArray[20];
  floatArray[0] = h->maximumKineticEnergy;
  floatArray[1] = h->minimumElectronKineticEnergy;
  floatArray[2] = h->nOriginalParticles;
  fwrite(floatArray, sizeof(float), 3, ostrm);
  unsigned int nPad=0;
  if (strncmp(h->mode, "MODE0", 5) == 0) { nPad=3; }
  if (strncmp(h->mode, "MODE2", 5) == 0) { nPad=7; }
  printf("\n Writing EGS header file for %s file, requires padding of %d", h->mode, nPad);
  fwrite(h->charArray, sizeof(char), nPad, ostrm); /* padding */
  printf("\nMax ke = %f\tMin ke = %f\tn Orig particles = %f (char %s)",
        h->maximumKineticEnergy,h->minimumElectronKineticEnergy,h->nOriginalParticles,h->charArray);

  printf("\n Wrote a header\n");
  return(OK);
}
/* *********************************************************************** */
int assignEGSLatch(particle_type *p) {
  if (p->charge == -1) 
    {p->egs4Latch = twoToThe30th;}
  else if (p->charge ==  0) 
    {p->egs4Latch = 0;}
  else if (p->charge ==  1) 
    {p->egs4Latch = twoToThe29th;}
  else {
      fprintf(stderr, "\n ERROR:  Charge (%d) not [-1,1]\n",p->charge);
      fprintf(stdout, "\n ERROR:  Charge (%d) not [-1,1]\n",p->charge);
      return (FAIL);
  }
  return(OK);
}
/* *********************************************************************** */
int readEpidParticle(particle_type* p, FILE *istrm)
{
  // note Latch changed in dosxyz. Should be OK for primary/scattered particles
  float floatArray[20];
  // set zlast as not passed from dosxyz
  p->zlast = 0.0;

  if( fread(&p->charge, sizeof(int),   1, istrm) != 1)
  {
    fprintf(stderr, "\n ERROR:  Failed to read charge at byte %ld, status %d",ftell(istrm), feof(istrm));
    fprintf(stdout, "\n ERROR:  Failed to read charge at byte %ld, status %d",ftell(istrm), feof(istrm));
    return (FAIL);
  }
  // set Latch as not passed from dosxyz
  //int particleIsNegatron = ( ( (p->egs4Latch/twoToThe30th) % 2) == 1);
  //int particleIsPositron = ( ( (p->egs4Latch/twoToThe29th) % 2) == 1);

  if(OK != assignEGSLatch(p)){
    printf("\n ERROR:readEpidParticle"); return(FAIL);
  }

  if( fread(floatArray, sizeof(float), 8, istrm) != 8)
  {
     fprintf(stderr, "\n ERROR: Failed to read Data\n");
     fprintf(stdout, "\n ERROR: Failed to read Data\n");
     return (FAIL);
  }
  p->energy = floatArray[0];
  p->x = floatArray[1];
  p->y = floatArray[2];
  p->z = floatArray[3];
  p->u = floatArray[4];
  p->v = floatArray[5];
  p->w = floatArray[6];
  p->weight = floatArray[7];

  //printf("\nQ %d E %f X %f Y %f Z %f %f %f %f %f",p->charge, p->energy, p->x, p->y, p->z, p->u, p->v, p->w, p->weight);

  //printf("\n Read epid particle");
  return(OK);
}
/* *********************************************************************** */
void particle_type::writeReducedPhaseSpace(FILE *oStream) {
  // E, r, u_r, v_a, 
  // c     Feb 17, 1999: JVS: Add output of E,r,u,v ONLY with
  //c                           E > 0, r > 0: photon
  //c                           E > 0, r < 0: electron
  //c                           E < 0, r < 0: positron
  float radius=sqrt(x*x+y*y);
  float outEnergy=energy;
  // if(-1==charge) radius*=-1; // electron
  if(charge) radius*=-1; // electron or positron
  if(+1==charge) outEnergy*=-1; // positron
  float u_r=0.0;
  float v_a=0.0;
  if(radius == 0.0) { 
    u_r = u;
    v_a = v;
  } else {                                 
    u_r    = (u*x+v*y)/radius;
    v_a    = (v*x-u*y)/radius;
  }
  float reducedPSCord[4];
  reducedPSCord[0]=outEnergy; reducedPSCord[1]=radius; reducedPSCord[2]=u_r; reducedPSCord[3]=v_a;

  int nWrite = (int) fwrite(reducedPSCord,sizeof(float),4,oStream); // binary write
  if(4 != nWrite ) {
    printf("\n ERROR: writing tiny phasespace\n"); exit(-1);
  }   
}
/* ************************************************************************* */





