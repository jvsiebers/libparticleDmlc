/** \file
 
**/
/* Modification History:
     Dec 22, 2014: JVS: Created based on vcuCoordinateTransform used for vmc++ source model
*/
#ifndef _coordinateTransform_h 
#define _coordinateTransform_h

/* *********************************************************************** */
// system libraries
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
// #include <vector>
// local headers
#include "typedefs.h"
#include "read_write_egs.h"

using namespace std;

/* ********************************************************** */
// Define the class and the set of functions that are a part of that class
class  coordinateTransform{
 public: // public functions are known to the outside world
  // Destructor
  ~coordinateTransform();
  // Constructor
  coordinateTransform(float inputGantryAngle, float inputCouchAngle, 
		      float inputCollimatorAngle, float inputSourceToAxisDistance, 
		      Point_3d_float *isocenterCoordinate);
  // coordinateTransform(float gantryAngle, float couchAngle, float collimatorAngle, vector<Float> isocenterCoordinates);
  // Public Functions
  /* ********************************************************************************** */
  void setCouchAngle(float newAngle){
    couchAngle = newAngle;
    createAcceleratorToPatientMatrix(); // update
  };
  void setCollimatorAngle(float newAngle){
    collimatorAngle = newAngle;
    createAcceleratorToPatientMatrix(); // update
  };
  void setGantryAngle(float newAngle){
    gantryAngle = newAngle;
    createAcceleratorToPatientMatrix(); // update
  };
  void reportAccelToPatientMatrix();
  int translateParticleToAcceleratorCoordinateSystem(particle_type *particle);
  int translateParticleToPatientCoordinateSystem(particle_type *particle); 
  void updateAcceleratorToPatientMatrix(){createAcceleratorToPatientMatrix();};
  //  int invert3x3Matrix(float *matrix);
 private:
   Point_3d_float isocenter; // float xIso, yIso, zIso;
   float pi;
   float degreesToRadians;
   float sourceToAxisDistance;
   float gantryAngle, couchAngle, collimatorAngle; 
   // Rotation matrix
   float accelToPatient[3][3]; ///< 3x3 rotation matrix that rotates particle from accelerator coordinate system to patient
   float patientToAccel[3][3]; ///< 3x3 rotation matrix that rotates particle from patient coordinate system to accelerator
   void createAcceleratorToPatientMatrix();
   void createPatientToAcceleratorMatrix();
};
  
#endif
