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
/*
 *	Header file for bitvector routines.
 */

#define	BITVEC_H_INCLUDED

#ifndef TYPEDEFS_H_INCLUDED
#include 	<typedefs.h>	/* for the Bitvec type */
#endif

/* return some non-zero quantity if bit i is set, else 0 */
#define		bit_is_set(bv,i)	(bv[ (i)>>3 ] & ( 1 << ((i)&07) ))

/* set bit i of bitvector bv to value v */
#define		set_bit(i,v,bv)	if (v) bv[ (i)>>3 ] |= ( 1 << ((i)&07) ); \
                                  else bv[ (i)>>3 ] &= ~( 1 << ((i)&07) )


/* prototypes of routines in bitvec.c here */
	
Bitvec create_bitvec( int num_bits );

	
void destroy_bitvec( Bitvec bv );


	
void bitvec_and( int num_bits, const Bitvec src, Bitvec dst );

	
void bitvec_or( int num_bits, const Bitvec src, Bitvec dst );

	
void copy_bitvec( int num_bits, const Bitvec src, Bitvec dst );

	
void clear_bitvec( int num_bits, Bitvec bv );

	
Logical bitvecs_are_eq( int num_bits, const Bitvec bv1, const Bitvec bv2 ); 

	
void bitvec_diff( int num_bits, const Bitvec bv1, const Bitvec bv2, 
		 Bitvec dst );


void set_bit_range( int start, int end, Logical val, Bitvec bv );

	
void print_bitvec( int num_bits, Bitvec bv );









