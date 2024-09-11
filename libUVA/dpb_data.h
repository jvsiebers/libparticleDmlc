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
/*B**************************************************************************

Structure Names: 
		dpb_data:
		contains information about the ONE component dpb library
	        This information is read in from the file onecomp_dpb.hdr

		onecomp_dpb:
		contains information about a particular dpb that has been read
		in from the 


Example Files

Condition Codes: 

Author(s) / Date : Lovelock / December 1992.

Modification History:
   Jan 5, 1999: JVS: Create one_comp
   March 9, 1999: JVS: Allow DBS's with energy specified in MeV (float energies)

*E****************************************************************************/

#define MAX_RADII	 99
#define MAX_DPTHS	198

typedef struct dpb_data
{
	int	num_energies;		/* The number of different energies dpbs available in the library*/
	float	*energies_p;		/* pointer to an array of energies, units are MeV's */
    /* int  *energies+p;                pointer to array of energies, units are 100's of keV's */
	float	*mu_p;			/* pointer to array of linear atten coeffics. the actual value that
				   	   was used by egs to generate the particular dpb */
	float	radii [MAX_RADII+1];	/* The logarithmic binning of the radii - cylindrical coords. */
	float	dpths [MAX_DPTHS+1];	/* The logarithmic binning of depths - cylindrical coords. */

}dpb_data_type;

typedef struct	onecomp_dpb
{
	float data [MAX_RADII][MAX_DPTHS];	/* the dose distribution resulting from 10**6
						   incident photons, units are MeV/ cm**3 */
    /*int   energy; */				/* Energy of this dpb, in units of 100 KeV */
    float energy;                           /* Energy of this dpb, in units of MeV */
	float mu;				/* The attenuation coeffic used to generate this dpb, in cm**-1. */
}onecomp_dpb_type;

typedef struct	twocomp_dpb
{
	float elect[MAX_RADII][MAX_DPTHS];	/* the short range dose distribution resulting from 10**6
						   incident photons, units are MeV/ cm**3 */
	float photo [MAX_RADII][MAX_DPTHS];	/* the  long range dose distribution resulting from 10**6
						   incident photons, units are MeV/ cm**3 */
    /* int   energy; */				/* Energy of this dpb, in units of 100 KeV */
    float energy;                           /* Energy of this dpb, in units of MeV */
	float mu;				/* The attenuation coeffic used to generate this dpb, in cm**-1. */
}twocomp_dpb_type;


