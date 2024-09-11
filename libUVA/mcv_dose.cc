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
/* dose_regions.cc
   Reads in Dose Region (from Pinnacle Format)
   Created: 6/9/98: JVS
   Modification History:
      Jan 26, 1999: JVS: Add get_dose_region calls
      Feb 1,  1999: JVS: Pinn Comm Stuff Added to read selected dose distribution from Pinnacle
      Feb 4,  1999: JVS: Add read_pinnacle_dose_region(dr) for reading dr from ACTIVE Pinnacle session
      April 6, 1999: JVS: Add smarts so can tell if active Pinnacle session or not...
      April 22, 1999: JVS: Bug fix: Now reads dose grid for specific trial (uses read_clif now)
      June 29, 1999: JVS: Add read_pin_dose_grid and write_mcv_dose_header
      Nov 5, 2007: JVS: get rid of myerrno
      Nov 27, 2007: JVS const char *
      Feb 15, 2011: JVS: Overload writeMCVDoseHeader 
      Mar 26, 2012: JVS: Get rid of dregs after 6th significant digit in dose matrix VoxelSize
      Mar 28, 2012: JVS: VoxelSize is limited to 5 digits past the decimal point
     Dec 25, 2015: JVS: Add readPinnacleDose file -- clipped from a program and create writePinnacleDoseFile
*********************************************************************************** */
#include <stdlib.h>      // for calloc
#include <stdio.h>
#include <string.h>      // for strcat and strcpy
#include <unistd.h> // for getcwd
#include <math.h> // for round
// #include "string_util.h" // for get_value
#include "utilities.h"   // for error routines
// #include "typedefs.h"
// #include "dose_region.h" // for definition of dose regions structure and function prototypes
#include "read_clif.h"
#include "readWriteEndianFile.h" // Mar 31, 2010: JVS : Headers now here
#include "mcv_dose.h"
/* ************************************************************************************************* */
/* ********************************************************************************* */
int readPinnacleDoseFile(char *doseFileName, volume_region_type *dg, float **dose){
  char doseUnits[MAX_STR_LEN];
  float fieldWeight = -1.;
  int nDoseArray=dg->n.x*dg->n.y*dg->n.z;
  // Read in dose file
  if( (read_mcv_dose_header(doseFileName, dg, doseUnits,&fieldWeight) != OK) ||
      (nDoseArray = dg->n.x*dg->n.y*dg->n.z ) <= 0 ||
      (readUnknownEndianBinaryDataFromFile(doseFileName,nDoseArray, dose) != OK) ){
         printf("\n ERROR: %s: Reading Pinnacle Dose File %s", __FUNCTION__, doseFileName);return(FAIL);
  }
  return(OK);
}
/* ********************************************************************************* */
int writePinnacleDoseFile(char *doseFileName, volume_region_type *dg, float *dose){
  char doseUnits[MAX_STR_LEN];
  strcpy(doseUnits,"unspecified");
  float fieldWeight = -1.;
  int nDoseArray=dg->n.x*dg->n.y*dg->n.z;
  // writeDose file
  if( (write_mcv_dose_header(doseFileName, dg, doseUnits, fieldWeight) != OK) ||
      (writeBigEndianBinaryFile((const char *) doseFileName,nDoseArray, dose) != OK) ){
         printf("\n ERROR: %s: Writing Pinnacle Dose File %s", __FUNCTION__, doseFileName);return(FAIL);
  }
  return(OK);
}
/* ********************************************************************************* */
int write_mcv_dose_header(char *filename, volume_region_type *dg, const char *file_type, float weight)
{
   /* write "header" information */
   char pindosefile[MAX_STR_LEN];
   sprintf(pindosefile,"%s.header",filename);
   FILE *pin_strm = fopen(pindosefile,"w");
   if (pin_strm == NULL)
   {
        printf("\n ERROR: %s: Can't open pinnacle header file %s \n", __FUNCTION__, pindosefile);
        char cwd[1024];
	if (getcwd(cwd, sizeof(cwd)) != NULL)
	  printf("\n\tCurrent working dir: %s\n", cwd);
	else
	  perror("ERRPR: Cannot determine current directory using getcwd(), error ="); 
        return(FAIL);
   }
     /* add any comments desired here */
     /* non-standard things... */
   fprintf(pin_strm,"DoseGrid .units = \"%s\";\n",file_type); /* tell if MCV or DOSXYZ */
   fprintf(pin_strm,"DoseGrid .weight = %f;\n", weight);/* fraction of run (if positive), if negative, unknown */
     /* standard things in the Pinnacle Dose grid description */
     fprintf(pin_strm,"DoseGrid .VoxelSize .X = %.5f;\n",dg->voxel_size.x);
     fprintf(pin_strm,"DoseGrid .VoxelSize .Y = %.5f;\n",dg->voxel_size.y);
     fprintf(pin_strm,"DoseGrid .VoxelSize .Z = %.5f;\n",dg->voxel_size.z);
     fprintf(pin_strm,"DoseGrid .Dimension .X = %d;\n",dg->n.x);
     fprintf(pin_strm,"DoseGrid .Dimension .Y = %d;\n",dg->n.y);
     fprintf(pin_strm,"DoseGrid .Dimension .Z = %d;\n",dg->n.z);
     fprintf(pin_strm,"DoseGrid .Origin .X = %f;\n",dg->origin.x);
     fprintf(pin_strm,"DoseGrid .Origin .Y = %f;\n",dg->origin.y);
     fprintf(pin_strm,"DoseGrid .Origin .Z = %f;\n",dg->origin.z);
     fclose(pin_strm);

   return(OK);
}
/* ************************************************************************************************* */
int writeMCVDoseHeader(char *filename, volume_region_type *dg, int fileEndian)
{
  int byteOrder;   // 0 means little endian, 1 means big endian
  switch(fileEndian )
  {
     case BIG_ENDIAN:
       byteOrder=1;
       break;
     case LITTLE_ENDIAN:
       byteOrder=0;
       break;
      default:
	printf("\n ERROR: fileEndian == %d is indeterminate byte order\n",fileEndian);
	return(FAIL);
  }
   /* write "header" information */
   char pindosefile[MAX_STR_LEN];
   sprintf(pindosefile,"%s.header",filename);
   FILE *pin_strm = fopen(pindosefile,"w");
   if (pin_strm == NULL)
   {
        printf("\n ERROR: Can't open pinnacle header file %s \n", pindosefile);
        char cwd[1024];
	if (getcwd(cwd, sizeof(cwd)) != NULL)
	  printf("\n\tCurrent working dir: %s\n", cwd);
	else
	  perror("ERROR: Cannot determine current directory using getcwd(), error ="); 
        return(FAIL);
   }
     /* add any comments desired here */
     /* non-standard things... */
     /* standard things in the Pinnacle Dose grid description */
     fprintf(pin_strm,"DoseGrid .VoxelSize .X = %f;\n",dg->voxel_size.x);
     fprintf(pin_strm,"DoseGrid .VoxelSize .Y = %f;\n",dg->voxel_size.y);
     fprintf(pin_strm,"DoseGrid .VoxelSize .Z = %f;\n",dg->voxel_size.z);
     fprintf(pin_strm,"DoseGrid .Dimension .X = %d;\n",dg->n.x);
     fprintf(pin_strm,"DoseGrid .Dimension .Y = %d;\n",dg->n.y);
     fprintf(pin_strm,"DoseGrid .Dimension .Z = %d;\n",dg->n.z);
     fprintf(pin_strm,"DoseGrid .Origin .X = %f;\n",dg->origin.x);
     fprintf(pin_strm,"DoseGrid .Origin .Y = %f;\n",dg->origin.y);
     fprintf(pin_strm,"DoseGrid .Origin .Z = %f;\n",dg->origin.z);
     fprintf(pin_strm,"DoseGrid .BinaryOrder .Endian = %d;\n",byteOrder);
     fclose(pin_strm);

   return(OK);
}
/* ************************************************************************** */
// #include "volume_region.h"
bool readDoseMatrix(const char *fileName, volume_region_type *dg)
{
  FILE *istrm = fopen(fileName,"r");
  if(NULL == istrm){
    printf("\n ERROR: readDoseMatrix: Cannot open %s\n",fileName); return false;
  }
  clifObjectType *doseGridClif = new(clifObjectType);
  if (OK != readClifFile(istrm,doseGridClif,"doseGrid")){
    printf("\n ERROR: readDoseMatrix: reading doseGridClif\n"); return false;
  }
  fclose(istrm);
  //
  //printf("\n Before ");
  //printf("\n dg->voxelSize = (%f,%f,%f)",dg->voxel_size.x, dg->voxel_size.y, dg->voxel_size.z);
  //printf("\n dg->origin = (%f,%f,%f)",dg->origin.x, dg->origin.y, dg->origin.z);
  //printf("\n dg->dimension = (%d,%d,%d)",dg->n.x, dg->n.y, dg->n.z);
  // Assign values from the CLIF
  int nx,ny,nz;
  nx=ny=nz=0;
  if(     OK != readFloatClifStructure(doseGridClif,"DoseGrid.VoxelSize.X",&dg->voxel_size.x)
       || OK != readFloatClifStructure(doseGridClif,"DoseGrid.VoxelSize.Y",&dg->voxel_size.y)
       || OK != readFloatClifStructure(doseGridClif,"DoseGrid.VoxelSize.Z",&dg->voxel_size.z)
       || OK != readFloatClifStructure(doseGridClif,"DoseGrid.Origin.X",&dg->origin.x)
       || OK != readFloatClifStructure(doseGridClif,"DoseGrid.Origin.Y",&dg->origin.y)
       || OK != readFloatClifStructure(doseGridClif,"DoseGrid.Origin.Z",&dg->origin.z)
       || OK != readIntClifStructure(doseGridClif,"DoseGrid.Dimension.X",&nx)
       || OK != readIntClifStructure(doseGridClif,"DoseGrid.Dimension.Y",&ny)
       || OK != readIntClifStructure(doseGridClif,"DoseGrid.Dimension.Z",&nz)
    ) {
    printf("\n ERROR: readDoseMatrix: Reading values from clif structure\n"); return false;
  }
  dg->n.x=(short)nx;
  dg->n.y=(short)ny;
  dg->n.z=(short)nz;
  delete(doseGridClif);
  //printf("\n After ");
  //printf("\n dg->voxelSize = (%f,%f,%f)",dg->voxel_size.x, dg->voxel_size.y, dg->voxel_size.z);
  //printf("\n dg->origin = (%f,%f,%f)",dg->origin.x, dg->origin.y, dg->origin.z);
  //printf("\n dg->dimension = (%d,%d,%d)",dg->n.x, dg->n.y, dg->n.z);
  return true;
}
/* *********************************************************************************************** */
int read_mcv_dose_header(char *filename, volume_region_type *dg, char *file_type, float *weight)
{
      char idosefile[MAX_STR_LEN];

      sprintf(idosefile, "%s.header", filename);
      FILE *istrm = fopen(idosefile,"r"); /* open the stream */
      if (istrm == NULL){
         printf("\n ERROR: %s: Can't open file %s! \n",__FUNCTION__,idosefile);return(FAIL);
      }
      if(clif_string_read(istrm,"DoseGrid .units =",file_type)!=OK){
         printf("\n ERROR: Reading dose grid units"); return(FAIL);
      }
      *weight = clif_get_value(istrm,"DoseGrid .weight =");
      if(checkClifError() != OK){
         printf("\n ERROR: Reading Weight"); return(FAIL);
      }
      if(read_pin_dose_grid(istrm, dg) != OK){
         printf("\n ERROR: Reading In Pinnacle Dose Grid");return(FAIL);
      }
      fclose(istrm);

   return(OK);
}
/* ********************************************************************************* */
int read_pin_dose_grid(FILE *fp, volume_region_type *dose_grid)
{
   // read in dose region
   dose_grid->voxel_size.x = clif_get_value(fp,"DoseGrid .VoxelSize .X =");
   dose_grid->voxel_size.y = clif_get_value(fp,"DoseGrid .VoxelSize .Y =");
   dose_grid->voxel_size.z = clif_get_value(fp,"DoseGrid .VoxelSize .Z =");
   if(checkClifError() != OK)
   {
      printf("\n ERROR: Reading in DoseGrid. VoxelSize from file ");
      return(FAIL);
   }
   float scaleFactor=1.0f/100000.0f;
   // get rid of extraneous garbage at the end of the voxel size... not clear if this makes any difference or not....
   dose_grid->voxel_size.x = (float) (scaleFactor*float(int(dose_grid->voxel_size.x/scaleFactor)));
   dose_grid->voxel_size.y = (float) (scaleFactor*float(int(dose_grid->voxel_size.y/scaleFactor)));
   dose_grid->voxel_size.z = (float) (scaleFactor*float(int(dose_grid->voxel_size.z/scaleFactor)));

   dose_grid->n.x =  (short) clif_get_value(fp,"DoseGrid .Dimension .X =");
   dose_grid->n.y =  (short) clif_get_value(fp,"DoseGrid .Dimension .Y =");
   dose_grid->n.z =  (short) clif_get_value(fp,"DoseGrid .Dimension .Z =");
   if(checkClifError() != OK)
   {
      printf("\n ERROR: Reading in DoseGrid .Dimension from file");
      return(FAIL);
   }
   dose_grid->origin.x =  clif_get_value(fp,"DoseGrid .Origin .X =");
   dose_grid->origin.y =  clif_get_value(fp,"DoseGrid .Origin .Y =");
   dose_grid->origin.z =  clif_get_value(fp,"DoseGrid .Origin .Z =");
   if(checkClifError() != OK)
   {
      printf("\n ERROR: Reading in DoseGrid .Origin from file");
      return(FAIL);
   }
#ifdef DEBUG
   printf("\n voxel_size = %f %f %f", dose_grid->voxel_size.x, dose_grid->voxel_size.y, dose_grid->voxel_size.z);
   printf("\n remainder  = %g %g %g", dose_grid->voxel_size.x-0.4f, dose_grid->voxel_size.y-0.4f, dose_grid->voxel_size.z-0.4f);
   printf("\n round = %g", round(1.0e6*dose_grid->voxel_size.z));
#endif
   return(OK);
}
/* **************************************************************************** */
/* ********************************************************************** */
int writeRCFDoseConfig(const char *fileName, const char *directory) {
  char configFileName[MAX_STR_LEN];
  if(strlen(directory))
    sprintf(configFileName,"%s/%s.config",directory,fileName);
  else
    sprintf(configFileName,"%s.config",fileName);
  FILE *strm = fopen(configFileName,"w");
  if (NULL == strm ) {
    printf("\n ERROR: Cannot create file %s in directory %s\n", configFileName,directory); return(FAIL);
  }
  fprintf(strm,"TYPE:DOSE:\n");
  fprintf(strm,"FORMAT:VCUPINN:\n");
  fprintf(strm,"NAME:%s:\n",fileName);
  fprintf(strm,"PATH:%s:\n",directory);
  fclose(strm);
  return(OK);
}
/* ********************************************************************** */
int splitFileNameFromPath(const char *iString, char *path, char *fileName) {
  int iSplit=(int) strlen(iString);
  if(iSplit == 0) {
    printf("\n ERROR: empty string passed to splitFileNameFromPath\n"); return(FAIL);
  }
  // printf("\n Initial String: len=%d, val=%c", iSplit,iString[iSplit]);
  int found=0;
  while(!found && --iSplit > 0 ) {
    if( iString[iSplit] == '/' ) found=1;
  }
  if(iSplit > (int) (strlen(iString)-1)) {
    printf("\n ERROR: cannot find file name on %s", iString);
  }
  if(iSplit>0) { //
    strcpy(fileName,iString+iSplit+1);
    strncpy(path,iString,iSplit);
    path[iSplit]=0; // null terminate the string....
  }else {
    strcpy(fileName, iString);
  }
  if(0==strlen(fileName)) {
    printf("\n ERROR: zero length fileName parsed from %s ", iString); return(FAIL);
  }
  // printf("\n iString[%d]=%c", iSplit, iString[iSplit]);
  // printf("\n fileName = %s\npath = %s\n", fileName, path);
  return(OK);
}
/* ********************************************************************** */

