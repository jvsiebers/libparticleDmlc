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
/* spline_data.cpp
   Utilities to operate on spline_data_type (old data_point type)
   2/26/97: JVS: extracted from driver.cpp
   5/28/98: JVS: Add table_value_extract
   10/07/98: JVS: Add splineErrno in return from functions with error.
   10/08/98: JVS: find_splined_value now looks at relative differance
   Sep 3, 1999: JVS: Change data_point type to spline_data_type
   March 25, 2002: Put spline_value in to here...
   NOv 5, 2007: JVS: Add splineErrno as local global, and add checkSplineError();
   April 29, 2011: JVS: updates to eliminate g++4.52 warnings
   May 6, 2015: JVS: std::fabs for solaris compile
   Jan 12, 2017: JVS: Fix end condition for locate_point.
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cmath> // 5/2015 addition for fabs on solaris
#include <string.h>
// #include <time.h> /* for fun time stuff */

#if !defined( __MINMAX_DEFINED)
#define __MINMAX_DEFINED
#define min(a, b)    (((a) < (b)) ? (a) : (b))
#define max(a, b)    (((a) > (b)) ? (a) : (b))
#define __max       max
#define __min       min
#endif

/* *********************************************************************** */
#include "spline_data.h"
#include "utilities.h"
#include "table.h"

int splineErrno = OK;

int checkSplineError() { return (splineErrno); }

int resetSplineError() {
    splineErrno = OK;
    return splineErrno;
}

/* ************************************************************************* */
int compute_specific_spline_x(spline_data_type *dat, float x_val, float *y_val) {
    /* Given data with spline interps, for given x_val, find corresponding y_val) */
    double dy_val;
    if (spline_interp(dat, (double) x_val, &dy_val) != OK) {
        printf("\n\t ERROR: Spline Interpolation at value %f", x_val);
        return (FAIL);
    }
    *y_val = (float) dy_val;
    return (OK);
}

int compute_specific_spline_y(spline_data_type *data, float y_val, float *x_val) {
    /* Given data with spline interps, for given y_val, find corresponding x_val) 
       Note: Search here starts from beginning of data, to search from last
             data point backwards, use -1 in locate_and_find...  */
    double dx_val;
    if (locate_and_find(data, &dx_val, y_val, +1) != OK) {
        printf("\n\t ERROR: Unable to compute for y = %f", y_val);
        return (FAIL);
    }
    *x_val = (float) dx_val;
    return (OK);
}

/* ************************************************************************ */
int find_splined_value(spline_data_type *data, double *X,
                       double x_delta, double Y_Target) {
    /* use the spline interpolation to find the value of X such that
       the spline_interpolant = Y_TARGET
       input X is the first guess at the answer,
       input x_delta is the initial offset for modifying the guess*/

    double y_value = 0.0;
    double old_y_val = 10.0 * Y_Target;
    if (Y_Target == 0.0) old_y_val = -1.0; // make sure initialized!!!!
    double x_value = 0.0;
    int count = 0;
    double deviation = 0.0;
    do {
        x_value = *X + x_delta;
        if (spline_interp(data, x_value, &y_value) != OK) {
            eprintf("\n\t ERROR: find_splined_value: SPLINE interpolation error ");
            splineErrno = FAIL;
            return (FAIL);
        }
        // determine if we found a better X value by comparing output of Y value
        if ((std::fabs(old_y_val - Y_Target) <
             std::fabs(y_value - Y_Target))) {  // old y value was better, so keep it and halve the step size
            x_delta = -0.5 * x_delta;
            count++;
        } else { // keep the new y_value which is better.
            old_y_val = y_value;
            *X = x_value;
            //            printf("\n New Best Found: %f %f",x_value, y_value);
        }
        // must check for special case of Y_TARGET=0
        if (Y_Target) deviation = std::fabs((y_value / Y_Target - 1.0));
        else deviation = std::fabs(y_value - Y_Target);
    } while ((deviation > SMALL_FRAC_LIMIT) &&
             (count < MAX_COUNT));
    //      printf("\n find_splined_value: count %d MAX_COUNT %d", count,MAX_COUNT);
    //      printf(" SMALL_LIMIT %f",SMALL_LIMIT);
    return (OK);
}

