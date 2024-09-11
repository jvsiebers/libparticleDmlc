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
/* get_functions.cpp
   a series of functions to get float, ints, and strings from files.
   Skips comments given in standard "c" syntax
   Created: 01-23-97: JVS
   Modification History:
       01-30-97: JVS: bug fix
       10-14-98: JVS: Allow rewind of file and add to library of routines.
                      Add get_next functions which prohibit stream rewinding
                      and require value to be on the next line
       10/23/98: JVS: Add fget_c_string
       Feb 26, 1999: JVS: These routines DON'T Work....
                          get_to_string does NOT return a string....
       March 22, 2000: JVS: Fix fget_c_string for c++ style comments (terminate string)
       June 4, 2008: JVS: typecast to avoid 64bit compiler warnings
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utilities.h" // for eprintf
#include "get_functions.h"
#define REWIND_STREAM 100
#define DEBUG
/* ************************************************************************* */
int strnset(char *string, char ch, int len)
{
   for(int i=0; i<len;i++)
      string[i] = ch;
   return(OK);
}
/* ************************************************************************** */
int get_to_string(FILE *fspec, char *cstring)
{
   /* gets to a line that has a given *cstring */
   char string[MAX_STR_LEN];
   int clen;
   int n_pass = 0;

    printf("\n cstring = %s", cstring);
   /* read in valid line without comments */
    clen = (int) strlen(cstring);
   do{
      int check = get_string(fspec,string);
      if( check == FAIL ) return(FAIL);
      if( check == REWIND_STREAM) n_pass++; /* increment the number of passes
                                               through the file               */
      /* check that it is a valid line */
#ifdef DEBUG
     printf("\n get_to_string: %s", string);
#endif
      if( strncmp(cstring,string,clen) == 0 ) return(OK);
   }while(n_pass < 2);
   return(FAIL);
}
/* ************************************************************************** */
int get_string_string(FILE *fspec, char *cstring, char *value)
{
   /* reads an string value after a given cstring
      10/14/98: JVS */

   char string[MAX_STR_LEN];
   int clen;
   int check;

   /* read in valid line without comments */
   check = get_to_string(fspec,string);
   if(check == FAIL)
      return(FAIL);

   /* check that it is a valid line */
   clen = (int) strlen(cstring);
   if( strncmp(cstring,string,clen) != 0 )
   {
      eprintf("ERROR: Strings %s and %s not similar",cstring,string);
      fclose(fspec);
      return(FAIL);
   }

   /* read in the value after the string */
   check = sscanf(string+clen,"%s",value);
   if(check !=1)
   {
      eprintf("ERROR: Reading %s from file",cstring);
      fclose(fspec);
      return(FAIL);
   }
   return(OK);
}
/* ************************************************************************** */
int get_int_string(FILE *fspec, char *cstring, int *value)
{
   /* reads an integer value after a given cstring
      01/23/97: JVS */

   char string[MAX_STR_LEN];
   int clen;
   int check;

   /* read in valid line without comments */
   check = get_to_string(fspec,cstring);
   if(check == FAIL)
      return(FAIL);
#ifdef DEBUG
   printf("\n string %s\n", string);
#endif

   /* check that it is a valid line */
   clen = (int) strlen(cstring);
   if( strncmp(cstring,string,clen) != 0 )
   {
      eprintf("ERROR: Strings %s and %s not similar",cstring,string);
      fclose(fspec);
      return(FAIL);
   }

   /* read in the value after the string */
   check = sscanf(string+clen,"%d",value);
   if(check !=1)
   {
      eprintf("ERROR: Reading %s from file",cstring);
      fclose(fspec);
      return(FAIL);
   }
   return(OK);
}
/* ************************************************************************** */
int get_float_string(FILE *fspec, char *cstring, float *value)
{
   /* reads a float value after a given cstring
      01/23/97: JVS */

   char string[MAX_STR_LEN];
   int clen;
   int check;

   /* read in valid line without comments */
   check = get_to_string(fspec,string);
   if(check == FAIL)
      return(FAIL);

   /* check that it is a valid line */
   clen = (int) strlen(cstring);
   if( strncmp(cstring,string,clen) != 0 )
   {
      eprintf ("ERROR: Strings %s and %s not similar",cstring,string);
      fclose(fspec);
      return(FAIL);
   }

   /* read in the value after the string */
	check = sscanf(string+clen,"%f",value);
   if(check!=1)
   {
      eprintf ("ERROR: Reading %s from file ",cstring);
      fclose(fspec);
      return(FAIL);
   }
   return(OK);
}
/* ************************************************************************** */
/* ************************************************************************** */
int get_next_int_string(FILE *fspec, char *cstring, int *value)
{
   /* reads an integer value after a given cstring
      01/23/97: JVS */

   char string[MAX_STR_LEN];
   int clen;
   int check;

   /* read in valid line without comments */
   check = get_to_string(fspec,string);
   if(check == FAIL)
      return(FAIL);

   /* check that it is a valid line */
   clen = (int) strlen(cstring);
   if( strncmp(cstring,string,clen) != 0 )
   {
      eprintf("ERROR: Strings %s and %s not similar",cstring,string);
      fclose(fspec);
      return(FAIL);
   }

   /* read in the value after the string */
   check = sscanf(string+clen,"%d",value);
   if(check !=1)
   {
      eprintf("ERROR: Reading %s from file",cstring);
      fclose(fspec);
      return(FAIL);
   }
   return(OK);
}
/* ************************************************************************** */
int get_next_float_string(FILE *fspec, char *cstring, float *value)
{
   /* reads a float value after a given cstring
      01/23/97: JVS */

   char string[MAX_STR_LEN];
   int clen;
   int check;

   /* read in valid line without comments */
   check = get_to_string(fspec,string);
   if(check == FAIL)
      return(FAIL);

   /* check that it is a valid line */
   clen = (int) strlen(cstring);
   if( strncmp(cstring,string,clen) != 0 )
   {
      eprintf ("ERROR: Strings %s and %s not similar",cstring,string);
      fclose(fspec);
      return(FAIL);
   }

   /* read in the value after the string */
	check = sscanf(string+clen,"%f",value);
   if(check!=1)
   {
      eprintf ("ERROR: Reading %s from file ",cstring);
      fclose(fspec);
      return(FAIL);
   }
   return(OK);
}
/* ************************************************************************** */
int get_int(FILE *fspec, int *value)
{
   /* reads an integer value
      01/23/97: JVS */

   char string[MAX_STR_LEN];
   int check;

   /* read in valid line without comments */
   check = get_to_string(fspec,string);
   if(check == FAIL)
      return(FAIL);

   /* read in the value after the string */
	check = sscanf(string,"%d",value);
   if(check !=1)
   {
      eprintf ("ERROR: get_int: No value found on line %s",string);
      fclose(fspec);
      return(FAIL);
   }
   return(OK);
}
/* ************************************************************************** */
/* ************************************************************************** */
int get_float(FILE *fspec, float *value)
{
   /* reads an integer value
      01/23/97: JVS */

   char string[MAX_STR_LEN];
   int check;

   /* read in valid line without comments */
   check = get_string(fspec,string);
   if(check == FAIL)
      return(FAIL);

   /* read in the value after the string */
	check = sscanf(string,"%f",value);
   if(check !=1)
   {
      eprintf ("ERROR: get_int: No value found on line %s",string);
      fclose(fspec);
      return(FAIL);
   }
   return(OK);
}
/* ************************************************************************** */
int get_string(FILE *fspec, char *string)
{
   return(fget_c_string(string, MAX_STR_LEN, fspec));
}
/* ************************************************************************** */
int fget_c_string(char *string, int Max_Str_Len, FILE *fspec)
{
   /* gets a string from the input and removes comments from it */
   /* allows comments in standard "c" syntax,
           starting with / *
	   ending with  * /  */
   /* also allows c++ type comments, // causes rest of line to be skipped */

   int check;
   char comment_start[4]="/*";  /* signals start of comment */
   char comment_stop[4]="*/";   /* signals end of comment   */
   int clen; /* length of string for start/stop*/
   int ilen; /* length of input string */
   char *istring; /* input string */
   int olen; /* location on output string */
   int icnt; /* location on string */

   int n_pass = 0; /* Number of passes through the file looking for a value */

   olen = 0;
   strnset(string,'\0',Max_Str_Len); /* null entire output string */
   /* allocate memory for input string */
   istring = (char *)calloc(Max_Str_Len,sizeof(char));
   if(istring == NULL)
   {
      eprintf("\n ERROR: Allocating memory for input string if fget_c_string");
      return(FAIL);
   }

   clen = (int) strlen(comment_start);
   /* read in the line, verify that it exists */
   do{
      /* read in a line from the file */
      while(fgets(istring, Max_Str_Len, fspec) == NULL) /* output warning if not a valid read */
      {
#ifdef DEBUG
        printf("\n istring: %s", istring);
#endif
	if(n_pass) /* if already gone through file once looking for value, quit */
	{
           eprintf ("ERROR: Reading File : End of File On Read ");
           fclose(fspec);
           free(istring);
           return(FAIL);
        }
	n_pass++;      /* increment the number of times through the file */
        rewind(fspec); /* rewind to the beginning of the file */
        free(istring);
        return(REWIND_STREAM);
      }
      ilen = (int) strlen(istring); /* length of input string */
      istring[ilen]='\0'; /* null terminate the string */
      if(ilen < clen) /* not possible to have comment on the line */
      {               /* so output the string as is */
         strcpy(string,istring);
         olen = (int) strlen(string);
      }
      else
      {
        /* strip comments out of input string */
        icnt=0;
        do{
          check = 1;
          if(icnt < ilen - clen)  /* make sure have enough characters for start of comment */
             check = strncmp(istring+icnt,comment_start,clen); /* check if start of comment */
          if(check == 0) /* comment found for standard c syntax */
          {
            /* find end of comment */
            icnt+=clen; /* advance past comment delimeter */
            clen= (int) strlen(comment_stop); /* get length of end of comment delimeter */
            /* look for end of comment till end of string */
            do{
               check = 1;
               if(icnt < ilen - clen)  /* make sure have enough characters for end of comment */
                  check = strncmp(istring+icnt, comment_stop,clen);
               if(check != 0)  /* if not end of comment */
               {
                  icnt++; /* increment location on string */
                  if(icnt>ilen) /* if advance past end of string, get a new one */
                  {
                     if(fgets(istring, Max_Str_Len, fspec) == NULL) /* output warning if not a valid read */
                     {
                        eprintf ("ERROR: Reading File, looking for end of comment %s",comment_stop);
                        fclose(fspec);
                        free(istring);
                        return(FAIL);
                     }
                     ilen = (int) strlen(istring); /* get length of this new string */
                     /* null terminate the string */
                     istring[ilen]='\0';
                     icnt = 0; /* reset the counter to the start of the string */
                  }
               }
               else
               {
                  icnt+=clen; /* advance past comment delimiter */
               }
            }while(check != 0); /* end of comment found */
          }  /* end if */
          else /* check if comment is in c++ format */
          {
             check = strncmp(istring+icnt, "//",2);
             if(check == 0) /* c++ style comment found */
             {  /* skip till end of string */
                icnt = ilen;
                string[olen++]='\n';
                string[olen]='\0'; // March 22, 2000 fix
             }
             else /* is a valid character for the string */
             {
                string[olen++] = istring[icnt++]; /* append value to the string */
                string[olen]='\0';
             }
          }
        }while(icnt < ilen          &&    /* do till end of string */
               olen < Max_Str_Len); /* and output string not too long */
        /* check for only carriage return (should have been caught above) */
        if(olen == 1 && string[0] == '\n') olen = 0;

      }  /* end else */
   }while(olen == 0); /* do till read in a string */
   if(olen >= Max_Str_Len)
   {
      eprintf ("ERROR: Input line too long");
      fclose(fspec);
      free(istring);
      return(FAIL);
   }
   free(istring);
   return(OK);
}
/* ***************************************************************************** */

