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
#ifndef  RTP_H_INCLUDED	  /* To prevent multiple inclusions of this file */

#define RTP_H_INCLUDED

/* rtp.h
This file contains symbolic constants and prototype definitions for ALL the 
library routines of the 3DRTP system.
*/

#define MAX_BDSS 16   /* The most BDS tables that can exist in a treatment plan */
#define MAX_INST_LEN 20
#define MAX_MALLOC_STACK_PTRS 100

#include "defined_values.h"

/* Prototype for get_object_id */

Function_type get_object_id(char *workarea,char *pt_folder,char *obj_name,int *obj_id);
Function_type init_object_id(char *workarea,char *pt_folder);

/* Prototype for init_pt_folder */
Function_type init_pt_folder(char *workarea, char *pt_folder);

/* Allow oveloading only if this is c++ */

#ifdef __cplusplus
Function_type init_pt_folder(char *workarea, char *pt_folder,char *study);
#endif


#endif /* RTP_H_INCLUDED, to prevent multiple inclusions of this file */
