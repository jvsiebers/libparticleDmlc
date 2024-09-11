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
/* plan_info.h
   Created: Jan 20, 1999: JVS
 */

typedef struct{
   int  id;
   char name[MAX_STR_LEN];
}plan_info_type;

int read_pinnacle_plans(char *plan_fname, int *n_plans, plan_info_type **plan);
