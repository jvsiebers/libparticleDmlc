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
/* ****************************************************************** */
/* get_point.c interpolates the corresponding value of a 
               point from an array
   Created: 30 March 1999: pjk
   Modification:

*/
/* ****************************************************************** */
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include "get_point.h"
#include "utils.h"

/* ****************************************************************** */
int get_val(double xin, double xtest[MAX_NUM_POINTS], double ytest[MAX_NUM_POINTS], int CARD_NUM, double yout)
{

  // determine if x limit is within array limits
  if ((xtest[0]<xin) || (xtest[CARD_NUM-1]>xin))
    {
      printf("\n x value outside bounds of array");
      return(AVE);
    }       

  else if (xtest[0]==xin)
    {
      yout = ytest[0];
      printf("\r x = %g, y = %g (values are equal at 0)",xin,yout);
      return(GOOD);
    }

  else if (xtest[CARD_NUM-1]==xin)
    {
      yout = ytest[CARD_NUM-1];
      printf("\r x = %g, y = %g (values are equal at %d)",xin,yout,CARD_NUM-1);
      return(GOOD);
    }

  else // determine value of out
    {
      // first find which element of the array in-> is greater than 
      int max_low = 0;
      while (xtest[max_low]<xin)
          max_low += 1;

      yout = ((xin-xtest[max_low])*ytest[max_low+1] +
              (xtest[max_low+1]-xin)*ytest[max_low])/
              (xtest[max_low+1]-xtest[max_low]);

      printf("\r x = %g, y = %g",xin,yout);
      return(GOOD);
    }

} // end of get_val
