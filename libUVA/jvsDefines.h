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
/* Header File for General Utilities for CPP programs
       June 30, 2008: JVS: Broke off from utilities.h
       Feb 24, 2011: JVS: Update MAX_STR_LEN from 200 to 1024, 200 was just too small for some paths.
*/
#ifndef _jvsDefines_H_INCLUDED
#define _jvsDefines_H_INCLUDED
#define OK     0
#define ERROR -1
#define FAIL  -1
#define ON     1
#define OFF    0
#define FAIL_SAFE -999  /* used to be NULL**JVS 11/7/96** */
#define MAX_STR_LEN 1024     /* maximum length of a string */
#define MAX_BUFFER_SIZE 2048 /* maximum number of characters in the output buffer */
#endif
//
