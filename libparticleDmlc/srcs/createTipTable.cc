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
/*  createTipTable.cc
    creates table of tip positions for each section of the MLC
    Modification History:       
       Jan 2, 2001: JVS: Created
       Jan 8, 2001: JVS: Patched
       Feb 7, 2007: JVS: Add C_CLASS parts
       Nov 11, 2008: JVS: Remove warnings for c++ class 
                 "warning: declaration of 'someVariable' shadows a member of 'this'"
                  for function createTwoSectionVarianTipTables
    

*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "utilities.h"
#include "typedefs.h"

#ifdef C_CLASS
#include "particleDmlcClass.h"
#else

#include "particleDmlc.h"

#endif
/* ********************************************************************************** */
#ifdef C_CLASS
int particleDmlc::printLeafTipSection(mlcSectionType *section)
#else

int printLeafTipSection(mlcSectionType *section)
#endif
{
    int i;
    printf("\n Section Enter %f Center %f Exit %f", section->pzEnter, section->pzCenter, section->pzExit);
    printf("\n Leading Tip:");
    for (i = 0; i < section->leadingTip.npts; i++) {
        printf("\n\t %d %f %f\t%f", i, section->leadingTip.points[i].x, section->leadingTip.points[i].y,
               section->leadingTip.points[i].y - section->leadingTip.points[section->leadingTip.npts - 1].y);
    }
    printf("\n Following Tip:");
    for (i = 0; i < section->followingTip.npts; i++) {
        printf("\n\t %d %f %f\t%f", i, section->followingTip.points[i].x, section->followingTip.points[i].y,
               section->followingTip.points[i].y - section->followingTip.points[section->followingTip.npts - 1].y);
    }
    return (OK);
}
/* ************************************************************************************** */
#ifdef C_CLASS
int particleDmlc::createTipSection(tableType *tip, int nTipSections, float tipRadius, float Angle, float maxTipThick)
#else

int createTipSection(tableType *tip, int nTipSections, float tipRadius, float Angle, float maxTipThick)
#endif
{
    // let intersection point of circle and lines be (Ix, Iz)
    // Iz = machineMLC->tipRadius*sin(machineMLC->tipAngle)
    // leaf edge a) is at Ix - machineMLC->tipRadius + machineMLC->tipRadius*cos(machineMLC->tipAngle)
    // leaf edge b) is at Ix + (machineMLC->maxTipThickness/2.0 - Iz)*sin(machineMLC->tipAngle)
    // width = b) - a)
    float tipAngle = Angle * (float) PI / 180.0f;
    float Ix = tipRadius * (1.0f - cos(tipAngle));
    float Iz = tipRadius * sin(tipAngle);

    //  printf ("\n tipRadius = %f", tipRadius);
    //  printf ("\n tipAngle = %f (%f)", Angle, tipAngle);
    tip->points[0].x = Ix - (Iz - maxTipThick) * tan(tipAngle);
    tip->points[0].y = maxTipThick;
    tip->points[1].x = Ix;
    tip->points[1].y = Iz;
    float deltaZ = Iz / (float) (nTipSections - 2);
    float zCircle = Iz;
    float tipRadiusSquared = tipRadius * tipRadius;

    for (int iTip = 2; iTip < nTipSections; iTip++) {
        zCircle -= deltaZ;
        tip->points[iTip].x = tipRadius - sqrt(tipRadiusSquared - zCircle * zCircle);
        tip->points[iTip].y = zCircle;
    }
    return (OK);
}
/* ********************************************************************************* */
#ifdef C_CLASS
int particleDmlc::createTwoSectionVarianTipTables() // 11/08 mlcType *machineMLC)
#else

