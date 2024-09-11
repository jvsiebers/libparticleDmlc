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
/* readPinnacle.h
   Copyright 2000: MCV
   Prototypes for readPinnacle routines
   Modification History:
     Nov 27, 2007: JVS: Add const char *
     Jan 20, 2011: JVS: remove repeat definitions with myPinnacle.h
     May 6, 2015: JVS: Adding in required headers
*/
#ifndef readPinnacle_H_Defined
#define readPinnacle_H_Defined
#include "typedefs.h"
#include "beams.h"
#include "apertures.h"
#include "read_clif.h"

// #include "myPinnacle.h"

int read_pinnacle_bolus(FILE *fp, beam_type *pbeam);
int read_pinnacle_mlc_list(FILE *fp,  TwoDContourType *contour);
int read_pinnacle_isocenters(char *, int n_beams, beam_type *pbeams);
// int read_pinnacle_patient_beams(char *pbeam_fname, char *Trial_Name, int *n_beams, beam_type **pbeams);
int read_pinnacle_pbeam(FILE *fp, beam_type *pbeam);
int read_pinnacle_mlc(FILE *istrm, beam_type *pbeam);
int read_pinnacle_point(char *pbeam_fname_stem, char *point_name, Point_3d_float *point);
int convert_fvertices_to_int(int n_vertices, float units,
                             Point_2d_float *fvertices, 
                             Point_2d_int **vertices);
int convert_wedge_name(char *name);
int read_pinnacle_curve(FILE *fp, TwoDContourType *contour);
int read_pinnacle_curve(FILE *fp, int *npts, Point_2d_float **contour);
int read_pinnacle_dosefile_id(FILE *fp, beam_type *pbeam);
/* ********************************************************************************** */
/* routines from readBeams.cc */
int readPinnacleIsocenters(char *pbeam_fname_stem, int nBeams, beam_type *pbeams);
int readPinnacleXDRId(char *iString, int *Value);
int readPinnacleDosefileID(clifObjectType *Beam,  beam_type *pbeam);
int readPinnacleBolus(clifObjectType *Bolus, beam_type *pbeam);
int readPinnacleRawData(clifObjectType *RawData, int *npts, Point_2d_float **contour);
int readPinnacleContourList(clifObjectType *ContourList, int *nContours, TwoDContourType **contour);
int closePinnacleCurve(int *npts, Point_2d_float **contour);
int closePinnacleContour(TwoDContourType *contour);
/* ********************************************************************************** */
int readPinnaclePbeam(clifObjectType *Beam, beam_type *pbeam);
int readPinnacleMLC(clifObjectType *MLC, beam_type *pbeam);
int readPinnacleCurve(clifObjectType *Curve, int *npts, Point_2d_float **contour);
int convertWedgeName(char *Name);
/* ************* readModifiers routines ******************* */
int readPinnacleAperture(clifObjectType *Aperture, aperture_type *ap);
int readPinnaclePbeamModifiers(clifObjectType *Modifier, beam_type *pbeam);
int readPinnacleTrayParameters(clifObjectType *Beam, beam_type *pbeam);
int readPinnacleCompensator(clifObjectType *Compensator, beam_type *pbeam);
int readPinnacleCompensator(clifObjectType *Compensator, char *fileName, compensator_type *comp);
int readPinnacleCompensatorFile(const char *compFileName, compensator_type *compensator);
int readPinnacleCompensatorFile(const char* trialName, int beamNum, compensator_type* compensator);

#endif
