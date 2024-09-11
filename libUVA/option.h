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
/* ************************************************************************* */
/* Modification History
   Nov 27, 2007: JVS: const char *option
   Feb 6, 2008: JVS: Add string 
   Feb 7, 2008: JVS: Add unsigned
   Oct 20, 2011: JVS: Add dumpOptions
 */
#ifndef OPTION_H_INCLUDED
#define OPTION_H_INCLUDED
#include <string>
int dumpOptions(int *argc, char *argv[]);
int get_option(const char *option, int *argc, char *argv[],char **flag, int *n_flag);
int get_option(const char *option, int *argc, char *argv[], char *flag);
int get_option(const char *option, int *argc, char *argv[], std::string *stringToSet);
int get_option(const char *option, int *argc, char *argv[]);
int get_option(const char *option, int *argc, char *argv[], long *n_flag);
int get_option(const char *option, int *argc, char *argv[], int *n_flag);
int get_option(const char *option, int *argc, char *argv[], unsigned *n_flag);
int get_option(const char *option, int *argc, char *argv[], float *n_flag);
int get_option(const char *option, int *argc, char *argv[], double *n_flag);
int get_option(const char *option, int *argc, char *argv[], int *n_flag, int *flag_array);
int get_option(const char *option, int *argc, char *argv[], int *n_flag, float *flag_array);
/* int get_option(const char *option, int *argc, char *argv[], int *n_flag, char **flag_array); */
int mySetEnv(const char *flag, const char *varName, int *argc, char *argv[]);
#endif
