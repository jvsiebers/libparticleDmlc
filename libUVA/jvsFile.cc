/** \file 
 simple tag auto added to force generation of simple doxygen comments
**/
/*
   Copyright 2008 Virginia Commonwealth University


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
/* 
   MOdification History: Feb 7, 2007: JVS: Created
                         May 30, 2008: JVS: Convoluted way of creating fullFileName so compile warning not complain about aggregrate value
                         april 29, 2011: JVS: Add stdio.h and stdlib.h includes so will compile with g++4.52
                         July 20, 2012: JVS: add + "/" to openConfigFile in case path does not end in a "/"
*/
#include <iostream>
#include <string>
#include <stdio.h> // for fopen
#include <stdlib.h> // for getenv
#include <string.h> // for strlen
using namespace std;
/* **************************************************************** */
FILE *openConfigFile(string configFileName, string mode){
  bool verbose=false;
  string fullFileName;
  // Try current working directory first
  fullFileName = configFileName;
  FILE *strm = fopen(fullFileName.c_str(), mode.c_str());
  if(NULL == strm) { //< not found, look in UVA_CONFIG
    if(verbose) cout << "\t" << fullFileName.c_str() << " not found " << endl; 
    char *cpath = getenv("UVA_CONFIG"); // search the path for the file
    if(NULL == cpath ) {
      cout << "\n ERROR: openConfigFile: UVA_CONFIG not defined\n"; return(NULL);
    }
    if( !strlen(cpath) ) {
      cout << "\n ERROR: openConfigFile: UVA_CONFIG is blank string\n"; return(NULL);
    }
    if(verbose) printf("\n UVA_CONFIG = %s",cpath);
    // look directly in UVA_CONFIG
    string path=cpath;
    fullFileName = path + "/";
    fullFileName +=configFileName;
    strm = fopen(fullFileName.c_str(),mode.c_str());
    if(NULL==strm){
      if(verbose) cout << "\t" << fullFileName.c_str() << " not found " << endl; 
      // look in UVA_CONFIG/general
      string morePath="/general/";
      fullFileName = path;fullFileName+=morePath;fullFileName+=configFileName;
      strm = fopen(fullFileName.c_str(),mode.c_str());
      if(NULL == strm){
	if(verbose) cout << "\t" << fullFileName.c_str() << " not found " << endl; 
	// look in UVA_CONFIG/../general
	morePath="/../general/";
	fullFileName = path;fullFileName+=morePath;fullFileName+=configFileName;
	strm = fopen(fullFileName.c_str(),mode.c_str());
	if(NULL == strm){
	  cout << "\nERROR: openConfigFile: Cannot find file " << configFileName;
	  cout << "\n Searched: localDirectory";
	  cout << "\n\tcpath = " << cpath;
	  cout << "\n\tpath0 = " << path;
	  cout << "\n\tpath1 = " << path << "/general/";
	  cout << "\n\tpath2 = " << path << "/../general/";
	  return(NULL);
	}
      }
    }
  }
  if (verbose) cout << "\n\topenConfigFile(" << fullFileName << "," << mode << ")   ";
  return(strm);
}
//
