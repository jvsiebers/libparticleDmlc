/// \file
/* \author J V Siebers
   coordinateTransform: A class to tranform particles from accelerator-to-patient 
   coordinate systems and back.  

   If couchAngle = 0 and isocenterCoordinate = (0, 0, 0) then will transform 
   from accelerator-to-room coordinate system and back
   
 ***************************************************************************/
/* ***************************************************************************
   Copyright 2014: JVS at the University of Virginia

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
****************************************************************************** */
/* Modification History
     Dec 22, 2014: JVS: Created based on vcuCoordinateTransform from my vmc++ source model routines
*/
/* ********************************************************************************************************** */
// #define DEBUG
//#define DEBUG_TRANSFORM
#define NDEBUG 3
#include <stdio.h>
#include <string.h> // for memcpy (needed for cygwin compile)
#include <math.h>   // for sin,cos,acos,...

#include "coordinateTransform.h"
/* ************************************************ */
/* ****************************************************** */
//void coordinateTransform::invert3x3Matrix(float *matrix) {
void invert3x3Matrix(float *matrix) {
  // Inverts a 3x3 matrix
  // load matrix elements into "a"
  float a[3][3];
  int iMatrix=0;
  for(int iRow=0; iRow<3; iRow++) {
    for(int iCol=0; iCol<3;iCol++) {
      a[iRow][iCol]=matrix[iMatrix++];
    }
  }

  float aInverted[3][3]; // memory storage for inverted matrix

  float determinant = a[0][0]*a[1][1]*a[2][2] + a[0][1]*a[1][2]*a[2][0] +
	              a[0][2]*a[1][0]*a[2][1] - a[2][0]*a[1][1]*a[0][2] - 
	              a[2][1]*a[1][2]*a[0][0] - a[2][2]*a[1][0]*a[0][1];
	
	aInverted[0][0] = (a[1][1]*a[2][2] - a[1][2]*a[2][1])/determinant;
	aInverted[0][1] = (a[0][2]*a[2][1] - a[0][1]*a[2][2])/determinant;
	aInverted[0][2] = (a[0][1]*a[1][2] - a[0][2]*a[1][1])/determinant;
	
	aInverted[1][0] = (a[1][2]*a[2][0] - a[1][0]*a[2][2])/determinant;
	aInverted[1][1] = (a[0][0]*a[2][2] - a[0][2]*a[2][0])/determinant;
	aInverted[1][2] = (a[0][2]*a[1][0] - a[0][0]*a[1][2])/determinant;
	
	aInverted[2][0] = (a[1][0]*a[2][1] - a[1][1]*a[2][0])/determinant;
	aInverted[2][1] = (a[0][1]*a[2][0] - a[0][0]*a[2][1])/determinant;
	aInverted[2][2] = (a[0][0]*a[1][1] - a[0][1]*a[1][0])/determinant;
        // copy matrix "aInverted" back to matrix "a"
        memcpy(matrix,aInverted,9*sizeof(float));
}
/* ********************************************************* */
  // Destructor
