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
Structure name(s):

File name: anat_str.h

Description:
This file defines various record structures of anatomic structure as they reside in the memory.

Designer(s): RM, BB

Author(s) (Date(s)): RM, BB 2/92

Modification History:
   5/8/98: JVS: Cleaning up for mcv pencil beam: renamed from anat.str to 
                anat_str.h

*/

#ifndef ANAT_STR_INCLUDED
#define ANAT_STR_INCLUDED

# ifndef TYPEDEFS_H_INCLUDED
	# include "typedefs.h"
# endif


/* this is the 'undefined' parameter to 'verify_anat_params' */
#define		NO_PARAM	-1

/* the number of new anat sets to add when we run out of space */
#define		ANAT_SETS_ALLOC_INCREMENT 	2
	


/* Structure for one branch of a contour on an image section */
struct branch_cont_2d_str
{
	int record_num;		/* contour record number in netCDF hdr */
				/* set to -1 if new branch */
    	int branch_code;  	/* Eternal internal branch code. */
    	int num_pnts;  		/* Number of points in the contour */
    	Point_2d_int *pnts;	/* Pointer to the array of x, y integer coordinates. */
			  	/* Point_2d_int is defined in typedefs.h */
};


/* A node of a list of all branches for one image, one anat */
struct branch_list_str
{
	struct branch_cont_2d_str *branch;	/* this branch */
    	struct branch_list_str *next;           /* subsequent branches */
};

/*
	A node of a list of all contours for one image.
	Each node contains all the contours for one anat on this image,
	and has links to the previous and next anats on this image.
*/
struct conts_on_img_str
{
   int anat_code;		/* code for this anat */
    /*   struct conts_on_img_str *prev_anat, *next_anat;*/ /* prev and next anats on this image */
   struct branch_list_str *branches;	/* ptr to the first node of a list of branches for this anat on this img */	

};


/*
	A node of a list of all contours for one anat.
	Each node contains all the contours for this anat on one image,
	and has links to the prev and next images on which this anat has contours.
*/
struct conts_of_anat_str
{
	float 	img_sec_z;  	/* Z-coordinate of this image */
	int	img_sec_num;	/* index of this image in image data set */
        struct conts_of_anat_str *prev_img, *next_img; 		/* prev and next images on which this anat is drawn */
	struct branch_list_str *branches;	/* ptr to the first node of a list of branches for this anat on this img */	
//     struct branch_cont_2d_str *branches;
};


/*
	Contours organized by image. 
	An array of these will allow the contours to be referenced by image.
*/
struct img_conts_str
{
	float 	img_sec_z;   		/* Z-coordinate of all contour points on this image */
	char 	*img_sec_id;		/* name of this image section */
	int	img_sec_num;		/* index of this image in image data set */
	int num_anats;		/* number of anats which have contours on this image */
				/*  - same as the number of elements in the following list */
	struct conts_on_img_str *img_conts; 	/* pointer to the first node of a list of all contours for this image */
};


/* 
	Contours organized by anatomic structure.
	Each node of this list contains all contours for an anat, and
	has a link to the next anatomic structure.
*/
struct anat_conts_str
{
	int	anat_code; 	/* "Eternal" internal code for the anatomic structure */
			   	/* given in the RTP defaults file */
	int num_imgs;		/* number of images on which this anat has contours */
				/*  - same as the number of elements in the following list */
	struct	conts_of_anat_str *anat_conts;	/* a list of contours for this structure */
	struct anat_conts_str *next_anat;	/* link to next anatomic structure */
};


/*
	This contains all the info for one set of anatomic data.
	An array of these represents all open anat sets.
*/
struct anat_set_str
{
	char *path;		/* path to file for these contours */
	char *pt_file_name;	/* name of pt_file for these contours */
	char *img_set_name;	/* Name of the image data set on which the anatomic structures were drawn. */
	char *img_name;		/* Name of the image on which the anatomic structures were drawn. */
	float pixel_size;	   	/* Set equal to pixel size for data structures in this format. */
			   	/* contour coordinate * pixel_size gives contour coordinate in cm. */
	int hdr_channel;	/* input channel (netCDF id) */
	int data_channel;	/* same thing for data file */
	int num_img_secs;
	struct img_conts_str *img_conts;
	struct anat_conts_str *anat_conts;
	int num_refs;		/* number of active references to this anat set */
};

#endif /* ANAT_STR_INCLUDED */
