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
/* read_clif.cc
   Created: 6/9/98: JVS:
   Routines to read in Pinnacle clif (c-like information file) with error checking.
   The advantage of using these routines is that they will ensure that the information being
   read is from the structure desired
  
   Values in the c-like information format:
      Numeric Values: use equal sign and ;
        x_dim = 512;  
      String Values: use 
        Name = "Trial_1";
      Comments: delimited by //
      Structures: Begin with ={  end with };
      Beam ={
        Name = "Ant";
        ... 
      }; 
 
   When reading in values of a structure, must check that do not go beyond the }; delimiting the
   end of the structure.

   Possible ways to do this:  
      1) Read the entire structure into a buffer, then read from the buffer.
      2) Keep track of where you are in the file (structure level), possibly use recursive calls
      3) Read in whole Pinnacle file, breaking structures on down....

   Modification History:
      Sept 3, 1998: JVS: Fix minor bugs found by insure
      October 23, 1998: JVS: Allow C and C++ Comments
                             Remove spaces from end of string
      10/29/98: JVS: Improving Output Error Messages
      Jan 26, 1999: clif_get_string allows : 
      Nov 3, 1999: JVS: clif_check_if_line returns END_OF_CONTEXT if finds EOF when looking for
                        start_of_context ={
      Nov 19, 1999: JVS: fgets_cstring -- do NOT close file if is an ERROR...if we try closing
                         the file again, then we get a segmentation fault on some computers...
                         if you need to close the stream, so so elsewhere....beware you insure maniacs
      Dec 20, 1999: JVS: clif_string_after now removes spaces after a string too...needed so
                         can have comments on the line...
                         remove strncasecmp
      Jan 18, 2000: JVS/PJK: added if len exists as looking for strings of
                             negative lengths
      Jun 2, 2000: JVS: replace eprintf's with printf's,add clifFgets with errno set on END_OF_FILE
      Jun 5, 2000: JVS: clifFgets will no longer return an empty string....
      Jan 17, 2001: JVS: clifFgets, order of operations check so not access [-1] element of a string
      Sept 18, 2001: JVS: clifFgets will read until sees the end of a line (string) ;
      Feb 7, 2003: JVS: Add checkForStoreString (replaces = { with ={ after each call to
                        fgets_cstring (simplest way to circumvent the fact that store
                        starts it clif structure as = { rather than ={
                        Also replaces = SimpleString { with ={ for ease of use...
                        To Turn this OFF, comment out lines that use checkForStoreString
     Nov 27, 2007: JVS: type cast to get rid of compiler warnings....
     May 16, 2012: JVS: change several int's to size_t to remove CC compiler warnings for 64 bit
     Dec 8, 2012: JVS: add mystrcpy to avoid valgrind errors/warning...
                       and prevent going beyond string bounds
     Feb 20, 2013: JVS: Change to myString[3]="={"; to avoid gcc4.4 compiler warning.
     June 26, 2015: JVS: Add in "= Float {" replacement, like SimpleString
     July 3, 2019: JVS: Remove unprintable chars from comment strings before output them
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> 

#include "utilities.h"
#include "read_clif.h"
extern int clifErrno; // for keeping track of errors in inner routines
// #define DEBUG_CLIF
// #define DEBUG 
/* ************************************************************** */
char *mystrcpy(char *dest, char *source){ // mystrcpy, mimics strcpy, but prevents memory overlap
#ifndef TRICKY
  char tmpString[MAX_STR_LEN];
  strcpy(tmpString,source);
  strcpy(dest,tmpString);
#else 
  if(NULL == dest) { printf("\n ERROR: mystrcpy: dest==NULL"); 
      clifErrno = FAIL; return(NULL);
  }
  if(NULL != source) {
    int length = strlen(source);
    if( length < MAX_STR_LEN && source[length] != 0){
      //printf("\n NULL TERMINATING STRING:  -1=%c   -2=%c",source[length-1], source[length-2]);
       source[length]=0; // make sure string is null terminated
    }
    if(strlen(source) >= MAX_STR_LEN ) { printf("\n ERROR: mystrcpy: strlen(source) > MAX_STR_LEN");
      printf("\t%s",source);
      clifErrno = FAIL; return(NULL);
    }
    switch (strlen(source)) {
    case 0:
      dest[0]=0;
      break;
    case 1:
      dest[0]=source[0];
      break;
    default:
      // printf("\n Entering mystrcpy with string X%sX of length %d",source,(int) strlen(source)); fflush(NULL);
      char *tmpString = (char *)calloc(strlen(source),sizeof(char));  // avoid valgrind error of memory overlap
      if(NULL==tmpString){printf("\n ERROR: memory allocation in mystrcpy in libmcv\n"); throw -1;}

      strcpy(tmpString,source);   
      // if(strlen(tmpString) < MAX_STR_LEN-1) strcat(tmpString,"\0"); // Make sure string is null terminated
      strcpy(dest, tmpString);
      free(tmpString);
    } // end switch
  } else {
    dest=NULL;
  }
#endif
  return(dest);
}
/* ************************************************************** */

