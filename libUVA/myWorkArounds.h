#ifndef _myWorkArounds_h_included
#define _myWorkArounds_h_included
/** \file 
 simple tag auto added to force generation of simple doxygen comments
**/
/*
   Copyright 2015 University Of Virginia


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
/* Jeffrey Siebers
     May 8, 2015: JVS: Add my_strcasecmp to overcome solaris lack of strcasecomp
*/
/* ************************************************************************* */
int my_strcasecomp(const char *str1, const char *str2);
int my_strncasecmp(const char *s1, const char *s2, size_t n);
char *my_strcasestr(const char *s, const char *find);
/* ************************************************************************** */
#endif
