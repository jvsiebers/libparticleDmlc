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
Structure names: bds_str

File name: bds.h

Description: 

	This file contains data structures defining conventions and constants of the "beam delivery system",
	a combination of treatment machine, energy, modality and scan pattern.  (This is the nomenclature used
	by the RTP Tools contractors.)

	This file also contains relevant symbolic constants and prototype definitions.
Notes:

	Independent_x_jaws, independent_y_jaws:  for a specified treatment, all jaw positions are defined whether jaws are 
	independent or not.  If machine table name is changed, the values are not changed.  However, at the time of 
	calculation of dose or other quantities, the second jaw is set to first jaw for the dependent mode.

	Values of jaw (and leaf) extensions and retractions are specified as signed quantities as appropriate for the machine.
	For example, x_jaw_extension = -2, x_jaw_retraction = 20, y_jaw_extension = -10 and y_jaw_retraction = 20.
	The same sign convention is used to specify the position of jaws or leaves.

	Standard machine:  Standard machine of the 3DRTP system follows certain conventions internally.  For each machine,
	information to map the internal data to the data presented to the user is provided in this data structure.  We
	use the vendor independent conventions similar to the one adopted by the RTP Tools Contract. Some of the conventions
	of the "standard" machine are the same as those adopted by IEC.  Standard Machine parameters:

	Gantry angle = 0.0 corresponds to vertically up.

	Gantry angle increases clockwise.

	Collimator angle = 0.0 corresponds to y jaws along gun target direction.

	Collimator angle increases clockwise in the collimator system (i.e., from x axis to y axis).
	
	Couch tabletop angle is zero when tabletop is farthest away from the gantry (most common treatment
	position).

	Couch tabletop angle increases clockwise when looking  down.  (***GET MM50 CONVENTION FROM GIG - IEC standard
	says opposite sense to Couch Turntable angle***)

	Couch turntable angle (floor) is zero when the turntable is in its normal position.

	Couch turntable angle (floor) increases clockwise when looking down.

	Couch z = 0.0 when table top is at the isocenter, couch x = 0 when couch is laterally centered at the
	isocenter, and couch y = 0 when the couch is longitudinally centered at the isocenter.

	Leaves (in the collimator system, looking down at a plane through the isocenter):

		Leaf direction is along x axis.

                                |
			1	|	3
		  -x <---------------------> +x
			2	|	4	
				|

		Quadrant 1 : Leaves in x-, y+ quandrant: [1] [1] to [1] [num_leaves/4] 

		Quadrant 2 : Leaves in x-, y- quandrant: [1] [num_leaves/4+1] to [1] [num_leaves/2] 

		Quadrant 3 : Leaves in x+, y+ quandrant: [2] [1] to [2] [num_leaves/4] 

		Quadrant 4 : Leaves in x+, y- quandrant: [2] [num_leaves/4+1] to [2] [num_leaves/2]

Defined by:	RM, LB

Date(s)/Author(s):	May 25, 92/RM, LB

Modification History:
   Sept 8, 1998:JVS replace Logicals with char since Logicals cause get_nml to crash.
   Dec 3, 1998: JVS: Add prototype for free_bds
   Dec 8, 1998: JVS: Add beam_data_id_type to get_bds so can error check
   Dec 14, 1998: JVS: Unused Fields removed from bds_type.
   Feb 16, 2000: JVS: Add collimator_direction to bds_type

*/

#define BDS_H_INCLUDED

# ifndef TYPEDEFS_H_INCLUDED
#	include <typedefs.h>
# endif

# ifndef BEAM_DATA_ID_H_INCLUDED
#	include <beam_data_id.h>
# endif

# ifndef WEDGE_H_INCLUDED
#	include <wedge.h>
# endif

# ifndef SRC_FN_H_INCLUDED
#	include <src_fn.h>
# endif

/* Gantry, collimator and couch angle specification sense */

#define CLOCKWISE 0
#define ANTICLOCKWISE 1

/* Leaf motion direction */

#define ALONG_X 0
#define ALONG_Y 1

/* Ouput units of length */

#define LEN_UNITS_MM 0
#define LEN_UNITS_CM 1

#define MAX_SRC_RADII 32

typedef struct bds_str
{
   struct   beam_data_id_str beam_data_id;      /* Machine, room, energy, modality */

   char   bds_path [MAX_PATH_LEN];      /* Not read but transferred from beam record. */
                     /* Used for checking if data for given BDS has */
                     /* already been read. */
   int   num_wedges;            /* # of wedges available for this machine */

   char   *wedge_names [MAX_WEDGES_PER_BDS];   /* Array of pointers to names of all wedges associated with */
                     /* the treatment machine. */
   float   src_isoctr_dist;            /* SAD */

   float   src_isoctr_dist_sq;         /* Computed (not read from file). Time saver quantity. */

   float   jaw_transmission;         /* jaw transmission factor */

   int    collimator_direction;     /* 1.0 or -1.0 */
}bds_type;