float clif_get_value(FILE *istrm, const char *string)
/* gets a value from a clif file, ignores comments, 
   complains if leave current scope */
{
   char str[MAX_STR_LEN];
   if(clif_string_after(istrm, string, str)!=OK) {
      printf("\n ERROR: clif_get_value, looking for %s", string);
      return(FAIL);
   }
   /* remove spaces from end of the string */
   size_t len= (strlen(str)-1);
   while( isspace(str[len])  && len ) {
      str[len--]='\0'; /* remove if it is a space */
   }
   /* ensure is a valid clif string */
   if(str[strlen(str)-1] != ';' ){
      printf("\n ERROR:clif_get_value: %s is an invalid clif string",str);
      clifErrno = FAIL;
      return(FAIL);
   }
   float n;
   if(sscanf(str,"%f",&n)!=1){
      printf("\n ERROR: get_value %s",string);
      clifErrno = FAIL;
      return( FAIL );
   }
   // printf("\n Clif value of %f returned",n);
   clifErrno = OK;
   return(n);
}
/* ************************************************************ */
/* **************************************************************** */
int clif_get_to_line(FILE *istrm, const char *string)
{
   char str[MAX_STR_LEN];
#ifdef DEBUG_CLIF
    printf("\n clif_get_to_line: Looking for %s (len = %d)", string, strlen(string));
#endif
   if(clif_string_after(istrm,string,str) != OK)
   {
       /* printf("\n ERROR: clif_get_to_line: looking for %s", string); */
       /* don't want to print the error, since one way to see if done is to
          see if no more exist */
      return(FAIL);
   }
#ifdef DEBUG
   printf("\n clif_get_to_line returns OK");
#endif
   return(OK);
}
/* ************************************************************ */
int clif_string_after(FILE *istrm, const char *string, char *str)
{
   char *pstr=str;
   int   ctmp;
   clifErrno = OK;
#ifdef DEBUG_CLIF
   printf("\n checking for clif string %s (len = %d)",string, strlen(string) );
#endif
   do{
      ctmp=clif_check_if_line(istrm,string, pstr);
#ifdef DEBUG_CLIF
      printf("\n check_if_line returns: %s (len = %d)",pstr, strlen(pstr));
#endif
      if(ctmp == END_OF_CONTEXT)
      {
	 // printf("\n WARNING: END_OF_CONTEXT found while looking for %s",string);
         clifErrno = END_OF_CONTEXT;
         return(END_OF_CONTEXT);
      }
      if(ctmp == FAIL)
      {
         printf("\n ERROR: return from clif_check_if_line");
         printf("\n\t Looking for %s",string);
         if( strcmp(pstr,"EOF") == 0)
	 {
	    printf("\n ERROR: clif_string_after: End of File on Read");
	    clifErrno = FAIL;
	    return( FAIL );
	 }
      }
   }while(ctmp != OK); // pstr == NULL);
#ifdef DEBUG_CLIF
   printf("\n Parsing the string %s",pstr);
   if(clifErrno != OK) printf("\n Myerrno is not OK");
   printf("\n String length = %d",strlen(pstr));
#endif
   while( (isspace(pstr[0]))  && strlen(pstr) )pstr++;  // remove leading space
   size_t len = (strlen(pstr));
#ifdef DEBUG_CLIF
   printf(" after String length = %d",strlen(pstr));
#endif
   
   // remove trailing space from the string
   while(len && isspace(pstr[len-1]))
   { 
      pstr[len-1] = '\0';
      len--;
   }
#ifdef DEBUG_CLIF
   printf(" len now = %d",len);
#endif
   size_t i;
   for(i=0; i<len && i < MAX_STR_LEN-1; i++) str[i]=*(pstr+i);
   if(i) str[i] = '\0'; /* NULL terminate the string */
#ifdef DEBUG_CLIF
   printf("\n clif_string_after returning OK");
#endif
   clifErrno = OK;
   return(OK);
}
/* ********************************************************************** */
/* dingo does not have strdup, so make it up */
char *strdup(char *originalString)
{
  // int length = strlen(originalString)+1;
  char *copyString = (char*) malloc( (strlen(originalString)+1)*sizeof(char));
  if( copyString == NULL) return(NULL);
  strcpy(copyString,originalString);
  return(copyString);
}
/* ********************************************************************* */
int checkForStoreString(char *input_string)
{
   /* Check for need to replace = { with ={ */
   /* IMPORTANT...Checks if is Store so only does for store */
   if( strstr(input_string, "Store = {") != NULL)
   {
     // Make replica of original string
     char *oldString;
     oldString = strdup(input_string);
     // get length of first part
     size_t keepLen= (strstr(input_string,"= {")-input_string);
     // put ={ on string
     {
       char myString[3]="={";
       mystrcpy(input_string+keepLen,myString); 
     }
     // and place rest on the string (in case more information)
     strcat(input_string,oldString+keepLen+3);
#ifdef DEBUG_STORE
     printf("\n Replacing %s with %s", oldString, input_string);
#endif
     free(oldString);
   } 
   else 
   if( strstr(input_string, "= SimpleString {") != NULL)
   {
     // Make replica of original string
     char *oldString;
     oldString = strdup(input_string);
     // get length of first part
     size_t keepLen=(strstr(input_string,"= SimpleString {")-input_string);
     // put ={ on string
     // July 15, 2003: JVS testing here
     {
       char myString[3]="={";
       mystrcpy(input_string+keepLen,myString); 
     }
     // strcpy(input_string+keepLen,"."); 
     // and place rest on the string (in case more information)
     strcat(input_string,oldString+keepLen+16);
#ifdef DEBUG_STORE
     printf("\n Replacing %s with %s", oldString, input_string);
#endif
     free(oldString);
   }
   else if( strstr(input_string, "= Float {") != NULL)
   {
     // Make replica of original string
     char *oldString;
     oldString = strdup(input_string);
     // get length of first part
     size_t keepLen=(strstr(input_string,"= Float {")-input_string);
     // put ={ on string
     // July 15, 2003: JVS testing here
     {
       char myString[3]="={";
       mystrcpy(input_string+keepLen,myString); 
     }
     // strcpy(input_string+keepLen,"."); 
     // and place rest on the string (in case more information)
     strcat(input_string,oldString+keepLen+16);
#ifdef DEBUG_STORE
     printf("\n Replacing %s with %s", oldString, input_string);
#endif
     free(oldString);
   }
   return(OK);
}
/* *********************************************************************** */
char *clifFgets(char *string, int Max_Len, FILE *fp)
{
   // string is an allocated input string
   clifErrno = OK;
   do{
      if( fget_cstring(string, Max_Len, fp) != OK )
      {
	//printf("\n ERROR: fget_cstring in clifFgets");
         clifErrno = END_OF_FILE;
         return(NULL); 
      }
      checkForStoreString(string);
      // remove spaces off the end of lines 
      size_t len= (strlen(string)-1); // look at last char of string 
      //      while( isspace(string[len])   && len >= 0 ) // Jan 17, 2001: order changes so neverl look at string[-1].....
      while( (int) len >= 0 && isspace(string[len]) ) 
      {
	string[len--]='\0'; // remove if it is a space 
      }
   }while(strlen(string) == 0);
   // Check that string ended properly with a ";" or a "{" 
   size_t lastChar= (strlen(string)-1);
   if( string[lastChar] != ';' && string[lastChar] != '{' )
   {
     // printf("\n Last Char of String %s is %c", string, string[lastChar]);
     // printf("\n String end not found, read next line from the file");
     char nextString[MAX_STR_LEN];
     if( clifFgets(nextString,MAX_STR_LEN, fp ) == NULL )
     {
       if( clifErrno == END_OF_CONTEXT ) return(NULL);
     }
     size_t nStrLen= strlen(nextString);
     size_t iNext=0;
     // printf("\n Checking for Space");
     while(iNext < nStrLen && isspace(nextString[iNext]) ) iNext++;
     if(iNext == nStrLen) {
       clifErrno = NOT_FOUND;
       return(NULL);
     }
     // printf ("\n Adding On String %s", nextString+iNext);
     strcat(string,nextString+iNext);
     //  printf("\n Final String is: %s", string);
     //  printf("\n Final String Length = %d", (int) strlen(string));
     if(strlen(string) >= (unsigned) Max_Len)
     {
       printf("\n ERROR: string Length too long, length = %d, maxLen = %d", (int) strlen(string), Max_Len );
       clifErrno = END_OF_FILE;
       return(NULL);
       } 
   }
   // check that not END_OF_CONTEXT 
   if( strstr(string, "};") != NULL)
   {
      clifErrno = END_OF_CONTEXT;
      return(NULL);
   }
   // printf("\n clifFgets returning (%d) %s", strlen(string), string);
   return(string);
}
/* ********************************************************************** */
char *clif_fgets(char *string, int Max_Len, FILE *fp)
{
   clifErrno = OK;
   if( fget_cstring(string, Max_Len, fp) != OK ){
      printf("\n ERROR:%s: END OF FILE ON READ : %s ", __FUNCTION__, string);
      clifErrno = FAIL;
      return(NULL); 
   }
   checkForStoreString(string);
   /* remove spaces off the end of lines */
   size_t len=strlen(string)-1; /* look at last char of string */
   while( isspace(string[len]) && len ) {
     string[len--]='\0'; // remove/terminate string if it is a space 
   }
   /* check that not END_OF_CONTEXT */
   if( strstr(string, "};") != NULL){
      clifErrno = END_OF_CONTEXT;
      return(NULL);
   }
   return(string);
}
/* ********************************************************************** */
int clif_check_if_line(FILE *istrm, const char *string, char *input_string) 
/* checks if string is on line*/
{                                              /* in a file, return NULL if  */
                                               /* line not start with string */
                                               /* returns pointer to end of  */
                                               /* string if found            */
   
  /* static int level=0; */
  if( fget_cstring(input_string, MAX_STR_LEN, istrm) != OK ){
    if(strstr(string, "={") != NULL ) return(END_OF_CONTEXT); // JVS: 11/3/99:
    printf("\n ERROR:%s: END OF FILE ON READ : %s ", __FUNCTION__,string);
    sprintf(input_string,"EOF"); /* 7-11-97: JVS */
    return(FAIL); 
  }
  checkForStoreString(input_string); // check for = { in string
#ifdef DEBUG_CLIF
  printf(" Line: %s (len = %d) \n",input_string, strlen(input_string) );
#endif
  size_t slength = strlen(string);  // compare over length of input string
  int s_val = 0;             // location in string
  // advance out space in the input string
  while(isspace( (input_string[s_val]) ) && s_val < (int) strlen(input_string)) s_val++;
  // check for match with the string, return if found 
  // use to be strncasecmp but new linux cannot find this, Dec 20, 1999 JVS
#ifdef DEBUG_CLIF
  printf("\n Comparing %s and %s for %d chars", string, input_string+s_val, slength);
  printf("\t s_val = %d, strlen(input_string) = %d ",s_val, strlen(input_string));
#endif
  if( strncmp(string, input_string+s_val, slength) == 0){
#ifdef DEBUG_CLIF
      printf("\n MATCH FOUND");
#endif
      mystrcpy(input_string, input_string+s_val+slength); 
      return(OK);
   }
   // check for end of context 
   if( strstr(input_string+s_val, "};") != NULL){
      return(END_OF_CONTEXT);
   }
   // check for beginning of context 
   int ctmp;
   if( strstr(input_string+s_val, "={") != NULL)
   { // advance till this context ends
     //  printf("\n Context level %d Started %s",++level,input_string);
      do{
 
         ctmp=clif_check_if_line(istrm,"xxEND_OF_CONTEXTxx",input_string);
         if(ctmp == FAIL)
	 {
            printf("\n ERROR: clif_check_if_line reports %s", input_string);
            printf("\n\t Looking for %s",string);
            return(FAIL);
         }
      }while(ctmp != END_OF_CONTEXT);
      // printf(" Context level %d exited",level--);
   }
   input_string = NULL;
   return(NOT_FOUND);
}
/* *********************************************************************** */
int clif_string_read(FILE *fp, const char *string, char *ostring)
{
   clifErrno = OK;
   char tstring[MAX_STR_LEN];  // setup string of correct length
   if(clif_string_after(fp,string,tstring)!=OK){ 
      printf("\n ERROR: clif_string_read: clif_string_after :Reading %s",string); 
      return(FAIL);
   }
   /* check if input string ended in : ,if so, then no outer " " or / / around it */
   size_t ilen = strlen(string);
   if( string[ilen-1] == ':' ){
      mystrcpy(ostring, tstring); 
      return(OK);
   }
   /* trim off leading " and ending "; from the string */
   size_t len=strlen(tstring);
   if(tstring[len-1] != ';' ||
        !(tstring[0] !=  '"' ||  tstring[0] != '\\') ||
        !(tstring[len-2] != '"' || tstring[len-2] != '\\' ))
   {
      printf("\n ERROR: Reading %s,\t %s is an invalid clif string",string,tstring);
      clifErrno = FAIL;
      return(FAIL);
   }
   mystrcpy(tstring,tstring+1); /* remove leading " */
   tstring[len-3]='\0';       /* null terminate the string */
   mystrcpy(ostring,tstring);
   return(OK);
}
/* *********************************************************************** */
/*
char *strnset(char *s, int ch, size_t n)
{  // mimic strnset command in dos/ os2/ win / ... 
   for(int i=0; i< (int) n; i++)
   {
     if(s[i] == ((char) 0) ) return(s); // return when find null
      s[i] = ch;
   }
   return(s);
}
*/
/* ************************************************************************** */
// #define DEBUG
int fget_cstring(char *string, int Max_Str_Len, FILE *fspec)
{
  // printf("\n fget_cstring....%s ", string);
  // gets a string from the input and removes comments from it 
  // allows comments in standard "c" syntax, 
  //        starting with /*
  //        ending with  */
  // also allows c++ type comments, // causes rest of line to be skipped */
  // "string" is allocated outside this function

   int check;
   char comment_start[4]="/*";  /* signals start of comment */
   char comment_stop[4]="*/";   /* signals end of comment   */
   size_t clen; /* length of string for start/stop*/
   size_t ilen; /* length of input string */

   size_t olen; /* location on output string */
   size_t icnt; /* location on string */

   olen = 0;
   for(int i=0;i<Max_Str_Len; i++) string[i]=0;
   // strnset(string,'\0',Max_Str_Len); // null entire output string 
   // allocate memory for input string 
      
   char istring[MAX_STR_LEN+1]; // input string
   if(Max_Str_Len > MAX_STR_LEN)
   {
      printf("\n ERROR: bad trouble in fget_cstring");
      printf("\n Need to increase MAX_STR_LEN");
      return(FAIL);
   }
   if(NULL == fspec) {
     printf("\nERROR: fget_cstring: Null File Pointer Found\n"); // return(FAIL);
   }

   clen = strlen(comment_start);
   /* read in the line, verify that it exists */
   do{
      /* read in a line from the file */
      // for(int i=0;i<MAX_STR_LEN; i++) istring[i]=0;
      if(fgets(istring, MAX_STR_LEN, fspec) == NULL) /* output warning if not a valid read */
      {
	  // fclose(fspec);  // JVS: Nov 19, 1999
         // free(istring);
         return(FAIL);
      }
      ilen = strlen(istring); /* length of input string */
      // printf("%s", istring);
#ifndef MSDOS
      /* get rid of <cr><lf> in pc based machines */
      if(ilen > 2 && istring[ilen-2] == '\x0d')
      {
       istring[ilen-2] = '\n';
       istring[ilen-1] = '\0';
       ilen=strlen(istring);
      }
#endif
      istring[ilen]='\0';     /* null terminate the string */
      if(ilen < clen)         /* not possible to have comment on the line */
      {                       /* so output the string as is */
         mystrcpy(string,istring);
         olen = strlen(string);
      } else {
        // strip comments out of input string */
        icnt=0;
        do{
          check = 1;
          if(icnt < ilen - clen)  // make sure have enough characters for start of comment */
             check = strncmp(istring+icnt,comment_start,clen); // check if start of comment */
          if(check == 0) // comment found for standard c syntax */
          {
#ifdef DEBUG
                printf("\n Comment Found on %s", istring+icnt);
#endif
#ifdef CLEAN_CONTROL
                // Clean out non-printable characters (that sometimes occur when line is a check sum) 
                char clean_istring[MAX_STR_LEN];
                for(unsigned int index=0, clean_index=0; index < strlen(istring); index++){
                    if ( !iscntrl(istring[index] ) ) clean_istring[clean_index++]=istring[index]; 
                    else {
                      printf("\n control char found (%c) at index %d in string %s", istring[index-1], index, istring);
                    }
                }
#ifdef DEBUG
                if ( strlen(istring) != strlen(clean_istring) )
                   printf("\n\tfget_cstring: non-printable characters removed from %s ", clean_istring);
#endif
                strcpy(istring, clean_istring);
#endif
                // end clean out non-printable characters
                /* find end of comment */
                icnt+=clen; /* advance past comment delimiter */
                clen=strlen(comment_stop); /* get length of end of comment delimiter */
                /* look for end of comment till end of string */
                do{
                   check = 1;
                   if(icnt < ilen - clen)  /* make sure have enough characters for end of comment */
                   {
		                  // printf("\n Comparing %s", istring+icnt);
                      check = strncmp(istring+icnt, comment_stop,clen);
                   }
                   if(check != 0)  /* if not end of comment */
                   {
                      icnt++; /* increment location on string */
                      if(icnt>ilen) /* if advance past end of string, get a new one */
                      {
                         if(fgets(istring, MAX_STR_LEN, fspec) == NULL) /* output warning if not a valid read */
                         {
                            printf ("\nERROR: fget_cstring: Reading File, looking for end of comment %s",comment_stop);
                            printf ("\n Looking for %s, Final String %s", string, istring);
#ifdef CLEAN_CONTROL
                            printf ("\n (control chars were removed from the string)")
#endif
                            return(FAIL);
                         }
                         ilen = strlen(istring); /* get length of this new string */
#ifndef MSDOS
                         /* get rid of "newline" in pc based machines */
                         if(ilen > 2 && istring[ilen-2] == '\x0d')
                         {
                             istring[ilen-2] = '\n';
                             istring[ilen-1] = '\0';
                             ilen=strlen(istring);
                         }
#endif
                        istring[ilen]='\0';     /* null terminate the string */
                        icnt = 0; /* reset the counter to the start of the string */
                      }
                   } else {
                     icnt+=clen; /* advance past comment delimiter */
                   }
                }while(check != 0); /* end of comment found */
            // printf("\n End Of Comment Found");
          }  /* end if */
          else /* check if comment is in c++ format */
          {
             check = strncmp(istring+icnt, "//",2);
             if(check == 0) /* c++ style comment found */
             {  /* skip till end of string */
                icnt = ilen;   /* end loop */
                string[olen++]=' ';
                string[olen]='\0'; /* terminate the string */
             }
             else /* is a valid character for the string */
             {
                string[olen++] = istring[icnt++]; /* append value to the string */
                string[olen]='\0';
             }
          }
        }while(icnt < ilen          &&    /* do till end of string */
               olen < (size_t) Max_Str_Len); /* and output string not too long */
        /* check for only carriage return (should have been caught above) */
        if(olen == 1 && (string[0] == '\0' || string[0] == '\n')){
         olen = 0;
/*         string[0] = '\0';*/
        }

      }  /* end else */
   }while(olen == 0); /* do till read in a string */
   if(olen == (size_t) Max_Str_Len)
   {
      printf ("ERROR: Input line too long");
      // fclose(fspec); // JVS: Nov 19, 1999
      // free(istring);
      return(FAIL);
   }
   // free(istring);
   // printf(" \n fget_cstring: len=%d (%d), string=%s", olen, strlen(string), string);
   return(OK);
}
/* ***************************************************************************** */