int writeRCFDoseConfig(const char *fileName) {
  char baseFileName[MAX_STR_LEN];
  char directory[MAX_STR_LEN];
  directory[0] = baseFileName[0] = 0;
  // Check for directory in the filename
  if(!splitFileNameFromPath(fileName, directory, baseFileName) ) {
    printf("\n ERROR: splitFileNameFromPath\n"); return(FAIL);
  }
  if(0 == strlen(directory) ) {
    // Get current directory
    if (getcwd(directory, sizeof(directory)) != NULL) {
      // printf("\n\tCurrent working dir: %s\n", cwd);
    } else {
      perror("ERROR: Cannot determine current directory using getcwd(), error ="); 
      return(FAIL);
    }
  }
  return(writeRCFDoseConfig(baseFileName,directory));
}
/* ********************************************************************** */
bool readBinaryDataFileWithHeader(char *iFile, volume_region_type *DoseGrid, float **fieldDose) {
    // Read in the Header and Dose file
    char doseHeaderFileName[MAX_STR_LEN];
    sprintf(doseHeaderFileName, "%s.header", iFile);
    if (! readDoseMatrix(doseHeaderFileName, DoseGrid) ){
        printf("\n ERROR: Reading doseHeaderFileName"); return false;
    }
    int nDoseArray = DoseGrid->n.x * DoseGrid->n.y * DoseGrid->n.z;
    // The Pinnacle Dose Grid is inverted in Y....
    DoseGrid->origin.y += (float) ((float) (DoseGrid->n.y - 1) * DoseGrid->voxel_size.y);
    DoseGrid->voxel_size.y *= -1;
    if (OK != readUnknownEndianBinaryDataFromFile(iFile, nDoseArray, fieldDose)) {
        printf("\n ERROR: Reading Pinnacle Dose File %s", iFile);
        return (false);
    }
    return (true);
}
/* ************************************ */
