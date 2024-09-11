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
/*  clipped_img.h 

Modification History:
   5/27/98: JVS: for clipped_img only
   6/05/98: JVS: add prototypes for read_clipped_img
   9/04/98: JVS: remove num_dims (not needed, always 3)
   Dec 10, 1998: JVS: Add offset of the image system 
                      (MSKCC System Assumed origin was at (0,0,Z), but Pinnacle and
                      other systems allow the image to start at an arbitrary location.)

   Dec 22, 1998: JVS: Adding Explaination, cleaning up, Add y_shift
   April 26, 2000: JVS: Replace y_shift with y_slope and y_intercept
   April 27, 2000: JVS: Replace offsets AND y_slope and y_intercept with translation_type
   April 28, 2000: JVS: Additional updates (not yet implemented) changing the image_header


Explaination of Clipped Image:
   A clipped-image is created from a CT-data set and an outer patient contour (SKIN).
   CT Data outside of the SKIN is discarded.  Data within the SKIN is truncated be
   an 8 bit image.
   The coordinate system for the clipped image 
      Pixel 0,0 has coordinates 0.0, 0.0
           X
       |------>
       |
      Y|
       |
      \/
         
Contours are given in a system with
       ^
       |
       |
      Y|
       |------->
           X

These two systems are related by y_shift

    y_image = y_shift - y_contour
    y_image_system = y_intercept + y_slope * y_contour;


Structure names: clipped_img_str

File name: clipped_img.str

Description: 

Valid pixel types are FLOAT_TYPE, SHORT_TYPE, BYTE_TYPE, and BIT_TYPE.

2D or 3D Image of Sections:

   Image sections are assumed to be equispaced in the x (column) and y 
   (row) directions but may or may not be so along the z direction.  
   Coordinate system the the 3D image is always right-handed system.  
   Image sections may be along transverse, sagittal, coronal or arbitrary 
   directions.  The transform field of the image provides data to convert 
   orientation to the reference system, typically the coordinate system 
   of the CT scanner.

   Note that, in order to facilitate digitizing of outlines from hard 
   copies, it is necessary to define "virtual" images.  The 
   distinguishing property of hard copy images is that their geometry 
   is defined but their pixel values are not available (the pointer is 
   set to NULL).  Simulator films, beam films and patients contours drawn 
   on a piece of paper are examples of hard copy images.

See coordinate systems document for conventions.  

Defined by:   RM, LB, BB

Date(s)/Author(s):   Jan 15, 92/RM
         June 92, BB
         April 94, RM,  added clipped image density
                        July 94, VL, added object_id and object_valid
                        Jan 95,  VL, added KVP field
*/
#ifndef   CLIPPED_IMG_STR_INCLUDED
#define   CLIPPED_IMG_STR_INCLUDED
#include "translation.h" /* make sure this is included */
/* #include "dose_region.h" */ /* for defintion of volume_region_type */
/*** image header, must be a part of all image headers */
typedef struct img_hdr_str
{
   /***********IMAGE GEOMETRY ***************************/
   /* FLOAT, SHORT, BYTE, BIT, NONE */
   int    pix_type;
   /* int num_dims; */     /* number of dimensions */
    /* volume_region_type pixel; */ /* contains origin, voxel_size, number of voxels for each direction */
   /* Number of pixels in each slice of this image (which may be modified, e.g. clipped). */
   int   num_x_pixs,
         num_y_pixs;
   /* Pixel dimensions in the image. */
   float x_pix_size,
         y_pix_size,
         z_pix_size;   /* z_pix_size is optional. It is specified only for equispaced */
                       /* set of images.  For clipped image, it is set = x_pix_size. */
   /* float y_shift;*/ /* shift between the image system and the system that contours are given in */
   /* float y_slope; */    /* slope and intercept between image systems */
   /* float y_intercept; */ /* */

   /* Point_3d_float offset; */ /* offset of the clipped image origin from the image origin */
   translation_type trans;  /* translation between coordinate systems */

   int   num_img_secs;    /* number of slices - set to 1 for a 2D image */

   float   *img_sec_zs;   /* Pointer to the array of z coords (cm) of the image */

   Logical min_max_defined;    /* set to FALSE if min_pix, max_pix not defined, else TRUE */

   float min_pix, max_pix;     /* min and max pixel values in image */

   float range_low, range_high; /* legal range for pixels */
   /* maximal pixel value for conversion to BYTE image -
      This will correspond to pixel value 255 in a byte image,
      and all higher values will be set to this value */
   float max_byte_pix;
   /* number of thicknesses - 0 if not available, 1 if all the same,
      else the number of image sections */
   int    num_img_sec_thicknesses;

   float   *img_sec_thicknesses;   /* Array of thicknesses. */
}img_hdr_type;


typedef struct clipped_img_str
{
   img_hdr_type img_hdr;

   /* pointer to cuboid and enclosing rectangles for each slice */   
   Box_and_rects box_and_rects;

   float conversion_factor;
   
   int   orig_num_x_pixs,         /* Dimensions of the original image. */
         orig_num_y_pixs;

   int size_of_pix_matrix;
   Byte *pix_matrix;

   int size_of_bit_map;

   Bitvec bit_map;

   float   edrs [EDR_TBL_SIZE];            /* For CT numbers from 0 to rang_high * conversion_factor. */

   struct eqs_1d_ptr_neighb_str *eqs_1d_ptr_neighb;

   struct eqs_1d_ptr_interp_str *eqs_1d_ptr_interp;

   float   density;               /* Applicable to homogeneous objects only */
                        /* Various programs set to 1, if none is specified. */
}clipped_img_type;

/* function prototypes */
Function_type read_clipped_image( char *filename, clipped_img_type *clp_img);
Function_type read_clipped_img (char *work_area, char *pt_name, char * img_set_name,
char * input_file, clipped_img_type *clipped_img);


#endif   /* IMG_STR_INCLUDED */
