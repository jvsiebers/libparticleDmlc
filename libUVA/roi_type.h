/** \file
    \author JVS, PJK

    roi_type.h

    This header file defines the structures used to read/write Pinnacle ROI's
    ROI's (regions of interest) are also known as contours and structures, and are used in
    the treatment planning program (e.g. Pinnacle) to identify regions of which some quantity
    (e.g. dose) is of interest
**/
/*
   Copyright 2000-2006 Virginia Commonwealth University


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
/* roi_type.h 

   Created: Jan 25, 1999: JVS:
   
   Modification History:
      Dec 26, 2000: PJK: Added name,volume and com,area to roi type and roi curve type
      Jan 6, 2000: JVS: name[MAX_STR_LEN] changed to name{MAX_ROI_NAME_LEN] since MAX_STR_LEN not defined
      July 25, 2002: PJK: Added write_pinnacle_roi
      August 29, 2002: PJK/RG: added volume_name to roi
      January 22, 2003: PJK: Added color to roi
      February 14,2005: KW: Added a new structure roi_mesh_type to roi_type. 
      This reads in the new varables roi: vertices, triangles, for 
      surface_mesh and  mean_mesh for Pinnacle 7.7 
      Nov 15, 2006: JVS: Start commenting using dooxygen
      Feb 24, 2011: JVS: Merge in additions from RCF December 14, 2007: CY: Added a new structure vtk_mesh_type
      May 16, 2012: JVS: remove roi_name from call to write_pinnacle_roi since not used
*/

#ifndef ROI_TYPE_H_INCLUDED
#define ROI_TYPE_H_INCLUDED

#ifndef MAX_ROI_NAME_LEN
#define MAX_ROI_NAME_LEN 200  ///< The maximum length of a string
#endif                        

#include "typedefs.h"    ///< Include the basic types

typedef struct {
    int npts;             ///< Number of points on a curve 
    Point_3d_float *point; ///< Pointer to the actual points
    Point_2d_float com;    ///< Unknown, added by PJK
    float area;          ///< Unknown, added by PJK
} roi_curve_type;          ///< A "curve" or contour of an ROI on a single slice

typedef struct {
  int n_vertices;
  int n_triangles;
  Point_3d_int *triangle;
  Point_3d_float *vertex;
} roi_mesh_type;           ///< The mesh-type of a Pinnacle ROI


typedef struct {
    int n_curves;         ///< The number of contours
    roi_curve_type *curve; ///< The contours
    roi_mesh_type *mesh;   ///< For the "mesh" format
   char name[MAX_ROI_NAME_LEN];
   char volume_name[MAX_ROI_NAME_LEN];
   char stats_volume_name[MAX_ROI_NAME_LEN];
   char color[MAX_ROI_NAME_LEN];
   float volume;
} roi_type;   ///< general type for storing roi information

typedef struct {
   int numberOfPoints;
   Point_3d_longint pointIndex;
}vtk_polygon_type;

typedef struct {
  int numberOfPoints;
  int numberOfPolygons;
  Point_3d_float* points;
  vtk_polygon_type* polygons;
}vtk_mesh_type;


int read_pinnacle_3d_points(FILE *fp, int npoints, Point_3d_float *point);
int read_pinnacle_3d_vertices(FILE *fp, int npoints, Point_3d_float *point);
int read_pinnacle_3d_triangles(FILE *fp, int npoints, Point_3d_int *triangle);
int read_pinnacle_roi(char *pbeam_fname_stem,
                      char *roi_name,
                      roi_type *roi);

int scan_pinnacle_rois(char *pbeam_fname_stem,
                      char **roi_list, int *nfound,
                      int roi_max);

bool sortROICurvesByZCoordinate(roi_type *roi);

int write_pinnacle_roi(char *pbeam_fname_stem,
		       //                      char *roi_name,
                      roi_type *roi);
int write_pinnacle_roi(char *pbeam_fname_stem,int mesh_on,
		       //                      char *roi_name,
                      roi_type *roi);

int writePinnacleRoi(char *pbeam_fname_stem,int mesh_on,
                      char *roi_name,
                      roi_type *roi);
int readVTKSurfaceMesh(char *vtkFileName, vtk_mesh_type *vtkMesh);

int writeVTKSurfaceMesh(char *vtkFileName, vtk_mesh_type *vtkMesh);

#endif
/* ***************************************************************************************************** */
