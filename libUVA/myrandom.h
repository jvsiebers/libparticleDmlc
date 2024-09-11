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
/* Modificaiton History
   Nov 27. 2007: Add #ifndef and const char * for nerror
   Dec 7, 2012: Add get_random_gaussian(double,double)
 */
#ifndef MYRANDOM_H_INCLUDED
#define MYRANDOM_H_INCLUDED
/* random number generation routines */
float ran1(int *idum);      /* the random number generator */
float random1(void);        /* returns a floating point random number */
int random_num(int num);    /* returns a random number between 0 and 1 */
void nerror(const char *string);  /* prints error report and exits */
double get_random_gaussian(int *seed,double std_dev, double mean);
double get_random_gaussian(double std_dev, double mean);
unsigned int getSeed(void);
float randomOne(void);
void initializeRandom(unsigned seed);
void printNRandomsSampled();
unsigned long getnRandomsSampled();
void advanceRandomToStride();
void setRandomStride(int newStride);
//
#endif
