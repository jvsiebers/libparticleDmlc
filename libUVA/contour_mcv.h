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
/* *********************************************************************** */
/* contour_mcv.h
   Created: 5/21/98: JVS
    Contains structures and function prototypes for contour programs

   Modification History:
     5/27/9*: JVS: Add to image_header_type
     Feb 15, 2000: JVS: Add check_ct_image
     April 27, 2000: JVS: Add translation_type * to read_contour
     Jan 28, 2002: PJK/SSV added db_name., zSlice[] to image_header_type
     July 28, 2002: PJK Added zSliceNumber for plunc
     Sept 30, 2004: PJK/KW added byte_order to image_header_type
     June 2, 2011: JVS: Add x_start_dicom, y_start_dicom, and version
 *********************************************************************** */
#ifndef CONTOUR_UVA_INCLUDED   // only include single occurance
#define CONTOUR_UVA_INCLUDED
#include "jvsDefines.h" // for definition of MAX_STR_LEN
#include "translation.h"
/* Global Variables and Type definitions                                   */
typedef struct
{ 
  char db_name[MAX_STR_LEN];
  char version[MAX_STR_LEN];
  int x_dim,y_dim,z_dim;              // number of pixels 
  int byte_order;              // byte_order in pinnacle
  float x_pixdim, y_pixdim, z_pixdim; // size of pixel
  float x_start, y_start, z_start;    // starting corrdinates
  float x_start_dicom, y_start_dicom;
  float min_pix;          // minimum observed value of a pixel
  float max_pix;          // maximum observed value of a pixel
  float range_low;        // min value possible for a pixel
  float range_high;       // max value possible for a pixel
  float zSlice[2000];     // for writing imageInfo
  int   zSliceNumber[2000];     // for writing imageInfo
} image_header_type;

typedef struct
{
   char *name;
   int  num_imgs;
   struct conts_of_anat_str *anat_conts; // list of contours 
   // contour_type *next_anat; // link to next structure
} contour_type;  // similar to anat_conts_str, but has *name
/* *********************************************************************** */
#define  ITYPE  short int      // Type of data for CT images
/* *********************************************************************** */
/* Function Prototypes                                                     */
int read_contour(char *contour_fname, 
                 char *contour_name, 
                 image_header_type *img_header,
                 translation_type *t,
                 contour_type *contour);
int free_contour(contour_type *contour);
/* ********* */
int find_mskcc_box(contour_type *contour);
int find_box(contour_type *contour, Box_int *box);
int find_box(contour_type *contour, Box_and_rects *bnr);
int find_rect(struct conts_of_anat_str *anat_cont, Rect_int *rect);
int output_contour(contour_type *contour);
/* *********************************************************************** */
#endif