/* ************************************************************************ */
int locate_point(spline_data_type *data, int *loc, double Target, int start_point,
                 int direct) {   // find loc such that data->y[loc] < Target != data->y[loc-direct] < Target
    // in other words, such that Target value is between index loc and loc-direct
    // direct can be +/-1
    // Assuming checking from ends of the data....
    // If direct = 1, then
    //       1st check is between index [1] and [0],            so jStart=1
    //       1st check is between [minIndex] and [minIndex-1],  so minIndex=1
    //       last check is between [nData-1]  and [nData-2]
    //       last check is between [maxIndex] and [maxIndex-1], so maxIndex=nData-1
    // If direct = -1, then
    //       1st check is between  [nData-2] and [nData-1]
    //       1st check is between  [maxIndex] and [maxIndex+1], so maxIndex=nData-2
    //       last check is between [0] and [1]
    //       last check is betwen  [minIndex] and [minIndex+1], so minIndex=0
    if (Target == data->y[start_point]) { // 2017: Added check for end...
        *loc = start_point;
        return (OK);
    }
// I think NEW_METHOD is the same as the old method (fix needed was checking endpoint)
// leave default at old method since it has had years of use.
    int j = start_point;
    int increment = 1;
#ifdef NEW_METHOD
    // New on 1/12/2017
    int maxIndex  = data->n_data-1;
    int minIndex  = 1;
    if(direct > 0) {
      j=max(minIndex,j);  // start from minIndex
    } else { // (direct < 0)
      increment = -1;
      maxIndex = data->n_data-2;
      minIndex = 0;
      j=min(j,maxIndex);
    }

    while( (data->y[j] < Target) == (data->y[j-increment] < Target)
           && j <= maxIndex
           && j >= minIndex ) j+=increment;
    if( (data->y[j] < Target) == (data->y[j-increment] < Target) ) {
      eprintf("\n\t ERROR: in locate_point\n\tABORTED: Values not monotomically increasing");
      splineErrno = FAIL;
      return(FAIL);
    }
#else // Prior to 1/12/17: I think it had problems with the ends. Unnecessarily re-wrote
    // increment while point is less than target and in range
    if (direct < 0)
        increment = -1;
    if (data->y[j] < (float) Target) {
        while ((data->y[j] < (float) Target) &&
               ((j < data->n_data) ||
                (j > 0))) {
            j += increment;
        }

        if (data->y[j - increment] > Target) // try to find POINT value
        {
            eprintf("\n\t ERROR: in locate_point\n\tABORTED: Values not monotomically increasing");
            splineErrno = FAIL;
            return (FAIL);
        }
    } else {
        // increment while point is greater than target and in range
        while ((data->y[j] >= Target) &&
               ((j < data->n_data) ||
                (j > 0))) {
            j += increment;
        }
        if (data->y[j - increment] < Target) // try to find POINT value
        {
            eprintf("\n\t ERROR: in locate_point\n\tABORTED: Values not monotomically decreasing");
            splineErrno = FAIL;
            return (FAIL);
        }
    }
#endif
    if (j < 0 || j >= data->n_data) {
        eprintf("\n\t ERROR: in locate_point");
        eprintf("\n\t  ABORTED:  VALUE OF %f OUTSIDE OF RANGE", Target);
        eprintf("\n\t  ARRAY[0] %f ARRAY[%d] %f", data->y[0],
                data->n_data - 1, data->y[data->n_data - 1]);
        splineErrno = FAIL;
        return (FAIL);
    }
    *loc = j;
    return (OK);
}

