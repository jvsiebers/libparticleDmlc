/** \file 
 simple tag auto added to force generation of simple doxygen comments
**/
/*
   Copyright 2000-2003,2012 Virginia Commonwealth University

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
/* readClif.cc

   Copyright 2000: JVS/ Virginia Commonwealth University

      Routines to read C Like Information Files (Pinnacle) 
      
      This code reads in an entire clif level, then
      allows you to access the data in the clif in an
      arbitrary (non-ordered) fashion.

      This should allow us to read / use data from 
      arbitrary versions of Pinnacle
 
      Use with test program ClifRead.cc


   Modification History:
   Sept 9, 1999: JVS: Developed Prototype of how this should work
   Jun 1, 2000: JVS:
   June 2, 2000: JVS: Routines for Reading Clif file into structure is completed...
   June 5, 2000: JVS: Routines to read item from clif structure
                      Allows using "Name" syntax and #"# syntax....
   Jun 7, 2000: JVS: Working With Pencil Beam
   Jun 21, 2000: JVS: added findClifObjectNumber (which does not report error)
                      and commented out unneeded code before getClifObjectNumber...delete someday..
   July 9, 2000: JVS: readClif for reading unnamed clif
   Aug 2, 2000:  JVS:  checkIfObjectList in parseClif...was checking next level down, changed to
                       check correct level....
   Aug 30, 2000: JVS: Add getMatchingClifAddress
   Sept 14, 2000: Change NULL to 0 so compiler not complain.
   Jan 17, 2001: JVS: bugfix in  getMatchingClifAddress (need ++iObject, not iObject++)
                      to prevent reading beyond end of array.
   July 16, 2003: JVS: Major modification so will parse lines like DoseGrid.Dimension.X into
                       their corresponding clif levels....Required so can access store objects...
    March 5, 2007: JVS: Change error message in getClifObjectNumber so does not dump core when cannot find an object
    Nov 5, 2007: JVS: make clifErrno local global, add checkClifError
    Nov 27, 2007: JVS: const char * for g++ 4.2
    May 16, 2012: JVS: change several int's to size_t to remove CC compiler warnings for 64 bit
    Sept 24, 2012: JVS: Permit having object names like #0, #1, ...
    Jan 23, 2013: JVS: Permit strings to have [] in them.  Bug before sent them to array code...
    May 5, 2015: JVS: Add bool readStringClifStructure(clifObjectType *clif, const char *messageHandle, std::string& outString)
    May 16, 2017: JVS: Add copyClifStructure(,)
*/
// #define DEBUG
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> // for isspace
#include <string>

