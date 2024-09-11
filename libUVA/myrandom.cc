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
/* ********************************************************************** */
/*
	routines for generation of random numbers
	20-Feb-1996: Extracted from sector.cpp code from des_scat program
   28-May-1997: JVS: add get_random_gaussian from gaussian code:
   notes on get_random_gaussian:
      compute random gaussian number for given mean and standard deviation
      used mean of sum of 12 uniform random numbers
      code stolen from LAHET by Jeff Siebers
    30-Aug-2001: JVS: add getSeed and randomOne...better for multi-computer use...
    Nov 27, 2007: JVS: const char * 
    Feb 20, 2013: JVS: type casting added in ran1 to avoid gcc 4.4 compiler warning  
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include "myrandom.h"

unsigned long  nRandomsSampled=0; // a counter for number of random numbers sampled
unsigned randomStride=100; // 
//float ran1(int *idum); /* the random number generator */
//float random1(void);   /* returns a floating point random number */
//int random_num(int num); /* returns a random number between 0 and 1 */
//void nerror(char *string); /* prints error report and exits */
/* *********************************************************************** */
void advanceRandomToStride()
{  // advance the random number generator till get to the next stride.
 while(nRandomsSampled%randomStride) { nRandomsSampled++; random(); }
}
/* *********************************************************************** */
void printNRandomsSampled()
{
  printf("\n %ld random numbers were sampled",nRandomsSampled);
}
/* *********************************************************************** */
void setRandomStride(int newStride)
{
  randomStride=newStride;
}
/* *********************************************************************** */
unsigned long getnRandomsSampled()
{
  return(nRandomsSampled);
}
/* *********************************************************************** */
void initializeRandom(unsigned seed)
{
     srandom(seed);
}
// create a odd number random number generator seed
unsigned int getSeed(void)
{
  return( (unsigned int ) (2*(time(NULL) / 2) +1) );
}
/* ********************************************************************** */
// generates random number in range 0 (inclusive) -> 1 (exlusive) ....
float randomOne( void)
{
  //  printf("\n Unsigned Assigned in randomOne......");
  // RAND_MAX == MAX_INT, thus, MAX_INT+1 < 0 (turns on the sign bit...)
  nRandomsSampled++;
  return( (float) (  (float) random()/ (float) ((unsigned) RAND_MAX+1) ) );
}
/* ******************************************************************* */

#define IA1    7141
#define IA2    8121
#define IA3    4561
#define M1   259200L
#define M2   134456L
#define M3   243000L
#define IC1   54773L
#define IC2   28411L
#define IC3   51349L
#define RM1 (1.0f/M1) // typecasting added to reduce compiler warnings
#define RM2  (1.0f/M2) // typecasting added to reduce compiler warnings
/* ****************************** */
float ran1(int *idum)
{
	static long ix1, ix2, ix3;
	static float r[98];
	float temp;
	static int iff = 0;
	int j;

	if(*idum < 0 || iff == 0)
	{
	  iff = 1;
	  ix1 = (IC1 - (*idum))   % M1;
	  ix1 = (IA1 * ix1 + IC1) % M1;
	  ix2 = ix1 % M2;
	  ix1 = (IA1 * ix1 + IC1) % M1;
	  ix3 = ix1 % M3;
	  for(j=1; j<97; j++)
	  {
	  ix1  = (IA1 * ix1 + IC1) % M1;
	  ix2  = (IA2 * ix2 + IC2) % M2;
	  r[j] = (float) ((((float) ix1 + (float) ix2 * RM2) * RM1)); // typecasting added to reduce compiler warnings
	  }
	  *idum = 1;
	}
	ix1 = (IA1 * ix1 + IC1) % M1;
	ix2 = (IA2 * ix2 + IC2) % M2;
	ix3 = (IA3 * ix3 + IC3) % M3;
	j   = (int) (1 + (( 97 * ix3) / M3));
	if( j > 97 || j < 1) {
	  printf("\n ERROR: RAND1: j![1,97], j=%d\n",j);
	   nerror("  ERROR: RAND1: THIS IS IMPOSSIBLE");
	}
	temp = r[j];
	r[j] = (float) ((((float) ix1 + (float) ix2 * RM2) *RM1)); // typecasting added to reduce compiler warnings
	return(temp);
}
/* **************** */
int random_num(int num)
{  /* return a randmon number between 0 and num-1 */
  return((int) (random1() * (float) num));
}
/* ****************** */
float random1()
{
	static int dummy;
	return(ran1(&dummy));
}
/* ******************* */
void nerror(const char *string)
{
	printf("%s",string);
	exit(1);
}
/* ************************ */
double get_random_gaussian(int *seed,double std_dev, double mean)
{
  int i;
  double temp = 0.0;
  for(i=0; i<12; i++)
  {
         temp+=ran1(seed);
  }
  return((temp - 6.0)*std_dev + mean);
}
/* ************************ */
double get_random_gaussian(double std_dev, double mean)
{
  int i;
  double temp = 0.0;
  for(i=0; i<12; i++){
    temp+=randomOne();
  }
  return((temp - 6.0)*std_dev + mean);
}
/* ************************ */
