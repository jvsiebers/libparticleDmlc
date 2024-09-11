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
/*  General Utilities for CPP programs
	File Created:
		18-December-1995: Combined ok_check.cpp and some open_file
	Modification History:
		01-Feb-1996: JVS: filename used in open_file has extension only in openfile
		09-Feb-1996: JVS: Add eprintf: outputs to screen and a buffer called pbuffer
								pbuffer is a global whose memory must be allocated
								this is useful for creating a "history" file
      17-June-1996: JVS: change latex_string so will work with win95/bc5.0
                         cannot have for(int i=0,j=0; ). j will not increment.
      05-Sept-1996: JVS: Add allocate_pbuffer and print_runtime_info
      23-April-1997: JVS: Add interpolate
      06-Jan-1998: JVS: Add array_read
      11-June-1998: jvs: add eprintf and view_errors
      22-Sept-1998: JVS: fix memory leak in eprintf
      07-Dec-1998: JVS: Add clean_name
      18-Feb-1999: JVS: eliminate atof in array_read because of failures
      25-Feb-1999: JVS: array_read will now read numbers that start with .
      26-Feb-1999: JVS: Add array_read for strings
      02-March-1999: JVS: Add global eprint_mode so can quite eprintf statements
      24-March-1999: JVS: Modify clean_name so names cannot have *'s in them
      July 20, 1999: JVS: eprintf modified to use fprintf(stdout), rather than printf
      Dec 3, 1999: JVS: Add check_byte_order
      Jan 11, 2000: JVS: Modify clean_name so names cannot have / in them
      Jun 16, 2000: JVS: Change open_file so will read in .extension properly when a . is in
                         the path name
      April 25, 2001: JVS: Add cp(SourceFile,DestinationFile)
      May 29, 2001: JVS: clean_name now removes & as well
      May 31, 2002: add reverse_short_byte_order
      Feb 18, 2004: JVS: Add writeBigEndianBinaryFile()
      Feb 10, 2005: JVS: Add writeLittleEndianBinaryFile() and writeBinaryFile()
      Feb 11, 2005: JVS: Add reverse_int_byte_order
      April 21, 2005: JVS: Add readBinaryDataFromFile
      August 13, 2007: JVS: Add fileExists
      October 17, 2007: JVS: get rid of "new" that resulted in variable-size array complaint
      Nov 27, 2007: JVS: const char *'s added so compatible with g++ 4.2
      Jan 23, 2008: JVS: Add int readBigEndianBinaryDataFromFile(char *iFileName, int nVoxels, float **dose)
      May 20, 2008: JVS: pbuffer and eprintf_mode removed from header file and placed in here, add free_pbuffer
     June 4, 2008: JVS: typecast to avoid 64bit compiler warnings
      June 30, 2008: JVS: Extract byte-order stuff to stand-alone readWriteEndianFile.cc
     April 26, 2010: JVS: clean_names gets rid of "(" and ")" in file names too
      Mar 7, 2012: JVS: clean_name now gets rid of ^'s too.
      Nov 10, 2013: JVS: update latex_string to changeToLatexString -- for outputting stuff from analyse
      Aug 26, 2014: JVS: interpolate returns FLT_MAX if xh=xl
      Oct 12, 2015: JVS: clean_name gets rid of #'s too.
      Oct 14, 2015: JVS: Add replaceEnvironmnetVariableInString();
      Jan 12, 2017: JVS: Clean up interpolate output xl==xh but yl!=yh.
      Feb 20, 2018: JVS: clean_name, avoids __ when input string is "this _that"
      May 10, 2018: JVS: clean_name removes ' from name
      Nov 6, 2019: JVS: clean_name removes leading and trailing space from string
*/
#include <stdio.h>
#include <ctype.h> // for isspace and isascii
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <math.h> // for round
#include <time.h>
#include <float.h> // For FLT_MAX
#include "utilities.h"
#include "printRunTimeInformation.h" // include at compile time
/*            */
// #ifdef MAIN
// int eprintf_mode = ON;
// #else
//extern int eprintf_mode;
// #endif
// #ifndef MAIN
// extern          // create global pbuffer
// #endif          
// char *pbuffer;  // pointer to buffer for output of run info for failures
/*            */
/* ************************************************************************** */
/* ********************************************************* */
bool replaceEnvironmentVariableInString(const char *variableName, char *instring) {
    int variableLen = (int) strlen(variableName);
    if (0 == strncmp(variableName, instring, variableLen)) {
        if (!(getenv(variableName))) {
            std::cerr << " ERROR: " << __FUNCTION__ << ": string " << instring << " requires environment variable" <<
                      variableName << " which is not defined " << std::endl;
            return false;
        }
        // make sure string is long enough...
        if (strlen(instring) + strlen(getenv(variableName)) >= MAX_STR_LEN) {
            std::cerr << " ERROR: " << __FUNCTION__ << " MAX_STR_LEN exceeded when replacing " << variableName <<
                      " in " << instring << std::endl;
            return false;
        }
        char tmpString[MAX_STR_LEN];
        strcpy(tmpString, getenv(variableName));
        strcat(tmpString, instring + variableLen);
        strcpy(instring, tmpString);
    }
    return true;
}

