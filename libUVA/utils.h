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
/* utils.h  A header file for functions used relatively often
   30 March 1999 created pjk
   Modifications:

*/
/* ****************************************************************** */
#define GOOD 100
#define BAD  -100
#define AVE  50

#define MAX_NUM_POINTS 600

typedef struct
{
    double x;
    double y;
    double z;
} POINT;

/* ****************************************************************** */
/* define prototypes here */
int get_val(double xin, double xtest[MAX_NUM_POINTS], double ytest[MAX_NUM_POINTS], int CARD_NUM, double yout);

/* ****************************************************************** */

