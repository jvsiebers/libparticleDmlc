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
/* modifiers.h
   For reading in a pinnacle compensator and beam modifiers
   Created: Oct. 20, 1999: JVS:
            Nov 4, 1999: JVS: Convert to modifiers.h
            Oct 11, 2000: JVS: readPinnacleCompensator()
            Dec 16, 2004: JVS: Add compensator_to_pinnacle(,,) and convertDoseFileToCompensator

*/
#ifndef MODIFIERS_H_INCLUDED
#define MODIFIERS_H_INCLUDED
#include "beams.h"


// Function Prototypes
int read_pinnacle_pbeam_modifiers(FILE *fp, beam_type *pbeam);
int read_pinnacle_compensator(FILE *fp, beam_type *pbeam);
int read_pinnacle_modifiers(FILE *fp, int *n_apertures, aperture_type **ap);
int read_pinnacle_aperture(FILE *fp, aperture_type *ap);

int read_pinComm_aperture(int ibeam, int iblock, aperture_type *ap);
int read_pinnacle_apertures(int ibeam, int *n_apertures, aperture_type **ap);
int read_pinnacle_compensator(int ibeam, compensator_type *comp);
int readPinnacleCompensator(const char *TrialName, int ibeam, compensator_type *comp);


int read_pinnacle_contour_list(FILE *fp, int *npts, TwoDContourType **contour);
float *read_binary(char *image_fname, int image_size);


int compensator_to_pinnacle(compensator_type *c, int ibeam);
int compensator_to_pinnacle(compensator_type *c, char *trialName, char *beamName);
int create_gnuplot_compensator(compensator_type *c, int ibeam);
int reset_pinnacle_block(int ibeam, int iblock);
int convertDoseFileToCompensator(char *fileNameStub, beam_type *pbeam);
#endif
