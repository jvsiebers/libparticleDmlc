/*
   Copyright 2015 University of Virginia


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
/* dynamicDelivery.h

   Header file for routines for handing Arc and other dynamic deliveries

Modification History:
   December 15, 2014: JVS: Created: Adding in dynamicDelivery Class
*/

#ifndef _dynamicDelivery_h 
#define _dynamicDelivery_h

/* *********************************************************************** */
// system libraries
#include <stdio.h>
#include <stdlib.h>

// other libraries
#include "typedefs.h"
#include "utilities.h"
#include "string_util.h"
#include "common.h"
#include "table.h"
//#include "read_write_egs.h"
#include "read_clif.h"
#include "defined_values.h"
//#include "compensator.h"
//#include "modifiers.h"
//#include "readPinnacle.h"
#include "myrandom.h"
#include "typesClass.h" //< contains definitions of keepType, mlcType, dmlc_type, ... things that used to be in dmlc.h --
#include "particleDmlcClass.h"
#include "coordinateTransform.h"
#include "controlPoint.h"

using namespace std;
//
class dynamicDelivery: public particleDmlc {
 public:
  dynamicDelivery();
  dynamicDelivery(const char *dmlFileName, const char *controlPointInfoFile);
  dynamicDelivery(const char *controlPointInfoFile);
  ~dynamicDelivery(){
    // delete[] CP;
    // nCP=0;
  }
  // dynamicDelivery(int *argc, char *argv[]);
  //protected:
 private:
  //  string fileName;

};
//
#endif
