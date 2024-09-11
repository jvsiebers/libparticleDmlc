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
/* ct_routines.h
   April 27, 2000: JVS: broken off of mcv_contours.h
   April 4, 2001: PJK: added very similar routines for gating
   July 25, 2002: PJK: added read_ct_imageInfo
   Feb 10, 2005: JVS: Added readCTImageAndHeader
*/
#ifndef CT_ROUTINES_INCLUDED
#define CT_ROUTINES_INCLUDED
#include "contour_mcv.h" /* to ensure image_header_type defined */
#include "translation.h" /* to ensure translation_type defined */
#define NONEQUI  9999 // for returning in gateCheckForNonEquispacedImage
#define NOSLICE -9999 // for returning in gateCheckForNonEquispacedImage
int readCTImageAndHeader(char *imageFileName, image_header_type **imageHeader, ITYPE **image);
int read_image_header(char *image_fname, image_header_type *img_header);
int read_ct_image(char *image_fname, image_header_type *img_header, 
ITYPE *image);
int check_ct_image(image_header_type *head, ITYPE *image);
ITYPE *read_ct_image(char *image_fname, image_header_type *img_header);
int determine_ct_translation(image_header_type *img_header, translation_type *t);
int checkForNonEquispacedImage(char *imageFilename);

int gateCheckForNonEquispacedImage(char *imageFilename);
int gateCheckForNonEquispacedImage(char *imageFilename, int *slice);
int gateReadCtImage(char *image_fname, image_header_type *img_header, 
ITYPE *image);
ITYPE *gateReadCtImage(char *image_fname, image_header_type *img_header);
int insertSlice(image_header_type *img_header, ITYPE *image, int sliceToFix);
int write_image_header(char *image_fname, image_header_type *img_header);
int write_ct_image(char *image_fname, image_header_type *img_header, 
ITYPE *image);
int write_ct_imageInfo(char *image_fname, image_header_type *img_header);
int read_ct_imageInfo(char *image_fname, image_header_type *img_header);
#endif
