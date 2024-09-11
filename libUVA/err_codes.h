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
   err_codes.h
   This file contains error codes definitions used by the
   library modules, as well as general-purpose error codes. 

   05/94...VL: Added NetCDF specific error codes and PATH_ errors

   06/94...VL: Added FILE_LOCKED code

   Jun 13, 2000: JVS: Add INTERP_RANGE_ERROR
   Jan 31, 2003: JVS: Add IMRT_GUI compatible information

*/
#ifndef ERR_CODES_H_INCLUDED

#define	ERR_CODES_H_INCLUDED


#define SUCCESS			1
#define NO_ERROR 		1


#define 	INTERP_RANGE_ERROR     		12004
/* File io error codes - 1000 to 1999 */
/* These start with very general error codes, progressing to
   more specific and descriptive codes. */

#define 	LIB__IO_ERROR     		1000
#define		LIB__FILE_OPEN_FAILURE		1001
#define		LIB__CREATE_FAILURE		1002
#define		LIB__READ_FAILURE		1003
#define		LIB__WRITE_FAILURE		1004
#define 	LIB__CLOSE_FAILURE		1005
#define 	LIB__DELETE_FAILURE		1006
#define		LIB__FILE_EXISTS		1007
#define		LIB__FILE_NOT_FOUND		1008
#define		LIB__RECORD_NOT_FOUND		1009
#define		LIB__DUPLICATE_KEY		1010
#define 	LIB__FILE_NOT_OPEN		1011
#define 	LIB__FILE_ALREADY_OPEN		1012
#define		LIB__NO_RECORD_READ		1013
#define		LIB__INV_NUMBER_OF_KEYS		1014
#define		LIB__INV_BUCKET_SIZE   		1015
#define		LIB__INV_NUMBER_OF_SEGMENTS	1016
#define		LIB__INV_SEGMENT		1017
#define		LIB__KEY_TOO_LONG		1018
#define		LIB__TOO_MANY_OPEN_FILES	1019
#define		LIB__INV_KEY_NUMBER		1020
#define		LIB__INV_READ_TYPE		1021
#define		LIB__KEY_UNDEFINED		1022
#define		LIB__INV_KEY_CHANGE		1023
#define		LIB__RECORD_LOCKED		1024
#define		LIB__RECORD_NOT_LOCKED		1025
#define		LIB__LOCK_FAILURE		1026
#define		LIB__UNLOCK_FAILURE		1027
#define		LIB__NO_RECORD_LOCKED		1028

/* NML errors */
#define		LIB__NML_ALREADY_OPEN		1029
#define		LIB__INV_NML_OR_VAR_NAME	1030
#define		LIB__NML_STRING_TOO_LONG	1031
#define		LIB__NML_NOT_OPEN		1032
#define		LIB__NML_NOT_FOUND		1033
#define		LIB__NML_VAR_NOT_FOUND		1034
#define		LIB__NML_UNDEFINED_DATA_TYPE	1035
#define		LIB__NML_DATA_ERROR		1036
#define		LIB__ENV_VAR_CONV_ERROR		1037
#define		LIB__NML_TOO_MANY_VARIABLES	1038
#define		LIB__NML_DATA_CONVERSION_ERR 	1039
#define		LIB__NML_STILL_OPEN		1042
#define		LIB__FILE_CLOSE_FAILURE		1043
#define		LIB__NML_NO_FILE_OPEN		1044
#define		LIB__NML_HEADER_NOT_WRITTEN	1045
#define		LIB__INV_FLOAT_CONSTANT   	1048
#define		LIB__INV_INT_CONSTANT     	1049
#define		LIB__INV_SHORT_CONSTANT   	1050
#define		LIB__INV_BYTE_CONSTANT    	1051
#define     	LIB__NML_INSUFFICIENT_STR_SPACE 1052

/* 05/94...VL: new PATH_ errors */
#define         LIB__PATH_NOT_FOUND             1053
#define         LIB__PATH_CREATE_FAILURE        1054

#define         LIB__FILE_LOCKED                1055

/* Miscellaneous system errors - codes 2000 to 2499  */
#define	LIB__MALLOC_ERROR		2000
#define	LIB__NAME_UNDEFINED		2001
#define	LIB__SYNTAX_ERROR		2002

/* X errors - 2500 to 2999 */
#define	LIB__COLORMAP_CREATE_FAILURE	2500

/* Math errors - 4000 to 4499 */

#define LIB__INDEP_VAR_OUT_OF_RANGE	4000
#define LIB__INTERP_VAR_OUT_OF_RANGE     4001
#define LIB__INTERP_GRID_SIZE_TOO_SMALL     4002

/* NetCDF errors - 4500 to 4999 */

#define LIB__NCDF_READ_ERROR    4500
#define LIB__NCDF_WRITE_ERROR   4501
#define LIB__NCDF_DIM_DEFINE    4502
#define LIB__NCDF_VAR_DEFINE    4503
#define LIB__NCDF_ATT_DEFINE    4504
#define LIB__NCDF_BAD_MODE      4505
#define LIB__NCDF_VAR_WRITE     4506
#define LIB__NCDF_NO_UNLIM_DIM  4507
#define LIB__NCDF_INV_DATA_TYPE 4508
#define LIB__NCDF_INV_VARIABLE  4509
#define LIB__NCDF_INV_REC_NUM   4510
#define LIB__NCDF_VARGET_ERROR  4511
#define LIB__NCDF_DIMINQ_ERROR  4512
#define LIB__NCDF_VARINQ_ERROR  4513
#define LIB__NCDF_ATTGET_ERROR  4514
#define LIB__NCDF_EOF_REACHED	4515
#define LIB__NCDF_ATTINQ_ERROR  4516
#define LIB__NCDF_ATTPUT_ERROR  4517

#define NML_NOT_MATCH           5555
#define CONTOURS_OVERLAP        5556
/* institution-specific error codes */
#include "inst_err_codes.h"

/* PinnComm Error */
#define LIB__PINNCOMM_ISSUE_ROOT_COMMAND_ERROR 5001
#define LIB__PINNCOMM_SET_FLOAT_ERROR 5002
#define LIB__PINNCOMM_SET_INT_ERROR 5003
#define LIB__PINNCOMM_SET_OBJECT_ERROR 5004
#define LIB__PINNCOMM_SET_STRING_ERROR 5005
#define LIB__PINNCOMM_QUERY_FLOAT_ERROR 5006
#define LIB__PINNCOMM_QUERY_INT_ERROR 5007
#define LIB__PINNCOMM_QUERY_OBJECT_ERROR 5008
#define LIB__PINNCOMM_QUERY_STRING_ERROR 5009

#endif	/* ERR_CODES_H_INCLUDED */