/* ************************************************************************* */
int fileExists(const char *fileName) {
    FILE *strm = fopen(fileName, "r");
    if (NULL == strm) return (FAIL); // file does not exist
    fclose(strm);
    return (OK);
}

bool doesFileExist(const char *fileName) {
    FILE *strm = fopen(fileName, "r");
    if (NULL == strm) return false; // file does not exist
    fclose(strm);
    return true;
}

/* ****************************************************************** */
void allocate_pbuffer() {
    pbuffer = (char *) malloc(MAX_BUFFER_SIZE * sizeof(char) + 2);
    if (pbuffer == NULL) {
        printf("\n Error Allocating Memory Buffer");
        exit(EXIT_FAILURE);
    }
}

/* ***************************************************************** */
void free_pbuffer() {
    free(pbuffer);
}

/* *********************************************************************** */
int advance(char *istr, int *sval, int len) {                           /* advances past white-space in file */
    while (!isspace(istr[*sval]) && (*sval < len))
        *sval += 1; /* advance to space */
    while (isspace(istr[*sval]) && (*sval < len))
        *sval += 1; /* advance to next thing */
    if (*sval > len)
        return (0); /* return 0 when fails */
    return (1);
}

/* ********************************************************************** */
int my_isascii(int c) {
    return (!(c < 0 || c > 0177));
}

/* ********************************************************************* */
int clean_name(char *name) {
    int len = (int) strlen(name);
    char *tname = (char *) calloc(len + 1, sizeof(char));
    if (tname == NULL) {
        eprintf("\n ERROR: memory allocation error");
        return (FAIL);
    }
    strcpy(tname, name);
    if (clean_name(tname, name) != OK) {
        eprintf("\n ERROR: cleaning Name");
        return (FAIL);
    }
    free(tname);
    return (OK);
}

/* ********************************************************************* */
int clean_name(const char *tmp_path, char *opath) {
    // removes leading and trailing spaces from string
    // replaces middle spaces with _
    // removes  *'s, :'s &'s ('s )'s and commas from the name
    int iStop = (int) strlen(tmp_path);
    while (isspace(tmp_path[iStop - 1]) && iStop) iStop--; // remove trailing space
    int iStart = 0;
    while (isspace(tmp_path[iStart]) && iStart < iStop) iStart++; // remove leading space
    int o_index = 0;
    for (int i = iStart; i < iStop; i++) {
        if (isspace(tmp_path[i])) {
            if (o_index &&              // add a _ if not first char
                opath[o_index - 1] != '_') // and if previous char not a _
                opath[o_index++] = '_';
        } else if (tmp_path[i] == '_' && o_index && opath[o_index - 1] == '_') { // avoid double _
            opath[o_index - 1] = '_'; // dummy statement, does nothing since already is a _
        } else if (my_isascii(tmp_path[i]) &&
                   tmp_path[i] != '#' &&
                   tmp_path[i] != '\'' &&
                   tmp_path[i] != '&' &&
                   tmp_path[i] != ',' &&
                   tmp_path[i] != '^' &&
                   tmp_path[i] != '*' &&
                   tmp_path[i] != '/' &&
                   tmp_path[i] != ')' &&
                   tmp_path[i] != '(' &&
                   tmp_path[i] != ':') {
            opath[o_index++] = tmp_path[i];
        }
    }
    opath[o_index] = '\0'; /* terminate the string */

    return (OK);
}

