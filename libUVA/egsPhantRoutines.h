/* egsPhantRoutines.h
   Created: Feb 23, 2012: JVS: Additional routines used with the PHANT_STRUCT data type
            Mar 12, 2012: JVS: Add getPhantomMaterials and assignEstep.
            Feb 18, 2013: JVS: Add regularizeDensity...

*/
#ifndef _egsPhantRoutines_h_included
#define _egsPhantRoutines_h_included


#include "phantomStructure.h" // PJK's read_phant routine and PHANT_STRUCT type def....
#include "readWriteEndianFile.h" // For writeBinaryDataToFile and reverse_float_byte_order
#include "mediaConfig.h" // for egs material type

int regularizePhantomBoundaries(float *bounds, int nBounds);
int reportOnPhantomDensity(PHANT_STRUCT *phantom);
int ensureWaterIsInPhantomDensity(PHANT_STRUCT *phantom);
int TESTwriteVoxelBoundaries(FILE *outputStream, int swab_flag, int nVoxels, float voxelOrigin, float voxelSize, float *bounds);
int writeVoxelBoundaries(FILE *outputStream, int swab_flag, int nVoxels, float voxelOrigin, float voxelSize);
int writeVMCppLittleEndianBinaryDensityFile(char *densityFileName,  PHANT_STRUCT *phantom);
/* ******************************************************* */
int getPhantomMaterials(PHANT_STRUCT *sourcePhantom);
int assignEstep(PHANT_STRUCT *phantom, const float eStep);
int dumpBounds(PHANT_STRUCT *phantom);
int readVMCPhantomFile(PHANT_STRUCT *phantom, char* fileName);
bool regularizeDensity(PHANT_STRUCT *sourcePhantom, float threshold, float newDensity);
// from egsPhantMassMapping.cpp May 12, 2013
int writeEgsPhantDensitiesAsBinary(char *fileName, PHANT_STRUCT *p);
int copyPhantomZeroDensity(PHANT_STRUCT *source, PHANT_STRUCT *destination);
int qaAndRepairPhantomBoundaries(PHANT_STRUCT *phantom);

#endif
