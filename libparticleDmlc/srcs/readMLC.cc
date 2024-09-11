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
/* readMLC.cc
   CopyBLeaf 2000: MCV
   Reads in MLC config file

   Modification History:
     Aug. 17, 2000: JVS: Created
     Jan 3, 2001:  JVS: Add readParticleMLC...\
     Jan 5, 2001: JVS: Add temp routine for readMLCIndexTable...
     Jan 16, 2001: JVS: Add grooveDistance and grooveThickness
     May 21, 2001: JVS: Add routines to read VarianMlcDataType
     Feb 5, 2002: JVS: make so will actually read from UVA_CONFIG (not always just general)
     April 12, 2002: JVS: Add physicalLeafOffset
     April 22, 2003: JVS: Have read configs from $UVA_DATA/dmlc
     May 13, 2004: JVS: Fix so reports full file name of config file...
     Nov 01, 2004: IBM: Changed the path where the MLC config fileas are read from. (for DEBUG purposes)
     Feb 2, 2005: JVS: Add freeClifStructure before delete operator to get rid of memory leaks reported by insure.
    Sept 11, 2006: JVS: Merge with IBM code  Config file name VCUparticleDmlc.config
       Feb 7, 2007: JVS: Add C_CLASS parts
    Nov 5, 2007: JVS: eliminate myerrno
    Nov 11, 2008: JVS: Remove warnings for c++ class 
                  "warning: declaration of 'someVariable' shadows a member of 'this'"
		  for routines 
		  readParticleMLC
		  readMLC

*/
// #define DEBUG_OPEN
#ifdef MAIN
float Revision=0.01;
char *Prog_Name="readMLC";
#endif
#include <stdio.h>
#include <stdlib.h>
// #include <iostream.h>
// #include <iomanip.h>
#include <string.h> // for strcmp
#include <math.h> // for fabs
#include "utilities.h"
#include "typedefs.h" // for definition of Point_2d_float
#include "table.h"    // for table_type
#include "read_clif.h"
#include "string_util.h" // for get_value
//#include "readPinnacle.h"
#ifdef C_CLASS
#include "particleDmlcClass.h"
#else
#include "particleDmlc.h"
#endif
#define SMALL_FP_LIMIT 1.e-5
/* ******************************************************************* */
#ifdef MAIN
int main()
{ 
  /*
   printf("\n Test program for reading in MLC configuration\n");
   mlcType mlc;
   if( readMLC(&mlc,"80") != OK ) 
   {
      printf("\n ERROR: Reading MLC config\n"); return(FAIL);
   }
  */
  VarianMlcDataType VarianMLC;
  if( readVarianMLCTable(&VarianMLC) != OK)
  {
    printf("\n ERROR: Reading VarianMLC Config\n"); return(FAIL);
  }
  /* Test To Get Physical Leaf Gap */
  checkPhysicalLeafGap(&VarianMLC);
  printf("\n");
  return(OK);
}
#endif
/* *********************************************************************** */
#ifdef C_CLASS
int particleDmlc::checkPhysicalLeafGap(VarianMlcDataType *VarianMLC)
#else
int checkPhysicalLeafGap(VarianMlcDataType *VarianMLC)
#endif
{
   char iString[MAX_STR_LEN];
   do{
   float aLeaf=0;  printf("\n Input A-Leaf Position > ");
   if ( NULL == fgets(iString,MAX_STR_LEN,stdin)) {
       printf("\n ERROR: checkPhysicalLeafGap: EOF on read\n"); return(FAIL);
   }
   sscanf(iString,"%f",&aLeaf);
   float bLeaf=0;  printf("\n Input B-Leaf Position > ");
   if ( NULL == fgets(iString,MAX_STR_LEN,stdin)) {
       printf("\n ERROR: checkPhysicalLeafGap: EOF on read\n"); return(FAIL);
   }
   sscanf(iString,"%f",&bLeaf);
   float aAtIso = table_value(aLeaf, &VarianMLC->data);
   float bAtIso = table_value(bLeaf, &VarianMLC->data);
   float aAtLeaf = aAtIso / VarianMLC->MagFactor;
   float bAtLeaf = bAtIso / VarianMLC->MagFactor;
   printf("\n %f %f %f %f %f %f %f", aLeaf, bLeaf, aAtIso, bAtIso, aAtLeaf, bAtLeaf, aAtLeaf+bAtLeaf);
   }while(ok_checks("Do Another ? > "));

   
    return(OK);
}
/* *********************************************************************** */
FILE *openDMLCConfigFile(char *filename)
{
  char fullFileName[MAX_STR_LEN];
  FILE *strm;
  strm = NULL;
  if(NULL == filename) {
    printf("\n ERROR: openDMLCConfigFile: cannot open NULL filename\n"); return(NULL);
  }
#ifdef DEBUG_OPEN
  printf("\n openDMLCConfigFile looking for file %s\n", filename);
#endif
  char *path=getenv("UVA_DATA");
  if(NULL == path) {
    printf("\n\t UVA_DATA environment variable not set. Cannot look in $UVA_DATA/dmlc");
  } else {
    sprintf(fullFileName,"%s/dmlc/%s",path,filename);
    strm = fopen(fullFileName,"r");
  } 
  if( strm== NULL) {
      printf("\n Cannot find %s...", fullFileName);
      path = getenv("UVA_CONFIG");
      if(path != NULL) {
         sprintf(fullFileName,"%s/%s",path,filename);
         printf("Trying %s", fullFileName);
         strm = fopen(fullFileName,"r");
      }
  }
  if( strm== NULL) {
      printf("\n Cannot find %s...", fullFileName);
      path = getenv("UVA_CONFIG");
      if(path != NULL) {
         sprintf(fullFileName,"%s/../general/%s",path,filename);
         printf("Trying %s", fullFileName);
         strm = fopen(fullFileName,"r");
      }
  }
  if( strm== NULL) {
      printf("\n Cannot find %s...", fullFileName);
      path = getenv("UVA_CONFIG");
      if(path != NULL) {
         sprintf(fullFileName,"%s/general/%s",path,filename);
         printf("Trying %s", fullFileName);
         strm = fopen(fullFileName,"r");
      }
  }
  if(strm == NULL) {
    printf("\n ERROR: Opening file %s in %s/dmlc, %s, or %s/../general", 
	   filename, getenv("UVA_DATA"),getenv("UVA_CONFIG"), getenv("UVA_CONFIG"));
    return(NULL);
  }
  strcpy(filename,fullFileName); /* copy full path back to aid in error reporting */
  return(strm);
}
/* *********************************************************************** */
#ifdef C_CLASS
int particleDmlc::readVarianMLCTable(VarianMlcDataType *VarianMLC)
#else
int readVarianMLCTable(VarianMlcDataType *VarianMLC)
#endif
{
  char fileName[MAX_STR_LEN];
  strcpy(fileName,"mlctable.txt");
  FILE *istrm = openDMLCConfigFile(fileName);
  if(istrm == NULL){
    printf("\n ERROR: readVarianMLCTable"); return(FAIL); 
  }
  VarianMLC->MagFactor = get_value(istrm,"Ratio =");
#ifdef DEBUG
  printf("\n Mag Factor Is %f", VarianMLC->MagFactor);
#endif
  if(checkStringError() != OK) {
    printf("\n ERROR: Getting Ratio mag factor from file %s",fileName); return(FAIL);
  }
  if(get_to_line( istrm, "Data") != OK){
    printf("\n ERROR: Finding Data in file %s", fileName); return(FAIL);
  }
  // Read The Data In as a 2d Table 
  char iString[MAX_STR_LEN];
  VarianMLC->data.n_elements=0;
  while(fgets(iString, MAX_STR_LEN, istrm) != NULL){
    float xTemp, yTemp;
    if(sscanf(iString,"%f : %f", &xTemp, &yTemp) == 2){
      VarianMLC->data.x[VarianMLC->data.n_elements] = xTemp;
      VarianMLC->data.y[VarianMLC->data.n_elements] = yTemp;
      VarianMLC->data.n_elements++;
    }
  }
  fclose(istrm);
#ifdef DEBUG
  printf("\n %d Values Read from file %s", VarianMLC->data.n_elements, fileName);
  for(int i=0; i<VarianMLC->data.n_elements; i++){
    printf("\n %f %f", VarianMLC->data.x[i], VarianMLC->data.y[i]);
  }
#endif
  return(OK);
}
/* *************************************************************************** */
#ifdef C_CLASS
int particleDmlc::readMLCIndexTable(int nLeaves, mlcSectionType *section)
#else
int readMLCIndexTable(int nLeaves, mlcSectionType *section)
#endif
{ // quick temp routine to read mlc index table... should be updated
  // but need some quick implementation
  // Get the file name and open it
   char fileName[2*MAX_STR_LEN];
   sprintf(fileName,"%s.table",section->Name);
   FILE *istrm = openDMLCConfigFile(fileName);
   if(istrm == NULL)
   {
      printf("\n ERROR: readMLCIndexTable"); return(FAIL); 
   }
#ifdef DEBUG
  printf("\n\t\t Reading in %s", fileName);
#endif
  // Determine the number of entries in the file
  section->nIndex = (int) get_value(istrm,"Number of Lines =");
  
  float pzCenterSection =  get_value(istrm,"Leaf Center =");
  float pzEntranceSection =  get_value(istrm,"Leaf Entrance =");
  float pzExitSection =  get_value(istrm,"Leaf Exit =");
  
  if(    fabs(pzCenterSection - section->pzCenter )   > SMALL_FP_LIMIT 
      || fabs(pzEntranceSection - section->pzEnter ) >  SMALL_FP_LIMIT
      || fabs(pzExitSection - section->pzExit )      > SMALL_FP_LIMIT )

  {
    printf("\n ERROR: Leaf Sections Not Match");
    printf("\n \t Center mlcConfig %f, %s %f", section->pzCenter, fileName, pzCenterSection);
    printf("\n \t Entrance mlcConfig %f, %s %f", section->pzEnter, fileName, pzEntranceSection);
    printf("\n \t Exit mlcConfig %f, %s %f", section->pzExit, fileName, pzExitSection);
    return(FAIL);
  }
  
  // allocate the table entries
  section->index = (mlcIndexType *)calloc(section->nIndex,sizeof(mlcSectionType));
  if(section->index == NULL)
  {
      printf("\n ERROR: Allocating Memory for table %s", fileName);
      return(FAIL);
  }
  //  read in the table entries
  char inString[MAX_STR_LEN]; 

  for(int iIndex=0; iIndex < section->nIndex; iIndex++)
  {
  //   Nov. 5, 2004: IBM: Added reading of section->index[iIndex].grooveThickness and 
  //                      section->index[iIndex].grooveDistance
    if( (fgets(inString,MAX_STR_LEN,istrm) == NULL ) ||
	sscanf(inString,"%f%d%f%f%f",&section->index[iIndex].yEnd, 
               &section->index[iIndex].leafIndex,
               &section->index[iIndex].thickness,
	       &section->index[iIndex].grooveThickness,
	       &section->index[iIndex].grooveDistance) != 5 )
    {
      printf("\n ERROR: Reading data for index %d from %s", iIndex, fileName);
      return(FAIL);
    }
    section->index[iIndex].leafIndex-=1;  // so index will start from 0

    if(section->index[iIndex].leafIndex < 0 || section->index[iIndex].leafIndex > nLeaves )
    {
      printf("\n ERROR: Reading data from %s", fileName);
      printf("\n leafIndex (%d) out of range", section->index[iIndex].leafIndex);
      return(FAIL);
    }	 	 
  } 

  return(OK);
}
/* ************************************************************************** */
#ifdef C_CLASS
int particleDmlc::readParticleMLC(mlcType *mlc) // 11/08 ,  keepType *keep, char *mlcName)
#else
int readParticleMLC(mlcType *mlc, char *mlcName)
#endif
{
   clifObjectType *mlcClif = new(clifObjectType);

   char fileName[MAX_STR_LEN];
   //  Name of the config file
   strcpy(fileName,"VCUparticleMLC.config");
   FILE *istrm = openDMLCConfigFile(fileName);
   if(istrm == NULL){
      printf("\n ERROR: readParticleMLC"); return(FAIL); 
   }
   /* Read in the Clif Data */
   if( readSpecificClif(istrm, mlcClif, "MLC", mlcName) != OK){
      printf("\n ERROR: Reading clif from file %s\n",fileName); return(FAIL);
   }
   fclose(istrm);
#ifndef C_CLASS
   char mu_file_name[MAX_STR_LEN];
#endif
#ifdef DEBUG
   printf("\n Read the data from the MLC Clif\n");
#endif
   if(   (readFloatClifStructure (mlcClif, "density", &mlc->density) != OK )
	 //      || (readFloatClifStructure (mlcClif, "leafWidth", &mlc->leafWidth) != OK ) // temporary
      || (readFloatClifStructure (mlcClif, "distance", &mlc->distance) != OK )
      || (readFloatClifStructure (mlcClif, "closedOffset", &mlc->closedOffset) != OK )
      || (readFloatClifStructure (mlcClif, "physicalLeafOffset", &mlc->physicalLeafOffset) != OK ) 
      || (readFloatClifStructure (mlcClif, "maxTipThickness", &mlc->maxTipThickness) != OK )
      || (readFloatClifStructure (mlcClif, "tipRadius", &mlc->tipRadius) != OK )
      || (readFloatClifStructure (mlcClif, "tipAngle", &mlc->tipAngle) != OK )
      || (readFloatClifStructure (mlcClif, "tipDistance", &mlc->tipDistance) != OK )
      || (readIntClifStructure (mlcClif, "nTipSections", &mlc->nTipSections) != OK )
      || (readFloatClifStructure (mlcClif, "specificationDistance", &mlc->specificationDistance) != OK )
      || (readIntClifStructure (mlcClif, "nLeaves", &mlc->nLeaves) != OK )
      || (readIntClifStructure (mlcClif, "nSections", &mlc->nSections) != OK )
#ifdef C_CLASS_WITH_KEEP
      || (readIntClifStructure (mlcClif, "keep.electron", &keep->electron) != OK )
      || (readIntClifStructure (mlcClif, "keep.primOpen", &keep->primOpen) != OK )
      || (readIntClifStructure (mlcClif, "keep.primLeaf", &keep->primLeaf) != OK )
      || (readIntClifStructure (mlcClif, "keep.primTip", &keep->primTip) != OK )
      || (readIntClifStructure (mlcClif, "keep.scatLeaf", &keep->scatLeaf) != OK )
      || (readIntClifStructure (mlcClif, "keep.scatTip", &keep->scatTip) != OK )
      || (readIntClifStructure (mlcClif, "keep.comptonElectron", &keep->comptonElectron) != OK )
#endif
      || (readStringClifStructure (mlcClif, "attenuationCoefficientFile", mu_file_name) != OK )
      ||  mlc->nSections < 1 )
   {
      printf("\n ERROR: Reading MLC parameters from %s",fileName); return(FAIL);
   }
   printf("\n density = %f", mlc->density);
   // printf("\n mu_file_name = %s", mu_file_name);
   // printf("\n physicalLeafOffset = %g", mlc->physicalLeafOffset);
   // printf("\n\t %d MLC Sections Detected",mlc->nSections);
   // Allocate memory for the sections, then read them 
   mlc->section = (mlcSectionType *) calloc(mlc->nSections, sizeof(mlcSectionType));
   if(mlc->section == NULL)
   {
      printf("\n ERROR: Allocating memory for MLC sections"); return(FAIL);
   }
   for(int iSection=0; iSection < mlc->nSections; iSection++)
   {
      char sectionName[MAX_STR_LEN];
      sprintf(sectionName,"Section_%d", iSection+1);
      clifObjectType *mlcSection;
      if(getClifAddress(mlcClif,sectionName, &mlcSection)!= OK)
      {
         printf("\n ERROR: Getting Address for mlcSection %s", sectionName); return(FAIL);
      } 
      if( readParticleMLCSection( mlcSection, &mlc->section[iSection] )  != OK)
      {
         printf("\n ERROR: Reading mlcSection %s",sectionName); return(FAIL);
      }
      if( readMLCIndexTable(mlc->nLeaves, &mlc->section[iSection] ) != OK)
      {
         printf("\n ERROR: Reading mlcIndexTable for section %d", iSection); return(FAIL);
      }
   }
   // free the structure 
   if(freeClifStructure(mlcClif) != OK)
   {
      printf("\n ERROR: Freeing memory for clif object");
   }
   delete(mlcClif);
   return(OK);
}
/* ******************************************************************* */
#ifdef C_CLASS
int particleDmlc::readMLC(mlcType *mlc) // 11/08 , char *mlcName)
#else
int readMLC(mlcType *mlc, char *mlcName)
#endif
{
   clifObjectType *mlcClif = new(clifObjectType);

   char fileName[MAX_STR_LEN];
   strcpy(fileName,"mlc.config");
   FILE *istrm = openDMLCConfigFile(fileName);
   if(istrm == NULL)
   {
      printf("\n ERROR: readMLC"); return(FAIL); 
   }

   /* Read in the Clif Data */
   if( readSpecificClif(istrm, mlcClif, "MLC", mlcName) != OK)
   {
      printf("\n ERROR: Reading clif from file %s\n",fileName); return(FAIL);
   }
   fclose(istrm);
   /* Read the data from the Clif */
   if(   (readFloatClifStructure (mlcClif, "density", &mlc->density) != OK )
	 //	 || (readFloatClifStructure (mlcClif, "leafWidth", &mlc->leafWidth) != OK ) // temporary
      || (readFloatClifStructure (mlcClif, "distance", &mlc->distance) != OK )
      || (readFloatClifStructure (mlcClif, "closedOffset", &mlc->closedOffset) != OK )
      || (readFloatClifStructure (mlcClif, "maxTipThickness", &mlc->maxTipThickness) != OK )
      || (readFloatClifStructure (mlcClif, "tipRadius", &mlc->tipRadius) != OK )
      || (readFloatClifStructure (mlcClif, "tipAngle", &mlc->tipAngle) != OK )
      || (readFloatClifStructure (mlcClif, "specificationDistance", &mlc->specificationDistance) != OK )
      || (readIntClifStructure (mlcClif, "nLeaves", &mlc->nLeaves) != OK )
      || (readIntClifStructure (mlcClif, "nSections", &mlc->nSections) != OK )
      ||  mlc->nSections < 1 )
   {
      printf("\n ERROR: Reading MLC parameters from %s",fileName); return(FAIL);
   }
   //   printf("\n density = %f", mlc->density);
   // Allocate memory for the sections, then read them 
   mlc->profile = (tableType *) calloc(mlc->nSections, sizeof(tableType));
   if(mlc->profile == NULL)
   {
      printf("\n ERROR: Allocating memory for MLC sections"); return(FAIL);
   }
   for(int iSection=0; iSection < mlc->nSections; iSection++)
   {
      char sectionName[MAX_STR_LEN];
      sprintf(sectionName,"Section_%d", iSection+1);
      clifObjectType *mlcSection;
      if(getClifAddress(mlcClif,sectionName, &mlcSection)!= OK)
      {
         printf("\n ERROR: Getting Address for mlcSection %s", sectionName); return(FAIL);
      } 
      if( readMLCSection( mlcSection, &mlc->profile[iSection] )  != OK)
      {
         printf("\n ERROR: Reading mlcSection %s",sectionName); return(FAIL);
      }

      //  for(int i=0; i<mlc->profile[iSection].npts; i++)
      //   printf("\n\t %f %f", mlc->profile[iSection].points[i].x, mlc->profile[iSection].points[i].y);
   }
   // free the structure 
   if(freeClifStructure(mlcClif) != OK)
   {
      printf("\n ERROR: Freeing memory for clif object");
   }
   delete(mlcClif);
   return(OK);
}
/* ******************************************************************* */
#ifdef C_CLASS
int particleDmlc::readParticleMLCSection( clifObjectType *mlcSection, mlcSectionType *section)
#else
int readParticleMLCSection( clifObjectType *mlcSection, mlcSectionType *section)
#endif
{

  if(   (readFloatClifStructure (mlcSection, "pzEnter", &section->pzEnter) != OK )
     ||(readFloatClifStructure (mlcSection, "pzExit", &section->pzExit) != OK )
     ||(readFloatClifStructure (mlcSection, "pzCenter", &section->pzCenter) != OK ) 
     // add Nov 01, 2004: IBM: This info is now in the ....table files
     // || (readFloatClifStructure (mlcSection, "grooveDistance", &section->grooveDistance) != OK )
     // || (readFloatClifStructure (mlcSection, "grooveThickness", &section->grooveThickness) != OK )
     ||(readStringClifStructure(mlcSection, "Name", section->Name) != OK))
  {
    printf("\n ERROR: Reading particleMLCSection"); return(FAIL);
  }
   return(OK);
}
/* ******************************************************************** */ 
#ifdef C_CLASS
int particleDmlc::readMLCSection( clifObjectType *mlcSection, tableType *profile)
#else
int readMLCSection( clifObjectType *mlcSection, tableType *profile)
#endif
{
   char local_mlcType[MAX_STR_LEN];
   if( readStringClifStructure( mlcSection, "Type", local_mlcType) != OK)
   {
      printf("\n ERROR: Reading mlcSection Type"); return(FAIL);
   }
   if( strcmp( local_mlcType,"Periodic") == 0 )
   {
     if( readPinnacleCurve(mlcSection, &profile->npts, &profile->points) != OK)
     {
        printf("\n ERROR: readPinnacleCurve for mlcSection"); return(FAIL);
     }
     //     printf("\n Read in the curve, now convert ");
     /* Since is periodic type, create the full MLC */
   }
   else
   {
      printf("\n ERROR: mlcType %s Not Currently Supported", local_mlcType); return(FAIL);
   }
#ifdef DEBUG_READ
   printf("\n Read in %d Points", profile->npts);
   for(int i=0; i<profile->npts; i++)
      printf("\n\t %f %f", profile->points[i].x, profile->points[i].y);
#endif
   return(OK);
}
/* ******************************************************************** */ 
