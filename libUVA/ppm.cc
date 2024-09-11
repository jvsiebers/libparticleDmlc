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
/* write_ppm.cc
   routine to write a grayscale portable pixelmap file from an array
   
   pjk: 3/2/99: Created

  Modification History:  8/2/99 fixed memory leaks (LBS)
                         16 Oct 2000: PJK: added readPpm
     Feb 23, 2012: JVS: typecast's added to remove compiler warnings

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <sys/syscall.h>
// jvs headers
#include "typedefs.h"
#include "defined_values.h"
#include "err_codes.h" // for SUCCESS
#include "dose_regions.h"
#include "dose_region.h"
#include "utilities.h"
// pjk headers
#include "common.h"

/* *********************************************************************** */
int write_ppm(TWOD_ARRAY *im, char *fname, float norm_val) {

    // check as suggested by LBS
    if (im == NULL) {
        printf("\n ERROR: running null pointer passed to write_ppm");
        return (FAIL);
    }

    TWOD_ARRAY scaled_im;

    // malloc matrix
    scaled_im.x_count = im->x_count;
    scaled_im.y_count = im->y_count;
    scaled_im.matrix = (float *) calloc(scaled_im.x_count * scaled_im.y_count, sizeof(float));

    float scale_fac = norm_val;
    if (norm_val < 0.0) {
        // find maximum
        float max_val = -1.0;
        for (int j = 0; j < scaled_im.y_count; j++)
            for (int i = 0; i < scaled_im.x_count; i++)
                if (ARR_VAL(im, i, j) > max_val)
                    max_val = ARR_VAL(im, i, j);

        scale_fac = 1.0f / max_val;
    }
    for (int j = 0; j < scaled_im.y_count; j++)
        for (int i = 0; i < scaled_im.x_count; i++)
            ARR_VAL(&scaled_im, i, j) = ARR_VAL(im, i, j) * MAX_IM_VAL * scale_fac;

    if (write_ppm(&scaled_im, fname) != OK) {
        printf("\n ERROR: running write_ppm");
        return (FAIL);
    }

    free(scaled_im.matrix);

#ifdef DEBUG
    printf("\n Normal Program Termination\n");
#endif
    return (OK);
}

/* *********************************************************************** */
int write_ppm(TWOD_ARRAY *im, char *fname, float low_val, float high_val) {

    // check as suggested by LBS
    if (im == NULL) {
        printf("\n ERROR: running null pointer passed to write_ppm");
        return (FAIL);
    }

    TWOD_ARRAY scaled_im;

    // malloc matrix
    scaled_im.x_count = im->x_count;
    scaled_im.y_count = im->y_count;
    scaled_im.matrix = (float *) calloc(scaled_im.x_count * scaled_im.y_count, sizeof(float));

    if ((low_val < 0.0) || (high_val < 0.0) || (low_val > high_val)) {
        printf("\n Warning: low Val (%f) and high Val (%f) strange", low_val, high_val);
        //return(FAIL);
    }

    for (int j = 0; j < scaled_im.y_count; j++)
        for (int i = 0; i < scaled_im.x_count; i++) {
            ARR_VAL(&scaled_im, i, j) = ((ARR_VAL(im, i, j) - low_val) / (high_val - low_val)) * MAX_IM_VAL;
            if (ARR_VAL(&scaled_im, i, j) < 0.0)
                ARR_VAL(&scaled_im, i, j) = 0.0;
            if (ARR_VAL(&scaled_im, i, j) > MAX_IM_VAL)
                ARR_VAL(&scaled_im, i, j) = MAX_IM_VAL;
        }
    if (write_ppm(&scaled_im, fname) != OK) {
        printf("\n ERROR: running write_ppm");
        return (FAIL);
    }

    free(scaled_im.matrix);

#ifdef DEBUG
    printf("\n Normal Program Termination\n");
#endif
    return (OK);
}