coordinateTransform::~coordinateTransform() {
  printf("\n~coordinateTransform()\n");
}
/* ********************************************************* */
coordinateTransform::coordinateTransform(float inputGantryAngle, float inputCouchAngle, 
                                         float inputCollimatorAngle, float inputSourceToAxisDistance, 
                                         Point_3d_float *isocenterCoordinate)
{
  cout << __FUNCTION__ <<  " compiled on " << __DATE__ << " " << __TIME__ << endl;
  pi = (float) acos(-1.0f);
  degreesToRadians = pi/180.0f;

  gantryAngle     = inputGantryAngle;
  couchAngle      = inputCouchAngle;
  collimatorAngle = inputCollimatorAngle;
  sourceToAxisDistance = inputSourceToAxisDistance;  // acceleratorOrigin to roomOrigin (isocenter) distance
  isocenter.x = isocenterCoordinate->x;
  isocenter.y = isocenterCoordinate->y;
  isocenter.z = isocenterCoordinate->z;
  createAcceleratorToPatientMatrix();
  createPatientToAcceleratorMatrix();
}
/* ****************************************************** */
void coordinateTransform::createAcceleratorToPatientMatrix(){ 
  // initialize the angles
     
  // Angles in radians
  double thetaInRadians   = (double) gantryAngle*degreesToRadians;             // theta = gantryAngle  DOSXYZ
  double phiInRadians     = (double) (couchAngle+90.0)*degreesToRadians;      // phi = turnTableAngle+PlusNinety in DOSXYZ
  double phicolInRadians  = (double) (-1.0*collimatorAngle)*degreesToRadians; // phicol in DOSXYZ consistant with Pinnacle
  // 
  double costheta = cos(thetaInRadians);  double sintheta = sin(thetaInRadians);
  double coscol   = cos(phicolInRadians); double sincol   = sin(phicolInRadians);
  double cosphi   = cos(phiInRadians);    double sinphi   = sin(phiInRadians);
  // Create the rotation matrix needed during sampling routine
  // Determine the forward transformation matrix                       ;
  accelToPatient[0][0] = (float) ( costheta*sinphi*coscol - cosphi*sincol);
  accelToPatient[0][1] = (float) (-costheta*sinphi*sincol - cosphi*coscol);
  accelToPatient[0][2] = (float) (-sintheta*sinphi)                       ;
  accelToPatient[1][0] = (float) (-sintheta*coscol)                       ;
  accelToPatient[1][1] = (float) ( sintheta*sincol)                       ;
  accelToPatient[1][2] = (float) (-costheta)                              ;
  accelToPatient[2][0] = (float) ( costheta*cosphi*coscol + sinphi*sincol);
  accelToPatient[2][1] = (float) (-costheta*cosphi*sincol + sinphi*coscol);
  accelToPatient[2][2] = (float) (-sintheta*cosphi);
}
/* ****************************************************** */
void coordinateTransform::createPatientToAcceleratorMatrix(){
  //
  memcpy(patientToAccel,accelToPatient,9*sizeof(float));
  invert3x3Matrix((float *) patientToAccel); // For translation to accelerator coordinate transform, we need the inverse transform
}
/* ****************************************************** */
void coordinateTransform::reportAccelToPatientMatrix(){
  printf("\n\tAccToPat: %f %f %f\n", accelToPatient[0][0], accelToPatient[0][1], accelToPatient[0][2]);
  printf("\t        : %f %f %f\n", accelToPatient[1][0], accelToPatient[0][1], accelToPatient[1][2]);
  printf("\t        : %f %f %f\n", accelToPatient[2][0], accelToPatient[0][1], accelToPatient[2][2]);
}
/* ****************************************************** */
int coordinateTransform::translateParticleToAcceleratorCoordinateSystem(particle_type *particle){
#ifdef DEBUG_TRANSFORM
    printf("TACEnter: x=(%g %g %g)  u=(%g %g %g)\n", particle->x,particle->y,particle->z, particle->u,particle->v,particle->w);
#endif
  // float sourceToAxisDistance = 100.0;
  float u = particle->u;
  float v = particle->v;
  float w = particle->w;
  // Transform (rotate) the particles direction cosines
  particle->u = patientToAccel[0][0]*u + patientToAccel[0][1]*v + patientToAccel[0][2]*w;  
  particle->v = patientToAccel[1][0]*u + patientToAccel[1][1]*v + patientToAccel[1][2]*w; 
  particle->w = patientToAccel[2][0]*u + patientToAccel[2][1]*v + patientToAccel[2][2]*w;
  // Translate the particle
  // printf("isocenter.x = %f, isocenter.y = %g, isocenter.z = %g\n", isocenter.x, isocenter.y, isocenter.z);
  float px = particle->x - isocenter.x;
  float py = particle->y - isocenter.y;
  float pz = particle->z - isocenter.z;
  // Rotate the particle 
  particle->x =  patientToAccel[0][0]*px + patientToAccel[0][1]*py + patientToAccel[0][2]*pz; //
  particle->y =  patientToAccel[1][0]*px + patientToAccel[1][1]*py + patientToAccel[1][2]*pz; 
  particle->z =  patientToAccel[2][0]*px + patientToAccel[2][1]*py + patientToAccel[2][2]*pz+sourceToAxisDistance; 
#ifdef DEBUG_TRANSFORM
    printf("TACexit:  x=(%g %g %g)  u=(%g %g %g)\n", particle->x,particle->y,particle->z, particle->u,particle->v,particle->w);
#endif
    return 0;
}
/* ********************************************************************* */
int coordinateTransform::translateParticleToPatientCoordinateSystem(particle_type *particle) {
  // The translations used in this code were directly taken from PJK's "epid.cc" code, which
  // inverts the transformation done in DOSXYZ
#ifdef DEBUG_TRANSFORM
  static int iCount=0;
  if(iCount<10)printf("TPPEnter: x=(%g %g %g)  u=(%g %g %g)\n", particle->x,particle->y,particle->z, particle->u,particle->v,particle->w);
#endif
    // float mySourceToAxisDistance = 100.0f;
  float u = particle->u;
  float v = particle->v;
  float w = particle->w;
  // Transform (rotate) the particles direction cosines
  particle->u = accelToPatient[0][0]*u + accelToPatient[0][1]*v + accelToPatient[0][2]*w;  
  particle->v = accelToPatient[1][0]*u + accelToPatient[1][1]*v + accelToPatient[1][2]*w; 
  particle->w = accelToPatient[2][0]*u + accelToPatient[2][1]*v + accelToPatient[2][2]*w;
  // Translate the particle
  // printf("isocenter.x = %f, isocenter.y = %g, isocenter.z = %g\n", isocenter.x, isocenter.y, isocenter.z);
  float px = particle->x;
  float py = particle->y;
  float pz = particle->z - sourceToAxisDistance;
  // Rotate the particle 
  particle->x =  accelToPatient[0][0]*px + accelToPatient[0][1]*py + accelToPatient[0][2]*pz + isocenter.x; //
  particle->y =  accelToPatient[1][0]*px + accelToPatient[1][1]*py + accelToPatient[1][2]*pz + isocenter.y; 
  particle->z =  accelToPatient[2][0]*px + accelToPatient[2][1]*py + accelToPatient[2][2]*pz + isocenter.z; 
#ifdef DEBUG_TRANSFORM
  if(iCount++<10)printf("TPPexit:  x=(%g %g %g)  u=(%g %g %g)\n", particle->x,particle->y,particle->z, particle->u,particle->v,particle->w);
#endif
    return 0;
}
/* ********************************************************** */

