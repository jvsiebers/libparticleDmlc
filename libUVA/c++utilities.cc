/* 
   Utilities needed for making coding easier

   Dec 21, 2012 : JVS
*/
/*
@(#)File:           $RCSfile: c++utilities.cc,v $
@(#)Version:        $Revision: 1.1 $
@(#)Last changed:   $Date: 2015/05/06 11:42:08 $
@(#)Purpose:        Create all directories in path
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1990-91,1997-98,2001,2005,2008,2012
*/

/*TABSTOP=4*/

//#include "jlss.h"
//#include "emalloc.h"

#include <errno.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#include <string.h>
//#include "sysstat.h"    /* Fix up for Windows - inc mode_t */
#include <sys/types.h> // for stat
#include <sys/stat.h>
#include <stdlib.h>


// typedef struct stat Stat;

//#ifndef lint
///* Prevent over-aggressive optimizers from eliminating ID string */
//const char jlss_id_mkpath_c[] = "@(#)$Id: c++utilities.cc,v 1.1 2015/05/06 11:42:08 jsiebers Exp $";
//#endif /* lint */

#if (defined(_WIN32) || defined(__WIN32__))
#define mkdir(A, B) mkdir(A)
#endif


static int do_mkdir(const char *path, mode_t mode)
{
  //    Stat            st;
  struct stat st;
    int             status = 0;

    if (stat(path, &st) != 0)
    {
        /* Directory does not exist. EEXIST for race condition */
        if (mkdir(path, mode) != 0 && errno != EEXIST)
            status = -1;
    }
    else if (!S_ISDIR(st.st_mode))
    {
        errno = ENOTDIR;
        status = -1;
    }

    return(status);
}

/**
** mkpath - ensure all directories in path exist
** Algorithm takes the pessimistic view and works top-down to ensure
** each directory in path exists, rather than optimistically creating
** the last element and working backwards.
*/
int mkpath(const char *path, mode_t mode)
{
    char           *pp;
    char           *sp;
    int             status;
    char           *copypath = strdup(path);

    status = 0;
    pp = copypath;
    while (status == 0 && (sp = strchr(pp, '/')) != 0)
    {
        if (sp != pp)
        {
            /* Neither root nor double slash in path */
            *sp = '\0';
            status = do_mkdir(copypath, mode);
            *sp = '/';
        }
        pp = sp + 1;
    }
    if (status == 0)
        status = do_mkdir(path, mode);
    free(copypath);
    return (status);
}

/* **************************************************************** */