/* *********************************************************************** */
FILE *open_file(char *filename, const char *extension, const char *access) {
    char string[MAX_STR_LEN];
    FILE *strm;

    if (filename[0] == '\0') {
        printf("\n INPUT FILENAME (%s) >", access);

        if (NULL == fgets(string, MAX_STR_LEN, stdin)) {
            printf("\n ERROR: fgets");exit(-1);
        }
        sscanf(string, "%s", filename);
        printf("\t %s", filename);
    }
    int len = (int) strlen(filename);

    if (len + (int) strlen(extension) >= MAX_STR_LEN) {
        printf("\n ERROR: String Length  of %s.%s Exceeds Maximum",
               filename, extension);
        return (NULL);
    }
    // char *filename1 = new(char[len+(int) strlen(extension)+1]);
    char *filename1 = (char *) calloc(len + (int) strlen(extension) + 1, sizeof(char));
    if (NULL == filename1) {
        printf("\n ERROR: Memory Allocation for filename1\n");
        return (NULL);
    }
    strcpy(filename1, filename); // temp filename for appending extension

    /* check if file name has .extension    */
    /* if it does not, add .extension to it */
    int i = len - 1;
    while (i > 0 && filename[i--] != '.');
    //   printf("\n Comparing %s to %s", extension, filename+i+1);
    if (strcmp(extension, filename + i + 1))
        strcat(filename1, extension);
    if ((strm = fopen(filename1, access)) == NULL) {
        printf("\n ERROR: Opening file %s (mode %s)", filename1, access);
        return (NULL);
    }
    // delete(filename1);
    free(filename1);
    return (strm);
}/* *********************************************************************** */
int ok_check(void)                          /* GETS RESPONSE FROM USER      */
{                                           /* IF OK TO DO SOMETHING        */
    char reply[MAX_STR_LEN];                 /* RETURNS 1 ONLY IF REPLY Y    */
    /* OR y ELSE RETURNS 0          */
    if (NULL == fgets(reply, MAX_STR_LEN, stdin)) {
        printf("\n ERROR: fgets");exit(-1);
    }
    if ((strncmp(reply, "Y", 1) == 0) || (strncmp(reply, "y", 1) == 0))
        return (1);
    return (0);
}

/* ***********************************************************************  */
int ok_checks(const char *string) {
    printf("\n %s", string);
    return (ok_check());
}
/* ********************************************************************** */
#include <stdarg.h> // for va function

int pprintf(const char *fmt, ...) {
    va_list argptr;         /* Argument list pointer   */
    char str[MAX_STR_LEN];           /* Buffer to build sting into   */
    int cnt;            /* Result of SPRINTF for return */
    va_start(argptr, fmt);      /* Initialize va_ functions   */
    cnt = vsprintf(str, fmt, argptr);   /* prints string to buffer   */
    if (str[0] == '\0') return (0);
    printf("%s", str);   /* Send  to screen */
    if (pbuffer != NULL && (int) strlen(pbuffer) + (int) strlen(str) < MAX_BUFFER_SIZE)
        strcat(pbuffer, str);
    else
        printf("\n ERROR: pbuffer is full");
    va_end(argptr);         /* Close va_ functions      */
    return (cnt);         /* Return the conversion count   */
}
/* *********************************************************************** */
/* eprintf: for buffering error reports, writes error messages to a buffer,
   and, also can echo them to the screen (if set at compile time) 
   at first instance, allocates memory for the error buffer              */
static char *ebuffer = NULL;

