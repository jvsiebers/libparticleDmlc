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
#ifndef BEAM_H_INCLUDED   /* To eliminate possibility of multiple inclusion of this file */

#define BEAM_H_INCLUDED

/* 
Structure names: beam_str

File name: beam.h

Description: 

   This file contains data structures (beam_str), symbolic constants and prototype definitions 
   for beam data.  

   There is one instance of beam_str data structure for each beam.
   Beams may exist within the program as an array.

Notes:

   All parameters in the beam record are according to the internal 3DRTP system conventions.  Depending upon the 
   beam data table selected, they are mapped to the display using the conventions specified in the machine
   constants file.

   Conventions (e.g., gantry +ve clockwise) do not exist in the beam parameter record.  They are obtained from the 
   machine table (BDS (beam delivery system) table) at the time of beam definition, dose calculation, or for mapping 
   to the outside world for any other purpose.  These conventions include:

      Gantry angle sense:  (+ve clockwise, e.g.)

      Gantry angle limits

      Collimator angle sense:

      Collimator angle limits

      Independent_x_jaws, independent_y_jaws:  All jaw positions are defined whether jaws are independent or not.
      If machine table name is changed, the values are not changed.  However, at the time of calculation of dose
      of other quantities, the second jaw is set to first jaw for independent mode.

      Leaf transmission (not between leaves but through them)

      Leaf width(s)

      Leaf travel extent (minimum and maximum)

      Leaf travel direction

      Number of leaves

      Leaf mapping scheme from "standard" machine (could be a function name)

      Jaw travel extent

      Independent x and/or y jaw modes available.

   In addition to conventions, beam data tables also contain the following information for each machine:

      Source to isocenter distance.

      Source to blocking tray distance

      Wedge data:  Names, coordinates with respect to the collimator system, material and linear attenuation
      coefficient (or transmission coefficients as a function of thickness) for each photon energy.

   Parameters in the machine data tables are specified in the machine's own frame of reference.  Where
   applicable, they are transformed to the frame of reference of the standard machine.
      
See coordinate systems document for conventions.  

Defined by:   RM, LB

Date(s)/Author(s):   June 5, 92/RM,LB

Modification History:
   Sept 2, 1998: JVS: typedef for structures too, upgraded for pb project
   Sept 21, 1998: JVS: remove apertures_defined from structure
   Nov 19, 1998: JVS: Add norm_factor and calc (1 = calculate, 0 = don't calculate)
   Dec 28 1998 :pjk: Add calibration output factor 
   Jan 6, 1999: JVS: Change length of fname to MAX_PATH_LEN
   May 20, 1999: pjk: Add mlc specific variables
   June 4, 1999: JVS: remove unused calib_output_factor
   Nov 4, 1999: JVS: Add compensator to beam_type structure
   Feb 12, 2000: PJK: Add weight
   Jun 7, 2000: Logical ind_jaws commented out since they are not used...
                change Logical to ints for wedge_defined and intensity_modulator_defined
   December 13, 2000: JVS: Add imrtMethod to beam_type...to be used by mcv Monte Carlo
                           to tell if should use intensity grid IMRT or dmlc IMRT
   NOv 5, 2005: jvS: Add icCorrectionFactor.
   Jan 14, 2015: JVS: Add in dynamicArc
   May 4, 2015: JVS: Change mlcType mlc to mlcType *mlc, numberOfControlPoints;
                     controlPointClass
   May 16, 2017: JVS: Add RxName, beamMonitorUnits, doseFileName
*/

/* Symbolic Constants */

#define MAX_BOLUSES 3

#include <string>
using namespace std;

# ifndef TYPEDEFS_H_INCLUDED
#   include "typedefs.h"
# endif

# ifndef RTP_H_INCLUDED
#   include "rtp.h"
# endif

# ifndef BEAM_DATA_ID_H_INCLUDED
#   include "beam_data_id.h"
# endif

# ifndef BDS_H_INCLUDED
#   include "bds.h"
# endif


