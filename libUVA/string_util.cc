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
/* Jeffrey Siebers
	copyright 1996 LLUMC
        1998: MCV
	string_util.cpp -- programs to operate on string data
		Created by: Jeff Siebers: 02/06/96: Extracted from make_rx.cpp
      9/23/96: Have get_value and get_int_value return null if not find value
      4/27/98: JVS: LINUX gcc compatible
      5/07/98: JVS: remove eprintf, 
                    with GCC, NULL returns from int and float routines
                    returns 0 (i.e., NULL = 0)
                    string_after: returns OK / FAIL
                    get_string_to_scan: returns OK/FAIL
                    get_int_value: returns OK 
		    have stringErrno set (to FAIL) if an error occurs.
     6/09/98: JVS: get_to_line returns fail if went to EOF.
     1/06/98: JVS: Add get_value_in_file (will rewind if not found )
     Feb 26, 1999: JVS: Add get_value_quiet, fix spelling on quiet
     April 25, 2002: JVS: Add check_if_line_quiet
     NOv 5, 2007: JVS: Wrapping into c++ Class (ifdef C_CLASS)
     Nov 27, 2007: JVS: const char* for get_value functions
     Nov 28. 2007: JVS: C_STRING_CLASS currently is not used by any code....
     June 4, 2008: JVS: typecast to avoid 64bit compiler warnings
     Mar 16, 2013: JVS: Clean up some error messages
     May 15, 2015: JVS: Clean up, simplify by removing repeatative coding
*/
/* ************************************************************************* */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
// #include "patches.h"
#include "utilities.h"
#include "string_util.h"
/* set up local-global */
char input_string[MAX_STR_LEN];
int stringErrno=OK;  // local global for keeping track of error
int checkStringError(){return stringErrno;}
int resetStringError(){stringErrno = OK; return stringErrno;}
/* ************************************************************************** */
#ifdef C_STRING_CLASS
int jvsString::string_after(FILE *istrm, const char *string, char *str, bool output)
#else
int string_after(FILE *istrm, const char *string, char *str, bool output)
#endif
{
  char *pstr;
  do{
    pstr = check_if_line(istrm,string);
    if(pstr != NULL){
      if( strcmp(pstr,"EOF") == 0){
	if(output) printf("\n ERROR: string_after: EOF detected when trying to read %s",string);
	stringErrno = FAIL;
	return( FAIL );
      }  
    }
  }while(pstr == NULL);
  while(isspace(pstr[0]))pstr++;
  int len = (int) strlen(pstr);
  int i;
  for(i=0; i<len && i < MAX_STR_LEN; i++) str[i]=*(pstr+i);
  str[i-1] = '\0'; // terminate the string 
  return(OK);
}
/* ************************************************************************** */
#ifdef C_STRING_CLASS
int jvsString::string_after(FILE *istrm, const char *string, char *str)
#else
int string_after(FILE *istrm, const char *string, char *str)
#endif
{
  return(string_after(istrm,string,str,true));
}
/* ************************************************************************** */
#ifdef C_STRING_CLASS
int jvsString::string_after_quiet(FILE *istrm, const char *string, char *str)
#else
int string_after_quiet(FILE *istrm, const char *string, char *str)
#endif
{
  return(string_after(istrm,string,str,false));
}
/* ************************************************************************** */
#ifdef C_STRING_CLASS
int jvsString::int get_string_to_scan(FILE *istrm, char *string, char *str)
#else
int get_string_to_scan(FILE *istrm, char *string, char *str)
#endif
{
   char *pstr;
   do{
      pstr = check_if_line(istrm,string);
      if(pstr != NULL)
      {
         if( strcmp(pstr,"EOF") == 0)
	 {
	    printf("\n ERROR: get_string_to_scan: END OF FILE ON READ");
	    stringErrno = FAIL;
	    return( FAIL );
	 }
      }
   }while(pstr == NULL);
   while(isspace(pstr[0]))pstr++;
   int len = (int) strlen(pstr);
   for(int i=0; i<len && i < MAX_STR_LEN; i++) str[i]=*(pstr+i);
   return(OK);
}
/* ************************************************************************** */
#ifdef C_STRING_CLASS
int jvsString::float get_value_in_file(FILE *istrm, const char *string)
#else
float get_value_in_file(FILE *istrm, const char *string)
#endif
  // like get value, but will rewind the file if the value is not found 
{
   char str[MAX_STR_LEN];
   int loop_count = 0; // count the number of times through the file looking for the string 

      while( string_after_quiet(istrm, string, str) == FAIL && 
             !loop_count++ )  // find the leading string
      {
         printf("\n *** REWINDING STREAM  ***");
         rewind(istrm);
	 stringErrno = OK;
      }
      float n;
      if(sscanf(str,"%f",&n)!=1)
      {
         printf("\n ERROR: get_value_in_file %s",string);
         stringErrno = FAIL;
         return( FAIL );
      }
   return(n);
}
/* ************************************************************** */
#ifdef C_STRING_CLASS
int jvsString::float get_value(FILE *istrm, const char *string, bool output)
#else
float get_value(FILE *istrm, const char *string, bool output)
#endif
{
   char str[MAX_STR_LEN];
   if(OK != string_after(istrm, string, str)) {
     if(output) printf("\n ERROR: get_value: string_after(,%s) returned !OK",string); 
     stringErrno = FAIL; return(FAIL);
   }
   float n;
   if(sscanf(str,"%f",&n)!=1){
      printf("\n ERROR: get_value: scanning value from %s",string);
      stringErrno = FAIL;
      return( FAIL );
   }
   return(n);
}
/* **************************************************************** */
#ifdef C_STRING_CLASS
int jvsString::float get_value(FILE *istrm, const char *string)
#else
float get_value(FILE *istrm, const char *string)
#endif
{
  return(get_value(istrm,string,true));
}
/* ************************************************************** */
#ifdef C_STRING_CLASS
int jvsString::float get_value_quiet(FILE *istrm, const char *string)
#else
float get_value_quiet(FILE *istrm, const char *string)
#endif
{
  return(get_value(istrm,string,false));
}
/* ************************************************************** */
#ifdef C_STRING_CLASS
int jvsString::int get_int_value(FILE *istrm, const char *string)
#else
int get_int_value(FILE *istrm, const char *string)
#endif
{
  char str[MAX_STR_LEN];
  if(OK != string_after(istrm, string, str) ) {
    printf("\n ERROR: get_int_value: string_after returned !OK"); 
    stringErrno = FAIL; return(FAIL);
  }
  int n;
  if(sscanf(str,"%d",&n)!=1)
  { 
     printf("\n ERROR: get_value %s",string);
     stringErrno = FAIL;
     return( FAIL );
  }
//  printf("\n INT: %d    %s", n, str);
  return(n);
}
/* **************************************************************** */
#ifdef C_STRING_CLASS
int jvsString::int get_to_line(FILE *istrm, const char *string)
#else
int get_to_line(FILE *istrm, const char *string)
#endif
{
   char *ptmp;
   while( (ptmp = check_if_line(istrm, string) )==NULL);  
   if(strcmp(ptmp,"EOF") == 0)
     return(FAIL);
   return(OK);
}
/* *************************************************************** */
/* ************************************************************* */
#ifdef C_STRING_CLASS
int jvsString::char *checkNextLineForString(FILE *istrm, const char *string, bool offsetStart,bool outputMessages)
#else
char *checkNextLineForString(FILE *istrm, const char *string, bool offsetStart, bool outputMessages)
#endif
{ 
  // Checks if next line in file starts with "string"
  // returns pointer to end of string if found
  // returns NULL otherwise
  int slength;                       
  if( fgets(input_string, MAX_STR_LEN, istrm) == NULL){
    if(outputMessages) printf("\n WARNING: END OF FILE ON READ : %s", string);
    sprintf(input_string,"EOF"); 
    return(input_string);
  }
  slength = (int) strlen(string);  // compare over length of input string
  int s_val = 0;             // location in string
  // advance out space in the input string
  while(isspace( (input_string[s_val])  )) s_val++;
  // compares strings without case sensitivity 
  if( strncasecmp(string, input_string+s_val, slength) != 0){
    return(NULL);
  }
  if(offsetStart) return(input_string+s_val+slength);
  return(input_string+s_val);
}
/* ************************************************************* */
#ifdef C_STRING_CLASS
int jvsString::char *check_if_line(FILE *istrm, const char *string)
#else
char *check_if_line(FILE *istrm, const char *string) 
#endif
{
   return(checkNextLineForString(istrm,string,true,true));
}
/* ************************************************************* */
#ifdef C_STRING_CLASS
int jvsString::char *check_if_line_quiet(FILE *istrm, const char *string)
#else
char *check_if_line_quiet(FILE *istrm, const char *string)
#endif
{
   return(checkNextLineForString(istrm,string,true,false));
}
/* ************************************************************ */
#ifdef C_STRING_CLASS
int jvsString::char *checkNextLineForString(FILE *istrm, const char *string)
#else
char *checkNextLineForString(FILE *istrm, const char *string)
#endif
{
   return(checkNextLineForString(istrm,string,false,true));
}
/* ************************************************************ */

#ifdef C_STRING_CLASS
int jvsString::char *checkNextLineForStringQuiet(FILE *istrm, const char *string)
#else
char *checkNextLineForStringQuiet(FILE *istrm, const char *string)
#endif
{
   return(checkNextLineForString(istrm,string,false,false));
}
/* ************************************************************ */
