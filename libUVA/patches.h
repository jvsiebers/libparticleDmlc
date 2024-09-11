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
/* patches.h
   Patches to maintain compatiblity between the compiler on the SUN and DOS/OS2
   machines:
  
    On SOLARIS side, must compile with -DSOLARIS as a compiler option


   Created: 12/27/95: JVS
                      string compare stuff
   Modification History:
            4/13/98: JVS: Change to -dUNIX=
*/
#ifdef SOLARIS
#define UNIX
#endif
#ifdef UNIX
#define strncmpi(x,y,z) strncasecmp(x,y,z)
#define strnicmp(x,y,z) strncasecmp(x,y,z)
#endif
