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
/* read/write file routines for analyse program
   JVS: 2/26/97: broken off of driver.cpp
   March 25, 2002: JVS: Moved to utilities area....
   Nov 10, 2005: Get rid of extern ostream, pstream, lstream, ...
   Sept 18, 2007: Add #define NEED_EXTERNS so can have the external globals and
                  FILE_ROUTINES_INCLUDED to prevent multiple inclusions
  4/1/2015: JVS: Hack in xmgraceStream;
*/
#include "spline_data.h"

#ifndef FILE_ROUTINES_INCLUDED
#define FILE_ROUTINES_INCLUDED
#ifdef NEED_EXTERNS
extern FILE *pstream; 
extern FILE *lstream; 
extern FILE *ostream;  // default output file 
extern FILE *xmgraceStream; // for xmgrace
extern int Plot_Format;
#endif


int read_data(spline_data_type *data);
int read_data(spline_data_type *data, char *filename);

int write_data(spline_data_type *data);
int write_data(spline_data_type *data, char *filename);

int write_and_plot_data(spline_data_type *data);

int sort_data(spline_data_type *data, float *norm);
#endif
