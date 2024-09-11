/** \file 
 simple tag auto added to force generation of simple doxygen comments
**/
#ifndef phantomStructure_h
#define phantomStructure_h

/* definition of the structure needed for the egs4phantom....
   Originally coded by PJK.
   Moved to its own file October 17, 2007: JVS
   Nov 11, 2008: JVS: Change MAX_IM_VAL to MAX_N_BOUNDS, and set to 301
   Jan 12, 2010: JVS: Add writeEgsPhant to allow writing VCU and default EGS formats.
   Feb 16, 2010: JVS: Convert phantom to unit density
   Feb 17, 2010: JVS: Added overrideMaterialDensity
   Mar 10, 2010: JVS: Change MAX_N_BOUNDS to 514.....

*/
#define MAX_N_BOUNDS          514
#define MAX_MED             100
//#define MAX_STR_LEN         250

#ifndef MIN
#define MIN(a,b)                (((a) < (b)) ? (a) : (b))
#endif

#include <typedefs.h> // for Point_3d_int

typedef struct
{
    int   x_num, y_num, z_num, num_mat;
    float x_bound[MAX_N_BOUNDS], y_bound[MAX_N_BOUNDS], z_bound[MAX_N_BOUNDS];
    float x_size, y_size, z_size;
    float x_start, y_start, z_start;
    // char med_name[MAX_STR_LEN][MAX_MED];
    char med_name[MAX_MED][MAX_STR_LEN]; // July 9, 2000: JVS:
    float estep[MAX_MED];
    int *mednum;
    float *densval;
} PHANT_STRUCT;

/* Function Prototypes */
int read_phant(char *fname, PHANT_STRUCT *phant); // VCU format (%3d for medium, 99 materials maximum)
int readVCUEgsPhant(char *fname, PHANT_STRUCT *p); // VCU format (%3d for medium, 99 materials maximum)
int readDefaultEgsPhant(char *fname, PHANT_STRUCT *p); // Default EGS format (%1d for medium, 9 material maximum)
int readEgsPhant(char *fname, PHANT_STRUCT *p, const char *medFormat); // specify the format
int write_phant(char *fname, PHANT_STRUCT *phant);
int writeEgsPhant(char *fname, PHANT_STRUCT *phant, const char *medFormat);
int writeVCUEgsPhant(char *fname, PHANT_STRUCT *p);
int writeDefaultEgsPhant(char *fname, PHANT_STRUCT *p);
int concat_phant(PHANT_STRUCT *p);
int readPhantomBoundaries(FILE *istrm, int nBounds, float *bounds);
int read_egsphantheader(char *fname, PHANT_STRUCT *phant);
int write_itkheader(PHANT_STRUCT *p, char* dvfFilePath, char *itkDvfFileName);
int convertEgsPhantToUnitDensity(PHANT_STRUCT *p);
int writeEgsPhantAsCT(const char *fileName, PHANT_STRUCT *p);
int createRatioPhantom(PHANT_STRUCT *phantom1,PHANT_STRUCT *phantom2,PHANT_STRUCT *targetPhantom);
int compareEgsPhantProperties(PHANT_STRUCT *phantom1, PHANT_STRUCT *phantom2);
int copyEGSPhantom(PHANT_STRUCT *phantom1,PHANT_STRUCT *targetPhantom);
int overrideEGSMaterialDensity(const PHANT_STRUCT *phantom, const int materialID, const float newDensity);
int findEGSPhantBoundingBox(const PHANT_STRUCT *p, Point_3d_int *lowerCorner, Point_3d_int *upperCorner );

#endif
