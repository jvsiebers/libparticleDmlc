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
/* read/write file routines for analyse program
   JVS: 2/26/97: broken off of driver.cpp
   Modification History:
      JVS: 12/17/98: Allow passing filenames to read and write routines
      Jan 7, 2000: pjk: removed space in lines containing open_file
      Nov 10, 2005: JVS: remove writing to ostream, add definitions for lstream and pstream, but
      April 29, 2011: JVS: updates to eliminate g++4.52 warnings
   4/1/2015: JVS: Hack in xmgraceStream;
   May 6, 2015: JVS: std::fabs for solaris compile
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <cmath>
//#include <conio.h> /* for putch */
#include "spline_data.h"
/* *********************************************************************** */
// #include "protos.h" // March 25, 2002: JVS
#include "utilities.h"
// #include "globals.h"  // March 25, 2002: JVS
#include "file_routines.h"

FILE *lstream; // latex
FILE *pstream; // plot
FILE *ostream; // output
FILE *xmgraceStream; // xmgrace
int Plot_Format;
//#include "table.h"
/* ********************************************************************** */
/* Updated to read in Wellhofer Format on 19-April-1994   */
int read_data(spline_data_type *data) {
    char in_string[MAX_STR_LEN];
    char filename[MAX_STR_LEN];

    printf("\n Input Name of File to Read >");
    if (NULL == fgets(in_string,MAX_STR_LEN,stdin)) {
        printf("\n ERROR: fgets");exit(-1);
    }
    sscanf(in_string, "%s", filename);
    printf("\t%s", filename);

    return (read_data(data, filename));
}

/* ********************************************************************** */
int read_data(spline_data_type *data, char *filename) {
    char in_string[MAX_STR_LEN];
    float xtemp, ytemp;
    int n_data = 0;
    int i;
    int file_format = 0; /* flag for file format */
    /* 0 = standard         */
    /* 1 = wellhofer        */
    int len;

    FILE *stream = open_file(filename, "", "r");
    if (stream == NULL) {
        eprintf("\n ERROR: Cannot open file %s in read_data", filename);
        // fclose(stream);
        return (FAIL);
    }
    /* read the data from the file */
    data->n_data = 0;
    /* read first line */
    if (fgets(in_string, MAX_STR_LEN - 4, stream) != NULL) {
        /* check format to see if it is a "Wellhofer" file */
        /* Wellhofer files are comma delimited */
        /* so will check for commas */
        len = (int) strlen(in_string);
        for (i = 0; i < len && !file_format; i++) {
            if (in_string[i] == ',') {
                file_format = 1;
            }
        }
        if (file_format == 1) /* for wellhofer file format, skip 5 lines */
        {
            printf("\n Wellhofer File Format ");
            for (i = 0; i < 4; i++) {
                if (NULL == fgets(in_string,MAX_STR_LEN,stdin)) {
                    printf("\n ERROR: fgets");exit(-1);
                }
            }
        }
    }
    /* new read the file */
    switch (file_format) {
        case 0: /* standard file format */
            do {
                if (sscanf(in_string, "%f%f", &xtemp, &ytemp) == 2) {
                    data->x[n_data] = xtemp;
                    data->y[n_data] = ytemp;
                    n_data++;
                }
            } while (fgets(in_string, MAX_STR_LEN, stream) != NULL && n_data < MAX_DATA);
            break;
        case 1: /* wellhofer file format */
            do {
                if (sscanf(in_string, "%f ,%f", &xtemp, &ytemp) == 2) {
                    data->x[n_data] = 0.01f * xtemp; /* change units to cm */
                    data->y[n_data] = ytemp;
                    n_data++;
                }
            } while (fgets(in_string, MAX_STR_LEN, stream) != NULL && n_data < MAX_DATA);
            break;
            /* then add the default statement */
        default:
            eprintf("\n ERROR: File format not supported");
            return (FAIL);
    }
    /* now check the data and all */
    if (n_data >= MAX_DATA) {
        printf("\nERROR: Number of Points (%d) greater than max data (%d) in file_routines.cc:read_data",
               n_data, MAX_DATA);
        return (FAIL);
    }
    data->n_data = n_data;
    printf("\n\t%d points read from %s", n_data, filename);
    fclose(stream);
    return (OK);
}

/* ********************************************************************** */
int write_data(spline_data_type *data) {
    char filename[MAX_STR_LEN];
    char in_string[MAX_STR_LEN];

    printf("\n Input Name of File to Write >");
    if (NULL == fgets(in_string,MAX_STR_LEN,stdin)) {
        printf("\n ERROR: fgets");exit(-1);
    }
    sscanf(in_string, "%s", filename);
    printf("\t%s", filename); /* write back file name */
    return (write_data(data, filename));
}

/* **************** */
int write_data(spline_data_type *data, char *filename) {
    FILE *stream;

    stream = open_file(filename, "", "w");
    if (stream == NULL) {
        eprintf("\n ERROR: Cannot open file %s in write_data", filename);
        fclose(stream);
        return (FAIL);
    }
    for (int i = 0; i < data->n_data; i++) {
        fprintf(stream, "%f %f\n", data->x[i], data->y[i]);
    }
    fclose(stream);
    printf("\n %d points written to %s", data->n_data, filename);
    return (OK);
}

