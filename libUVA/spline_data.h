/** \file 
 simple tag auto added to force generation of simple doxygen comments
**/
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
/* 
   Modification History:
   JVS: Sept 3, 1999: JVS: Rename data_point to spline_data_type
*/
#ifndef DATA_POINT_H_INCLUDED
#define DATA_POINT_H_INCLUDED
/* **********  DEFINES */
#define MAX_DATA 5096
/* ****************** */
#define SMALL_FRAC_LIMIT  0.0001 /* precision to find splined value */ 
#define MAX_COUNT 35             /* max number of loops to find splined value */
/* ********** TYPEDEFS */
/* #ifndef MAIN extern */
#ifdef NO_SPLINE_CLASS
typedef struct { float x[MAX_DATA];
		 float y[MAX_DATA];
		 int n_data;
		 double x2[MAX_DATA];
		 double y2[MAX_DATA];
		}spline_data_type;
#else
class spline_data_type { 
 public:
  float x[MAX_DATA];
  float y[MAX_DATA];
  int n_data;
  double x2[MAX_DATA];
  double y2[MAX_DATA];
  ~spline_data_type(){
    n_data=0;
   };
  spline_data_type() {
    n_data=0;
    for(int iData=0; iData<MAX_DATA; iData++) {
      x[iData] = y[iData] = 0.0f;
      x2[iData] = y2[iData] = 0.0;
    }
  };
};


#endif
/* #else */
/* ******************************************************************************* */
int find_splined_value(spline_data_type *data, double* X,
		       double x_delta, double Y_Target);
int locate_point(spline_data_type *data, int* loc, double Target, int start_point,
                 int direct); 
int locate_and_find(spline_data_type *data, double *X, double Y_Target, int direct);
float table_value(float x_val, spline_data_type *tab);
float table_value_extrap(float x_val, spline_data_type *tab);
int spline(spline_data_type *dat, float yp1, float ypn);
int spline_interp(spline_data_type *dat, double  x2, double  *y2);
int read_spline_data(spline_data_type *dat, char *filename);
int write_spline_data(spline_data_type *dat, char *filename);
int compute_specific_spline_x(spline_data_type *dat, float value, float *result);
int compute_specific_spline_y(spline_data_type *dat, float value, float *result);
double spline_value(double x, spline_data_type *data);
int checkSplineError();
int resetSplineError();

#endif

/* ******************************************************************************* */