int eprintf(const char *fmt, ...) {
    va_list argptr;         /* Argument list pointer   */
    char str[MAX_STR_LEN];           /* Buffer to build sting into   */
    int cnt;            /* Result of SPRINTF for return */
    va_start(argptr, fmt);      /* Initialize va_ functions   */
    cnt = vsprintf(str, fmt, argptr);   /* prints string to buffer   */
    if (str[0] == '\0') return (0);

    if (eprintf_mode == ON)
        fprintf(stdout, "%s", str);   /* Send  to screen */

    // allocate memory for the error message
    int ilen = 0;
    if (ebuffer != NULL) {
        ilen += (int) strlen(ebuffer);
        ebuffer = (char *) realloc(ebuffer, (ilen + (int) strlen(str) + 1) * sizeof(char));
    } else
        ebuffer = (char *) calloc(ilen + (int) strlen(str) + 1, sizeof(char));

    if (ebuffer == NULL) {
        printf("\n ERROR: ebuffer cannot be allocated in eprintf");
    } else
        strcat(ebuffer, str);
    va_end(argptr);         /* Close va_ functions      */
    return (cnt);         /* Return the conversion count   */
}

int view_errors(void) {
    printf("\n%s\n", ebuffer);
    return (OK);
}

/* ************************************************************************** */
int latex_string(char *string, char *nstring) {
    // adds \\ in front of % so % will show up in the comment when printed
    // with LaTeX
    // must change all %'s to \% for latex output
    // also, must do the same for $, &, # _  { and }
    int len = (int) strlen(string);
    int sval = 0;
    int j;
    while (isspace(string[sval]))sval++; // remove space from start of string
    while (isspace(string[len - 1]))len--; // remove space from end to string

    j = 0;
    for (int i = sval; i < len; i++) {
        if (string[i] == '%' ||
            string[i] == '$' ||
            string[i] == '&' ||
            string[i] == '#' ||
            string[i] == '_' ||
            string[i] == '{' ||
            string[i] == '}') {
            nstring[j++] = '\\';
        } else if (string[i] == '<' ||
                   string[i] == '>') {
            nstring[j++] = '$';
        }
        nstring[j++] = string[i];
        if (string[i] == '<' ||
            string[i] == '>') {
            nstring[j++] = '$';
        }
    }
    nstring[j] = '\0';
/*   printf("\n string: %s", string);
   printf("\n nstring: %d %s",j, nstring); */
    return (OK);
}
/* ************************************************************************** */
/* ************************************************************************** */
int changeToLatexString(char *string, char *nstring) {
    // adds \\ in front of % so % will show up in the comment when printed
    // with LaTeX
    // must change all %'s to \% for latex output
    // also, must do the same for $, &, # _  { and }
    int len = (int) strlen(string);
    int sval = 0;
    int j;
    int endMathNextSpace = 0;
    while (isspace(string[sval]))sval++; // remove space from start of string
    while (isspace(string[len - 1]))len--; // remove space from end to string
    j = 0;
    for (int i = sval; i < len; i++) {
        if (string[i] == '%'
            || string[i] == '$'
            || string[i] == '&'
            || string[i] == '#'
            //|| string[i]=='_'
            || (string[i] == '{' && !endMathNextSpace)
            || (string[i] == '}' && !endMathNextSpace)
                ) {
            nstring[j++] = '\\';
        } else if (((string[i] == '_') && !endMathNextSpace) || (endMathNextSpace && isspace(string[i]))) {
            nstring[j++] = '$';
            if (endMathNextSpace) endMathNextSpace = 0;
            else endMathNextSpace = 1;
        } else if (string[i] == '<' || string[i] == '>') {
            nstring[j++] = '$';
        }
        nstring[j++] = string[i];
        if (string[i] == '<' ||
            string[i] == '>') {
            nstring[j++] = '$';
        }
    }
    if (endMathNextSpace) nstring[j++] = '$';
    nstring[j++] = '\\';  // put newline on end of every latex string ...
    nstring[j++] = '\\';
    nstring[j] = '\0';
/*   printf("\n string: %s", string);
   printf("\n nstring: %d %s",j, nstring); */
    return (OK);
}

