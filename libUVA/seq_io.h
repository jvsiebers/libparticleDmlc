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

   Modification History:
      Sept 21, 2009: JVS: Get rid of // extern "C" so compliles well with c++ compilers
*/
#ifndef SEQ_IO_H_INCLUDED
#define SEQ_IO_H_INCLUDED

#ifdef myCC
// // extern "C"
#endif 
Function_type open_seq(char *filename, char *mode, FILE **fp);

#ifdef myCC
//// extern "C"
#endif 
Function_type close_seq(FILE *fp);

#ifdef myCC
// // extern "C"
#endif
Function_type write_seq(FILE *fp, ... );

#ifdef myCC
// // extern "C"
#endif
Function_type read_seq(FILE *fp, ... );

#endif

