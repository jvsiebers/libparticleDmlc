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
#ifndef LIB_H_INCLUDED	/* To prevent multiple inclusions of this file */
/* lib.h
This file contains prototype declarations and constants for the library modules.
This file may include prototype definition files of individual
groups of routines (e.g. FFT routines, interpolation routines, etc.)

Modification History:
   Sept 2, 1998: JVS: Unix Compatible
*/

#define LIB_H_INCLUDED		/* To prevent multiple inclusions of this file */

/* get macros file */

#ifndef	LIB_MACROS_H_INCLUDED
#include 	"lib_macros.h"
#endif

#define FALSE 0
#define TRUE 1

#define NORMALIZED 0
#define UNNORMALIZED 1

#define	   TWO_PI	6.28318530717959
#define	   PI		3.14159265358979
#define DEGREES_TO_RADIANS PI/180.0		/* Multiplicand */

/* Coordinate system types */

#define RECTILINEAR 0
#define SPHERICAL 1
#define POLAR 1
#define CYLINDRICAL 2

/* Logical operators */

#define NOOP '0'
#define AND '&'
#define OR '|'
#define NOT '!'
#define XOR '^'

#ifndef FILE_IO_H_INCLUDED
#include "file_io.h"
#endif

#include "defined_values.h"

#endif		/* To prevent multiple inclusions of this file */
