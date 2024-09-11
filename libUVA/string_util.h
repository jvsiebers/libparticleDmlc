/** \file 
 simple tag auto added to force generation of simple doxygen comments
**/
#ifndef STRING_UTIL_H
#define STRING_UTIL_H
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
/* string_util.h
  header file to go along with string_util.cpp
  06-Feb-1996: JVS
  05-Nov-2007: JVS: Add C_CLASS stuff.  Add checkMyErrno
  27-Nov-2007: JVS: const char * for get_value functions
  28-Nov-2007: JVS: Get working with class
  13-May-2015: JVS: add fileName and filePointer to class (not tested or used) add checkNextLineForString
*/
#ifdef C_STRING_CLASS
class jvsString {
 public:
    jvsString(){
      fileName=NULL;
      filePointer=NULL;
      returnFlagFromConstructor = OK;
    }
    ~jvsString(){
      if(NULL != fileName) delete [] fileName;
      if(NULL != filePointer) delete [] filePointer;
    }
    int jvsStringIsReady(){ return(returnFlagFromConstructor); }  
    int jvsStringError(){ return(checkStringError()); }
    void setFilePointer(FILE *fp){filePointer=fp);
    void setFileName(char *name){
      fileName = new char[strlen(name)+];
      strcpy(fileName,name);
    }
    ~jvsString(); // destructor
    //
 private:
    char *fileName;
    FILE *filePointer;
    int returnFlagFromConstructor;
#endif
    // function prototypes (private functions for the C-Class)
    int string_after(FILE *stream, const char *, char *, bool);
    int string_after(FILE *stream, const char *, char *);
    int string_after_quiet(FILE *stream, const char *, char *);
    float get_value(FILE *, const char *, bool);
    float get_value(FILE *, const char *);
    float get_value_quiet(FILE *, const char *);
    int   get_int_value(FILE *, const char *);
    int   get_to_line(FILE *, const char *);
    char *checkNextLineForString(FILE *, const char *, bool, bool);
    char *checkNextLineForString(FILE *, const char *);
    char *checkNextLineForStringQuiet(FILE *, const char *);
    char *check_if_line(FILE *, const char *);
    char *check_if_line_quiet(FILE *, const char *);
    int get_string_to_scan(FILE *istrm, char *string, char *str);
    float get_value_in_file(FILE *istrm, const char *string);
    int checkStringError();
    int resetStringError();
#ifdef C_STRING_CLASS
};
#endif
//
#endif
//