int createTwoSectionVarianTipTables(mlcType *machineMLC)
#endif
{
    // Create standard half tip profile
    if (machineMLC->nTipSections <= 0) {
        printf("\n ERROR: nTipSections = %d", machineMLC->nTipSections);
        return (FAIL);
    }
    //  printf("\n\t nTipSections = %d", machineMLC->nTipSections);
    tableType testTip;
    testTip.points = (Point_2d_float *) calloc(machineMLC->nTipSections, sizeof(Point_2d_float));
    if (testTip.points == NULL) {
        printf("\n ERROR: testTip memory allocation");
        return (FAIL);
    }
    //  float maxTipThick=6.13*0.5;
    //  float centerZPlane=51.33;
    if (createTipSection(&testTip,
                         machineMLC->nTipSections,
                         machineMLC->tipRadius,
                         machineMLC->tipAngle,
                         machineMLC->maxTipThickness * 0.5f) != OK) {
        printf("\n ERROR: createTipSection");
        return (FAIL);
    }

    for (int iSection = 0; iSection < 2; iSection++) {
        machineMLC->section[iSection].leadingTip.points = (Point_2d_float *) calloc(machineMLC->nTipSections,
                                                                                    sizeof(Point_2d_float));
        machineMLC->section[iSection].followingTip.points = (Point_2d_float *) calloc(machineMLC->nTipSections,
                                                                                      sizeof(Point_2d_float));
        if (machineMLC->section[iSection].leadingTip.points == NULL ||
            machineMLC->section[iSection].followingTip.points == NULL) {
            printf("\n ERROR: Memory Allocation for MLC Tips");
            return (FAIL);
        }
        machineMLC->section[iSection].leadingTip.npts = machineMLC->nTipSections;
        machineMLC->section[iSection].followingTip.npts = machineMLC->nTipSections;
    }
    // Assign thicknesses for each section
    for (int iTip = 0; iTip < machineMLC->nTipSections; iTip++) {
        machineMLC->section[0].leadingTip.points[iTip].x = testTip.points[iTip].x;
        machineMLC->section[0].leadingTip.points[iTip].y = machineMLC->section[0].pzExit - testTip.points[iTip].y;
        machineMLC->section[0].followingTip.points[iTip].x = -1.0f * testTip.points[iTip].x;
        machineMLC->section[0].followingTip.points[iTip].y = machineMLC->section[0].pzExit - testTip.points[iTip].y;

        machineMLC->section[1].leadingTip.points[iTip].x = testTip.points[iTip].x;
        machineMLC->section[1].leadingTip.points[iTip].y = machineMLC->section[1].pzEnter + testTip.points[iTip].y;
        machineMLC->section[1].followingTip.points[iTip].x = -1.0f * testTip.points[iTip].x;
        machineMLC->section[1].followingTip.points[iTip].y = machineMLC->section[1].pzEnter + testTip.points[iTip].y;
    }
    free(testTip.points);

#ifdef DEBUG_TIP
    printf("\n Section[0]");printLeafTipSection(&machineMLC->section[0]);
    printf("\n Section[1]");printLeafTipSection(&machineMLC->section[1]);
#endif

    return (OK);
}
/* ********************************************************************************* */
#ifdef TEST_PROG
int main()
{
  int nPoints = 12;
  float maxTipThick=6.13*0.5;
  float centerZPlane=51.33;
  char string[MAX_STR_LEN];
  printf("\n Input MaxTipThickness (%f) > ", maxTipThick);
  fgets(string,MAX_STR_LEN,stdin); sscanf(string,"%f", &maxTipThick);
  printf("\n Input CenterZPlane (%f) > ", centerZPlane);
  fgets(string,MAX_STR_LEN,stdin); sscanf(string,"%f", &centerZPlane);
  printf("\n Input Number of Points in Tip (%d) ", nPoints);
  fgets(string,MAX_STR_LEN,stdin); sscanf(string,"%d", &nPoints);
  tableType testTip;
  //  testTip.points = new ( nPoints * Point_2d_float ) ;
  testTip.points = new ( Point_2d_float[nPoints] ) ;
  createTipSection(&testTip, nPoints, 8.0, 11.3,maxTipThick);
  FILE *tipFile=fopen("tipFile","w");
  if(tipFile == NULL)
  {
    printf("\n ERROR: opening tipFile");
  }
  /* Print out the tip section */
  for(int iTip=0; iTip < nPoints; iTip++)
     fprintf(tipFile,"\n %f %f", testTip.points[iTip].x, centerZPlane-testTip.points[iTip].y);

  fprintf(tipFile,"\n");
  /* Following leaf, x->-x */
  for(int iTip=0; iTip < nPoints; iTip++)
     fprintf(tipFile,"\n %f %f", -1.0*testTip.points[iTip].x, centerZPlane-testTip.points[iTip].y);
  fprintf(tipFile,"\n");
  /*Bottom of leading leaf, y->-y */
  for(int iTip=0; iTip < nPoints; iTip++)
     fprintf(tipFile,"\n %f %f", testTip.points[iTip].x, centerZPlane+testTip.points[iTip].y);
  fprintf(tipFile,"\n ");
  /* bottom of following leaf */
  for(int iTip=0; iTip < nPoints; iTip++)
     fprintf(tipFile,"\n %f %f", -1.0*testTip.points[iTip].x, centerZPlane+testTip.points[iTip].y);


  fclose(tipFile);
  delete testTip.points;
}
#endif
/* ********************************************************************************* */
