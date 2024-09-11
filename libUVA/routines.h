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
/* routines.h **********************************************************/
/* header files for pjk's libraries in routines.cc (see also common.h)
   Created 3 Sept 1999 pjk
   Modification history:
*/



// prototypes
void pexit(spline_data_type *sp);
void pexit(table_type *tab);
float slope(table_type *tab);
float slope(table_type *tab, float lower, float upper);
float intercept(table_type *tab);
float intercept(table_type *tab, float lower, float upper);
float min_x(table_type *tab);
float max_x(table_type *tab);
float average(table_type *tab, float lower, float upper);
int plotgnu(char *file, int sleepe, int type);
int plotgnups(char *file, int sleepe, int type);

