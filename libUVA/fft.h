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
#ifndef FFT_H_INCLUDED
#define FFT_H_INCLUDED

/*
 *	Definitions for fft routines.

   Modification History
      June 14, 2000: JVS; ADD TYPE TO real_fft_2d
      Jan 10, 2007: JVS: Add in #ifdef myCC in front of many functions so c++ version of pencil beam will compile
 */

#define		FORWARD_FFT		1
#define		INVERSE_FFT		-1

#define		EVEN_KERNEL		1
#define		ARBITRARY_KERNEL	0
#define		ODD_KERNEL		-1

#define	   TWO_PI	6.28318530717959
#define	   PI		3.14159265358979

#include "typedefs.h" /* need for definition of function type */

/* fft function prototypes */
#ifdef myCC
// extern "C"
#endif
void fft_cplx
(
  float data[],
  int size, 	
  int step, 	
  int isign	
);
#ifdef myCC
// extern "C"
#endif
void fft_1d
( 
  const float *data, 
  int size, 
  int direction,
  float *output
);
#ifdef myCC
// extern "C"
#endif
void fft_2d
( 
  const float *data, 
  int x_size, int y_size, 
  int direction,
  float *output
 );
#ifdef myCC
// extern "C"
#endif
void real_fft_1d
( 
  const float *input, 
  int size, 
  int direction, 
  float *output 
 );

#ifdef myCC
// extern "C"
#endif
Function_type real_fft_2d
( 
  const float *data, 
  int x_size, int y_size, 
  int direction,
  float *output
 );

void convolve_1d
(
  const float *input,
  const float *kernel,
  int size,	 
  int kernel_type,
  float *output 
);

#ifdef myCC
// extern "C"
#endif
void convolve_transformed_1d
(
  const float *input, 
  const float *xformed_kernel, 
  int size,	 
  int kernel_type,
  float *output 
);

#ifdef myCC
// extern "C"
#endif
void real_mult_fft_1d
(
  const float *fft1, 
  const float *fft2, 
  int size,	 
  int kernel_type,
  float *output 
);

#ifdef myCC
// extern "C"
#endif
Function_type convolve2d
(
  const float *input,
  const float *kernel,
  int x_dim, int y_dim, 
  int kernel_type,
  float *output 
);

#ifdef myCC
// extern "C"
#endif
Function_type convolve_2d
(
  const float *input,
  const float *kernel,
  int x_dim, int y_dim, 
  int kernel_type,
  float *output 
);
#ifdef myCC
// extern "C"
#endif
Function_type convolve_transformed_2d
( 
  const float *input, 
  const float *xformed_kernel, 
  int x_dim, int y_dim,
  int kernel_type, 
  float *output 
);
#ifdef myCC
// extern "C"
#endif
Function_type real_mult_fft_2d
( 
  const float *fft1, 
  const float *fft2, 
  int x_dim, int y_dim,
  int kernel_type, 
  float *output 
);

/* Nov 26, 2003: JVS: prototypes needed for convolve2D */
int checkIf2n(int m);
Function_type realFFT2d
( 
  const float *data, 		/* input data */
  int x_size, int y_size, 	/* dimensions */
  int direction,		/* FORWARD_FFT or INVERSE_FFT */
  float *output			/* the output */
  );
Function_type realMultFFT2d
(
  const float *fft1, 
  const float *fft2, 
  int x_dim, int y_dim,
  int kernel_type, 
  float *output
  );
Function_type convolveTransformed2d
( 
  const float *input, 
  const float *xformed_kernel, 
  int x_dim, int y_dim,
  int kernel_type, 
  float *output 
  );
#endif
