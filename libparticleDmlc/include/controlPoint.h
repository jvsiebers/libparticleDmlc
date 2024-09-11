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
/* controlPoint.h

   Header file for routines for handing Arc and other dynamic deliveries

Modification History:
   December 15, 2014: JVS: Created: Adding in dynamicDelivery Class
   May 28, 2015: JVS: Add getTotalFractionalWeight
   Oct 24, 2022: JVS: Add jaws
*/

#ifndef _controlPoint_h 
#define _controlPoint_h

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
// #include "particleDmlcClass.h"
// #include "coordinateTransform.h"

using namespace std;
//
class controlPoint{
 public:
  // constructors
  controlPoint(){
    Index=0; Weight=0.0f;FractionalMUs=0.0f;
    // initialize jaws to be > 40x40 field so if used, does nothing
    LeftJawPosition = 25.0f;
    RightJawPosition = LeftJawPosition;
    TopJawPosition   = LeftJawPosition;
    RightJawPosition = LeftJawPosition;
  }
  controlPoint(char *iString){
    setControlPoint(iString);
  }
  ~controlPoint(){ };
  bool  setControlPoint(char *iString);
  float getWeight() { return(Weight); };
  float getFractionalMUs() { return(FractionalMUs); };
  float getPercentOfArc() { return(PercentOfArc); };
  float getDoseRate() { return(DoseRate); };
  float getDeliveryTime() { return(DeliveryTime); };
  float getGantryAngle() { return(GantryAngle); };
  float getCouchAngle() { return(CouchAngle); };
  float getCollimatorAngle() { return(CollimatorAngle); };
  float getLeftJawPosition()   { return(LeftJawPosition); };
  float getRightJawPosition()  { return(RightJawPosition); };
  float getTopJawPosition()    { return(TopJawPosition); };
  float getBottomJawPosition() { return(BottomJawPosition); };
  int   getIndex() { return(Index); };
  //void  setIntegralWeight(float value) { IntegralWeight = value; };
  //float getIntegralWeight() { return(IntegralWeight); };
 protected:
 private:
  int   Index;
  float Weight;
  float FractionalMUs;
  float PercentOfArc;
  float DoseRate;
  float DeliveryTime;
  float GantryAngle;
  float CouchAngle;
  float CollimatorAngle;
  // Include the jaw settings
  float LeftJawPosition;   // x1
  float RightJawPosition;  // x2
  float TopJawPosition;    // y2
  float BottomJawPosition; // y1
};
//
//
#endif