/* THESE ELEMENTS WERE FORMERLY PART OF THE BDS STRUCTURE, BUT ARE NOT USED BY
   THE PENCIL BEAM DOSER CALCULATION CODE */
typedef struct unused_bds_str
{

  struct   wedge_str *wedge_ptrs [MAX_WEDGES_PER_BDS];  
                     /* Array of pointers to wedge data structures.  */
                     /* Note that there is a separate namelist block */
                     /* for each wedge with name = wedge_wedge_name in the bds file */
   char   gantry_sense;            /* Positive increment CLOCKWISE or ANTICLOCKWISE */

   float   gantry_range [3];         /* Minimum and maximum settable angles and a middle angle */
                     /* in the frame of reference of the standard machine */

   float   gantry_zero;            /* The gantry angle reading on the standard machine when the */
                     /* angle on the actual machine is zero */

   char   coll_sense;            /* Same as for gantry */

   float   coll_range [3];            /* Same as for gantry */

   float   coll_zero;            /* The coll angle reading on the standard machine when the */
                     /* angle on the actual machine is zero */

   char   tabletop_sense;            /* Positive increment CLOCKWISE or ANTICLOCKWISE */

   float   tabletop_range [3];         /* Minimum and maximum settable angles and a middle angle */
                     /* in the frame of reference of the standard machine */

   float   tabletop_zero;            /* The tabletop angle reading on the standard machine when the */
                     /* angle on the actual machine is zero */

   char   turntable_sense;         /* (Couch base) Positive increment CLOCKWISE or ANTICLOCKWISE */

   float   turntable_range [3];         /* Minimum and maximum settable angles and a middle angle */
                     /* in the frame of reference of the standard machine */

   float   turntable_zero;            /* The turntable angle reading on the standard machine when the */
                     /* angle on the actual machine is zero */
      
    /*Logical */
        char   x_jaws_ind,
      y_jaws_ind;

   float   x_jaw_extension,         /* Dist the jaw can move across the isocenter */
      x_jaw_retraction;         /* Dist the jaw can move away from the isocenter */

   float   y_jaw_extension,         /* Distance the jaw can move across the isocenter */
      y_jaw_retraction;         /* Distance the jaw can move away from the isocenter */

   char   upper_jaw_direction;         /* ALONG_X or ALONG_Y, lower jaw direction is complementary */

    /* Logical */
   char   x_jaws_exist,            /* For the MM50, for example, one of the pair is absent. */
      y_jaws_exist;

   int   num_leaves;              /* # of leaves in MLC; = 0 if no MLC available */

   float   leaf_transmission;         /* MLC leaf transmission factor */

   float   leaf_width;            /* MLC leaf width in cms */

   float   leaf_extension,            /* distance each leaf may travel out past isocenter, (- value), */
      leaf_retraction;         /* distance each leaf may travel back from isocenter (+ value) */

   char   leaf_direction;            /* Enumerated type ALONG_X or ALONG_Y */

   char   leaf_quadrant_label [4] [2];      /* Leaf quadrant mapping parameters.  Labels in 4 quadrants */
                     /* Quadrants are in the order -+, --, ++ and +- */
                     /* Quadrants are for the standard machine. */
                     /* MM50 labels = 1, 2, 4, 3; 2100C = 1A, 2A, 1B, 2B */
                     
   char   lowest_leaf_nums [4],         /* Leaf number ranges in four quadrants */
      highest_leaf_nums [4];

   float   max_leaf_separation;         /* Maximum allowable difference between leaf settings on one
                     side of the MLC; eg., on 2100C, among 1A and 2A or 1B and 2B */

   float   closed_leaf_offset;         /* Offset from the central plane to ensure that there is no leakage */
                     /* between leaves, important for machines with rounded edges. */

   float   closed_leaf_overlap;         /* Required for MM50 when opposing leaves abut, to force machine */
                     /* to push leaves together, so that no gaps smaller than the allowed */
                     /* leaf positioning tolerance may remain between opposing leaves. */

   float   src_block_tray_dist;

   struct   src_fn_str *src_fn;

/*   Output calculation data:  To be incorporated later */

   float   couch_x_y_z_mapping [3] [4];      /* For each couch translation parameter there are four quantities */
                     /* necessary to map the standard machine values to the acutal machine */
                     /* settings.  For example, for couch z, the values are z_max: */
                     /* z value for the highest couch position, z_>: the reading when couch */
                     /* just above the zero position, z_<: the reading when couch is just */
                     /* below the zero position, and z_min: the lowest couch position. */
   
   float   couch_x_y_z_zero [3];         /* Couch translation position reading on the standard machine when the */
                     /* position on the actual machine is zero. */

   char   output_len_units;         /* LEN_UNITS_MM or LEN_UNITS_CM */

}unused_bds_type;

/* Prototype definitions */
#ifdef myCC
// // extern "C" 
#endif
Function_type get_bds (char *, beam_data_id_type *, bds_type **);
#ifdef myCC
// // extern "C" 
#endif
void free_bds(void);

