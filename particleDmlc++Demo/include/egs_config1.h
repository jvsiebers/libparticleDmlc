/****************************************************************************** 
 * $Id: egs_config1.h,v 1.4 2008/03/16 21:45:28 iwan Exp $
 *****************************************************************************/

#ifndef EGS_CONFIG1_
#define EGS_CONFIG1_

#ifdef WIN32
#include <windows.h>
#endif
/* Without the above include file, gcc on Windows does not know about 
   __int64
 */

/*! Use the following macro for functions that don't have an underscore in 
    in their name.
 */
#define F77_OBJ(fname,FNAME) fname##_

/*! Use the following macro for functions with underscore(s) in their name
 */
#define F77_OBJ_(fname,FNAME) fname##_

/*! The name of the configuration
 */
#define CONFIG_NAME "x86_64-unknown-linux-gnu"

#ifdef __cplusplus
#define __extc__ "C"
#else
#define __extc__ 
#endif

#ifdef SINGLE
typedef float  EGS_Float;
#else
typedef double EGS_Float;
#endif

typedef short EGS_I16;
typedef int   EGS_I32;
#ifdef WIN32
typedef __int64 EGS_I64;
#else
typedef long long EGS_I64;
#endif

#endif
