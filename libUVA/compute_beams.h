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
/* ******************************************************************************** */
/* compute_beams.h
      Routines for computing a single beam with the pencil beam algorithm.
      Must provide the set-up parameters (for Pinnacle, use dose_comp.cc)


   Modification History:
      Dec 11, 1998: JVS: Created by breaking of from dose_comp.cc
      Dec 15, 1998: JVS: Do not need to pass dose array to compute_one_beam
      Jan 20, 2000: PJK/JVS: Added pinnacle_dose_region to 
                             compute_one_pinnacle_beam
      May 31, 2000: JVS: Change to computeOnePinnacleBeam
      Sept 20, 2000: JVS: change compute_one_beam to computeOneBeam and add doseFileName
      Jan 31, 2001: JVS: Add routines for re-use of dose calc
*/
/* ********************************************************************************* */
int write_dose_file(char *name, volume_region_type *grid, float *dose);
int computeOneBeam(beam_type *beam,
                     computation_options_type *computation_options,
                     raytrace_data_type *rpl_raytrace,
                     clipped_img_type *clipped_image,
                     volume_region_type *dose_region, 
                     char *dosefileName);
int compute_one_pinnacle_beam(int ibeam,
                              beam_type *beam,
                              computation_options_type *computation_options,
                              raytrace_data_type *rpl_raytrace,
                              clipped_img_type *clipped_image,
                              volume_region_type *dose_region,
                              volume_region_type *pinnacle_dose_region,
                              long PinnWindowID);
int computeOnePinnacleBeam(char *TrialName,
                              beam_type *beam,
                              computation_options_type *computation_options,
                              raytrace_data_type *rpl_raytrace,
                              clipped_img_type *clipped_image,
                              volume_region_type *dose_region,
                              volume_region_type *pinnacle_dose_region,
                              long PinnWindowID);

int computeOneBeam(beam_type *beam,
                     computation_options_type *computation_options,
                     clipped_img_type *clipped_image,
                     volume_region_type *dose_region,
                     char *doseFileName);
/* ******************************************* */
/* New routines */

int computeOneBeam(beam_type *beam,
                   beam_data_type *beam_data,
                     computation_options_type *computation_options,
                     clipped_img_type *clipped_image,
                     volume_region_type *dose_region,
                     char *doseFileName);
int computeOneBeam(beam_type *beam,
                   beam_data_type *beam_data,
                     computation_options_type *computation_options,
                     clipped_img_type *clipped_image,
                     volume_region_type *dose_region,
                     float *dose_array);

int setupIndividualBeam(beam_type *beam,
                        beam_data_type *beam_data,
                        computation_options_type *computation_options,
                        raytrace_data_type *rpl_raytrace);
int computeOnePinnacleBeam(char *TrialName,
                              beam_type *beam,
                              beam_data_type *beam_data,
                              computation_options_type *computation_options,
                              clipped_img_type *clipped_image,
                              volume_region_type *dose_region,
                              volume_region_type *pinnacle_dose_region);

int computeOnePreparedBeam( beam_type *beam,
                            beam_data_type *beam_data,
                            computation_options_type *computation_options,
                            volume_region_type *dose_region,
                            float *doseArray);
int prepareOneBeamWithoutIDD( beam_type *beam,
                              beam_data_type *beam_data,
                              clipped_img_type *clipped_img,
                              volume_region_type *volume);
/* ********************************************************************************* */