/* ************************************************************************* */
int write_and_plot_data(spline_data_type *data) {
    FILE *stream;
    char in_string[MAX_STR_LEN];
    char filename[MAX_STR_LEN];
    int i;

    printf("\n Input Name of File to Write >");
    if (NULL == fgets(in_string,MAX_STR_LEN,stdin)) {
        printf("\n ERROR: fgets");exit(-1);
    }
    sscanf(in_string, "%s", filename);
    // make string all lower case
    for (unsigned int j = 0; j < strlen(filename); j++)
        filename[j] = (char) tolower(filename[j]);
    printf("\t%s", filename); // write back file name
    stream = fopen(filename, "w");
    if (stream == NULL) {
        eprintf("\n ERROR: Cannot open file %s in write_data", filename);
        fclose(stream);
        return (FAIL);
    }
    for (i = 0; i < data->n_data; i++) {
        fprintf(stream, "%f %f\n", data->x[i], data->y[i]);
    }
    fclose(stream);

    printf("\n %d points written to %s", data->n_data, filename);

    strcpy(in_string, filename); // put full file name into in_string
    // Now that data is written, check plot file options
    printf("\n Do you want New PLOT? [y/n] > ");
    if (ok_check()) {
        i = 0;
        while (filename[i] != '.' && i++ < (int) strlen(filename));
        filename[i] = '\0';  // Null Terminator
        fprintf(lstream, "\n \\clearpage");
        printf("\n writing plot in format %d", Plot_Format);
        if (Plot_Format) {
            fprintf(lstream, "\n \\begin{figure}");
            fprintf(lstream, "\n \\htmlimage{thumbnail=0.5}");
            fprintf(lstream, "\n \\includegraphics[width=5in]{%s.ps}", filename);
            char ostring[MAX_STR_LEN];
            latex_string(filename, ostring);
            fprintf(lstream, "\n \\caption[%s]{%s}", ostring, ostring);
            fprintf(lstream, "\n \\end{figure}");
            fprintf(pstream, "\n set output \"%s.ps\" ", filename);
        } else {
            fprintf(lstream, "\n \\noindent \\input{%s}\\\\", filename);
            fprintf(pstream, "\n set output \"%s.tex\" ", filename);
        }
        /* ----------- */
        fprintf(pstream, "\n set title \"%s\" ", filename);
        fprintf(pstream, "\n plot \"%s\" ", in_string);
    } else {
        fprintf(pstream, ", \"%s\" ", in_string);
    }
    {
        /* output the file name */
        char ostring[MAX_STR_LEN];
        latex_string(in_string, ostring);
        fprintf(lstream, "\n \\htmladdnormallink{%s}{../%s}\\\\", ostring, ostring);
    }
    return (OK);
}

/* ************************************************************************** */
int sort_data(spline_data_type *data, float *norm) {
    int i;
    int j;
    float xtemp, ytemp;
    float min, max;

    // Sort the Data
    // printf("\n\tSorting %d data elements ", data->n_data);
    i = 0;
    do {
        if ((i < data->n_data - 1) &&
            (data->x[i] > data->x[i + 1])) // if not ascending /
        {                      // swap
            xtemp = data->x[i + 1];
            ytemp = data->y[i + 1];
            data->x[i + 1] = data->x[i];
            data->y[i + 1] = data->y[i];
            data->x[i] = xtemp;
            data->y[i] = ytemp;
            if (i) i--;         /* force recursion */
        } else {
            i++;
        }
    } while (i < data->n_data);
    /* then check for duplicate points and remove */
    i = 1;
    do {
        if (data->x[i] == data->x[i - 1]) /* if find duplicate */
        {                              /* then store average value */
            data->y[i] = (data->y[i] + data->y[i - 1]) * 0.5f;
            for (j = i; j < data->n_data; j++)  /* and bubble up other points */
            {
                data->x[j] = data->x[j + 1];
                data->y[j] = data->y[j + 1];
            }
            data->n_data--;
            i--;
        }
    } while (++i < data->n_data);

    /* also check the sign of the data */
    /* since for some data, we get Negative charge */
    max = min = data->y[0];
    for (i = 0; i < data->n_data; i++) {
        if (data->y[i] > max) max = data->y[i];
        if (data->y[i] < min) min = data->y[i];
    }
    if (std::fabs(min) > std::fabs(max)) {
        printf("\n Absolute Value of Data Minimum greater than");
        printf("\n  Absolute Value of Data Maximum \n");
        printf(" Likely Negative Charge ");
        printf("\n Automatic sign change taking place for all data");
        for (i = 0; i < data->n_data; i++) {
            data->y[i] *= -1;
        }
        *norm = -1.000;
    }
/*
   for(i=0; i< data->n_data; i++)
   {
     printf("\n %f %f", data->x[i], data->y[i]);
   }
*/
    return (OK);
}
/* ********************************************************************** */