/* ************************************************************************** */
float interpolate(float xh, float xl, float xm, float yh, float yl) {
    if (xh == xl) {
        // printf ("\n INTERPOLATE: ");
        if (xm == xh && yl == yh) return (yl); // special case of exact match...
        if (xm == xh) return (0.5f * (yl + yh));    // When xh=xl=xm, then interpolation = averaging
        printf("\n ERROR: interpolate: xh=xl=%g, xm=%g, yl=%g, yh=%g", xh, xm, yl, yh);
        return (FLT_MAX);
    }
    return (yh - (xh - xm) / (xh - xl) * (yh - yl));
}
/* *********************************************************************** */
// #define DEBUG_ARRAY
/* ********************************************************************** */
int array_read(char *in_string, float *array, int max_array) {
    char delimeter_string[MAX_STR_LEN];
    sprintf(delimeter_string, " ,\t"); /* spaces, commas, and tabs */

    char *p; /* pointer to string read in */
    p = strtok(in_string, delimeter_string);
    int i = 0;
    if (p != NULL) {
        array[i++] = (float) atof(p); /* get the first value */
        // if( sscanf(p,"%f",&array[i]) == 1) i++; // sscanf rounds values....
        do {             /* get remaining values */
            p = strtok(NULL, delimeter_string);
            if (p != NULL) {
                //array[i++] = atof(p);
                if (sscanf(p, "%f", &array[i]) == 1) i++;
                // printf("\n Got Value of %f", array[i-1]);
            }
        } while (p != NULL && i < max_array);
    }

#ifdef DEBUG_ARRAY
    printf("\n atof %d", i);
    for(int j=0; j<i; j++) {
      //     array[j] = 0.0001*round(1000.0*array[j]);
       printf("\n i = %d, %f",j,array[j]);
    }
#endif
    return (i);

}

int array_read(FILE *istrm, float *array, int max_array) {
    // reads in an array of floats from a single line of istrm
    // returns the number of elements read in

    char in_string[MAX_STR_LEN];

    if (fgets(in_string, MAX_STR_LEN, istrm) == NULL) return (FAIL);
#ifdef DEBUG_ARRAY
    printf("\nInput String\n %s", in_string);
#endif

    int slen = (int) strlen(in_string);

    int k = 0;
    while (isspace(in_string[k]) && k < slen) k++;
    if (slen == 0 || !(isdigit(in_string[k])
                       || in_string[k] == '.'
                       || in_string[k] == '+'
                       || in_string[k] == '-')) {
        return (0); // skip blank and non-numerical lines
    }
    int nread = array_read(in_string, array, max_array);

    return (nread);  // return the number of elements read
}

/* ********************************************************************** */
int copy(const char *SourceFile, const char *DestinationFile) {
    /* Copies sourceFile to destination file like unix cp command */
    FILE *sStream = fopen(SourceFile, "rb");
    if (sStream == NULL) {
        perror("\n ERROR: copy: ");
        printf("\n ERROR: copy: Opening Source File %s", SourceFile);
        return (FAIL);
    }
    FILE *dStream = fopen(DestinationFile, "wb");
    if (dStream == NULL) {
        perror("\n ERROR: copy:");
        printf("\n ERROR: copy: Opening Destination File %s", DestinationFile);
        return (FAIL);
    }
    char buffer[1000];
    int nRead;
    do {
        nRead = (int) fread(buffer, sizeof(char), 1000, sStream);
        if (nRead)
            fwrite(buffer, sizeof(char), nRead, dStream);
    } while (!feof(sStream) && !ferror(dStream) && !ferror(sStream));
    if (ferror(sStream) || ferror(dStream)) {
        perror("ERROR: Copy: ");
        printf("\n ERROR: source %s, destination %s", SourceFile, DestinationFile);
        return (FAIL);
    }
    fclose(sStream);
    fclose(dStream);
    return (OK);
}
/* ********************************************************************** */
