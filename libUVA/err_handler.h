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
/* err_handler.h
This file contains symbolic constants and prototype definitions for err_handler
library functions.

Modification History:
   Dec 2, 1998: jvs: function prototypes must return a type, (void) if not specified
   Nov 27, 2007: JVS: const char * for compose_err and others
*/

#ifndef ERR_HANDLER_H_INCLUDED			/* To prevent multiple inclusions of this file */
#define ERR_HANDLER_H_INCLUDED

#ifndef TYPEDEFS_H_INCLUDED
#include	"typedefs.h"
#endif

/* Prototype definitions */


#ifdef myCC
// extern "C"
#endif
void initialize_error_handler(void);

/* #ifdef __cplusplus */
#ifdef myCC
// extern "C"
#endif
void init_err_handler (int mode,
int num_app_err_code_files,
char *app_err_code_files[]
);

#ifdef myCC
// extern "C"
#endif
void close_err_handler(void);

#ifdef myCC
// extern "C"
#endif
void compose_err ( Function_type *err_code, const char *err_context, const char *err_msg );

#ifdef myCC
// extern "C"
#endif
void get_err_msgs( char **msg1, char **msg2 );

#ifdef myCC
// extern "C"
#endif
void display_err (void);

#ifdef myCC
// extern "C"
#endif
void log_err (void);

#ifdef myCC
// extern "C"
#endif
char *context_msg( const char *function_name, const char *file_name, int lin_num );

#ifdef myCC
// extern "C"
#endif
char *concatenate_msg ( const char *msg_1, const char *msg_2 );

/* Symbolic Constants */

/* Mode controls how much error info to display to user */

#define USER_MODE 0
#define PROGRAMMER_MODE 1

#include "defined_values.h"

#endif /* ERR_HANDLER_H_INCLUDED */