/* *********************************************************************** */
int write_ppm(TWOD_ARRAY *im, char *fname) {
    // check as suggested by LBS
    if (im == NULL) {
        printf("\n ERROR: running null pointer passed to write_ppm");
        return (FAIL);
    }

    FILE *imfile;
    imfile = fopen(fname, "wb");
    if (imfile == NULL) {
        printf("\n ERROR: Can't open ppm file %s! \n", fname);
        return (FAIL);
    }
#ifdef DEBUG
    printf("\nOpened file %s. Now write it\n",fname);
#endif

    // check size
    if ((im->x_count < 1) || (im->y_count < 1)) {
        printf("\n ERROR: x size (%d) or ysize (%d) of image < 0 \n", im->x_count,
               im->y_count);
        return (FAIL);
    }

    if (im->x_count * im->y_count > MAX_IM_SIZE) {
        printf("\n ERROR: x size (%d) time ysize (%d) of image > max (%d) \n",
               im->x_count, im->y_count, MAX_IM_SIZE);
        return (FAIL);
    }

    // write image header
    // to understand more do a od -c imagename.ppm | more
    int k;
    char tval[MAX_STR_LEN];
    sprintf(tval, "P6");
    char ret[MAX_STR_LEN];
    sprintf(ret, "\n");
    fwrite(tval, 2, 1, imfile);
    fwrite(ret, 1, 1, imfile);
    sprintf(tval, "%d", im->x_count);
    k = (int) strlen(tval);
    fwrite(tval, k, 1, imfile);
    fwrite(ret, 1, 1, imfile);
    sprintf(tval, "%d", im->y_count);
    k = (int) strlen(tval);
    fwrite(tval, k, 1, imfile);
    fwrite(ret, 1, 1, imfile);
    sprintf(tval, "%d", MAX_IM_VAL);
    k = (int) strlen(tval);
    fwrite(tval, k, 1, imfile);
    fwrite(ret, 1, 1, imfile);

    // write image itself
    unsigned char sh_str;
    short unsigned int val;
    for (int j = 0; j < im->y_count; j++)
        for (int i = 0; i < im->x_count; i++) {
            // should do error check to please jvs
            val = (short) ARR_VAL(im, i, j);
            // (don't need as unsigned) if (val<0) val = 0;
            if (val > 255) val = 255;
            sh_str = (unsigned char) val;
            // one byte each for R G and B
            for (int p = 0; p < 3; p++) {
                fwrite(&sh_str, 1, 1, imfile);
            }
        }
    fclose(imfile);

#ifdef DEBUG
    char command[MAX_STR_LEN];
    sprintf(command,"xv %s &",fname);

    system(command);

    printf("\n Normal Program Termination\n");
#endif
    return (OK);
}

/* ************************************************************************* */
int readPpm(TWOD_ARRAY *im, char *fname) {
    // check as suggested by LBS
    if (im == NULL) {
        printf("\n ERROR: running null pointer passed to write_ppm");
        return (FAIL);
    }

    FILE *imfile;
    imfile = fopen(fname, "rb");
    if (imfile == NULL) {
        printf("\n ERROR: Can't open ppm file %s! \n", fname);
        return (FAIL);
    }
#ifdef DEBUG
    printf("\nOpened file %s. Now read it\n",fname);
#endif

    // write image header
    // to understand more do a od -c imagename.ppm | more
    int k;
    char tval[MAX_STR_LEN];
    char ret[MAX_STR_LEN];
    // fread(tval,2,1,imfile); // P6
    if (fread(tval, 2, 1, imfile) < 1) { // P6
        printf("\n ERROR: fread\n");
    }

    //fread(ret,1,1,imfile);
    //fread(tval,3,1,imfile);
    if (fscanf(imfile, "%s", tval) < 1) {
        printf("\n ERROR: fscanf\n");
    }

    ;
    im->x_count = atoi(tval);
    //fread(ret,1,1,imfile);
    //fread(tval,3,1,imfile);
    // fscanf(imfile,"%s",tval);
    if (fscanf(imfile, "%s", tval) < 1) {
        printf("\n ERROR: fscanf\n");
    }

    im->y_count = atoi(tval);

    if (fread(ret, 1, 1, imfile) < 1) {
        printf("\n ERROR: fread\n");
    }
    k = (int) strlen(tval);
    if (fread(tval, k, 1, imfile) < 1) {
        printf("\n ERROR: fread\n");
    }
    if (fread(ret, 1, 1, imfile) < 1) {
        printf("\n ERROR: fread\n");
    }
    k = (int) strlen(tval);
    if (fread(tval, k, 1, imfile) < 1) {
        printf("\n ERROR: fread\n");
    }
    if (fread(ret, 1, 1, imfile) < 1) {
        printf("\n ERROR: fread\n");
    }

    // read image itself
    unsigned char sh_str;
    short unsigned int val;
    for (int j = 0; j < im->y_count; j++)
        for (int i = 0; i < im->x_count; i++) {
            // one byte each for R G and B
            for (int p = 0; p < 3; p++) {
                if (fread(&sh_str, 1, 1, imfile) < 1) {
                    printf("\n ERROR: fread\n");
                }
            }
            val = sh_str;
            ARR_VAL(im, i, j) = (float) val;
        }
    fclose(imfile);

#ifdef DEBUG
    char command[MAX_STR_LEN];
    sprintf(command,"xv %s &",fname);

    system(command);

    printf("\n Normal Program Termination\n");
#endif

    // check size
    if ((im->x_count < 1) || (im->y_count < 1)) {
        printf("\n ERROR: x size (%d) or ysize (%d) of image < 1 \n", im->x_count,
               im->y_count);
        return (FAIL);
    }

    if (im->x_count * im->y_count > MAX_IM_SIZE) {
        printf("\n ERROR: x size (%d) time ysize (%d) of image > max (%d) \n",
               im->x_count, im->y_count, MAX_IM_SIZE);
        return (FAIL);
    }

    return (OK);
}
/* ************************************************************************* */


