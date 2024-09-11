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
/* common.h **********************************************************/
/* common header files for pjk's libraries
   Created 7 june 1999 pjk
   Modification history:
     9 June 1999:  LBS:  Added #ifndef COMMON_H_INCLUDED
     2 Sept 1999:  pjk: added integratesss
     5 April 2000: pjk: improved read_phant, added write_phant
     9 July  2000: JVS: Add EGS4PhantomType and readEGS4Phantom....
                        VERY similar to the PJK structures and routines, 
                        except in how the memory is dealt with
                        and how some things are named....to be more consistant 
                        with other code...NOT IMPLEMENTED..Time Constraints
     16 Oct 2000: PJK: added readPpm
     6 January 2003: PJK: Added Point_3d_Int and WARP_ARRAY
     24 July 2003: PJK: Added arrays for WARP_ARRAY so can use trilinear code
     11 Nov 2008: JVS: PHANT_STRUCT definition is now only in phantomStructure.h
*/

#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#ifndef  DOSE_REGION_H_INCLUDED
#    include "dose_region.h"
#endif// defines
#include "phantomStructure.h" // for PHANT_STRUCT

#define MAX_IM_SIZE         100000000
#define MAX_IM_VAL          255
#define MAX_DGRID_SIZE      201

// #define MAX_MED             100
// TWOD_ARRAY_VAL
#define ARR_VAL(GRID_ptr, i, j)\
    ((GRID_ptr)->matrix[(i) + (GRID_ptr)->x_count *(j)])

// KW define velocity vector here

/*
#define VELOCITY(GRID_ptr, i, j)\
    ((GRID_ptr)->matrix[(i) + (GRID_ptr)->x_count *(j)])
*/

#define ARR_VAL3D(GRID_ptr, i, j, k)\
    ((GRID_ptr)->matrix[(i) + (GRID_ptr)->x_count *(j) + (GRID_ptr)->x_count*(GRID_ptr)->y_count*(k)])

#define PK_CAL(a)    (a *)  calloc(1,sizeof(a))
#define PK_CALCHECK(a)  ( ((a)==NULL) ? (pexit(a)) : (printf(" ")) )   

#define NUM_SD 4 // number of std deviations to worry about
#ifndef MIN
#define MIN(a,b)                (((a) < (b)) ? (a) : (b))
#endif

// structures

typedef struct
{
    int x_count;
    int y_count;
    double max;
    double min;
    float *matrix;
} TWOD_ARRAY;

typedef struct
{
    int x_count;
    int y_count;
    int z_count;
    double max;
    double min;
    float *matrix;
} THREED_ARRAY;

// following are JVS's are only short ints not ints
typedef struct
{
  int x,y,z;
} Point_3d_Int;

/* KW: Added new variables to the structure WARP_ARRAY so that it is 
   compattible with matrix wrriten in PLUNC format.*/ 

typedef struct
{
    int version;
    Point_3d_Int   n;
    int num_t; 
    int type;
    int roi_flag;
    Point_3d_Int   dim;
    Point_3d_Int   offset;
    int compressed;
    float scale;
    Point_3d_float *matrix; 
    float *matrixX, *matrixY, *matrixZ; // added to use trilinear interpolation
} WARP_ARRAY;

/* PHANT_STRUCT is in phantomStructure.h 
typedef struct
{
    int   x_num, y_num, z_num, num_mat;
    float x_bound[MAX_IM_VAL], y_bound[MAX_IM_VAL], z_bound[MAX_IM_VAL];
    float x_size, y_size, z_size;
    float x_start, y_start, z_start;
    // char med_name[MAX_STR_LEN][MAX_MED];
    char med_name[MAX_MED][MAX_STR_LEN]; // July 9, 2000: JVS:
    float estep[MAX_MED];
    int *mednum;
    float *densval;
} PHANT_STRUCT;
*/

/*
typedef struct EGS4PhantomType
{
    Point_3d_int   nVoxels;   // int x_num, y_num, z_num;// number of voxels in each dimension
    Point_3d_float *Bounds;  // float x_bound[MAX_IM_VAL], y_bound[MAX_IM_VAL], z_bound[MAX_IM_VAL];
    Point_3d_float voxelSize;// float x_size, y_size, z_size; // size of voxels in each direction
    int   nMaterials;       // num_mat; // number of media/materials
    char  **materialName;   // name of each material  
    float *estep;           // estep values for each material
    int   *materialNumber;  // material number for each material
    float *density;         // densities   
};
*/

typedef struct
{
  float dose[MAX_DGRID_SIZE][MAX_DGRID_SIZE][MAX_DGRID_SIZE];
} DM_STRUCT;

typedef struct
{
    float index[MAX_DGRID_SIZE];
} ARRAY_STRUCT;

// prototypes
int write_ppm(TWOD_ARRAY *im, char *fname); // writes image without changing
int readPpm(TWOD_ARRAY *im, char *fname); // read image without changing
int write_ppm(TWOD_ARRAY *im, char *fname, float norm_val); 
// if norm_val < 0 scale max value in array to 255, else scale by norm_val
int write_ppm(TWOD_ARRAY *im, char *fname, float low_val, float high_val); 
int gauss(double *sum_rn);
int gauss(float *sum_rn);
// int read_phant(char *fname, PHANT_STRUCT *phant);
// int write_phant(char *fname, PHANT_STRUCT *phant);
int read_dose(char *fname, DM_STRUCT *dose, PHANT_STRUCT *phant);
int write_dose(char *fname, DM_STRUCT *dose, PHANT_STRUCT *phant);
// int concat_phant(PHANT_STRUCT *p);
int convolve_dose(volume_region_type *dr, float sml, float sap, float ssi, float *dose);
int convolve_dose(DM_STRUCT *dose, PHANT_STRUCT *phant, float sx, float sy, float sz);
int sum_dose(DM_STRUCT *dm, PHANT_STRUCT *p);
int int_gauss(int range, ARRAY_STRUCT *filter, int limits, float sigma);
int createArray(TWOD_ARRAY *array, int row, int col);

#endif