#include "utilities.h"
#include "read_clif.h"
int clifErrno=OK;
int checkClifError(){ return ( clifErrno);}
int resetClifError(){ clifErrno = OK; return(clifErrno);}
char myStaticString[MAX_STR_LEN]; // Hack to make count work...
// #define DEBUG
int reportLevel=0; // for outputting debugging comments
/* ******************************************************************** */
int dumpClifStructure(clifObjectType *clif)
{   // dumps the clif structure for debugging... 
    char spaceString[MAX_STR_LEN];
    //printf("\n -----> outputting %s (%d)",clif->Handle, clif->clifLevel);
    for(int i=0; i < clif->clifLevel; i++)
       spaceString[i]=' ';
    spaceString[clif->clifLevel]=0;
   
    // Output the data-- NEED to put quotes on strings!!! NOT DONE YET!!!
    printf("\n%s%s ={",spaceString,clif->Handle);
    for(int iData=0; iData<clif->nDataObjects; iData++)
       printf("\n%s  %s = %s;", spaceString,clif->Data[iData].Handle, clif->Data[iData].Value);
    // Output the Arrays
    for(int iArray=0; iArray<clif->nArrayObjects; iArray++)
    {
       printf("\n%s  %s[] ={",spaceString,clif->Array[iArray].Handle);
       for(int itmp=0; itmp<clif->Array[iArray].nValues; itmp++)
       {
          if((itmp+1)%2)printf("\n%s    ",spaceString);
          printf("%f", clif->Array[iArray].Value[itmp]);
          if(itmp<clif->Array[iArray].nValues-1) printf(",");
       }
       printf("\n%s  };",spaceString);
    }
    // Output the clifObjects
    //printf("\n ------> Outputting the %d clif objects ",clif->nObjects);
    for(int iObjects=0; iObjects<clif->nObjects;iObjects++)
        dumpClifStructure(&clif->Object[iObjects]);
    printf("\n%s};",spaceString);
    return(OK);
}
/* ********************************************************************* */
bool copyClifStructure(clifObjectType *destClif, clifObjectType *sourceClif)
{  // modeled after freeClifStructure (nObjects, nDataObjects, nArrayObjects)
   //printf("\n\n\n -----> Copying clif %s   (%d)", sourceClif->Handle, (int) strlen(sourceClif->Handle));
   destClif->clifLevel = sourceClif->clifLevel;
   destClif->Handle = (char *)calloc(strlen(sourceClif->Handle)+2, sizeof(char));
   destClif->Data = (clifDataType  *)calloc(sourceClif->nDataObjects, sizeof(clifDataType));
   destClif->Array = (clifArrayType *)calloc(sourceClif->nArrayObjects,sizeof(clifArrayType));
   if(NULL == destClif->Handle || NULL == destClif->Data || NULL == destClif->Array){
        printf("\n ERROR: copyClifStructure, memoryAllocation"); return false;
   }
   strcpy(destClif->Handle,sourceClif->Handle);
   destClif->nDataObjects  = sourceClif->nDataObjects;
   destClif->nArrayObjects = sourceClif->nArrayObjects;
   destClif->nObjects      = sourceClif->nObjects;
   
   //printf("\n Copying source %s to dest %s", sourceClif->Handle, destClif->Handle);
   //printf("\n\t sourceClif->clifLevel = %d, destClif->clifLevel=%d",sourceClif->clifLevel, destClif->clifLevel);
   //printf("\n\t nObjects: sourceClif = %d, destClif =%d",sourceClif->nObjects, destClif->nObjects);
   //printf("\n\t nDataObjects: sourceClif = %d, destClif =%d",sourceClif->nDataObjects, destClif->nDataObjects);
   // Copy the Data Values
   //printf("\n\t Copying %d data objects of %s ", sourceClif->nDataObjects, sourceClif->Handle);
   
   for(int iData=0; iData < sourceClif->nDataObjects; iData++){
      destClif->Data[iData].Value  = (char *) calloc(strlen(sourceClif->Data[iData].Value)+2, sizeof(char));
      destClif->Data[iData].Handle = (char *) calloc(strlen(sourceClif->Data[iData].Handle)+2, sizeof(char));
      if(NULL == destClif->Data[iData].Handle || NULL == destClif->Data[iData].Value){
        printf("\n ERROR: copyClifStructure, memoryAllocation"); return false;
      }
      //printf("\n\t\t data: copying %s   (%d)",sourceClif->Data[iData].Handle);
      strcpy(destClif->Data[iData].Handle,sourceClif->Data[iData].Handle);
      strcpy(destClif->Data[iData].Value,sourceClif->Data[iData].Value);
   }
   //printf("\n\t destClif->nDataObjects=%d",destClif->nDataObjects);
   // Copy all of the array objects
   // printf("\n\t Copying array objects of %s ", sourceClif->Handle);
   for(int iArrays=0; iArrays < sourceClif->nArrayObjects; iArrays++){
      destClif->Array[iArrays].Value  = (float *)calloc(sourceClif->Array[iArrays].nValues, sizeof(float));
      destClif->Array[iArrays].Handle = (char *) calloc(strlen(sourceClif->Array[iArrays].Handle)+2, sizeof(char));
      if(NULL == destClif->Array[iArrays].Handle || NULL == destClif->Array[iArrays].Value){
        printf("\n ERROR: copyClifStructure, memoryAllocation"); return false;
      }
      destClif->Array[iArrays].nValues = sourceClif->Array[iArrays].nValues;
      strcpy(destClif->Array[iArrays].Handle,sourceClif->Array[iArrays].Handle);
      //printf("\n\t arrays: copying %s",sourceClif->Array[iArrays].Handle);
      for(int iVal=0; iVal < sourceClif->Array[iArrays].nValues; iVal++)
         destClif->Array[iArrays].Value[iVal] = sourceClif->Array[iArrays].Value[iVal];
   }
   // Copy all of the objects
   //printf("\n Copying sub objects of %s ", sourceClif->Handle);
   destClif->Object = (clifObjectType *)calloc(sourceClif->nObjects, sizeof(clifObjectType));
   for(int iObjects=0; iObjects < sourceClif->nObjects; iObjects++){
      // initialize the data for this new clif object
      destClif->Object[iObjects].clifLevel = destClif->clifLevel+1;
      destClif->Object[iObjects].nObjects = 0;
      destClif->Object[iObjects].nDataObjects = 0;
      destClif->Object[iObjects].nArrayObjects = 0;
      destClif->Object[iObjects].Data  = NULL;
      destClif->Object[iObjects].Array  = NULL;
      destClif->Object[iObjects].Object = NULL;
      if(!copyClifStructure(&destClif->Object[iObjects], &sourceClif->Object[iObjects])){
         printf("\n ERROR: copyClifStructure (1)");
      }
   }
   return true;
}
/* ********************************************************************* */
/* ******************************************************************** */
int checkIfObjectList(clifObjectType *List)
{
   for(int iObj=1; iObj < List->nObjects; iObj++)
   {
     if(strcmp( List->Object[0].Handle, List->Object[iObj].Handle) )
     {
        printf("\n ERROR: checkIfObjectList(clifObjectType *): Expected Object list, object names do not match (%s,%s)",
               List->Object[0].Handle,List->Object[iObj].Handle);
        return(FAIL);
     }
   }
   return(OK);
} 
/* ******************************************************************** */
int checkIfObjectList(clifObjectType *Object, int nObjects)
{
   for(int iObj=1; iObj < nObjects; iObj++)
   {
     if(strcmp( Object[0].Handle, Object[iObj].Handle) )
     {
        printf("\n ERROR: checkIfObjectList(clifObjectType *, int): Expected Object list, object names do not match (%s,%s)",
               Object[0].Handle,Object[iObj].Handle);
        printf("\n\t nObjects = %d", nObjects);
        return(FAIL);
     }
   }
   return(OK);
} 
/* ********************************************************************* */
int parseClifObjectNumber(int nObjects, clifObjectType *Object, char *objectHandle, int *iObject)
{
   /* check to make sure all of the objects in this list have the
      same name (ie, that it is a list of similar objects */
    /*   printf("\n Old Check\n");
   for(int iObj=1; iObj < nObjects; iObj++)
   {
     printf("\n Comparing %s and %s for object list", Object[0].Handle, Object[iObj].Handle);
     if(strcmp( Object[0].Handle, Object[iObj].Handle) )
     {
        printf("\n ERROR: Expected Object list, object names do not match (%s,%s)",
               Object[0].Handle,Object[iObj].Handle);
        return(FAIL);
     }
   }
   printf("\n Old Check Passed, Do New Check\n"); */
   // if(checkIfObjectList(Object) != OK)  // Aug 2, 2000: this was checking next level down
   if(checkIfObjectList(Object, nObjects) != OK)
      return(FAIL);
   
   /* Parse out the object number */
   size_t objectLen=strlen(objectHandle);
   if(objectLen < 5 ||  // make sure has #,",#, at least 1 digit, and "
      objectHandle[1] != '"' ||  // make sure has beginning quote"
      objectHandle[objectLen-1] != '"' || // make sure has ending quote"        
      objectHandle[2] != '#' )   // make sure has # before number
   {
      printf("\n ERROR: # syntax is improper in %s",objectHandle);return(FAIL);
   }
   // parse the value out of the string
   objectHandle[objectLen-1] = 0;
   if(sscanf(objectHandle+3,"%d", iObject) != 1) 
   {
      printf("\n ERROR: Scanning Object Number from %s", objectHandle); return(FAIL);
   }
   if(*iObject < 0 || *iObject >= nObjects)
   {
      printf("\n ERROR: Invalid Object Number Scanned from %s", objectHandle); return(FAIL);
   }
   return(OK);
}
/* ************************************************************************************* */
int getClifObjectNumber(int nObjects, clifObjectType *Object, const char *objectHandle, int *iObject)
{
  /* Given an Object Name, determine the Object Number, return error if not found */
  if( findClifObjectNumber(nObjects, Object, objectHandle, iObject) != OK)
  {
    printf("\n ERROR: getClifObjectNumber: Cannot find objectHandle: ");
    if(NULL==objectHandle ) {
      printf("\t objectHandle == NULL");
    } else {
      printf("\t objectHandle == %s", objectHandle);
    }
    // Note, cannot find the object, thus Object->Handle cannot be found, so not work.
    /*    if(NULL==Object->Handle ) {
      printf("\t Object->Handle == NULL");
    } else {
      printf("\t Object->Handle == %s", Object->Handle);
      } */
    // printf("\n ERROR: getClifObjectNumber: Finding Object %s in %s", objectHandle , Object->Handle); 
    return(FAIL);
  }
  return(OK);
}
/* *************************************************************************************** */
int findClifObjectNumber(int nObjects,  clifObjectType *Object, const char *objectHandle, int *iObject)
{
  /* Given an Object Name, determine the Object Number */
    // printf("\n getClifObjectNumber");
  int iObj = 0;
  while( iObj < nObjects &&                             // Not Exceed number of objects
         strcmp( Object[iObj].Handle, objectHandle) )   // Check for Match of Handle (Name)
  {
      //      printf("\n Compared %s and %s", Object[iObj].Handle, objectHandle);
     iObj++;
  }
  if(iObj >= nObjects)  // Object Not Found, check in "Name" of the data sub-objects
  {
     iObj = 0; 
     int found=0;
     while(iObj < nObjects && !found)
     {
        int isubObjects = 0;
        while( isubObjects < Object[iObj].nDataObjects  &&
              !( strcmp( Object[iObj].Data[isubObjects].Handle,"Name") == 0  &&   
                 strcmp( Object[iObj].Data[isubObjects].Value, objectHandle)==0 ) )
	{
	    /*  printf("\n Compared %s with %s and %s with %s", 
                    Object[iObj].Data[isubObjects].Handle,"Name",
                    Object[iObj].Data[isubObjects].Value, objectHandle); */
           isubObjects++;
        }
        if(isubObjects < Object[iObj].nDataObjects)
        {
           found++;
        }
        else
	{
           iObj++;
        }
     }
     if(!found)
     { 
        return(FAIL);  
     }
   }
   *iObject = iObj;
   //   printf("\n Found Match of %s and %s", Object[iObj].Handle, objectHandle);
   return(OK);
}
/* ******************************************************************** */
int checkForClifObject(clifObjectType *clif, const char *messageHandle)
{   /* returns OK if clifObject with the messageHandle exists, else, returns FAIL
       needed so can use read routines with various versions of Pinnacle */
   size_t nameLen=strlen(messageHandle);
   size_t imsgHandle=0;
   // look for a period
   while( messageHandle[imsgHandle] != '.' && imsgHandle < nameLen) imsgHandle++;
   if(messageHandle[imsgHandle] == '.' ) // period found, now must find this name or number
   {                                     // in the structure list and call recursive 
      char objectHandle[MAX_STR_LEN];
      strncpy(objectHandle, messageHandle,imsgHandle);
      objectHandle[imsgHandle] = 0; // NULL terminate the string
      // check if given a "#" syntax
      int iObjects=0;
      if( objectHandle[0]=='#' && strlen(objectHandle)>2 && objectHandle[1]=='"' )
      {
         if( parseClifObjectNumber(clif->nObjects, clif->Object, objectHandle, &iObjects) != OK )
	 {
            printf("\n ERROR: checkForClifObject for %s", messageHandle);
            printf("\n ERROR: checkForClifObject: Parsing Clif Object Number from %s", objectHandle); return(FAIL);
         }
      }
      else
      {     // check through the objects (when not given in # format)
         if( getClifObjectNumber(clif->nObjects, clif->Object, objectHandle, &iObjects) != OK )
	 {
            printf("\n ERROR: checkForClifObject for %s", messageHandle);
            printf("\n ERROR: Determining Clif Object Number for %s", objectHandle); return(FAIL);
         }
      }
      // if found, read next level of the structure
      if( checkForClifObject( &clif->Object[iObjects], messageHandle+imsgHandle+1)!=OK )
      {
         printf("\n ERROR: checkForClifObject for %s", messageHandle);
	 printf("\n ERROR: checkForClifObject with Message %s", messageHandle+imsgHandle+1); return(FAIL);
      }
   }
   else /* Look for the name in the Objects*/
   {
       // printf("\n Looking for %s in the Objects", messageHandle);
      int iObject=0; /*
      while(  iObject < clif->nObjects && strcmp( clif->Object[iObject].Handle, messageHandle) )
      iObject++; */

      // check if object found, if not, look in list of names
      /* if( iObject >= clif->nObjects && */
       if(  findClifObjectNumber(clif->nObjects, clif->Object, messageHandle, &iObject) != OK )
       {
         return(FAIL);
      }
   }
  
   return(OK);
}
/* ********************************************************************************** */
/* This routine tries to match the "value" for the object "handleToMatch" with the 
   value contained in "valueToMatch"for the clifObject clif, it returns the address of 
   the machingObject...Note: This routine does NOT climb down clif levels currently, thus,
   it is typcially used with getClifAddress to get the address of the parent clifs.
*/ 
int getMatchingClifAddress(clifObjectType *clif, const char *handleToMatch, char *valueToMatch, clifObjectType **matchingObject)
{
#ifdef DEBUG
  printf("\n getMatchingClifAddress: looking for %s %s",handleToMatch, valueToMatch);
  printf("\t nObjects = %d, nDataObjects = %d", clif->nObjects, clif->nDataObjects);
#endif
  int iObject=0;
  char testValue[MAX_STR_LEN];
  do{
     if( readStringClifStructure(&clif->Object[iObject], handleToMatch, testValue) != OK )
     {
        printf("\n ERROR: Reading %s for object %d",handleToMatch,  iObject);return(FAIL);
     }
  }while( strcmp(valueToMatch,testValue) && ++iObject < clif->nObjects );

  if(iObject >= clif->nObjects)
  {
     printf("\n ERROR: Finding Object %s = %s;", handleToMatch, valueToMatch); return(FAIL);
  }
  // printf("\n Found PlanList at Object %d", iObject);
  *matchingObject = &clif->Object[iObject];
  return(OK);
}
/* ******************************************************************** */
int getClifAddress(clifObjectType *clif, const char *messageHandle, clifObjectType **Object)
{
   size_t nameLen=strlen(messageHandle);
   size_t imsgHandle=0;
   // look for a period
   while( messageHandle[imsgHandle] != '.' && imsgHandle < nameLen) imsgHandle++;
   if(messageHandle[imsgHandle] == '.' ) // period found, now must find this name or number
   {                                     // in the structure list and call recursive 
      char objectHandle[MAX_STR_LEN];
      strncpy(objectHandle, messageHandle,imsgHandle);
      objectHandle[imsgHandle] = 0; // NULL terminate the string
      // check if given a "#" syntax
      int iObjects=0;
      if( objectHandle[0]=='#' && strlen(objectHandle)>2 && objectHandle[1]=='"' )
      {
         if( parseClifObjectNumber(clif->nObjects, clif->Object, objectHandle, &iObjects) != OK )
	 {
            printf("\n ERROR: getClifAddress: for %s", messageHandle);
            printf("\n ERROR: getClifAddress: Parsing Clif Object Number from %s", objectHandle); return(FAIL);
         }
      }
      else
      {     // check through the objects (when not given in # format)
         if( getClifObjectNumber(clif->nObjects, clif->Object, objectHandle, &iObjects) != OK )
	 {
            printf("\n ERROR: getClifAddress for %s", messageHandle);
            printf("\n ERROR: Determining Clif Object Number for %s", objectHandle); return(FAIL);
         }
      }
      // if found, read next level of the structure
      if( getClifAddress( &clif->Object[iObjects], messageHandle+imsgHandle+1, Object)!=OK )
      {
         printf("\n ERROR: getClifAddress for %s", messageHandle);
	 printf("\n ERROR: getClifAddress with Message %s", messageHandle+imsgHandle+1); return(FAIL);
      }
   }
   else /* Look for the name in the Objects*/
   {
       // printf("\n Looking for %s in the Objects", messageHandle);
     /* JVS: .... getClifObjectNumber does this.....*/
      int iObject=0;
      /*
      while(  iObject < clif->nObjects && strcmp( clif->Object[iObject].Handle, messageHandle) )
	 iObject++;
     */

      // check if object found, if not, look in list of names
     /*      if( iObject >= clif->nObjects && */
       if(  getClifObjectNumber(clif->nObjects, clif->Object, messageHandle, &iObject) != OK )
       {
         printf("\n ERROR: getClifAddress for %s", messageHandle);
         printf("\n ERROR: Finding Object Named %s", messageHandle); 
         printf("\n\tObject List is");
         for(int iObj=0; iObj<clif->nObjects; iObj++)
            printf("\n\t\t%s", clif->Object[iObj].Handle);
         return(FAIL);
      }
      *Object = &clif->Object[iObject];
   }
   return(OK);
}
/* *********************************************************************************** */
bool readStringClifStructure(clifObjectType *clif, const char *messageHandle, std::string& outString) {
  char tmpString[MAX_STR_LEN];
  if(OK != readStringClifStructure(clif,messageHandle,tmpString)) {
    printf("\nERROR: %s ", __FUNCTION__ ); return false;
  }
  //printf ("\n %s : tmpString = %s", __FUNCTION__, tmpString); 
  outString = tmpString;
  //printf ("\n %s : outString = %s", __FUNCTION__, outString.c_str()); 
  return true;
}
/* *********************************************************************************** */
int readStringClifStructure(clifObjectType *clif, const char *messageHandle, char *Value)
{  /* Reads a pre-allocated string from a clif structure */
#ifdef DEBUG
   printf("\n readStringClifStructure %s", messageHandle);
#endif
   char *tString;
   if(readClifStructure(clif, messageHandle, &tString)!= OK )
   {
      printf("\n ERROR: Reading %s from clif %s", messageHandle, clif->Handle); return(FAIL);
   }
   // printf(" readStringClifStructure:len=(%d):%s = %s\n", strlen(tString),messageHandle, tString );
   strcpy(Value,tString);
   return(OK);
}
int readFloatClifStructure(clifObjectType *clif, const char *messageHandle, float *Value)
{  /* Reads a pre-allocated string from a clif structure */
#ifdef DEBUG
  printf("\n readFloatClifStructure %s", messageHandle);
#endif
   char *tString;
   if(readClifStructure(clif, messageHandle, &tString)!= OK )
   {
      printf("\n ERROR: Reading %s from clif %s", messageHandle, clif->Handle); return(FAIL);
   }
   if(sscanf(tString,"%f",Value) != 1)
   {
      printf("\n ERROR: Reading %s from clif %s, cannot read float from %s", 
             messageHandle, clif->Handle, tString); return(FAIL);
   }
   return(OK);
}
int readIntClifStructure(clifObjectType *clif, const char *messageHandle, int *Value)
{  /* Reads a pre-allocated string from a clif structure */
#ifdef DEBUG
   printf("\n readIntClifStructure %s", messageHandle);
#endif
   char *tString;
   if(readClifStructure(clif, messageHandle, &tString)!= OK )
   {
      printf("\n ERROR: Reading %s from clif %s", messageHandle, clif->Handle); return(FAIL);
   }
   if(sscanf(tString,"%d",Value) != 1)
   {
      printf("\n ERROR: Reading %s from clif %s, cannot read float from %s", 
             messageHandle, clif->Handle, tString); return(FAIL);
   }
   return(OK);
}
/* *********************************************************************************** */
int readClifStructure(clifObjectType *clif, const char *messageHandle, char **Value)
{
   /* if find a . in the object, then must find upper clif level and then
      go to next lower level
      if no . exists, then check in the clif Data objects (clifData and
      ClifArray)

      Also note: Can use by specifying a "Name" or by a #"#0" type syntax
   */
#ifdef DEBUG
   printf("\n readClifStructure reports %d Objects", clif->nObjects);
   printf("\n readClifStructure reports %d DataObjects", clif->nDataObjects);
#endif
   size_t nameLen=strlen(messageHandle);
   size_t imsgHandle=0;
   // look for a period
   while( messageHandle[imsgHandle] != '.' && imsgHandle < nameLen) imsgHandle++;
   if(messageHandle[imsgHandle] == '.' ) // period found, now must find this name or number
   {                                     // in the structure list and call recursive 
#ifdef DEBUG
     printf("\n Found period (.)in message %s", messageHandle);
#endif
      char objectHandle[MAX_STR_LEN];
      strncpy(objectHandle, messageHandle,imsgHandle);
      objectHandle[imsgHandle] = 0; // NULL terminate the string
      // check if given a "#" syntax
      int iObjects=0;
      if( objectHandle[0]=='#' && strlen(objectHandle)>2 && objectHandle[1]=='"')
      {
         if(parseClifObjectNumber(clif->nObjects, clif->Object, objectHandle, &iObjects) != OK)
	 {
            printf("\n ERROR: readClifStructure: Parsing Clif Object Number from %s", objectHandle); 
	    printf("\n\t: strlen(objectHandle) = %d", (int) strlen(objectHandle));
            return(FAIL);
         }
      }
      else
      {     // check through the objects (when not given in # format)
         if( getClifObjectNumber(clif->nObjects, clif->Object, objectHandle, &iObjects) != OK)
	 {
            printf("\n ERROR: Determining Clif Object Number for %s", objectHandle); return(FAIL);
         }
      }
      // if found, read next level of the structure
      if(readClifStructure( &clif->Object[iObjects], messageHandle+imsgHandle+1, Value)!=OK)
      {
	printf("\n ERROR: readClifStructure with Message %s", messageHandle+imsgHandle+1); return(FAIL);
      }
   }
   else /* Look for the name in the Data Objects*/
   {
      if(0==strcmp(messageHandle,"Count")) {
         // printf("\n WARNING: Count message not implemented/tested");
         // char myStaticString[20];
         sprintf(myStaticString,"%d",clif->nObjects);
         *Value = myStaticString;
      } else {
        int iObject=0;
        //      printf("\n nDataObjects = %d (%s)", clif->nDataObjects, messageHandle);
        while(  iObject < clif->nDataObjects && strcmp( clif->Data[iObject].Handle, messageHandle) )
	   iObject++;
        if(iObject >= clif->nDataObjects){
          printf("\n ERROR: Finding Data Object Named %s", messageHandle); return(FAIL);
        }
        // printf("\n Found %s with value %s", clif->Data[iObject].Handle, clif->Data[iObject].Value);
        *Value = clif->Data[iObject].Value;
      }
   }
   return(OK);
}
/* ************************************************************************************* */
int readClifArrayStructure(clifObjectType *clif, const char *messageHandle, int *nArray, float **Array)
{
   /* if find a . in the object, then must find upper clif level and then
      go to next lower level
      if no . exists, then check in the clif Data objects (clifData and
      ClifArray)

      Also note: Can use by specifying a "Name" or by a #"#0" type syntax
   */
   size_t nameLen=strlen(messageHandle);
   size_t imsgHandle=0;
   // look for a period
   while( messageHandle[imsgHandle] != '.' && imsgHandle < nameLen) imsgHandle++;
   if(messageHandle[imsgHandle] == '.' ) // period found, now must find this name or number
   {                                     // in the structure list and call recursive 
      char objectHandle[MAX_STR_LEN];
      strncpy(objectHandle, messageHandle,imsgHandle);
      objectHandle[imsgHandle] = 0; // NULL terminate the string
      // check if given a "#" syntax
      int iObjects=0;
      if( objectHandle[0]=='#' && strlen(objectHandle)>2 && objectHandle[1]=='"' )
      {
         if(parseClifObjectNumber(clif->nObjects, clif->Object, objectHandle, &iObjects) != OK)
         {
            printf("\n ERROR: readClifArrayStructure: Parsing Clif Object Number from %s", objectHandle); return(FAIL);
         }
      }
      else
      {     // check through the objects (when not given in # format)
         if( getClifObjectNumber(clif->nObjects, clif->Object, objectHandle, &iObjects) != OK)
	 {
            printf("\n ERROR: Determining Clif Object Number for %s", objectHandle); return(FAIL);
         }
      }
      // if found, read next level of the structure
      if(readClifArrayStructure( &clif->Object[iObjects], messageHandle+imsgHandle+1, nArray, Array)!=OK)
      {
	printf("\n ERROR: readClifArrayStructure with Message %s", messageHandle+imsgHandle+1); return(FAIL);
      }
   }
   else /* Look for the name in the Array Objects*/
   {
      int iObject=0;
      while(  iObject < clif->nArrayObjects && strcmp( clif->Array[iObject].Handle, messageHandle) )
      {
         printf("\n Array %s not match %s",  clif->Array[iObject].Handle, messageHandle);
	 iObject++;
      }
      if(iObject >= clif->nArrayObjects)
      {
         printf("\n ERROR: Finding Array Object Named %s", messageHandle); return(FAIL);
      }
      *nArray = clif->Array[iObject].nValues;
      /* Copy Array into local memory */
      float *localArray = (float *) calloc(*nArray, sizeof(float));
      if(localArray == NULL) 
      {
         printf("\n ERROR: Allocating ARRAY memory"); return(FAIL);
      }
      for(int iArray=0; iArray<*nArray; iArray++) localArray[iArray]= clif->Array[iObject].Value[iArray];
      *Array  = localArray;
   }
   return(OK);
}
/* ------------------------------------------------------------------------------------------ */
/* *********************************************************************************** */
int getClifNObjects(clifObjectType *clif, const char *myHandle)
{
   /* if find a . in the object, then must find upper clif level and then
      go to next lower level
      if no . exists, then check in the clif Data objects (clifData and
      ClifArray)
      Also note: Can use by specifying a "Name" or by a #"#0" type syntax
    *  returns number of objects found, and -1 if no objects found.
   */
   {
     clifObjectType *myList;
     if(getClifAddress(clif,myHandle,&myList)!= OK){
      printf("\n ERROR: getClifNObjects: Getting Address for %s",myHandle); return(FAIL);
     }
     if(checkIfObjectList(myList) != OK){
       printf("\n ERROR: getClifNObjects: Invalid Object List"); return(FAIL);
     }
     int nObjects = myList->nObjects;
     return(nObjects);
  }
}
/* ********************************************************************* */
int freeClifStructure(clifObjectType *clif)
{  // free's all of the memory allocated by a clif
#ifdef DEBUG_FREE
   printf("\n Freeing Memory for clif %s", clif->Handle);
#endif
   for(int iObjects=0; iObjects < clif->nObjects; iObjects++)
   {
      if(freeClifStructure(&clif->Object[iObjects]) != OK)
      {
         printf("\n ERROR: Freeing memory for clif object");
      }
   }
   free(clif->Object);
   // free all of the arrays
   for(int iArrays=0; iArrays < clif->nArrayObjects; iArrays++)
   {
      if(clif->Array[iArrays].nValues) free(clif->Array[iArrays].Value);
      free(clif->Array[iArrays].Handle);
   }
   free(clif->Array);
   // free all of the Data Values
   for(int iData=0; iData < clif->nDataObjects; iData++)
   {
#ifdef DEBUG_FREE
     printf(" Freeing iData = %d / %d\n", iData, clif->nDataObjects);
     printf(" Handle: %s Data: %s", clif->Data[iData].Handle, clif->Data[iData].Value);
#endif
      free(clif->Data[iData].Value);
      free(clif->Data[iData].Handle);
   }
   free(clif->Data);
#ifdef DEBUG_FREE
   printf("\n Completed Freeing Memory for Clif %s", clif->Handle);
#endif
   free(clif->Handle);
   return(OK);
}
/* ********************************************************************* */
int readClif(FILE *istrm, clifObjectType *clif, const char *clifHandle)
{
   char clifStringHandle[MAX_STR_LEN];
   sprintf(clifStringHandle,"%s ={", clifHandle);
   clif->Handle = (char *)calloc(strlen(clifHandle)+1, sizeof(char));
   if( clif->Handle == NULL){
     printf("\n ERROR: readClif: Allocating Memory for Object Handle"); return(FAIL);
   }
   // copy the string to the name
   strcpy(clif->Handle, clifHandle);

   if(clif_get_to_line(istrm,clifStringHandle)!=OK){
         printf("\n ERROR: readClif: finding %s ", clifStringHandle);
         return(FAIL);
   }
   if(readClifLevel(istrm, clif) != OK){
      printf("\n ERROR: readClif: Reading Clif %s", clifStringHandle);
      return(FAIL);
   }
   return(OK);
}
/* ********************************************************************* */
/* ************************************************************************************ */
int readSpecificClif(FILE *istrm, clifObjectType *clif, const char *clifHandle, const char *clifName)
{ // Use to read a clif of type clifHandle with Name = clifName into the clif Object from
  // file istrm
   char clifStringHandle[MAX_STR_LEN];
   sprintf(clifStringHandle,"%s ={",clifHandle);
#ifdef DEBUG
   printf ("\n readSpecificClif: ");
#endif

   int beamFound=0;
   char currentClifName[MAX_STR_LEN];
   long iFileOffset; // Offset in the file to the beginning of the clif
   do{
      if(clif_get_to_line(istrm,clifStringHandle)!=OK)
      {
         printf("\n ERROR: readSpecificClif: finding %s ", clifStringHandle);
         printf("\n\tlooking for %s", clifName);
         return(FAIL);
      }
      // istrmClifStart = istrm; // Pointer to location in the file where this clif starts
      if( (iFileOffset = ftell(istrm)) == -1L)
      {
         printf("\n ERROR:  readSpecificClif: getting file offset"); return(FAIL);
      }
      // get Name
      if(clif_string_read(istrm,"Name =",currentClifName) != OK)  
      {
         printf("\n ERROR:  readSpecificClif:Reading Clif Name = for %s", clifHandle); return(FAIL); 
      }
      /* check if current name matches */
#ifdef DEBUG
      printf("\n Comparing %s to %s ", currentClifName, clifName);
#endif
      if(strcmp(currentClifName,clifName) == 0) beamFound = 1;
      if(!beamFound)
      {  /* close the Trial cliff */
         if(clif_get_to_line(istrm,"};")!=OK)
         {
            printf("\n ERROR:  readSpecificClif: Closing Trial structure in file");
            return(FAIL);
         }
      }
   }while(!beamFound);
#ifdef DEBUG
   printf("\n Found %s %s in file", clifHandle, clifName);
#endif
   /* Assign the "Name" Object in this clif, and read the rest...*/
   if(fseek(istrm,iFileOffset,SEEK_SET)!=0)
   {
      printf("\n ERROR: seeking start of clif in file"); return(FAIL);
   }
   // Read the Clif in starting at the file pointer location
   if( readClifFile(istrm, clif,clifHandle) != OK)
   {
      printf("\n ERROR: Reading %s from file", clifHandle); return(FAIL);
   }
#ifdef DEBUG   
   printf("\n Completed Reading in Clif from the file");
#endif
   return(OK);
}
/* ******************************************************************************* */
int readClifFile(FILE *istrm, clifObjectType *clif, const char *clifHandle)
{
  if(NULL == istrm) {
    printf("\n ERROR: NULL pointer passed to readClifFile for istrm\n"); return(FAIL);
  }
   clif->Handle = (char *)calloc(strlen(clifHandle)+1, sizeof(char));
   if( clif->Handle == NULL){
      printf("\n ERROR: readClifFile: Allocating Memory for Object Handle"); return(FAIL);
   }
   // copy the string to the name
   strcpy(clif->Handle, clifHandle);
   // printf("\n Reading Clif File with Handle %s", clif->Handle);
   clif->clifLevel = 0;
   if(readClifLevel(istrm, clif) != OK){
      printf("\n ERROR: readClifFile: Reading Clif %s", clifHandle);
      // dumpClifStructure(clif);
      return(FAIL);
   }
   return(OK);
}
/* ******************************************************************* */
int readClifLevel(FILE *istrm, clifObjectType *clif)
{
   // initialize values for new clif
   clif->nObjects         = 0;
   clif->nDataObjects     = 0;
   clif->nArrayObjects    = 0;
   clif->Data  = NULL;
   clif->Array = NULL;
   clif->Object = NULL;
   // read in the Clif Data 
#ifdef DEBUG
   if(clif->Handle != NULL) printf("\n Reading Clif %s at level %d........", clif->Handle, clif->clifLevel);
#endif
  if(NULL == istrm) {
    printf("\n ERROR: NULL pointer passed to readClifLevel for istrm\n"); return(FAIL);
  }
   if(readClifData(istrm, clif) != OK)
   {
      printf("\n ERROR: readClifLevel: Reading Clif\n"); 
      // dumpClifStructure(clif);
      return(FAIL);
   }
   return(OK);
}
/* ************************************************************** */
int readClifArray(FILE *istrm, clifArrayType *Array )
{
   // allocate memory for the data
   // read values till end of clif level
   // parse out data values (save as floats?) using strtok....
   //
   // printf("\n reading Clif Array\n");
   char inString[MAX_STR_LEN];
   const char *delimiterString=",";
   Array->nValues=0;  // initialize number of values
   while( clif_fgets(inString,MAX_STR_LEN,istrm ) ) /* read in the string */
   {
     char *pString; /* pointer to string read in */
     float Value;
     pString = strtok(inString,delimiterString);
     while(pString!=NULL)
     {
        if( sscanf(pString,"%f",&Value) == 1)
	{  // allocate memory
           if(Array->nValues==0) 
              Array->Value = (float *) malloc(sizeof(float));
           else
              Array->Value = (float *) realloc(Array->Value, (Array->nValues+1)*sizeof(float));
           if(Array->Value == NULL)
           {
              printf("\n ERROR: Allocating Array Memory"); return(FAIL);
           }
           // assign value in the Array
           Array->Value[Array->nValues] = Value;
           // increment the number in the array
           Array->nValues++;
        }
        pString = strtok(NULL,delimiterString);
     }
   }
   // printf("\n %d Values read for the Array", Array->nValues);
   return(OK);
}
/* ************************************************************** */
int readClifDataObject(int *nDataObjects, clifDataType **Data, char *string)
{
   clifDataType *localData;
   /* Allocate Memory for the object*/
   if(*nDataObjects == 0)
      localData = (clifDataType *) malloc(sizeof(clifDataType));
   else
      localData = (clifDataType *) realloc((clifDataType *) *Data, 
                                        (*nDataObjects+1)*sizeof(clifDataType));
   if(localData == NULL){
      printf("\n ERROR: Allocating Memory for Clif Data"); return(FAIL);
   }
   *Data = localData; 

   if(reportLevel>3) printf("\n readClifDataObject( , ,%s)",string);
   /* Get the Name of the Clif */
   if(readClifStringName(string,"=", &localData[*nDataObjects].Handle) != OK){
      printf("\n ERROR: Getting String Name"); return(FAIL);
   } 
   /* Get the Value of the Clif */
   if(readClifStringValue(string,"=", &localData[*nDataObjects].Value) != OK){
      printf("\n ERROR: Getting String Value"); return(FAIL);
   }
#ifdef DEBUG_DATA
   printf("\n %d : %s %s", *nDataObjects, localData[*nDataObjects].Handle, localData[*nDataObjects].Value);
#endif
   *nDataObjects=*nDataObjects+1; 

   //   printf("\n Update nDataObjects = %d", *nDataObjects);

   return(OK);
}
/* ***************************************************************************************** */
int processClifData(FILE *istrm, clifObjectType *clif, char *string)
{
  if(reportLevel>3)printf("\nprocessClifData( , ,%s)",string);
     char *strLoc = NULL;
     /* decide what to do with it based upon the types given */
     /* if see a [] ={, then, must make an array (till ends with }; 
                    ={  create a new object type (and recusrively call read_clif_level) (till ends with };
                    = "string";  Then, string (look for " ")
                    = float;     then float (look for . )
                    = int;       Then int
     */
          char *equalLoc = NULL;
     if(   (strLoc=strstr(string, ".")) != NULL &&  // Check for . in name
	   (equalLoc=strstr(string, "=")) != NULL &&  // and check if before =
	   strLoc < equalLoc )               
     {
        // printf("\n Discovered . in string %s that must be further parsed", string);
        // Extract Clif Handle (Get name of the Clif)
        char *tempHandle;
        if(readClifStringName(string,".", &tempHandle) != OK)
        {
           printf("\n ERROR: processClifData: Getting String Name"); return(FAIL);
        } 
        // printf("\n tempHandle = %s", tempHandle);
        // See if already exist
        int iObject = 0;
        // printf("\n Finding clif object number from %d objects", clif->nObjects);
        if(  findClifObjectNumber(clif->nObjects, clif->Object, tempHandle, &iObject) != OK )
	  // if( checkForClifObject(clif, tempHandle) != OK)
	{
           // Create if not already exist
	  // printf("\n Clif Object %s not exist, creating ",tempHandle);
          if(clif->nObjects == 0)
             clif->Object = (clifObjectType *) malloc(sizeof(clifObjectType));
          else
             clif->Object = (clifObjectType *) realloc((clifObjectType *) clif->Object, 
                                           (clif->nObjects+1)*sizeof(clifObjectType));
          if(clif->Object == NULL)
          {
             printf("\n ERROR:  processClifData: Allocating Memory for Clif Object"); return(FAIL);
          }
          iObject = clif->nObjects; // assign iObject to current object
          // get the name of this object
          clif->Object[iObject].Handle = tempHandle;
          // initialize the data for this new clif object
          clif->Object[iObject].clifLevel = clif->clifLevel+1;
          clif->Object[iObject].nObjects = 0;
          clif->Object[iObject].nDataObjects = 0;
          clif->Object[iObject].nArrayObjects = 0;
          clif->Object[iObject].Data  = NULL;
          clif->Object[iObject].Array  = NULL;
          clif->Object[iObject].Object = NULL;
          clif->nObjects++;   // increment number of objects 
          // printf("\t clifLevel is %d", clif->Object[iObject].clifLevel);
	}
        else
	{
          // printf("\n Object %s Already Exists and is object %d",tempHandle, iObject);
          free(tempHandle);  // free the memory used by the name
	}
        // strLoc is the location of the remaining string starting at the .
        strLoc++; // increment past the period
        // printf("\n Remaining String = %s", strLoc);
        if(processClifData(istrm, &clif->Object[iObject], strLoc ) != OK)
        {
           printf("\n ERROR: processClifData  processClifData: ");
           return(FAIL); 
        }
        // Add item  processClifData(istrm
     } else if( strstr(string, "[]") != NULL && strstr(string, "={" ) != NULL )  // Check if is array
     {
       /* allocate memory for a new object */
       if(clif->nArrayObjects == 0)
          clif->Array = (clifArrayType *) malloc(sizeof(clifArrayType));
       else
          clif->Array = (clifArrayType *) realloc((clifArrayType *) clif->Array, 
                                        (clif->nArrayObjects+1)*sizeof(clifArrayType));
       if(clif->Array == NULL)
       {
          printf("\n ERROR:  processClifData: Allocating Memory for Clif Array"); return(FAIL);
       }
       // get the name of this array object
       if(readClifStringName(string,"[]", &clif->Array[clif->nArrayObjects].Handle) != OK)
       {
          printf("\n ERROR:  processClifData: Getting Array Name"); return(FAIL);
       }  
       /* Read In the Array */
       if(readClifArray(istrm, &clif->Array[clif->nArrayObjects] ) != OK)
       {
          printf("\n ERROR:  processClifData: Reading in Array Object"); return(FAIL);
       }
       clif->nArrayObjects++; 
     }
     else if( (strLoc = strstr(string, "={" )) != NULL )  /* Create a new object */
     { 
       /* allocate memory for a new object */
#ifdef DEBUG
       printf("\n Parsing as ={");
       printf("\n nObjects = %d", clif->nObjects);
#endif
       if(clif->nObjects == 0)
          clif->Object = (clifObjectType *) malloc(sizeof(clifObjectType));
       else
          clif->Object = (clifObjectType *) realloc((clifObjectType *) clif->Object, 
                                        (clif->nObjects+1)*sizeof(clifObjectType));
       if(clif->Object == NULL)
       {
          printf("\n ERROR:  processClifData: Allocating Memory for Clif Object"); return(FAIL);
       }
       // get the name of this object
       if(readClifStringName(string,"={", &clif->Object[clif->nObjects].Handle) != OK)
       {
          printf("\n ERROR:  processClifData: Getting Clif Name"); return(FAIL);
       }
       // assign the level of the clif
       clif->Object[clif->nObjects].clifLevel = clif->clifLevel+1;
       // Read the Object 
       if(readClifLevel(istrm, &clif->Object[clif->nObjects]) != OK)
       {
          printf("\n ERROR:  processClifData: readClifLevel");
          return(FAIL); 
       }
       clif->nObjects++;   // increment number of objects 
     }  
     else if ( ( strLoc=strstr(string, "=" ) ) != NULL )
    {  /* Read in a VALUE Object */
#ifdef DEBUG
      printf("\n Parsing as =");
#endif
       if( readClifDataObject( &clif->nDataObjects, &clif->Data, string) != OK){
          printf("\n ERROR:  processClifData: Reading Clif Data Object from %s", string); return(FAIL);
       }
     }
     //     else if(strlen(string) == 0){ return(OK); } // return if OK if blank line found
     else
     { 
        printf("\n ERROR:  processClifData: readClifData: %s is invalid clif_string", string);
        printf("\n \t or programmer must learn to handle this situation\n");
        return(FAIL);
     }
     return(OK);
}
/* ***************************************************************************************** */
int readClifData(FILE *istrm, clifObjectType *clif)
{
  /* Reads Data  within a CLIF    */
  /* Called when sees a Name ={ */
  /* Returns when sees a };     */
  char string[MAX_STR_LEN];

  if(NULL == istrm) {
    printf("\n ERROR: NULL pointer passed to readClifData for istrm\n"); return(FAIL);
  }
  while( clifFgets(string,MAX_STR_LEN,istrm ) != NULL ) /* read in the string */
  {
#ifdef DEBUG
     printf("\n readClifData: %s", string);
#endif
    if( processClifData(istrm, clif, string) == FAIL) {
      printf("\n ERROR: readClifData: Processing string %s", string);
       return(FAIL);
   }
  }
  /* return on end of context */
  if(clifErrno == END_OF_CONTEXT)
  {
     clifErrno = OK; /* reset errno */ 
     return(OK); 
  }
  // check for end of file and outside of all clifLevels
  if(clifErrno == END_OF_FILE && clif->clifLevel == 0 )
  {
     clifErrno = OK; return(OK);
  }
  return(FAIL);
}
/* *********************************************************************************** */ 
int readClifStringName(char *inString, const char *delimiter, char **outString)
{
  // printf("\n Getting clif name from %s", inString);
  char *strLoc = strstr(inString, delimiter);  // get location of delimeter
  /* Remove Space from the Name */
  int iStringStart=0;  
  while( isspace( inString[iStringStart] ) ) iStringStart++;
  size_t iStringEnd=strLoc-inString;

  while( isspace( inString[iStringEnd-1] ) )   iStringEnd--;
  size_t Length=iStringEnd-iStringStart; // length of final string
  // allocate memory for the string
  char *nameString = (char *)calloc(Length+1, sizeof(char));
  if( nameString == NULL)
  {
     printf("\n ERROR: getClifName: Allocating Memory for Object Name"); return(FAIL);
  }
  // copy the string to the name
  strncpy(nameString, inString+iStringStart, Length);
  // printf("\n Name is (%d) %s\n", strlen(nameString), nameString);
  // printf("\t -%c-  +%c+", nameString[0],nameString[Length-1]);
  *outString = nameString; // assign pointer to name
  return(OK);
}
/* *********************************************************************************** */ 
int readClifStringValue(char *inString, const char *delimiter, char **outString)
{
  if(reportLevel>3) printf("\n Getting clif Value from %s", inString);
  char *strLoc = strstr(inString, delimiter);  // get location of delimeter
  // determine where the string starts
  size_t iStringStart=strLoc-inString+strlen(delimiter);  
  if(iStringStart >= strlen(inString) ){
     printf("\n ERROR: Getting Value from string %s", inString); return(FAIL);
  }
  // remove space from the string
  while( isspace( inString[iStringStart] ) ) iStringStart++;
  size_t iStringEnd=strlen(inString);
  while( isspace( inString[iStringEnd] ) )   iStringEnd--;
  // ensure string ends in semicolon
  if( inString[iStringEnd-1] != ';' ){
     printf("\n ERROR: readClifStringValue: Getting value from string %s", inString);
     printf("\n\t String %s does not end in ';'", inString); return(FAIL);
  }
  iStringEnd--;  // remove the semicolon
  // check to see if it is in quotes
  if( inString[iStringEnd-1] == '"'  && inString[iStringStart] == '"' ){
    // printf("\n Removing Quotes from %s", inString);
    iStringEnd--; // remove the closing quote
    iStringStart++; // remove the opening quote
  }
  
  //size_t 
  int Length=(int) (iStringEnd-iStringStart); // length of final string
  // check the string length
  if((int) Length < 0 || Length >  (int) strlen(inString) ){
     printf("\n ERROR: Invalid string length for string %s", inString); return(FAIL);
  }
  // allocate memory for the string
  char *nameString = (char *)calloc(Length+1, sizeof(char));
  if( nameString == NULL){
     printf("\n ERROR: getClifName: Allocating Memory for Object Name"); return(FAIL);
  }
  // copy the string to the name
  strncpy(nameString, inString+iStringStart, Length);
  /*
     for(unsigned iStr=0;iStr < Length; iStr++) {
    if(iscntrl(nameString[iStr]) ) {
      char myX='X';
      printf("\n Control character found in string %s, replacing with an X",inString+iStringStart);
      //      nameString[iStr]=myX;
    } 
  }
  */
  nameString[Length]=0; // Null terminate the string
  if(reportLevel>3) printf("\t Name is %s\n", nameString);
  *outString = nameString; // assign pointer to name
  return(OK);
}
/* *********************************************************************************** */ 
