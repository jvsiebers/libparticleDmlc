/** \file 
 simple tag auto added to force generation of simple doxygen comments
**/
/*
   Copyright 2014: University of Virginia


   Adviso
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
/* dynamicDelivery.cc

  for dynamicArcs (Arc) and other such dynamic deliveries

Modification History:
   Dec 15, 2014: JVS: Created.  Put in dynamicDelivery class
   May 15, 2015: JVS: Update so weights are properly assigned in terms of cumulative weight
   Oct 24, 2022: JVS: Add in reading of jaw positions
*/
/* *********************************************************************** */
/* *********************************************************************** */
// system libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>   // for fabs 
#include <iostream> // for cout
// other libraries

#include "utilities.h"
#include "option.h"
#include "string_util.h"
#include "common.h"
#include "spline_data.h"
#include "read_write_egs.h"
#include "myrandom.h"
//local libraries
#ifdef C_CLASS
#include "particleDmlcClass.h"
#else

#include "particleDmlc.h"

#endif

#include "dynamicDelivery.h"
//
/* ********************************************************************** */
#ifdef C_CLASS
using namespace std;
/* ********************************************************************** */
dynamicDelivery::dynamicDelivery() {
  cout << " dynamicDelivery constructor -- default is empty " << endl;
  setnCP(0);
}
/* ********************************************************************** */
dynamicDelivery::dynamicDelivery(const char *controlPointInfoFile){
  if(!readControlPointInfoFile(controlPointInfoFile)){
    cout << " ERROR:"<<__FUNCTION__ << "::dynamicDelivery(fileName)" << endl;
    throw "ERROR: dynamicDelivery constructor";
  }
}
/* ********************************************************************** */
dynamicDelivery::dynamicDelivery(const char *mydmlFileName, const char *mycontrolPointInfoFile){
  // cout << "\tReading control point info file " << endl;
  if(!readControlPointInfoFile(mycontrolPointInfoFile)){
    cout << " ERROR:"<<__FUNCTION__ << "::dynamicDelivery(,)" << endl;
    throw "ERROR: dynamicDelivery(,) constructor";
  }
  //cout << "\t Initializing particleDmlc " << endl;
  // InitializeParticleDmlcPointersToNull();
  if(!initializeParticleDmlc(getIsArc(),mydmlFileName)){
    cout << " ERROR:"<<__FUNCTION__ << " dynamicDelivery(,) : initializeParticleDmlc " << endl;
    throw "ERROR: dynamicDelivery(,) constructor";
  }
  // cout << " getIsCPInfoSet() = " << getIsCPInfoSet() << endl;
  if( !getIsCPInfoSet() && !setFractionalMUs()) {
    cout << " ERROR:"<<__FUNCTION__ << " dynamicDelivery(,) : setFractionalMUs " << endl;
    throw "ERROR: dynamicDelivery(,) constructor";
  }
  if(!initializeCoordinateTransform()){
    cout << " ERROR:"<<__FUNCTION__ << " initializing coordinate transform " << endl;
    throw " ERROR: dynamicDelivery(,) constructor";
  }
}
/* ********************************************************************** */
bool particleDmlc::setFractionalMUs() {
  float fractionalMUs=0.0f;

  if( nCP && 0.0f != CP[nCP-1].getWeight() ) {
    cout << "\n ERROR:"<<__FUNCTION__<<" last weight !=0, so cannot set weights " << endl; return(false);
  }
  // accumulate and set fractionalMUs for each control point
  for(int iCP=0; iCP<nCP; iCP++){
    setDmlcFractionalMU(iCP, fractionalMUs); // set first since first CP corresponds with fractionalMU=0
    fractionalMUs+=CP[iCP].getWeight();
  }
  // make sure fractionaMUs are within tolerance
  float weightTolerance=0.005f; // 0.5% seems good
  if(fabs(fractionalMUs - 1.0f) > weightTolerance) {
    cout << "\n ERROR:"<<__FUNCTION__ << " setFractionalMUs : 1.00f != totalWeight = " << fractionalMUs << endl; 
    for(int iCP=0; iCP<nCP; iCP++)
      cout << " CP[" << iCP << "].weight =" << CP[iCP].getWeight() << endl; 
    return(false);
  }

  return(true);
}
/* ********************************************************************** */
bool particleDmlc::readControlPointInfoFile(const char *tmpcontrolPointInfoFile){
  controlPointFileName=tmpcontrolPointInfoFile;
  return(readControlPointInfoFile());
}
/* ********************************************************************** */
bool particleDmlc::readControlPointInfoFile(){
  cout << "\tReading dynamic delivery info from " << controlPointFileName << endl;
  // Open the file
  FILE *fp;
  if( NULL==(fp = fopen(controlPointFileName.c_str(),"r") ) ){
    cout << " ERROR:"<<__FUNCTION__ << " readControlPointInfoFile: can't open file " << controlPointFileName << endl; return(false);
  }
  // Get the number of control points
  {
    char tmpString[MAX_STR_LEN];
    if (  (OK != string_after(fp,"nControlPoints =",tmpString))
       || ( 1 != sscanf(tmpString,"%d",&nCP)) ){
    cout << " \n ERROR:"<<__FUNCTION__ << " reading nControlPoints from " << controlPointFileName << endl; return(false);
    }
  }
  // Allocate memory for control points    
  CP = new controlPoint[nCP];
  // loop over lines to get the data
  char  iString[MAX_STR_LEN];
  for(int iCP=0; iCP<nCP; iCP++){
    if( NULL == fgets(iString, MAX_STR_LEN, fp) ) {
      cout << " ERROR:"<<__FUNCTION__ << " reading string from controlPointInfoFile" << endl; 
      return(false);
    }
    if(!CP[iCP].setControlPoint(iString)){
      cout << " ERROR:"<<__FUNCTION__ << " translating iString to controlPoint" << endl;  return(false);
    }
    if(CP[iCP].getIndex() != iCP){
      cout << " ERROR:"<<__FUNCTION__ << " indicies are out of order"<<endl; return(false);
    }
  }
  fclose(fp);
  return(true);
}
/* ********************************************************************** */
void particleDmlc::checkIfArc() {
  // find out if it is Arc or not
  bool isBeamAnArc=false;
  for(int iCP=0;iCP<nCP;iCP++){
    if(CP[0].getGantryAngle() != CP[iCP].getGantryAngle()){ isBeamAnArc=true; break;}
  }
  //
  setIsArc(isBeamAnArc);
  return;
}
/* ********************************************************************** */
void particleDmlc::checkIfJawTracking() {
  // find out if Jaw tracking is used or not
//   cout << "===> checkIfJawTracking :";
  bool isBeamJawTracking=false;
  for(int iCP=0;iCP<nCP;iCP++){
    cout << CP[0].getLeftJawPosition() << " " << CP[iCP].getLeftJawPosition() << endl;
    if(CP[0].getLeftJawPosition()   != CP[iCP].getLeftJawPosition() ||
       CP[0].getRightJawPosition()  != CP[iCP].getRightJawPosition() ||
       CP[0].getTopJawPosition()    != CP[iCP].getTopJawPosition() ||
       CP[0].getBottomJawPosition() != CP[iCP].getBottomJawPosition() ){
        isBeamJawTracking=true;
        cout << "\tJaw positions vary with CP, enabling Jaw Tracking" << endl;
        break;
    }
  }
  //
  setIsJawTracking(isBeamJawTracking);
  // cout << getIsJawTracking() << endl; 
  return;
}
/* ********************************************************************** */
bool particleDmlc::validateControlPoints() { // Ensures CP's are valid
  
  if(nCP < 0) {
    cout << " ERROR:"<<__FUNCTION__ << " 0 > nCP = " << nCP << endl; return(false);
  }
  if(0==nCP) { 
    cout << " WARNING: nCP = 0 \t This is valid, but probably not what you want" << endl;
    return(true); 
  }
  float sumOfWeights=0.0f;
  for(int iCP=0; iCP<nCP; iCP++) {
    float currentWeight = CP[iCP].getWeight();
    sumOfWeights+=currentWeight;
    if( currentWeight < 0.0f) {
      cout << "ERROR:"<<__FUNCTION__ << " CP=" << iCP << " Weight = " << CP[iCP].getWeight() << endl; return(false);
    }
    if(iCP && CP[iCP].getFractionalMUs() <  CP[iCP-1].getFractionalMUs()) {
      cout << "ERROR:"<<__FUNCTION__ << " CP[" << iCP << "].fractionalMUs ="<<  CP[iCP].getFractionalMUs() << " < CP[" << iCP-1 << "].fractionalMUs =" << CP[iCP-1].getFractionalMUs() << endl; return(false);
    }
    if(CP[iCP].getPercentOfArc() < 0.0f) {
      cout << "ERROR:"<<__FUNCTION__ << " CP=" << iCP << " PercentOfArc = " << CP[iCP].getPercentOfArc() << endl; return(false);
    }
    if(CP[iCP].getDoseRate() < 0.0f) {
      cout << "ERROR:"<<__FUNCTION__ << " CP=" << iCP << " DoseRate = " << CP[iCP].getDoseRate() << endl; return(false);
    }
    if(CP[iCP].getDeliveryTime() < 0.0f) {
      cout << "ERROR:"<<__FUNCTION__ << " CP=" << iCP << " DeliveryTime = " << CP[iCP].getDeliveryTime() << endl; return(false);
    }
    if( CP[iCP].getGantryAngle() < 0.0f || CP[iCP].getGantryAngle() > 360.0f ) {
      cout << "ERROR:"<<__FUNCTION__ << " CP=" << iCP << " GantryAngle = " << CP[iCP].getGantryAngle() << endl; return(false);
    }
    if(CP[iCP].getCouchAngle() != CP[0].getCouchAngle() ) {
      cout << "ERROR:"<<__FUNCTION__ << " CP=" << iCP << " CouchAngle changed..." << CP[iCP].getCouchAngle() << endl; return(false);
    }
    if(CP[iCP].getCollimatorAngle() != CP[0].getCollimatorAngle() ) {
      cout << "ERROR:"<<__FUNCTION__ << " CP=" << iCP << " CollimatorAngle changed..." << CP[iCP].getCollimatorAngle() << endl; return(false);
    }
    // Jaw checks are not yet done
  }
  float weightTolerance=0.005f; // 0.5% seems good
  if(fabs(sumOfWeights - 1.00f) > weightTolerance) {
    cout << " ERROR:"<<__FUNCTION__ << " 1.00f != totalWeight = " << sumOfWeights << endl; return(false);
  }
  if(nCP && (fabs(CP[nCP-1].getFractionalMUs()-1.00f) > weightTolerance)) {
    cout << " ERROR:"<<__FUNCTION__ << " 1.00f != CP[last].fractionaMUs " << endl; return(false);
  }
  cout << " validateControlPoints finds all CP's valid, sumOfWeights = " << sumOfWeights << endl;
  //cout << "\t CP[last].getFractionalMUs = " << CP[nCP-1].getFractionalMUs() << endl;
  return(true);
}
/* ********************************************************************** */
bool particleDmlc::validateDMLArc(){
  // check redundant information to make sure files are consistent
  // make sure have same number of shapes
  if(nCP == numApertures() ){
    for(int iCP=0;iCP<nCP;iCP++){
      if(CP[iCP].getGantryAngle() != getDmlcIndex(iCP) ) {
    cout << " ERROR:"<<__FUNCTION__ << ": (" <<CP[iCP].getGantryAngle() << " = gantryAngle) != index = " << getDmlcIndex(iCP) << endl; return(false);
      }
      if(CP[iCP].getCollimatorAngle() != getDmlcCollimatorAngle(iCP) ) {
    cout << " ERROR:"<<__FUNCTION__ << ": (" <<CP[iCP].getCollimatorAngle() << " = collimatorAngle) != index = " << getDmlcCollimatorAngle(iCP) << endl; return(false);

      }
    }
  } else if(2*nCP == numApertures() ){
    cout << "\t This will be computed as a segmented step-and-shoot field " << endl;
  } else {
    cout << " ERROR:"<<__FUNCTION__ << ": (nCP = " << nCP << ") != (" << numApertures() <<" = numApertures)" << endl;return(false);
  }
  return(true);
}
/* ********************************************************************** */
bool particleDmlc::initializeCoordinateTransform(){
  float sad=100.0f;
  Point_3d_float isoCoordinate;
  isoCoordinate.x= 0.0;
  isoCoordinate.y= 0.0;
  isoCoordinate.z= 0.0;

  if(nCP < 1) {
    cout << " ERROR:"<<__FUNCTION__ << " nCP= " << nCP << " : cannot initialize transformation " << endl; return(false);
  }

  try{
    transform = new coordinateTransform(CP[0].getGantryAngle(),
                        CP[0].getCouchAngle(),
                    CP[0].getCollimatorAngle(),
                    sad,
                    &isoCoordinate);
  }
  catch(...){
    cout << " ERROR:"<<__FUNCTION__ << " creating coordinate transform " << endl; return(false);
  }
  return(true);
}
/* ********************************************************************** */
bool controlPoint::setControlPoint(char *iString){ // converts iString into control point.  Returns index of the control point, -1 if in error
  if(0==strncmp("iCP=",iString,4)){
      if(NULL != strstr(iString, "y2=")){ // read in jaw settings
          // cout << "----reading jaws ---" << endl;
          if(10!=sscanf(iString,"iCP=%d w=%f fMU=%f gantry=%f collimator=%f couch=%f jaw_x1=%f jaw_x2=%f jaw_y1=%f jaw_y2=%f",
             &Index,&Weight,&FractionalMUs,&GantryAngle,&CollimatorAngle,&CouchAngle,
             &LeftJawPosition, &RightJawPosition, &BottomJawPosition, &TopJawPosition)){
            cout << " ERROR:"<<__FUNCTION__ << " reading control point from " << iString << endl; return(false);
          }
      } else {
          if(6!=sscanf(iString,"iCP=%d w=%f fMU=%f gantry=%f collimator=%f couch=%f",
             &Index,&Weight,&FractionalMUs,&GantryAngle,&CollimatorAngle,&CouchAngle)){
            cout << " ERROR:"<<__FUNCTION__ << " reading control point from " << iString << endl; return(false);
          }
      }
  } else {
    if(8!=sscanf(iString,"%d%f%f%f%f%f%f%f",
         &Index,&Weight,&PercentOfArc,&DoseRate,&DeliveryTime,&GantryAngle,&CouchAngle,&CollimatorAngle)){
      cout << " ERROR:"<<__FUNCTION__ << " reading control point from " << iString << endl; return(false);
    }
  }
  return(true);
}
/* ********************************************************************** */
#endif // C_CLASS 
/* ************************************************************************** */

