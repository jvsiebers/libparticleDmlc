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
/***********************************************************************************/
/* if err checks for errors and composes error messages */
/* displays and logs them */
/* to be used if there is nothing further to be done if there is no error */

#define if_err(wwww,xxxx,yyyy)							\
	if (wwww != SUCCESS)							\
	{									\
		compose_err (&(wwww),xxxx,yyyy) ;				\
		display_err () ;						\
		log_err () ;							\
	}									\

/**********************************************************************************/
/* if err checks for errors and composes error messages */
/* displays and logs them */
/* to be used if there are things to be done if there is no error */
/* if_err_else must be followed by code enclosed in braces {} */

#define if_err_else(wwww,xxxx,yyyy)						\
	if (wwww != SUCCESS)							\
	{									\
		compose_err (&(wwww),xxxx,yyyy) ;				\
		display_err () ;						\
		log_err () ;							\
	}									\
	else									\

