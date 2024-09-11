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
#ifndef TYPEDEFS_H_INCLUDED	/* To prevent multiple inclusions of this file */

/* typedefs.h
This file contains all typedefs forthe 3DRTP system.
                               
*/

#define TYPEDEFS_H_INCLUDED	/* To prevent multiple inclusions of this file */

typedef		unsigned char	Logical;

typedef		unsigned char 	Byte;

typedef		unsigned short	Pix;			/* Pixel memroy size for original image */

typedef		unsigned char	Pix_dc;			/* Pixel memory size for image used in dose calculations */

typedef 	unsigned char   *Bitvec;

typedef		struct		point_3d_float_str
		{
			float	x, y, z;
		} Point_3d_float;

typedef		struct		point_2d_float_str
		{
			float	x, y;

		} Point_2d_float;

typedef		struct		delta_3d_float_str
		{
			float	dx, dy, dz;
		} Delta_3d_float;

typedef		struct		delta_2d_float_str
		{
			float	dx, dy;

		} Delta_2d_float;

typedef		struct		point_3d_int_str
		{
			short int	x, y, z;

		} Point_3d_int;

typedef		struct		point_3d_longint_str
		{
			int	x, y, z;

		} Point_3d_longint;

typedef		struct		point_2d_int_str
		{
			short int	x, y;

		} Point_2d_int;

typedef		struct		line_2d_int_str
		{
			Point_2d_int	begin, end;

		} Line_2d_int;

typedef		struct		line_2d_float_str
		{
			Point_2d_float	begin, end;

		} Line_2d_float;

typedef		struct		line_3d_int_str
		{
			Point_3d_int	begin, end;

		} Line_3d_int;

typedef		struct		line_3d_float_str
		{
			Point_3d_float	begin, end;

		} Line_3d_float;

typedef		struct		rect_int_str
		{
			Point_2d_int	min, max;

		} Rect_int;

typedef		struct		rect_float_str
		{
			Point_2d_float	min, max;

		} Rect_float;

typedef		struct		box_int_str
		{
			Point_3d_int	min, max;

		} Box_int;

typedef		struct		box_and_rects_int_str
		{
			Box_int box;
			Rect_int *p_rects;
			int num_rects;
		} Box_and_rects;

typedef		int		Function_type;

#endif	/* TYPEDEFS_H_INCLUDED, to prevent multiple inclusions of this file */
