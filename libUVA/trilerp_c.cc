/** \file 
 simple tag auto added to force generation of simple doxygen comments
**/
/*
 * C code from the article
 * "Tri-linear Interpolation"
 * by Steve Hill, sah@ukc.ac.uk
 * in "Graphics Gems IV", Academic Press, 1994
 *
 * compile with "cc -DMAIN ..." to create a test program
 *
 * Modification History:
 *   Feb 23, 2012: JVS: typecast's added to remove compiler warnings
            May 6, 2015: JVS: Add <cmath> and namespace std for solaris compile
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <cmath>
#include "typedefs.h"
#include "GraphicsGems.h"
#include "utilities.h"

float errorMessage(int x0, int y0, int z0, int xsize, int ysize, int zsize)
{
  printf("\nERROR: Array (%d, %d, %d) out of bounds (%d, %d, %d) in trilerp_c.cc. \nEXITING\n", x0, y0, z0, xsize, ysize, zsize); 
  exit(-1);
  //  return -1;
}
int trilinear(Point_3d_float *p, float *d, int xsize, int ysize, int zsize, float *dxyz)
{
#   define DENS(X, Y, Z) d[(X)+xsize*((Y)+ysize*(Z))]

    int	       x0, y0, z0,
	       x1, y1, z1;
    float       *dp,
	       fx, fy, fz,
	       d000, d001, d010, d011,
	       d100, d101, d110, d111,
	       dx00, dx01, dx10, dx11,
	       dxy0, dxy1;

    x0 = (int) (std::floor(p->x)); fx = p->x - (float) x0;
    y0 = (int) (std::floor(p->y)); fy = p->y - (float) y0;
    z0 = (int) (std::floor(p->z)); fz = p->z - (float) z0;

    x1 = x0 + 1;
    y1 = y0 + 1;
    z1 = z0 + 1;

    if (x0 >= 0 && x1 <= xsize &&
	y0 >= 0 && y1 <= ysize &&
	z0 >= 0 && z1 <= zsize)
    {
	dp = &DENS(x0, y0, z0);
	d000 = dp[0];
	d100 = dp[1];
	dp += xsize;
	d010 = dp[0];
	d110 = dp[1];
	dp += xsize*ysize;
	d011 = dp[0];
	d111 = dp[1];
	dp -= xsize;
	d001 = dp[0];
	d101 = dp[1];
    }
    else
    {
#	define INRANGE(X, Y, Z) \
		  ((X) >= 0 && (X) <= xsize && \
		   (Y) >= 0 && (Y) <= ysize && \
		   (Z) >= 0 && (Z) <= zsize)

	d000 = INRANGE(x0, y0, z0) ? DENS(x0, y0, z0) : errorMessage(x0, y0, z0, xsize, ysize, zsize);
	d001 = INRANGE(x0, y0, z1) ? DENS(x0, y0, z1) : errorMessage(x0, y0, z0, xsize, ysize, zsize);
	d010 = INRANGE(x0, y1, z0) ? DENS(x0, y1, z0) : errorMessage(x0, y0, z0, xsize, ysize, zsize);
	d011 = INRANGE(x0, y1, z1) ? DENS(x0, y1, z1) : errorMessage(x0, y0, z0, xsize, ysize, zsize);

	d100 = INRANGE(x1, y0, z0) ? DENS(x1, y0, z0) : errorMessage(x0, y0, z0, xsize, ysize, zsize);
	d101 = INRANGE(x1, y0, z1) ? DENS(x1, y0, z1) : errorMessage(x0, y0, z0, xsize, ysize, zsize);
	d110 = INRANGE(x1, y1, z0) ? DENS(x1, y1, z0) : errorMessage(x0, y0, z0, xsize, ysize, zsize);
	d111 = INRANGE(x1, y1, z1) ? DENS(x1, y1, z1) : errorMessage(x0, y0, z0, xsize, ysize, zsize);
    }

    dx00 = LERP(fx, d000, d100);
    dx01 = LERP(fx, d001, d101);
    dx10 = LERP(fx, d010, d110);
    dx11 = LERP(fx, d011, d111);

    dxy0 = LERP(fy, dx00, dx10);
    dxy1 = LERP(fy, dx01, dx11);

    *dxyz = LERP(fz, dxy0, dxy1);

    return(OK);
}
// end of trilinear
/* *********************************************************************** */

#ifdef MAIN

/* test program for trilinear interpolation */

#include <stdio.h>

#define XSIZE	2
#define YSIZE	2
#define ZSIZE	2

main()
{
#define TDENS(X,Y,Z) d[(X)+XSIZE*((Y)+YSIZE*(Z))]

	extern double trilinear();
	double	*d;
	int	 x, y, z;
	Point_3d_float	p;

	d = (double *) malloc(sizeof(double) * XSIZE * YSIZE * ZSIZE);

	printf("Test for trilinear interpolation\n");
	printf("Enter the densities for the corners of a cube\n");

	for (x = 0; x < XSIZE; x++)
	for (y = 0; y < YSIZE; y++)
	for (z = 0; z < ZSIZE; z++)
	{
		printf("Point (%d, %d, %d) is: ", x, y, z);
		scanf("%lf", &TDENS(x, y, z));
	}

	printf("Enter the default density: ");
	scanf("%lf", &def);
	printf("Enter point of interest: ");
	while (scanf("%lf %lf %lf", &p.x, &p.y, &p.z) == 3)
	{
		double	n;

		n = trilinear(&p, d, XSIZE, YSIZE, ZSIZE);
		printf("Density at (%lf, %lf, %lf) is %lf\n", p.x, p.y, p.z, n);
		printf("Enter point of interest: ");
	}
}

#endif


