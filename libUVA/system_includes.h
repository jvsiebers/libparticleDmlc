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
	This file contains all of the system-dependent '.h' header files.
	Include this whenever a system file is needed.
	
	The 'standard' include files, as described by Kernighan & Ritchie,
	are not included here to reduce compilation time.
	
	The files NOT included are:
	
		assert.h	ctype.h		errno.h		float.h
		limits.h	locale.h	math.h		setjmp.h
		signal.h	stdarg.h	stddef.h	stdio.h
		stdlib.h	string.h	time.h
	
        Modifcation History:
          JVS: 8/7/98: Make is system independent (Require define for 
                       non-standard VMS
*/
#define		SYSTEM_INCLUDES_H_INCLUDED

#ifdef VMS
#include	<file.h>
#include	<stat.h>
#include	<processes.h>
#include	<signal.h>
#else
#include	<sys/file.h>
#include	<sys/stat.h>
/* #include	<processes.h> */
#include	<sys/signal.h>
#endif
