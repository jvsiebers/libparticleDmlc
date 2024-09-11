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
  Modification History:
      Dec 18, 1998: JVS: Add bds_path to write_bds
   Dec 28, 1998: JVS: Remove reading/referances to linear_atten_coeff since NOT used anywhere
   Jan 5, 1999: JVS: One component DPB model
   March 9, 1999: JVS: Allow DBS's with energy specified in MeV (float energies)
   March 10, 1999: JVS: extent not used doe eliminated
*/


/* include file for the pencil beam preparation program */
/* this file defines the beam delivery system, grids, */
/* depths and energy spectrum structures used in the */
/* pencil beam preparation program */

#define MIN_DEPTH_DIFF 0.01
#define MIN_DEPTH_DIFF_2 0.0001

#ifdef TWO_COMPONENT_DPB
#define LONG_COMPT 1 
#define SHORT_COMPT 2
#endif

#define DEPTH_NOT_FOUND 100000

#define NUM_RADII 99

typedef struct bds         /* beam delivery system structure */
{
   char   *machine_name ; /* machine type eg MM50 */
   char    *modality ;     /* treatment mode (x, electrons) */
        char    *energy ;        /* energy of beam (MV or MeV) */
}bds_type; 

typedef struct depths                   /* depths for single compt pb */
{
   int   num_depths ;    /* number of depths */
   float   *depths_array ;       /* array of depths */
}depths_type;

typedef struct depth_arrays             /* depth arrays for 2 compt pbs */
{
   struct depths *depths ;    /* merged depths */
}depth_array_type;   

struct square_grid_parameters   /* grid parameters for single compt pb */
{
   float   resolution ;   /* grid spacing (cms) */
   int   num_steps ;     /* number of elements = num_steps**2 */
    /* float   extent ; */ /* extent unused so eliminated */ /* physical size of grid (cms) */
} ;

struct grid_parameters_structs  /* grid parameters for 2 compt pbs */
{
   struct square_grid_parameters   *grid ;   /* short range compt grid params */
} ;

typedef struct energy_spectrum     /* energy spectrum */
{
   int    num_energies ;     /* number of energies */
   float   *energies ;        /* array of energies */
   float   *weights ;         /* spectral weights */
   float   max_energy ;       /* maximum energy */
} energy_spectrum_type;   

struct compt_interpolation
{
   float    fine_depth_step ;    /* depth resolution of final calculation */
   float    depth_extension[2] ;   /* smallest and largest depth for extrapolation */
} ;

struct interpolation
{
   struct compt_interpolation *interpolation ;
} ;

#ifdef TWO_COMPONENT_DPB
struct monochromatic_pb           /* monochromatic pb structure for both long and short compt, all depths and radii */
{
   float *long_single_energy_pb_p; /* pntr to long compt array ((data at all radii) at all depths) at one energy */
   float *short_single_energy_pb_p;/* pntr to long compt array ((data at all radii) at all depths) at one energy */
} ;                                /* machine pbs can be made from an array of these together with energy weights */

struct machine_pb                  /* machine pb structure for both long and short compt, all depths and radii */
{
   float *long_machine_pb_p;    /* pntr to long compt array ((data at all radii) at all depths) for the machine */
   float *short_machine_pb_p;   /* pntr to short compt array ((data at all radii) at all depths) for the machine */
} ;

typedef struct machine_pbxy
{
   float  *machine_pbxy_p ;
   float  *long_machine_pbxy_p ;
   float  *short_machine_pbxy_p ;
   float  *long_norm_p ;
   float  *short_norm_p ;
}machine_pbxy_type;
#else
typedef struct monochromatic_pb     /* monochromatic pb structure for ONE compt PB, all depths and radii */
{
   float    *single_energy_pb_p ;   /* pntr to compt array ((data at all radii) at all depths) at one energy */
} monochromatic_pb_type;            /* machine pbs can be made from an array of these together with energy weights */

typedef struct machine_pb           /* machine pb structure for both long and short compt, all depths and radii */
{
   float    *machine_pb_p ;         /* pntr to compt array ((data at all radii) at all depths) for the machine */
}machine_pb_type;

typedef struct machine_pbxy
{
   float *machine_pbxy_p;
   float *norm_p;
}machine_pbxy_type;
#endif

struct machine_pbxy_fft
{
   float    *machine_pbxy_fft_p ;
} ;

/* ***************************************************************************************************************** */

/* function to read in pb specifications from make_pb_ffts.nml */

   Function_type read_pb_parameters                
   (                           
      char          *pb_dir_and_file,          /* path string for make_pb_ffts.nml */
      struct depth_arrays    *depths_p,             /* pointer to depth specs str */
      struct energy_spectrum    *energy_spectrum_p,          /* pointer to energy spect and atten coeff str */
      struct grid_parameters_structs    *grid_parameters_p,       /* pointer to grid param specs str */
      struct bds       *beam_delivery_p,         /* pointer to beam delivery system str */
      struct interpolation   *interpolation_p         /* pointer to interp specs str */
   ) ;

/* function to get mono pbs at specified depths and energy, by integrating mono dpbs over depth with atten coeffs */

   Function_type get_monochromatic_pencil_beams             
   (                                             
      struct depth_arrays    *depths_p, 
      struct energy_spectrum    *energy_spectrum_p, 
      struct monochromatic_pb **mon_pb_p             /* pointer to array of mono_pb strs to be created */
   ) ;

/* function to get machine pbs at specified depths, by integrating mono bs over energy with spectral weights */

   Function_type make_machine_pencil_beams 
   (
      struct depth_arrays    *depths_p, 
      struct energy_spectrum    *energy_spectrum_p, 
      struct monochromatic_pb *mon_pb_p,             /* pointer to array of mono_pb strs */
      struct machine_pb    *mach_pb_p            /* pointer to machine pb to be created */
   ) ;

/* function to reformat machine pbs at specified depths (from ln radial format to xy format) and take ffts of machine pb */

   Function_type make_xy_grid_pb 
   (
      struct depth_arrays   *depths_p, 
      struct machine_pb   *mach_pb_p,             /* pointer to machine pb in log radial format */
      struct grid_parameters_structs   *grid_parameters_p,       
      struct machine_pbxy   *mach_pbxy_p,            /* pointer to mach pb in xy format to be created */
      struct machine_pbxy_fft *mach_pbxy_fft_p         /* pntr to fft of mach pb in xy fmt to be created */
   ) ; 

/* function to write pb_str to file */

   Function_type write_pb_ffts
   (
      char *bds_path,
      struct depth_arrays   *depths_p,             /* data to be written */
      struct grid_parameters_structs   *grid_parameters_p,
      struct bds       *beam_delivery_p,
      struct machine_pbxy_fft *mach_pbxy_fft_p,
      struct interpolation *interpolation_p
   ) ;

