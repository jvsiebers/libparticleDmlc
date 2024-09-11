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
#ifndef CHECK_IO_H_INCLUDED
#define CHECK_IO_H_INCLUDED

/* check_io.h
This file contains data structures, prototype declarations and constants for the 
check i/o library modules.

Modification History:
   Dec 2, 1998: jvs: created
*/


Function_type check_float( char * );
Function_type check_int(   char * );
Function_type check_short( char * );
Function_type check_byte(  char * );
Function_type check_var_name( char * );

#endif   /* CHECK_IO_H_INCLUDED */

