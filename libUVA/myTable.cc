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
/* myTable.cc
   Table routines for dynamically allocated tableType

   Feb 5, 2002: JVS
   Jan 11, 2017: JVS: Add sortTable
*/
/* ***************************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include "table.h"
#include "utilities.h"

// #define DEBUG
/* ***************************************************************************************** */
bool sortTable(tableType *data) {
  // Sort the Data
  printf("\n Sorting %d data elements\n ", data->npts);

  if (data->npts < 2) {
    printf("\n\t: ERROR: Need at least 2 points to sort ");
    return false;
  }
  {
  int i = 0;
  do {
    if ( (i < data->npts - 1) &&
         (data->points[i].x > data->points[i + 1].x) ) // if not ascending /
    { // swap
      float xtemp        = data->points[i + 1].x; float ytemp        = data->points[i + 1].y;
      data->points[i + 1].x = data->points[i].x;   data->points[i + 1].y = data->points[i].y;
      data->points[i].x   = xtemp;  data->points[i].y        = ytemp;
      if (i) i--;        // force recursion
    } else {
      i++;
    }
  } while ( i < data->npts);
  }
  //for(int i=0; i<data->npts; i++)printf("%f\t%f\n", data->points[i].x, data->points[i].y);

  return true;
}
/* ***************************************************************************************** */
int readDataFileToTable(tableType *tab, char *filename)
/* reads in a table, call with filename[0]=NULL=0 to prompt for file name
  else, call with the file name to read the table directly
*/
{
#ifdef DEBUG
  printf("\n readDataFileToTable reading in file %s\n", filename);
#endif
  FILE *stream = NULL;
  char in_string[MAX_STR_LEN];
  int i;

  if (filename[0] != 0 ) stream = fopen(filename, "r");
  if (stream == NULL)
  {
    stream = open_file(filename, ".dat", "r");
    if (stream == NULL)
    {
      printf("\n Error Opening Table file %s", filename); return (ERROR);
    }
  }
  tab->npts = 0;
  int allocationIncrement = 128;
  int nAllocated = allocationIncrement;
  tab->points = (Point_2d_float *) malloc(nAllocated * sizeof(Point_2d_float));
  if (tab->points == NULL)
  {
    printf("\n ERROR: Allocating memory for table\n"); return (ERROR);
  }
#ifdef DEBUG
  printf("Allocated initial memory, reading file\n");
#endif
  i = 0;
  while (fgets(in_string, MAX_STR_LEN, stream) != NULL )
  {
    if (i >= nAllocated )
    {
      nAllocated += allocationIncrement;
#ifdef DEBUG
      printf("Increasing memory allocation to %d .....\n", nAllocated);
#endif
      tab->points = (Point_2d_float *) realloc(tab->points, nAllocated * sizeof(Point_2d_float));
      // printf("\n Post Memory Allocation i=%d, nAllocated=%d\n",i,nAllocated);
      if (tab->points == NULL)
      {
        printf("\n ERROR: Allocating memory for table\n"); return (ERROR);
      }
    }
    if (sscanf(in_string, "%f %f", &tab->points[i].x, &tab->points[i].y) == 2)
    {
      i++;
    }
  }
  /* resize buffer to exact size */
#ifdef DEBUG
  printf("Resizing buffer\n");
#endif
  tab->npts = nAllocated = i;
  tab->points = (Point_2d_float *) realloc(tab->points, nAllocated * sizeof(Point_2d_float));
  if (tab->points == NULL)
  {
    printf("\n ERROR: Allocating memory for table (downsizing) \n"); return (ERROR);
  }
  fclose(stream);
  printf("\n %d points read from file %s ....", tab->npts, filename);
  return (OK);
}
/* ***************************************************************************************** */
int writeTable(tableType *tab, char *filename)
// writes a table, call with filename[0]=NULL to prompt for file name
//  else, call with the file name to read the table directly
{
  FILE *wstream;
  wstream = open_file(filename, "", "w");
  if (wstream == NULL) {
    printf("\n ERROR: Opening Table file %s", filename);
    return (ERROR);
  }
  printf("\n Writing %d points to %s", tab->npts, filename);
  int iWrote=0;
  for (int i = 0; i < tab->npts; i++) {
    //printf("%f\t%f\n", tab->points[i].x, tab->points[i].y);
    fprintf(wstream, "%f\t%f\n", tab->points[i].x, tab->points[i].y);
    iWrote++;
  }
  printf("\t, %d written\n",iWrote);
  fclose(wstream); 
  return (iWrote);
}
/* ***************************************************************************************** */