/* ************************************************************************ */
int locate_and_find(spline_data_type *data, double *X, double Y_Target, int direct) { /* Uses Spline Interpolation on X,Y data, (X monotonically increasing)
     locate_and_find determines the X which corresponds to a Y_TARGET value
     --direct specifies the direction to start the search from,
       + search forward
       - search backward
       the magnitide of direct gives the number of points to offset from the
       start or end of the array. */

    int loc;
    int start;

/*  printf("\n Locate_and_find");
  printf("\n Searching for Y_Target of %f from direction %d",Y_Target,direct); */
    // determine the offset location to start search from
    if (direct < 0) {  // start n points from back
        start = data->n_data + direct;
        direct = -1;      // start n points from back
    } else {
        start = direct - 1; // start n points from front
        direct = 1;
    }
    // locate point where Y_TARGET is between data->y[loc] and data->y[loc-direct]
    if (locate_point(data, &loc, Y_Target, start, direct) != OK) {
        eprintf("\n\t ERROR: from locate_point call in locate_and_find");
        splineErrno = FAIL;
        return (FAIL);
    }
    // printf("\n locate_point : start=%d, direct=%d, n_data=%d, loc=%d, target=%f",start,direct,data->n_data,loc, Y_Target);
    /* determine increment to look between the points --
       previously, this was 0.5 of the increment, this proved to be
       problematic when the spline would predict a "bump", value was changed
       to 0.05 of the increment.  This requires more itterations, but it
       vitrually eliminates the effect caused by "bumps" */
    double x_delta = (double) ((data->x[loc] - data->x[loc - direct]) * 0.1);
    // use linear interpolation for a first guess
    *X = data->x[loc] + (data->x[loc - direct] - data->x[loc]) *
                        (Y_Target - data->y[loc]) / (data->y[loc - direct] - data->y[loc]);
    //  *X = (double) data->x[loc-direct];
    if (find_splined_value(data, X, x_delta, Y_Target) != OK) {
        eprintf("\n\t ERROR: find_splined_value in locate_and_find");
        splineErrno = FAIL;
        return (-10);
    }
    return (OK);
}
/* ************************************************************************ */
/* ************************************************************************ */
float table_value(float x_val, spline_data_type *tab)
//    equivalent to old find_table_value
{
    int j;
    if (x_val < tab->x[0] || x_val > tab->x[tab->n_data - 1]) {
        putc(7, stdout);
        eprintf("\n\t ERROR: Value %f outside of table_values: FAIL_SAFE returned", x_val);
        eprintf("\n \t Table Range %f - %f", tab->x[0], tab->x[tab->n_data - 1]);
        splineErrno = FAIL_SAFE;
        return (FAIL_SAFE);
    }
    locate(tab->x, tab->n_data, x_val, &j);
    return (interp_table(tab->x, tab->y, x_val, j));
}

/* ************************************************************************* */
float table_value_extrap(float x_val, spline_data_type *tab)
//   finds y value at a given X, extrapolates if value not in table range
{
    int j;
    if (x_val < tab->x[0]) j = 0;
    else if (x_val > tab->x[tab->n_data - 1]) j = tab->n_data - 2;
    else if (clocate(tab->x, tab->n_data, x_val, &j) != OK) {
        eprintf("\n\t ERROR: clocate: values not monotonic");
        splineErrno = FAIL;
        return (FAIL);
    }
    return (interp_table(tab->x, tab->y, x_val, j));
}

