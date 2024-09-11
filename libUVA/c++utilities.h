/* 
   Utilities needed for making coding easier

   Dec 21, 2012 : JVS
*/
#ifndef cpputilities_H_INCLUDED
#define cpputilities_H_INCLUDED
#include <sys/stat.h> // for S_IREAD, ... used for mode in mkpath
#include <errno.h>    // for EEXIST
int mkpath(const char *path, mode_t mode); // Makes path.  Operates like mkdir -p
#endif
