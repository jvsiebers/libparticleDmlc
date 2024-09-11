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
/* option.cpp
   for getting command line options clipped from dcomment.cpp
   24-June-1997: JVS:
   18-August-1997: JVS: *bug from strcpy(argv[j],argv[j+i]) in get_option routines
                         corrected by direct pointer assignment. get_option would not
                         work correctly if the length of the strings for the flags
                         increased.
   22-Sept-1997: JVS: Add option,argc,argv,int and float versions
   25-Sept-1997: JVS: Update error messages to state "value"
   23-June-1998: jvs: add double version
   05-Nov-1998: JVS: Add int* int*
   03-Dec-1998: JVS: Clean up warning messages
   15-Dec-1998: JVS: Add int* char**
   03-Feb-1999: JVS: Add int get_option(char *option, int *argc, char *argv[], int *n_flag, float *flag_array)
   05-Feb-2002: JVS: add mySetEnv
   19-mar-2002: JVS: change setenv to putenv since setenv not exist on dingo, 
                     tested with simple program on wolf and dingo
   10-April-2002: JVS: Change to use putenv ONLY on solaris since causes problems on LINUX
   17-Oc6-2007: JVS: Get rid of #ifdef for SUN4SOL2
   Nov 27, 2007: JVS: const char *option 
   Nov 29, 2007: JVS: must use setenv on linux, cannot on solaris....
   Feb 6, 2008: JVS: Add <string> based definitions
   Feb 7, 2008: JVS: Add unsinged
   April 16, 2008: JVS: make sure string is initially null
     June 4, 2008: JVS: typecast to avoid 64bit compiler warnings
   April 26, 2010: JVS: Allow multi-word strings when getting single string flag
                        (e.g. -trial "this trial name" )
    April 29, 2011: JVS: Include <string.h> so have strlen and strcmp for gcc4.52
   Oct 20, 2011: JVS: Add dumpOptions(int argc, char *argv[]) so can see what options remain on the command line
   Nov 10, 2013: JVS: Have return when 1st matching flag is found
*/
#include <stdio.h>
#include <stdlib.h>
//#include <string.h>
#include <string>
#include <string.h> // so have strlen and strcmp
#include "option.h"
#include "utilities.h"
/* ************************************************************************** */
int mySetEnv(const char *flag, const char *varName, int *argc, char *argv[])
{
    char newValue[MAX_STR_LEN];
    newValue[0]=0;
    // printf("\n Checking for flag %s", flag);
    if(get_option(flag,argc,argv,newValue) != OK)
    {
       printf("\n ERROR: Reading %s flag\n",newValue);
       return(FAIL);
    }
    if( newValue[0] != 0 ) 
    {
      char envString[2*MAX_STR_LEN];
      sprintf(envString,"%s=%s",varName,newValue);
#ifdef linux
      if(setenv(varName,newValue,1) != 0)  // setenv definitely works on linux, but does not exist in SUN4SOL2
#else
	if( putenv(envString)!=0 )  // putenv does NOT work on linux
#endif
      {
	printf("\n ERROR: setting %s",flag); return(FAIL);
      } 
      printf("\n Set %s to %s ", varName,getenv(varName));
    }
    return(OK);
}
/* ********************************************************************* */
int dumpOptions(int *argc, char *argv[])
{
  char optionDelimiter[10];
  strcpy(optionDelimiter,"-");
  printf("\n Options remaining on command line for program %s\n", argv[0] );
  for(int i=1;i<*argc;i++){
    printf("\t%s", argv[i]);
    if(strncmp(argv[i],optionDelimiter,1) != 0) printf("\n");
  }
  printf("\n End Options\n");
  return(OK);
}
/* ********************************************************************* */
int get_option(const char *option, int *argc, char *argv[], char *flag)
{
   /* check for command line option */
  bool foundFlag=false;
   unsigned olen = (unsigned) strlen(option);
   for(int i=1;i<*argc && !foundFlag;i++)
   {
      if(strncmp(argv[i],option,olen) == 0)
      {  foundFlag = true;   /* found option , next token data for the flag */
         /* allocate memory for the flag */
         int n_tokens = 1; /* number of tokens consumed by the flag */
         if((unsigned) strlen(argv[i]) > olen ) /* no space between flag and name */
         {
            sscanf(argv[i]+olen,"%s",flag);
            n_tokens = 1;
         }
         else if(i == *argc-1) /* no name for flag given */
         {
            printf("\n ERROR: %s option given without value\n",option);
            exit(FAIL);
         }else  /* space between flag and name (flag next token) */
         {
           //Before April 26, 2010, this used sscanf
	   // sscanf(argv[i+1],"%s",flag);
           // However, in able to read multi-word flags (like trialNames that 
           // have a space in them), then just use strcpy...
            strcpy(flag,argv[i+1]);
            n_tokens = 2;
         }
         // remove the flags used from the flag array
         for(int j=i; j<*argc-n_tokens; j++)
         {
/*            strcpy(argv[j],argv[j+n_tokens]);*/
              *(argv+j)=*(argv+(j+n_tokens));
         }
         *argc-=n_tokens; /* reduce the number of arguments by the ones removed */
         i--;     /* reset to run this i again */
         /* output the "new" command line */
         // print_runtime_info(*argc,argv);
      }
   }
   return(OK);
}
/* ************************************************************************** */
int get_option(const char *option, int *argc, char *argv[], float *n_flag)
{
  bool foundFlag=false;
  /* check for command line option */
   unsigned olen = (unsigned) strlen(option);
   for(int i=1;i<*argc && !foundFlag;i++)
   {
      if(strncmp(argv[i],option,olen) == 0)
      {  foundFlag = true;   /* found option , next token data for the flag */
         /* allocate memory for the flag */
         int n_tokens = 1; /* number of tokens consumed by the flag */
         if((unsigned) strlen(argv[i]) > olen ) /* no space between flag and name */
         {
            sscanf(argv[i]+olen,"%f",n_flag);
            n_tokens = 1;
         }
         else if(i == *argc-1) /* no name for flag given */
         {
            printf("\n ERROR: %s option given without a value\n",option);
            exit(FAIL);
         }else  /* space between flag and name (flag next token) */
         {
            sscanf(argv[i+1],"%f",n_flag);
            n_tokens = 2;
         }
         for(int j=i; j<*argc-n_tokens; j++)
         {
/*            strcpy(argv[j],argv[j+n_tokens]);*/
              *(argv+j)=*(argv+(j+n_tokens));
         }
         *argc-=n_tokens; /* reduce the number of arguements by the ones removed */
         i--;     /* reset to run this i again */
         /* output the "new" command line */
         // print_runtime_info(*argc,argv);
      }
   }
   return(OK);
}
/* ************************************************************************* */
int get_option(const char *option, int *argc, char *argv[], double *n_flag)
{
  bool foundFlag=false;
  /* check for command line option */
   unsigned olen = (unsigned) strlen(option);
   for(int i=1;i<*argc && !foundFlag;i++)
   {
      if(strncmp(argv[i],option,olen) == 0)
      {  foundFlag = true;   /* found option , next token data for the flag */
         /* allocate memory for the flag */
         int n_tokens = 1; /* number of tokens consumed by the flag */
         if((unsigned) strlen(argv[i]) > olen ) /* no space between flag and name */
         {
            sscanf(argv[i]+olen,"%lf",n_flag);
            n_tokens = 1;
         }
         else if(i == *argc-1) /* no name for flag given */
         {
            printf("\n ERROR: %s option given without a value\n",option);
            exit(FAIL);
         }else  /* space between flag and name (flag next token) */
         {
            sscanf(argv[i+1],"%lf",n_flag);
            n_tokens = 2;
         }
         for(int j=i; j<*argc-n_tokens; j++)
         {
/*            strcpy(argv[j],argv[j+n_tokens]);*/
              *(argv+j)=*(argv+(j+n_tokens));
         }
         *argc-=n_tokens; /* reduce the number of arguements by the ones removed */
         i--;     /* reset to run this i again */
         /* output the "new" command line */
         // print_runtime_info(*argc,argv);
      }
   }
   return(OK);
}
/* ************************************************************************* */
int get_option(const char *option, int *argc, char *argv[], int *n_flag)
{
  bool foundFlag=false;
  /* check for command line option */
   unsigned olen = (unsigned) strlen(option);
   for(int i=1;i<*argc && !foundFlag;i++)
   {
      if(strncmp(argv[i],option,olen) == 0)
      {  foundFlag = true;   /* found option , next token data for the flag */
         /* allocate memory for the flag */
         int n_tokens = 1; /* number of tokens consumed by the flag */
         if((unsigned) strlen(argv[i]) > olen ) /* no space between flag and name */
         {
            sscanf(argv[i]+olen,"%d",n_flag);
            n_tokens = 1;
         }
         else if(i == *argc-1) /* no name for flag given */
         {
            printf("\n ERROR: %s option given without value\n",option);
            exit(FAIL);
         }else  /* space between flag and name (flag next token) */
         {
            sscanf(argv[i+1],"%d",n_flag);
            n_tokens = 2;
         }
         for(int j=i; j<*argc-n_tokens; j++)
         {
/*            strcpy(argv[j],argv[j+n_tokens]);*/
              *(argv+j)=*(argv+(j+n_tokens));
         }
         *argc-=n_tokens; /* reduce the number of arguements by the ones removed */
         i--;     /* reset to run this i again */
         /* output the "new" command line */
         // print_runtime_info(*argc,argv);
      }
   }
   return(OK);
}
/* ************************************************************************* */
int get_option(const char *option, int *argc, char *argv[], unsigned *n_flag)
{
  bool foundFlag=false;
  /* check for command line option */
   unsigned olen = (unsigned) strlen(option);
   for(int i=1;i<*argc && !foundFlag;i++)
   {
      if(strncmp(argv[i],option,olen) == 0)
      {  foundFlag = true;   /* found option , next token data for the flag */
         /* allocate memory for the flag */
         int n_tokens = 1; /* number of tokens consumed by the flag */
         if((unsigned) strlen(argv[i]) > olen ) /* no space between flag and name */
         {
            sscanf(argv[i]+olen,"%u",n_flag);
            n_tokens = 1;
         }
         else if(i == *argc-1) /* no name for flag given */
         {
            printf("\n ERROR: %s option given without value\n",option);
            exit(FAIL);
         }else  /* space between flag and name (flag next token) */
         {
            sscanf(argv[i+1],"%u",n_flag);
            n_tokens = 2;
         }
         for(int j=i; j<*argc-n_tokens; j++)
         {
/*            strcpy(argv[j],argv[j+n_tokens]);*/
              *(argv+j)=*(argv+(j+n_tokens));
         }
         *argc-=n_tokens; /* reduce the number of arguements by the ones removed */
         i--;     /* reset to run this i again */
         /* output the "new" command line */
         // print_runtime_info(*argc,argv);
      }
   }
   return(OK);
}
/* ************************************************************************* */
int get_option(const char *option, int *argc, char *argv[], int *n_flag, int *flag_array)
{
  //bool foundFlag=false;
  /* check for command line option */
  unsigned olen = (unsigned) strlen(option);
  // for(int i=1;i<*argc && !foundFlag;i++){
  for(int i=1;i<*argc;i++){
    if(strncmp(argv[i],option,olen) == 0){  
      //foundFlag = true;   /* found option , next token data for the flag */
      /* allocate memory for the flag */
      int n_tokens = 1; /* number of tokens consumed by the flag */
      if((unsigned) strlen(argv[i]) > olen ) /* no space between flag and name */
	{
	  sscanf(argv[i]+olen,"%d",&flag_array[*n_flag]);
	  *n_flag += 1; /* increment the number of flags read in */
	  n_tokens = 1;
	}
      else if(i == *argc-1) /* no name for flag given */
	{
	  printf("\n ERROR: %s option given without value\n",option);
	  exit(FAIL);
	}else  /* space between flag and name (flag next token) */
	  {
            sscanf(argv[i+1],"%d",&flag_array[*n_flag]);
            *n_flag +=1;
            n_tokens = 2;
	  }
      for(int j=i; j<*argc-n_tokens; j++)
	{
	  /*            strcpy(argv[j],argv[j+n_tokens]);*/
	  *(argv+j)=*(argv+(j+n_tokens));
	}
      *argc-=n_tokens; /* reduce the number of arguements by the ones removed */
      i--;     /* reset to run this i again */
      /* output the "new" command line */
      // print_runtime_info(*argc,argv);
    }
  }
  return(OK);
}
/* ************************************************************************* */
/* ************************************************************************* */
int get_option(const char *option, int *argc, char *argv[], int *n_flag, float *flag_array)
{
  //bool foundFlag=false;
  /* check for command line option */
   unsigned olen = (unsigned) strlen(option);
   // for(int i=1;i<*argc && !foundFlag;i++)
   for(int i=1;i<*argc;i++)
   {
      if(strncmp(argv[i],option,olen) == 0)
	{  //foundFlag = true;   /* found option , next token data for the flag */
         /* allocate memory for the flag */
         int n_tokens = 1; /* number of tokens consumed by the flag */
         if((unsigned) strlen(argv[i]) > olen ) /* no space between flag and name */
         {
            sscanf(argv[i]+olen,"%f",&flag_array[*n_flag]);
            *n_flag += 1; /* increment the number of flags read in */
            n_tokens = 1;
         }
         else if(i == *argc-1) /* no name for flag given */
         {
            printf("\n ERROR: %s option given without value\n",option);
            exit(FAIL);
         }else  /* space between flag and name (flag next token) */
         {
            sscanf(argv[i+1],"%f",&flag_array[*n_flag]);
            *n_flag +=1;
            n_tokens = 2;
         }
         for(int j=i; j<*argc-n_tokens; j++)
         {
/*            strcpy(argv[j],argv[j+n_tokens]);*/
              *(argv+j)=*(argv+(j+n_tokens));
         }
         *argc-=n_tokens; /* reduce the number of arguements by the ones removed */
         i--;     /* reset to run this i again */
         /* output the "new" command line */
         // print_runtime_info(*argc,argv);
      }
   }
   return(OK);
}
/* ************************************************************************* */
int get_option(const char *option, int *argc, char *argv[], int *n_flag, char **flag_array)
{
  //bool foundFlag=false;
  /* check for command line option */
   unsigned olen = (unsigned) strlen(option);
   for(int i=1;i<*argc;i++)
   {
      if(strncmp(argv[i],option,olen) == 0)
	{  //foundFlag = true;   /* found option , next token data for the flag */
         /* allocate memory for the flag */
         int n_tokens = 1; /* number of tokens consumed by the flag */
         if((unsigned) strlen(argv[i]) > olen ) /* no space between flag and name */
         {
            sscanf(argv[i]+olen,"%s",flag_array[*n_flag]);
            *n_flag += 1; /* increment the number of flags read in */
            n_tokens = 1;
         }
         else if(i == *argc-1) /* no name for flag given */
         {
            printf("\n ERROR: %s option given without value\n",option);
            exit(FAIL);
         }else  /* space between flag and name (flag next token) */
         {
            sscanf(argv[i+1],"%s",flag_array[*n_flag]);
            *n_flag +=1;
            n_tokens = 2;
         }
         for(int j=i; j<*argc-n_tokens; j++)
         {
/*            strcpy(argv[j],argv[j+n_tokens]);*/
              *(argv+j)=*(argv+(j+n_tokens));
         }
         *argc-=n_tokens; /* reduce the number of arguements by the ones removed */
         i--;     /* reset to run this i again */
         /* output the "new" command line */
         // print_runtime_info(*argc,argv);
      }
   }
   return(OK);
}
/* ************************************************************************* */
int get_option(const char *option, int *argc, char *argv[], long *n_flag)
{
  bool foundFlag=false;
   /* check for command line option */
   unsigned olen = (unsigned) strlen(option);
   for(int i=1;i<*argc && !foundFlag;i++)
   {
      if(strncmp(argv[i],option,olen) == 0)
      {  foundFlag = true;   /* found option , next token data for the flag */
         /* allocate memory for the flag */
         int n_tokens = 1; /* number of tokens consumed by the flag */
         if((unsigned) strlen(argv[i]) > olen ) /* no space between flag and name */
         {
            sscanf(argv[i]+olen,"%ld",n_flag);
            n_tokens = 1;
         }
         else if(i == *argc-1) /* no name for flag given */
         {
            printf("\n ERROR: %s option given without value\n",option);
            exit(FAIL);
         }else  /* space between flag and name (flag next token) */
         {
            sscanf(argv[i+1],"%ld",n_flag);
            n_tokens = 2;
         }
         for(int j=i; j<*argc-n_tokens; j++)
         {
/*            strcpy(argv[j],argv[j+n_tokens]);*/
              *(argv+j)=*(argv+(j+n_tokens));
         }
         *argc-=n_tokens; /* reduce the number of arguements by the ones removed */
         i--;     /* reset to run this i again */
         /* output the "new" command line */
         // print_runtime_info(*argc,argv);
      }
   }
   return(OK);
}
/* ************************************************************************** */
int get_option(const char *option, int *argc, char *argv[],
               char **flag, int *n_flag)
{
  bool foundFlag=false;
   /* check for command line option */
   unsigned olen = (unsigned) strlen(option);
   for(int i=1;i<*argc && !foundFlag;i++)
   {
      if(strncmp(argv[i],option,olen) == 0)
      {  foundFlag = true;   /* found option , next token data for the flag */
         /* allocate memory for the flag */
         int n_tokens = 1; /* number of tokens consumed by the flag */
         flag[*n_flag]=(char *) malloc(MAX_STR_LEN*sizeof(char));
         if(flag[*n_flag] == NULL)
         {
            printf("\n ERROR: Cannot Allocate Memory for flag");exit(FAIL);
         }
         if((unsigned) strlen(argv[i]) > olen ) /* no space between flag and name */
         {
            strcpy(flag[*n_flag],argv[i]+olen);
            n_tokens = 1;
         }
         else if(i == *argc-1) /* no name for flag given */
         {
            printf("\n ERROR: %s option given without values\n",option);
            exit(FAIL);
         }else  /* space between flag and name (flag next token) */
         {
            strcpy(flag[*n_flag],argv[i+1]);
            n_tokens = 2;
         }
         // printf("\n ****  Flag %s ****",flag[*n_flag]);
         *n_flag=*n_flag+1;
         /* then remove -i and the include_file_name from the list to process */

         for(int j=i; j<*argc-n_tokens; j++)
         {
/*            strcpy(argv[j],argv[j+n_tokens]);*/
              *(argv+j)=*(argv+(j+n_tokens));
         }
         *argc-=n_tokens; /* reduce the number of arguements by the ones removed */
         i--;     /* reset to run this i again */
         /* output the "new" command line */
         // print_runtime_info(*argc,argv);
      }
   }
   return(OK);
}
/* **************************************************************** */
int get_option(const char *option, int *argc, char *argv[])
{
  bool foundFlag=false;
   /* check for command line option,
      returns OK if option found
      FAIL if option not found */
   int rvalue = FAIL;
   unsigned olen = (unsigned) strlen(option);
   for(int i=1;i<*argc && !foundFlag;i++)
   {
      if(strncmp(argv[i],option,olen) == 0)
      {  foundFlag = true;   /* found option , next token data for the flag */
         /* allocate memory for the flag */
         rvalue = OK;
         int n_tokens = 1; /* number of tokens consumed by the flag */
         for(int j=i; j<*argc-n_tokens; j++)
         {
/*            strcpy(argv[j],argv[j+n_tokens]);*/
              *(argv+j)=*(argv+(j+n_tokens));
         }
         *argc-=n_tokens; /* reduce the number of arguements by the ones removed */
         i--;     /* reset to run this i again */
         /* output the "new" command line */
         // print_runtime_info(*argc,argv);
      }
   }
   return(rvalue);
}
/* ************************************************************************** */
int get_option(const char *option, int *argc, char *argv[], std::string *stringToSet){
  char cString[MAX_STR_LEN];
  cString[0]=0; // make sure string is initially null terminated
  int rValue = get_option(option, argc, argv, cString);
  if(OK == rValue)
    *stringToSet = cString; //< Set the string only if it was OK
  return(rValue);
}
/* ************************************************************************** */