/* ************************************************************************* */
int read_spline_data(spline_data_type *dat, char *filename) {
    FILE *stream;
    char in_string[MAX_STR_LEN];
    double xtmp = 0.0, ytmp = 0.0;
    int i = 0;

    if (filename[0] == '\0') {
        printf("\n Input Name of File To Spline >");
        if (NULL == fgets(in_string, MAX_STR_LEN, stdin)) {
            printf("\n ERROR: fgets");
            exit(-1);
        }
        sscanf(in_string, "%s", filename);
        printf("\t%s", filename);
    }

    stream = fopen(filename, "r");
    if (stream == NULL) {
        printf("\n\t ERROR: OPENING FILE %s", filename);
        return (FAIL);
    }
    /* now read the data from the file */

    while (fgets(in_string, MAX_STR_LEN, stream) != NULL && i < N_DATA) {
        if (sscanf(in_string, "%lf%lf", &xtmp, &ytmp) == 2) {
            dat->x[i] = (float) xtmp;
            dat->y[i] = (float) ytmp;
#ifdef DEBUG
            printf("\n %d  %g %g",i,dat->x[i],dat->y[i]);
#endif
            i++;
        }
    }
    if (i >= N_DATA) {
        printf("\n\t ERROR: Number of Points in File %s exceeds maximum (%d)", filename, N_DATA);
        exit(FAIL);
    }
    dat->n_data = i;
    printf("\n %d Data Points read from %s", dat->n_data, filename);
    fclose(stream);
    return (OK);
}
/* ******************************************************************************************** */
/* ******************************************************************************************** */
int write_spline_data(spline_data_type *dat, char *filename) {
    FILE *stream;
    char in_string[MAX_STR_LEN];
    int cnt = 0;
    double delta_x, min_x, max_x;
    double x_val = 0.0, y_val = 0.0;
    char all_points = 0;

    if (filename[0] == '\0') {
        printf("\n Input Name of File To Write Spline To >");
        if (NULL == fgets(in_string,MAX_STR_LEN,stdin)) {
            printf("\n ERROR: fgets");exit(-1);
        }
        sscanf(in_string, "%s", filename);
    }

    stream = fopen(filename, "w");
    if (stream == NULL) {
        printf("\n\t ERROR: opening file %s", filename);
        return (FAIL);
    }
    min_x = dat->x[0];
    max_x = dat->x[dat->n_data - 1];
    delta_x = 0.05;
    printf("\n Input Minimum x for the spline data (%f) >", min_x);
    if (NULL == fgets(in_string,MAX_STR_LEN,stdin)) {
        printf("\n ERROR: fgets");exit(-1);
    }
    sscanf(in_string, "%lf", &min_x);
    printf(" Input Maximum x for the spline data (%f) >", max_x);
    if (NULL == fgets(in_string,MAX_STR_LEN,stdin)) {
        printf("\n ERROR: fgets");exit(-1);
    }
    sscanf(in_string, "%lf", &max_x);
    printf(" Input Delta x for the spline data (%f) >", delta_x);
    if (NULL == fgets(in_string,MAX_STR_LEN,stdin)) {
        printf("\n ERROR: fgets");exit(-1);
    }
    sscanf(in_string, "%lf", &delta_x);
    printf(" Would You like Input Data points included in the output (y/n)> ");
    if (ok_check()) all_points = 1;

    /* now, write the spline information to a file */
    x_val = dat->x[cnt = 0];
    for (x_val = min_x; x_val <= max_x && cnt < dat->n_data; x_val += delta_x) {
        if (spline_interp(dat, x_val, &y_val) != OK) {
            printf("\n Spline Interpolation ERROR:");
            fclose(stream);
            exit(FAIL);
        }
        fprintf(stream, "%g\t%g\n", x_val, y_val);
        printf("\n%g\t%g", x_val, y_val);
        if (all_points) {
            if (dat->x[cnt + 1] <= (double) (x_val + delta_x)) /* so can get all points */
            {
                cnt++;
                x_val = dat->x[cnt];
            }
        }
    }

    fclose(stream);
    return (OK);
}

/* ********************************************************************** */
double spline_value(double x, spline_data_type *data) {
    double y_value;
    if (spline_interp(data, x, &y_value) != OK) {
        printf("\n\t ERROR: SPLINE INTERPOLATION ERROR");
        splineErrno = FAIL;
        return (FAIL);
    }
    return (y_value);
}
/* ********************************************************************** */
