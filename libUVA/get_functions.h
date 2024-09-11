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
/* 02/10/97 :JVS
  get_functions.h
  prototypes for get functions
  Modification History:  10/23/98: JVS: Add fget_c_string
*/
int get_int_string(FILE *fspec, char *cstring, int *value);
int get_float_string(FILE *fspec, char *cstring, float *value);
int get_string_string(FILE *fspec, char *cstring, char *string);
int get_next_int_string(FILE *fspec, char *cstring, int *value);
int get_next_float_string(FILE *fspec, char *cstring, float *value);
int get_int(FILE *fspec, int *value);
int get_float(FILE *fspec, float *value);
int get_string(FILE *fspec, char *string);
int get_to_string(FILE *fspec, char *cstring);
int fget_c_string(char *string, int Max_Str_Len, FILE *fspec);
