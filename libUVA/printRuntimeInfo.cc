/* printRunTimeInformation.h
   directly include in your source doe to get output of runtime information
   Modification History:
           Feb 24, 2009: JVS: Broke off of utilities.cc
           March 17, 2009: JVS: printRunTimeInformation.h
           May 11, 2010: JVS: only include source if MAIN is defined
*/
#ifndef _printRunTimeInformation_h_included
#define _printRunTimeInformation_h_included
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
// #error "printRunTimeInformation.h is included "
/* ************************************************** */
#ifdef MAIN
// #error "printRunTimeInformation.h functions are being declared "
void printRunTimeInformation(int argc, char *argv[])
{  // print file header stuff
   printf("\n Command Line: ");
   for(int i=0; i<argc; i++) printf(" %s", argv[i]);
   // printf("\n Program %s Revision %f",  Prog_Name,Revision);
   // printf("\n\tCopyright 2009,2011 Virginia Commonwealth University\n\tAll Rights Reserved\n");
   // printf ("\n\tModule %s\n\tCompiled on %s at %s",__FILE__, __DATE__ , __TIME__);
   printf ("\n\tModule %s\n\tCompiled on %s at %s",argv[0], __DATE__ , __TIME__);
   time_t t;
   t = time(NULL);
   printf("\n\tRun on %s\n",  ctime(&t) );
}
/* ************************************************** */
void printRunTimeInformation(int argc, char *argv[], const char *myProg_Name, const float myRevision)
{  // print file header stuff
   printf("\n Command Line: ");
   for(int i=0; i<argc; i++) printf(" %s", argv[i]);
   printf("\n Program %s Revision %f",  myProg_Name,myRevision);
   // printf("\n\tCopyright 2009,2011 Virginia Commonwealth University\n\tAll Rights Reserved\n");
   printf("\n\tCopyright 2015, The University of Virginia\n\tAll Rights Reserved\n");
   // printf ("\n\tModule %s\n\tCompiled on %s at %s",__FILE__, __DATE__ , __TIME__);
   printf ("\n\tModule %s\n\tCompiled on %s at %s",argv[0], __DATE__ , __TIME__);
   time_t t;
   t = time(NULL);
   printf("\n\tRun on %s\n",  ctime(&t) );
}
/* *********************************************************** */
void print_runtime_info(int argc, char *argv[]) {
  printRunTimeInformation(argc, argv);
}
/* *********************************************************** */
#else
void printRunTimeInformation(int argc, char *argv[]);
void print_runtime_info(int argc, char *argv[]);
#endif
#endif
