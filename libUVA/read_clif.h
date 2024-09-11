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
/* read_clif.h
   prototypes to read pinnacle clif format files
   Created: 6/9/98: JVS
   Modification History:
      June 2, 2000: JVS: Add END_OF_FILE
      Jun 7, 2000: JVS: Add new ClifRead structures and routines...
      Jan 6, 2000: Add array names so SGI compiler not complain.
      Nov 27, 2007: JVS: Add const char * for g++ 4.2
      Feb 24, 2011: JVS: clif_set_errstat and clif_get_errstat from RCF put in here.
      May 18, 2012: JVS: #include <stdio.h> for -stlport4 compile on solaris/i386 64 bit
      Feb 5, 2015:  JVS: change comments to // 
      May 16, 2017: JVS: Add copyClifStructure

*/
#ifndef read_clif_h_included
#define read_clif_h_included
#include <stdio.h>
#include <string>

#define END_OF_CONTEXT 10
#define NOT_FOUND      20
#define END_OF_FILE    30

// Define the "types" needed for these routines 
typedef struct clifDataType{
  char *Handle;
  char *Value; 
}clifDataType;
typedef struct clifArrayType{
  char *Handle;
  float *Value;
  int nValues;
}clifArrayType;
typedef struct clifObjectType{
  char *Handle;              // name of the object     
  clifDataType   *Data;      // Array of Data Objects  
  clifArrayType  *Array;     // Array of Array Objects 
  clifObjectType *Object;    // Array of Sub-objects   
  int nDataObjects;          // Number of each type of object 
  int nArrayObjects;
  int nObjects;
  int clifLevel;             // for keeping track of the level of the clif 
}clifObjectType;
int  dumpClifStructure(clifObjectType *clif);
int  freeClifStructure(clifObjectType *clif); // free's all of the memory allocated by a clif struture
bool copyClifStructure(clifObjectType *destClif, clifObjectType *sourceClif);
// Routines used for Reading a Value from a Clif Structure 
int readClifStructure(clifObjectType *clif, const char *Name, char **Value);
int readClifArrayStructure(clifObjectType *clif, const char *messageHandle, int *nArray, float **Array);
int getClifNObjects(clifObjectType *clif, const char *Name);
// Routines used for Reading Clif From a File 
int readClifStringName(char *inString, const char *delimiter, char **outString);
int readClifStringValue(char *inString, const char *delimiter, char **outString);
int readClifLevel(FILE *istrm, clifObjectType *clif);
int readClifData(FILE *istrm, clifObjectType *clif);
int readClif(FILE *istrm, clifObjectType *clif, const char *clifName);
int readClifFile(FILE *istrm, clifObjectType *clif, const char *clifName);
int readSpecificClif(FILE *istrm, clifObjectType *clif, const char *clifHandle, const char *clifName);
int getMatchingClifAddress(clifObjectType *clif, const char *handleToMatch, char *valueToMatch, clifObjectType **matchingObject);
int getClifAddress(clifObjectType *Clif, const char *messageHandle, clifObjectType **Object);
int getClifObjectNumber(int nObjects,  clifObjectType *Object, const char *objectHandle, int *iObject);
int findClifObjectNumber(int nObjects,  clifObjectType *Object, const char *objectHandle, int *iObject);
int readStringClifStructure(clifObjectType *clif, const char *messageHandle, char *Value);// read preallocted string
bool readStringClifStructure(clifObjectType *clif, const char *messageHandle, std::string& outString);
int readIntClifStructure(clifObjectType *clif, const char *messageHandle, int *Value);
int readFloatClifStructure(clifObjectType *clif, const char *messageHandle, float *Value);
int checkIfObjectList(clifObjectType *Object);
int checkForClifObject(clifObjectType *clif, const char *messageHandle);
// Old clif routine names and objects 

int clif_check_if_line(FILE *istrm, const char *string, char *ostring);
int clif_string_after(FILE *istrm, const char *string, char *str);
float clif_get_value(FILE *istrm, const char *string);
int clif_get_to_line(FILE *istrm, const char *string);
int clif_string_read(FILE *fp, const char *string, char *ostring);
char *clif_fgets(char *string, int Max_Len, FILE *fp);
char *clifFgets(char *string, int Max_Len, FILE *fp);
int fget_cstring(char *string, int Max_Str_Len, FILE *fspec);
int checkClifError();// checks if myerrno set in clif read routines
int resetClifError(); // resets clif error to OK
void clif_set_errstat(int errstat);
int clif_get_errstat();

#endif
