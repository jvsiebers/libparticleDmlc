/** \file 
 simple tag auto added to force generation of simple doxygen comments
**/
/*
   Copyright 2015 University Of Virginia Commonwealth 


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
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
/* ************************************************************************** */
int my_strcasecomp(const char *str1, const char *str2){
  char *lowerCaseStr1 = new char[strlen(str1)+1];
  char *lowerCaseStr2 = new char[strlen(str2)+1];
  for(size_t i=0; i<(size_t)strlen(str1);i++) lowerCaseStr1[i]=(char)tolower(str1[i]);
  for(size_t i=0; i<(size_t)strlen(str2);i++) lowerCaseStr2[i]=(char)tolower(str2[i]);
  int result=strcmp(lowerCaseStr1,lowerCaseStr2);
  delete [] lowerCaseStr1;
  delete [] lowerCaseStr2;
  return(result);
}
/* ************************************************************************** */
int my_strncasecmp(const char *s1, const char *s2, size_t n){
  if (n != 0) {
    const unsigned char *us1 = (const unsigned char *)s1,
      *us2 = (const unsigned char *)s2;
    do {
      if (tolower(*us1) != tolower(*us2++))
	return tolower(*us1) - tolower(*--us2);
      if (*us1++ == '\0')
	break;
    } while (--n != 0);
  }
  return 0;
}
/* ************************************************************************** */
char *my_strcasestr(const char *s, const char *find){
  char c, sc;
  size_t len;
  if ((c = *find++) != 0) {
    c = (char) tolower((unsigned char)c);
    len = strlen(find);
    do {
      do {
	if ((sc = *s++) == 0)
	  return (NULL);
      } while ((char)tolower((unsigned char)sc) != c);
    } while (my_strncasecmp(s, find, len) != 0);
    s--;
  }
  return (char *)(intptr_t)(s);
}
/*
char *my_strcasestr(const char *str1, const char *str2){
  char *lowerCaseStr1 = new char[strlen(str1)+1];
  char *lowerCaseStr2 = new char[strlen(str2)+1];
  for(size_t i=0; i<strlen(str1);i++) lowerCaseStr1[i]=tolower(str1[i]);
  for(size_t i=0; i<strlen(str2);i++) lowerCaseStr2[i]=tolower(str2[i]);
  char *result=strstr(lowerCaseStr1,lowerCaseStr2);
  delete [] lowerCaseStr1;
  delete [] lowerCaseStr2;
  return(result);
}
*/
/* ************************************************************************** */
