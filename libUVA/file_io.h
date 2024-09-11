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
#ifndef FILE_IO_H_INCLUDED
#define FILE_IO_H_INCLUDED

/* file_io.h
This file contains data structures, prototype declarations and constants for the 
file i/o library modules.

Modification History:
   Dec 2, 1998: jvs: function prototypes must return a type, (void) if not specified
   Nov 9, 2013: jvs: const char * for put_nml_vals
*/

#ifndef	FORTRAN

#ifndef TYPEDEFS_H_INCLUDED
#	include <typedefs.h>
#endif

/* C-specific definitions */

/* Namelist constants */

enum {NML_BYTE, NML_CHAR, NML_STR_ARRAY, NML_SHORT, NML_INT, NML_FLOAT, NML_DOUBLE};

#define NML_VAR_OPTIONAL 128

#define MAX_FORMAT_LEN 31

/* Prototype definitions */
#ifdef myCC
// extern "C"
#endif
Function_type get_nml_vals (const char *nml_name, const char *var_name, int var_type, int max_num_vals, 
	int *num_vals, void *vals );
#ifdef myCC
// extern "C"
#endif
Function_type open_nml_input (const char *file_path_and_name);

#ifdef myCC
// extern "C"
#endif
Function_type close_nml (void);

#ifdef myCC
// extern "C"
#endif
Function_type get_nml_strings (const char *nml_name, const char *var_name, char **string_ptrs,
                                       int max_strings, int *num_strings, 
                                       char *string_storage, int max_string_storage   );

#ifdef myCC
// extern "C"
#endif
Function_type open_nml_output (const char *file_path_and_name  );

#ifdef myCC
// extern "C"
#endif
Function_type close_nml_output(void);

#ifdef myCC
// extern "C"
#endif
Function_type open_nml_block( const char *nml_name );

#ifdef myCC
// extern "C"
#endif
Function_type close_nml_block(void);

#ifdef myCC
// extern "C"
#endif
Function_type put_nml_vals ( const char *var_name, 
                             int var_type, 
                             int num_vals, 
                             void *vals );

#ifdef myCC
// extern "C"
#endif
Function_type put_nml_vals_2 ( const char *var_name, int var_type, int num_vals, const char *format, void *vals );

/* some other stuff */
/* dirwalk.c */

#ifdef myCC
// extern "C"
#endif
void dirwalk
( 
	const char *dir, 	/* the directory to apply the function to */
	void (*fcn)(char *) 	/* the function to apply */
 );


/* filter_directory.c */
#ifdef myCC
// extern "C"
#endif
void free_directory(void);

#ifdef myCC
// extern "C"
#endif
void filter_directory
( 
	const char *dir, 
	const char *suffix_prm, 
	int *no_entries_prm, 
	char ***list_prm 
 );


/* parse_pat_path.c */
#ifdef myCC
// extern "C"
#endif
Function_type parse_pat_path
(
 char *full_path,
 char *pat_name,
 char *area
 );

/* filter_filenames.c */
#ifdef myCC
// extern "C"
#endif
void filter_filenames( char **filenames, int num_files, char *suffix,
			char ***new_files );


/* rtp_paths.c */			
#ifdef myCC
// extern "C"
#endif
void build_ptfile_path( char *path, char *ptfile, char *ptpath );

#ifdef myCC
// extern "C"
#endif
void build_img_set_path( char *path, char *ptfile, char *img_set, char *imgpath );

			
/* directory listing */
#ifdef myCC
// extern "C"
#endif
Function_type next_dir_file(char *dir_name,char *dir_mask,int *contxt,char *fil_name);

/* Unix/VMS name translations */
#ifdef myCC
// extern "C"
#endif
Logical unix_to_vms(char *name);
#ifdef myCC
// extern "C"
#endif
void vms_to_unix(char *name);

/* Prototype for pt_folder_dirs */
#ifdef myCC
// extern "C"
#endif
Function_type pt_folder_dirs(char *workarea,char ***pt_folders,int *num_pt_folders) ;

/* Prototype for diskquota function */
#ifdef myCC
// extern "C"
#endif
Function_type diskquotas(char *uname,char *disk,int *used,int *available,int *overdraft);

/* Object directories */

enum {PATIENT_DIR, STUDY_DIR, BEAMS_DIR, DOSE_DIR, IMAGES_DIR, PIX_DIR, REGIONS_DIR};

#ifdef myCC
// extern "C"
#endif
char *file_path(int object,char *area,char *ptfolder,char *imgset);

#endif	/* FORTRAN */

/* Symbolic constants */

#define FILE_IO_H_INCLUDED		/* To prevent multiple inclusions of this file */

#define MAX_FILE_NAME_LEN 31


#define MAX_NML_VAR_NAME_LEN 31		/* Maximum length of namelist variable name including terminating NULL */

#endif   /* FILE_IO_H_INCLUDED */

