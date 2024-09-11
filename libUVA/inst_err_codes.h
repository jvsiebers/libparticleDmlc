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
/* inst_err_codes.h
This file contains error code definitions used for the modules specific to an institution.  
The purpose of these codes to make the 3DRTP system open and extensible.  Each institution
is assigned a two digit (digits 5 and 6) code that is prefixed to the four digit error code.  
Note that a two digit (digits 7 and 8) private programmer specific code may be prefixed
to the six digit.  By convention make all error codes as even numbers and all information 
codes as odd numbers */

#define ERROR_ZERO_NORM 12001
#define ERROR_OUT_OF_DATA 12002
#define GENERAL_ERROR 12003

/* #define INTERP_RANGE_ERROR 12004 */

