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
/* rtp_err_codes.h
  This file contains error codes definitions used by the 3DRTP system.
  Error codes in this file start at 3000.
*/

/* Error codes related to anatomic structures. Codes 3000-3099 */
#define	RTP__ANAT_NOT_FOUND		3000
#define	RTP__BRANCH_NOT_FOUND		3001
#define	RTP__BAD_ANAT_SET_ID		3002
#define	RTP__BRANCH_ALREADY_EXISTS	3003
#define	RTP__ANAT_SET_NOT_OPEN		3004
#define	RTP__ANAT_CODES_NOT_READ	3005
#define	RTP__BRANCH_CODES_NOT_READ	3006
#define RTP__BAD_BRANCH_CODE		3007
#define RTP__BAD_ANAT_CODE		3008
#define RTP__NO_CONTS_FOUND		3009

/* Error codes related to regions.  Codes 3100-3199 */
#define RTP__NO_ANAT_VOLUMES_FOUND	3100
#define RTP__REGION_NOT_FOUND		3101

/* errors reading / creating object id. Error code 3200-3299 */

#define RTP__NO_OBJ_ID_FILE     3200    /* Object id file does not exist  */
#define RTP__OBJ_ID_LOCKED      3201    /* File locked by another user    */
#define RTP__OBJ_ID_ERROR       3202    /* General file error             */
#define RTP__NO_OBJ_LOG_FILE    3210    /* Object log file does not exist */
#define RTP__OBJ_LOG_LOCKED     3211    /* File locked by another user    */
#define RTP__OBJ_LOG_ERROR      3212    /* General file error             */

/* Image error codes 3500 - 3699 */
#define	RTP__IMG_NOT_FOUND		3500
#define	RTP__BAD_IMG_SEC_NUM		3501
#define RTP__IMG_OPEN_FAIL 		3502
#define RTP__NO_MORE_IMG_SECS		3503
#define RTP__NO_IMG_FOUND		3504
#define RTP__DRR_CLIP_IMG_INTERCEPT    	3505
#define RTP__NO_PIXEL_SIZE_FOUND    	3506
#define RTP__REQUIRES_3D_IMAGE		3507
#define RTP__INV_IMAGE_TYPE             3508

/* Beam error codes 3700 - 3899 */

#define RTP__BEAM_NOT_FOUND		3700
#define RTP__NO_BEAMS_FOUND		3701
#define RTP__NUM_APERS_EXCEEDS_LIM	3702
#define RTP__UNKNOWN_WEDGE_ORIENTATION	3703
#define RTP__COLLIMATOR_UNITS_UNDEFINED 3704
#define RTP__NO_APERTURES_FOUND		3705

/* Dose calculation error codes 3900 - 3999 */

#define RTP__PB_BEAM_DIAMETER_TOO_BIG 	3900
#define RTP__PB_DEPTH_NOT_FOUND		3901
#define RTP__PB_RED_EXP_TOO_BIG		3902
#define RTP__POINT_OUT_OF_PAT		3903
#define RTP__NO_RAY_PAT_INTERSECTION	3904
#define RTP__NUM_CHAINS_EXCEEDS_LIM	3905
#define RTP__RBD_INV_BEAM_DOSE          3950
#define RTP__RBD_INV_REG_TYPE           3951
#define RTP__RBD_INV_BEAM               3952
#define RTP__RBD_INV_REGION             3953
#define RTP__RBD_NOT_FOUND              3954
#define RTP__RBD_BEAM_DOSE_NOT_FOUND    3955

/* Miscellaneous error codes 9000 - 9999 */

#define RTP__INVALID_NUM_INTERSECTIONS	9000
#define RTP__UNSPECIFIED_ERROR		9999