#include "apertures.h"
#include "compensator.h"
#include "read_clif.h"
#include "mlc.h"
//# ifndef ODM_DATA_H_INCLUDED
//#   include "odm_data.h"
//# endif

#define NoIMRT         0
#define CompensatorIMRT 111
#define DmlcIMRT       222
#define particleDmlcIMRT 333
#define stepAndShootIMRT 444
#define ibmParticleDmlcIMRT 555 // Feb 17, 2004: JVS: added 
#define dynamicArc 777


typedef struct beam_str
{
    char    fname[MAX_PATH_LEN];      /* file name that contains the data in this structure */
                                      /* fname is used to read in ancillary binary files like the compensator */
    char    name [MAX_OBJ_NAME_LEN];  /* Beam name */
    char    BeamType[MAX_OBJ_NAME_LEN]; // Type of beam (Static, Step-and-Shoot, ...)
    int     id_num;                   /* id number of this beam */
    int     dose_grid_id;                 /* id number of the dose file, plan.Trial.binary.dose_grid_id */

    char    image_name[MAX_OBJ_NAME_LEN]; /* name of the ct image for this patient */

    beam_data_id_type beam_data_id;

    int   wedge_defined; /* was Logical */
    char   wedge_name [MAX_OBJ_NAME_LEN];   /* = "\0" if no wedge.  Used as name of the wedge */
                                            /* transmission matrix file */
    int   wedge_orientation;                /* = 0, 90, 180, or 270 only */
    int   wedge_angle; // Added for use with EDW

    int   intensity_modulator_defined; /* was Logical */
  //struct   odm_data_str odm_dataset;   /* "Opening Density Matrix" data set */ // 5/4/2015: JVS: Not used
                                         /* Contains all intensity modulator data */
   
    /*   Logical   apertures_defined; */ /* NOT NEEDED, if num_apertures > 0, then defined */
   int   n_apertures;            /* number of apertures defined for this beam */
   aperture_type *aperture;      /* Pointer to the array of aperture structures */
                                 /* N.B.: Apertures will contain beam_boluses */
   compensator_type *comp;       /* Compensator */

   mlc_type mlc;                 //  to the mlc structure

   Logical  boluses_defined;
   char bolus_name[MAX_OBJ_NAME_LEN];
   int   num_boluses;                  /* # of boluses defined for this beam */
   int bolus_object_id [MAX_BOLUSES];  /* Bolus identifier */
   int mlc_defined;                    /* 1 have mlc, 0 otherwise */

   float   gantry;                  // Gantry angle in signed degrees 

   float   arc;                     /* Arc length in degrees */

   float   coll;                    /* Collimator angle in signed degrees */

   float   jaw_x1, 
           jaw_x2, 
           jaw_y1, 
           jaw_y2;

    /*   Logical   ind_jaw_x_mode_on,
	 ind_jaw_y_mode_on;  */

   float   tabletop_angle,
           turntable_angle;            /* In degrees */

   Point_3d_float couch;             /* x= lateral, y = longitudinal, z = vertical */

   char IsocenterName[MAX_OBJ_NAME_LEN]; 
   Point_3d_float initial_isoctr_in_is;          /* WITH BEAM IN ZERO POSITION.  Needed for BEV transformations */
                      /* (Equivalent to "tumor center" in the old system.) */
                      /* Coordinates in cm relative to the image system. */
    float norm_factor; /*normalization factor for MC calculations */
    float icCorrectionFactor; /* Backscatter and UFF part of calibration factor */
    /*    float calib_output_factor; */
    int calc;  /* flag to tell if should calculate beam or not 1=yes, 0=no */
    float weight; /* weight of beam */
    int imrtMethod; 
    char  RxName[MAX_OBJ_NAME_LEN];
    //float RxMonitorUnits; // Total MU for the prescription
    float beamMonitorUnits; // RxMonitorUnits * Weight
    char doseFileName[1024];
}beam_type,Patient_beam_type;


#endif /* BEAM_H_INCLUDED to eliminate possibility of multiple inclusion of this file */
