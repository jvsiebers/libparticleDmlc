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
/*
pvt_defs.h
This file includes temporary definitions of constants, error codes and 
prototypes for routines that may end up in one of the libraries.

The purpose of this file is avoid the conflicts in access to 
shared files (err_codes.h, rtp_err_codes.h, lib.h, rtp.h, typedefs.h)
by different software developers.  Once temporary definitions
are no longer temporary, they must be transferred to appropriate .h
files and erased from this file.  The transfer must be coordinated with
the individual managing the project.

This file resides in your sys$login directory.  It is automatically included
by C and Fortran templates.
*/

/* For rtp_err_codes.h */

/* The four digit error codes are prefixed with two digit (digits 5 and 6) institution code (01) 
and two digit (digits 7 and 8) private code specific to the the programmer (01 for Mohan) */
