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
/* read_scanner.cc
   Reads in CT # to electron density fro Scanner.db file
   Created: Jan 17, 2000: PJK
   Modification History:
       July 16, 2007: JVS: Improve error message when cannot find CtToDensityTable


*/
/* ******************************************************************** */
#include <stdlib.h>      
#include <stdio.h>
#include <string.h>     
#include "utilities.h"   // for error routines
#include "table.h"
// #include "spline_data.h"
#include "read_clif.h"
#include "read_scanner.h"
/* ******************************************************************** */
int read_ct_scanner(char *scan_file_name, char *scanner_type, table_type *tab)
{
#ifdef DEBUG
  printf("\nSearching for %s in %s",scanner_type,scan_file_name);
#endif
  FILE *fp;
  fp = fopen(scan_file_name,"r");
  if(fp == NULL)
    {
     printf("\n ERROR: opening file %s",scan_file_name);
     return(FAIL);
    }
  // get to line
  char temp_scanner_type[MAX_STR_LEN];
  char clif_name[MAX_STR_LEN];
  strcpy(clif_name,"CtToDensityTable ={");
  int found = -1;
  while (found<0)
   {
     // open clif, compare strings, close the clif
     if(clif_get_to_line(fp,clif_name)!=OK)
     {
        printf("\n ERROR: Cannot Find\n\t%s\n\t   Name = %s;\n\tin %s ", clif_name, scanner_type, scan_file_name);
        return(FAIL);
     }

     if(clif_string_read(fp,"Name =",temp_scanner_type)!=OK)
     {
        printf("\n ERROR: Reading scanner type from %s",scan_file_name);
        return(FAIL);
     }

     if (strcmp(scanner_type, temp_scanner_type) == 0)
     {
        if (read_conversion(tab, fp) != OK)
        {
          printf("\n ERROR: reading scan conversion");
          return(FAIL);
        }
        found  = 1;
     }

     if(clif_get_to_line(fp,"};")!=OK)
     {
        printf("\n ERROR: Closing %s structure",clif_name);
        return(FAIL); 
     }

     //printf("\n Closing %s structure",clif_name);
   }
  //printf ("\n n_elements = %d", tab->n_elements);
  fclose(fp);
  //printf("\n Wolf crashes here");
  return(OK);
} 
/* ******************************************************************** */
int read_conversion(table_type *tab, FILE *fp)
{
  char clif_n[MAX_STR_LEN];
  strcpy(clif_n,"Table ={");
  if(clif_get_to_line(fp,clif_n)!=OK)
  {
     printf("\n ERROR: finding CtToDensityTable ");
     return(FAIL);
  }
  int Num_Dim =      (int) clif_get_value(fp,"NumberOfDimensions =");
  if (Num_Dim != 2)
  {
     printf("\n ERROR: Num_Dim in CT table not 2");
     return(FAIL); 
  }
  tab->n_elements =  (int) clif_get_value(fp,"NumberOfPoints =");

  strcpy(clif_n,"Points[] ={");
  if(clif_get_to_line(fp,clif_n)!=OK)
  {
     printf("\n ERROR: finding CtToDensityTable ");
     return(FAIL);
  }
#ifdef DEBUG
  printf("\n Table values are: ");
#endif
  for (int i=0;i<tab->n_elements;i++)
  {
     if (fscanf(fp,"%f,%f,",&tab->x[i], &tab->y[i]) < 2){
         printf("\n ERROR: fscanf");
     }
#ifdef DEBUG
     printf("\n \t\t%f\t %f",tab->x[i], tab->y[i]);
#endif
  }

  if(clif_get_to_line(fp,"};")!=OK)
  {
     printf("\n ERROR: Closing %s structure",clif_n);
     return(FAIL); 
  }
  
  if(clif_get_to_line(fp,"};")!=OK)
  {
     printf("\n ERROR: Closing structure");
     return(FAIL); 
  }

  return(OK);
} 
/* ******************************************************************** */

