/** \file 
 simple tag auto added to force generation of simple doxygen comments
**/
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
/* scatter.cc

   Routines to create scattered photons within the dmlc
   Limitations: 1. Only Compton without binding energies.
                2. Uses default C "multiplicative congruential random-number gen-erator  with  period  2^32  that  returns successive pseudo-random numbers in the range from 0 to (2^15)-1."

Created: 15 Feb 2000: PJK

Modification History:
  12 July 2001: JVS: IMprove error message to assist debugging
                     change typecasts for rand to (double) from (float)
  30-Aug-2001: JVS: change to randomOne so better when run on multiple systems
                    ( used to use rand()/RAND_MAX...not sure if should have been change in all places.)
  08-Nov-2001: JVS: Add VERY crude Compton electrons
  14-Nov-2005: JVS: Re-enter: weight updates must be *= to allow non 1.0 input weights to work properly
       Feb 7, 2007: JVS: Add C_CLASS parts
   Dec 5, 2007: JVS: Improve error message when scat->w < 0.0
                     Replace all instances of 0.0 with 0.0f (to avoid type conversion and associated troubles)
   Nov 11, 2008: JVS: Remove warnings for c++ class 
                 "warning: declaration of 'someVariable' shadows a member of 'this'"
                 for routines    
                 scatteredPhotonAndElectron
                 scatteredPhoton
*/
/* *********************************************************************** */
// system libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> /* for fabs */
#include <sys/syscall.h>

// other libraries
#include "utilities.h"
#include "option.h"
#include "string_util.h"
#include "common.h"
#include "table.h"
#include "spline_data.h"
#include "routines.h"
#include "read_write_egs.h"
#include "myrandom.h"

//nova libraries
#include "mc.h"
#include "mcrand.h"
#include "mcrng.h"

//local libraries
#ifdef C_CLASS
#include "particleDmlcClass.h"
#else

#include "particleDmlc.h"

//
// Local Prototypes
int getComptonProducts(particle_type *prim, particle_type *scat, particle_type *electron);

int rotateByRandomPhi(particle_type *prim, particle_type *scat, particle_type *electron);

#endif
/* ************************************************************************ */
#ifdef C_CLASS
int particleDmlc::scatteredPhotonAndElectron(particle_type* prim, particle_type* scat, particle_type* electron,
        float thick, float mu, // 11/08 table_type *E_mu, table_type *E_mu_Compt
                float wt_not_int, float psdDistance)
#else

int scatteredPhotonAndElectron(particle_type *prim, particle_type *scat, particle_type *electron,
                               float thick, float mu, table_type *E_mu, table_type *E_mu_Compt,
                               float wt_not_int, float psdDistance)
#endif
{
    // Same as PJK scattered_photon, but does not do the writing....
    // and back-projects particle by psdDistance...
    // Essentially assumption is that dmlc is infinitely thin layer.
    // Set particle to same position and charge
    /* copy prim particle to scattered */
    // dose_region = pinnacle_dose_region;
    // printf("\n Processing scatteredPhotonAndElectron ");
    memcpy(scat, prim, sizeof(particle_type));
    memcpy(electron, prim, sizeof(particle_type));
    if (memcmp(scat, prim, sizeof(particle_type)) != 0 ||
        memcmp(electron, prim, sizeof(particle_type)) != 0) {
        printf("\n ERROR: copying memory\n");
        return (FAIL);
    }
    // 1. sample interation site of original particle
    float scat_thick;
    if (forced_interaction(thick, mu / prim->w, &scat_thick) != OK) {
        printf("\n ERROR: Forcing interaction");
        error_part(prim, scat);
        return (FAIL);
    }
    // 2. sample probability of photon energy and angle
    if (getComptonProducts(prim, scat, electron) != OK) {
        printf("\n ERROR: Sampling KN");
        return (FAIL);
    }
    if (scat->energy <= E_mu->x[0]) //energy too low. Return
    {
        scat->weight = electron->weight = 0.0f;
        printf("\nEnergy of scattered particle (%f) too low", scat->energy);
        return (OK);
    }
    // 3. determine u, v, w. Return if w < 0
    if (rotateByRandomPhi(prim, scat, electron) != OK) {
        printf("\n ERROR: Rotating particle");
        return (FAIL);
    }
    // 4 determine mu_C/mu for original particle, and get mu for new particle
    float mu_c = table_value_extrap(prim->energy, E_mu_Compt);

    float comptonProbability = wt_not_int * mu_c / mu;
    if (scat->w <= 0.0f)  // particle going backward, so, set weight = 0.0f
    {
        scat->weight = 0.0f;
    } else {
        float muscat = table_value_extrap(scat->energy, E_mu);
        float muscat_over_w = muscat / scat->w;
        if ((muscat <= 0.0f) || (mu_c <= 0.0f) || (mu <= 0.0f) || (muscat_over_w <= 0.0f)) {
            printf("\n ERROR: Attenuation coefficient < 0: Muscat=%g wscat=%g mu_c=%g mu=%g mu_over_w=%g",
                   muscat, scat->w, mu_c, mu, muscat_over_w);
            printf("\n\t particle energy = %g", prim->energy);
            printf("\n \t Please check your cross-section table ");
            error_part(prim, scat);
            return (FAIL);
        }
        // 5. determine weight
        scat->weight *= comptonProbability * (float) (exp(-muscat_over_w * scat_thick)); // Nov 14, 2005
    }
    //printf("\n Scat wt in scat phot = %f ",scat->weight);
    if ((scat->weight > 1.0) || (scat->weight < 0.0f)) {
        printf("\n ERROR: scatteredPhotonAndElectron: Weight Error (%g)", scat->weight);
        printf("\n\t comptonProbability = %g", comptonProbability);
        printf("\n\t scat_thick = %g", scat_thick);
        error_part(scat);
        return (FAIL);
    }
    // 6. Back project particle to the psdPlane
    // printf("\n psdDistance %f scat_thick %f (thick = %f)", psdDistance, scat_thick, thick);
    psdDistance -= scat_thick;
    // printf(" New PSD Distance = %f", psdDistance);
    scat->x = prim->x + scat->u / scat->w * (psdDistance);
    scat->y = prim->y + scat->v / scat->w * (psdDistance);
    //  scat->z = prim->z;
    //  printf("\n SCAT: %f %f %f %f %f %g", scat->x, scat->y, scat->u, scat->v, scat->energy, scat->weight);
    //
    //
    /* ************************************ Set parameters of the electron *********************** */
    // Electron weight is the comptonProbability
    electron->weight *= comptonProbability;
    electron->charge = -1;
    // electron->egs4Latch = prim->egs4Latch & twoToThe30th;
    electron->egs4Latch = twoToThe30th;
    //   int particleIsNegatron = ( ( (p->egs4Latch/twoToThe30th) % 2) == 1);
    //  int particleIsPositron = ( ( (p->egs4Latch/twoToThe29th) % 2) == 1);
    // electron->zlast = ;
    //
    // back project the electron to the PSD plane...
    electron->x = prim->x + electron->u / electron->w * (psdDistance);
    electron->y = prim->y + electron->v / electron->w * (psdDistance);
    float pathLengthThickness = scat_thick / electron->w;
    // for now, use No electron scattering AND assume energy loss = 20.6 MeV/cm //< JVS: 11/2/2011 ?? When was this added?? Guess this should effectively turn off electrons.....
    // and do electron energy loss...
    electron->energy -= pathLengthThickness * 20.6f;
    if (electron->energy <= 0.0f) {
        electron->energy = 0.0f;
        electron->weight = 0.0f;
    }
    //
    if (scat->w <= 0.0f) // return as particle going backwards
    {
        scat->weight = 0.0f; // play no further part in calcs
    }
    if (electron->w <= 0.0f) // return as particle going backwards
    {
        electron->weight = 0.0f; // play no further part in calcs
    }
    return (OK);
}
/* *********************************************************************** */
#ifdef C_CLASS
int particleDmlc::scatteredPhoton(particle_type* prim, particle_type* scat, 
                float thick, float mu, 
        // 11/08 table_type *E_mu, table_type *E_mu_Compt,
                float wt_not_int, float psdDistance)
#else

int scatteredPhoton(particle_type *prim, particle_type *scat,
                    float thick, float mu, table_type *E_mu, table_type *E_mu_Compt,
                    float wt_not_int, float psdDistance)
#endif
{
    // Same as PJK scattered_photon, but does not do the writing....
    // and back-projects particle by psdDistance...
    // Essentially assumption is that dmlc is infinitely thin layer.
    // Set particle to same position and charge
    /* copy prim particle to scattered */
    //dose_region = pinnacle_dose_region;
    memcpy(scat, prim, sizeof(particle_type));
    if (memcmp(scat, prim, sizeof(particle_type)) != 0) {
        printf("\n ERROR: copying memory\n");
        return (FAIL);
    }
    // 1. sample interation site of original particle
    float scat_thick;
    if (forced_interaction(thick, mu / prim->w, &scat_thick) != OK) {
        printf("\n ERROR: Forcing interaction");
        error_part(prim, scat);
        return (FAIL);
    }
    // 2. sample probability of photon energy and angle
    if (sample_KN(prim, scat) != OK) {
        printf("\n ERROR: Sampling KN");
        return (FAIL);
    }
    if (scat->energy <= E_mu->x[0]) //energy too low. Return
    {
        printf("\nEnergy of scattered particle (%f) too low", scat->energy);
        return (OK);
    }
    // 3. determine u, v, w. Return if w < 0
    if (rotateByRandomPhi(prim, scat) != OK) {
        printf("\n ERROR: Rotating particle");
        return (FAIL);
    }
    if (scat->w <= 0.0f) // return as particle going backwards
    {
        scat->weight = 0.0f; // play no further part in calcs
        return (OK);
    }
    // 4 determine mu_C/mu for original particle, and get mu for new particle
    float mu_c = table_value_extrap(prim->energy, E_mu_Compt);
    float muscat = table_value_extrap(scat->energy, E_mu);
    float muscat_over_w = muscat / scat->w;
    if ((muscat <= 0.0f) || (mu_c <= 0.0f) || (mu <= 0.0f) || (muscat_over_w <= 0.0f)) {
        printf("\n ERROR: Muscat %f wscat %f muc %f mu %f mu_over_w %f<= 0",
               muscat, scat->w, mu_c, mu, muscat_over_w);
        error_part(prim, scat);
        return (FAIL);
    }
    // 5. determine weight
    scat->weight *= wt_not_int * mu_c / mu * exp(-muscat_over_w * scat_thick);
    //printf("\n Scat wt in scat phot = %f ",scat->weight);
    if ((scat->weight > 1.0f) || (scat->weight < 0.0f)) {
        printf("\n ERROR: scatteredPhoton: Weight Error (%g)", scat->weight);
        printf("\n\t mu_c = %g", mu_c);
        printf("\n\t mu = %g", mu);
        printf("\n\t wt_not_int = %g", wt_not_int);
        error_part(scat);
        return (FAIL);
    }
    // 6. Back project particle to the psdPlane
    // printf("\n psdDistance %f scat_thick %f (thick = %f)", psdDistance, scat_thick, thick);
    psdDistance -= scat_thick;
    // printf(" New PSD Distance = %f", psdDistance);
    scat->x = prim->x + scat->u / scat->w * (psdDistance);
    scat->y = prim->y + scat->v / scat->w * (psdDistance);
    //  scat->z = prim->z;
    //  printf("\n SCAT: %f %f %f %f %f %g", scat->x, scat->y, scat->u, scat->v, scat->energy, scat->weight);
    return (OK);
}
/* *********************************************************************** */
/* *********************************************************************** */
#ifdef C_CLASS
// scattered_photon is not used anymore....thus, not even defined for C_CLASS
#else

int scattered_photon(ps_header_type *ps_hd, particle_type *prim, particle_type *scat,
                     FILE *ps, float thick, float mu, table_type *E_mu, table_type *E_mu_Compt,
                     int *count, float wt_not_int, russianRoulette *rr) {
    // Essentially assumption is that dmlc is infinitely thin layer. Set particle
    // to same position and charge
    /* copy prim particle to scattered */
    //dose_region = pinnacle_dose_region;
    memcpy(scat, prim, sizeof(particle_type));
    if (memcmp(scat, prim, sizeof(particle_type)) != 0) {
        printf("\n ERROR: copying memory\n");
        return (FAIL);
    }

    // 1. sample interation site of original particle
    float scat_thick;
    if (forced_interaction(thick, mu / prim->w, &scat_thick) != OK) {
        printf("\n ERROR: Forcing interaction");
        error_part(prim, scat);
        return (FAIL);
    }

    // 2. sample probability of photon energy and angle
    if (sample_KN(prim, scat) != OK) {
        printf("\n ERROR: Sampling KN");
        return (FAIL);
    }
    if (scat->energy <= E_mu->x[0]) //energy too low. Return
    {
        printf("\nEnergy of scattered particle (%f) too low", scat->energy);
        return (OK);
    }

    // 3. determine u, v, w. Return if w < 0
    if (rotateByRandomPhi(prim, scat) != OK) {
        printf("\n ERROR: Rotating particle");
        return (FAIL);
    }
    if (scat->w <= 0.0f) // return as particle going backwards
    {
        scat->weight = 0.0f; // play no further part in calcs
        return (OK);
    }

    // 4 determine mu_C/mu for original particle, and get mu for new particle
    float mu_c = table_value_extrap(prim->energy, E_mu_Compt);
    float muscat = table_value_extrap(scat->energy, E_mu);
    float muscat_over_w = muscat / scat->w;
    if ((muscat <= 0.0f) || (mu_c <= 0.0f) || (mu <= 0.0f) || (muscat_over_w <= 0.0f)) {
        printf("\n ERROR: Muscat %f wscat %f muc %f mu %f mu_over_w %f<= 0",
               muscat, scat->w, mu_c, mu, muscat_over_w);
        error_part(prim, scat);
        return (FAIL);
    }

    // 5. determine weight
    scat->weight *= wt_not_int * mu_c / mu * exp(-muscat_over_w * scat_thick);
    //printf("\n Scat wt in scat phot = %f ",scat->weight);
    if ((scat->weight > 1.0f) || (scat->weight < 0.0f)) {
        printf("\n ERROR: scattered photon weight = %g", scat->weight);
        error_part(scat);
        return (FAIL);
    }

    // write_particle
    int ret_val = write_part(ps_hd, scat, ps, rr);
    if (ret_val == NO_COUNT) {
        // don't increment counter
    } else if (ret_val != OK) {
        printf("\n ERROR: Writing scat part");
        error_part(prim, scat);
        return (FAIL);
    } else // ret_val OK
        (*count)++;

    return (OK);
}

#endif
/* *********************************************************************** */
#ifdef C_CLASS
int particleDmlc::forced_interaction(float thick, float mu_over_w, float *scat_thick )
#else

int forced_interaction(float thick, float mu_over_w, float *scat_thick)
#endif
{
    // Note: this returns the thickness still to be traversed by scattered photon
    //  double rn = (double) random()/RAND_MAX;
    double rn = (double) randomOne();
    // casting changed to double above and inside_log changed to double 7/12/2001: JVS

    double inside_log = 1.0 - rn * (1.0 - exp(-mu_over_w * thick));

    if (rn == 0.0) {
        *scat_thick = thick;
        return (OK);
    } else if ((rn == 1.0) || (inside_log <= 0.0)) {
        *scat_thick = 0.0f;
        return (OK);
    } else
        *scat_thick = thick + (float) log(inside_log) / mu_over_w;

    if ((*scat_thick < 0.0f) || (*scat_thick > thick)) {
        printf("\n ERROR: in forced_interaction, scat_thick = %g (mlc_thick = %g)", *scat_thick, thick);
        printf("\n ERROR: details.... inside_log = %g,  random_number=%g, mu_over_w=%g", inside_log, rn, mu_over_w);
        return (FAIL);
    }


    return (OK);
}
/* *********************************************************************** */
#ifdef C_CLASS
int particleDmlc::getComptonProducts(particle_type* prim, particle_type* scat, particle_type* electron)
#else

int getComptonProducts(particle_type *prim, particle_type *scat, particle_type *electron)
#endif
{
    // stolen shamelessly from Jim's code
    float eScaled_in = prim->energy / (float) EMASS;
    float r;
    float rPrime;
    float oneMinusR;   /* 1-r  */
    float oneOverR0;   /* 1/r0 */
    float alpha1, alpha2, alphaSum;
    float rnd0;
    float oneMinusCos;
    float sinSquared;
    //float momentumSquared;

    /* Compute parameters that depend only on e_in: */
    oneOverR0 = 1.0f + 2.0f * eScaled_in;
    alpha1 = log(oneOverR0);
    alpha2 = eScaled_in * (oneOverR0 + 1.0f) / (oneOverR0 * oneOverR0);
    alphaSum = alpha1 + alpha2;  /* Used repeatedly inside loop below */

    /* Loop until an interaction is accepted: */
    do {
        if (alpha1 >= ((double) randomOne()) * alphaSum) {
            /* Use 1/r part of distribution: */
            r = (float) exp(alpha1 * ((double) randomOne())) / oneOverR0;
        } else {
            /* Use linear (r) part of distribution: */
            rPrime = (float) ((double) randomOne());
            if (eScaled_in >= (eScaled_in + 1.0f) * ((float) randomOne())) {
                rnd0 = (float) ((double) randomOne());
                rPrime = MAX(rPrime, rnd0);
            }
            r = (1.0f + (oneOverR0 - 1.0f) * rPrime) / oneOverR0;
        }
        (scat->energy) = r * prim->energy;
        oneMinusR = 1.0f - r;
        oneMinusCos =(float)  EMASS * oneMinusR / (scat->energy);
        sinSquared = oneMinusCos * (2.0f - oneMinusCos);
        sinSquared = MAX( (float) ZERO, (float) sinSquared);
    } while (((double) randomOne()) > 1.0f - r * sinSquared / (1.0f + SQUARE(r)));

    /* Finish calculating scattering angle for photon: */
    scat->w = 1.0f - oneMinusCos;
    /* Finish calculating scattering angle for photon: */
    //     (*sinTheta_phot) = sqrt(sinSquared);

    /* Calculate energy and scattering angle for electron: */
    electron->energy = prim->energy - scat->energy;
    float momentumSquared = electron->energy * (electron->energy + (float) TWICE_EMASS);
    if (momentumSquared <= 0.0f) {
        /* Avoid division by zero: */
        electron->w = 0.0f;
        // (*sinTheta_elec) = -1.0;
    } else {
        electron->w = (electron->energy + (float) EMASS + scat->energy) *
                      oneMinusR / sqrt(momentumSquared);
        //        sinSquared = (1.0 - electron->w) * (1.0 + electron->w);
        //        sinSquared = MAX(ZERO, sinSquared);
        //        (*sinTheta_elec) = -sqrt(sinSquared);
    }

    return (OK);
}
/* *********************************************************************** */
#ifdef C_CLASS
int particleDmlc::sample_KN(particle_type* prim, particle_type* scat)
#else

int sample_KN(particle_type *prim, particle_type *scat)
#endif
{
    // stolen shamelessly from Jim's code
    float eScaled_in = prim->energy / (float) EMASS;
    float r;
    float rPrime;
    float oneMinusR;   /* 1-r  */
    float oneOverR0;   /* 1/r0 */
    float alpha1, alpha2, alphaSum;
    float rnd0;
    float oneMinusCos;
    float sinSquared;
    //float momentumSquared;

    /* Compute parameters that depend only on e_in: */
    oneOverR0 = 1.0f + 2.0f * eScaled_in;
    alpha1 = log(oneOverR0);
    alpha2 = eScaled_in * (oneOverR0 + 1.0f) / (oneOverR0 * oneOverR0);
    alphaSum = alpha1 + alpha2;  /* Used repeatedly inside loop below */

    /* Loop until an interaction is accepted: */
    do {
        if (alpha1 >= ((double) randomOne()) * alphaSum) {
            /* Use 1/r part of distribution: */
            r = (float) exp(alpha1 * ((double) randomOne())) / oneOverR0;
        } else {
            /* Use linear (r) part of distribution: */
            rPrime = (float) ((double) randomOne());
            if (eScaled_in >= (eScaled_in + 1.0) * ((float) randomOne())) {
                rnd0 = (float) ((double) randomOne());
                rPrime = MAX(rPrime, rnd0);
            }
            r = (1.0f + (oneOverR0 - 1.0f) * rPrime) / oneOverR0;
        }
        (scat->energy) = r * prim->energy;
        oneMinusR = 1.0f - r;
        oneMinusCos = (float) EMASS * oneMinusR / (scat->energy);
        sinSquared = oneMinusCos * (2.0f - oneMinusCos);
        sinSquared = MAX((float) ZERO, sinSquared);
    } while (((double) randomOne()) > 1.0 - r * sinSquared / (1.0 + SQUARE(r)));

    /* Finish calculating scattering angle for photon: */
    scat->w = 1.0f - oneMinusCos;

    return (OK);
}
/* *********************************************************************** */
#ifdef C_CLASS
int particleDmlc::rotateByRandomPhi(particle_type* prim, particle_type* scat)
#else

int rotateByRandomPhi(particle_type *prim, particle_type *scat)
#endif
{
    // based on t_calc_prim_tracks in SMC code
    // get alpha and ohm
    float alpha = (float) atan2(prim->v, prim->u);
    float ohm = (float) atan2(double(sqrt(prim->u * prim->u + prim->v * prim->v)),
                      double(sqrt(1.0 - (prim->u * prim->u +
                                         prim->v * prim->v))));
    float ca = cos(alpha);
    float sa = sin(alpha);
    float co = cos(ohm);
    float so = sin(ohm);

    if (scat->w < -1.0)
        scat->w = -1.0;
    else if (scat->w > 1.0)
        scat->w = 1.0;

    float theta = acos(scat->w);
    double phi = ((double) randomOne()) * TWOPI;
    float us = (float) cos(phi) * sin(theta);
    float vs = (float)  sin(phi) * sin(theta);
    float ws = scat->w; // = cos(theta)

    scat->u = ca * co * us - sa * vs + ca * so * ws;
    scat->v = co * sa * us + ca * vs + sa * so * ws;
    scat->w = -so * us + co * ws;

    if (fabs(scat->u * scat->u + scat->v * scat->v + scat->w * scat->w - 1.) > DOUBLE_LIMIT) {
        printf("\n ERROR: In rotateByRandomPhi uvw");
        error_part(prim, scat);
        return (FAIL);
    }

    return (OK);
}
/* *********************************************************************** */
/* *********************************************************************** */
#ifdef C_CLASS
int particleDmlc::rotateByRandomPhi(particle_type* prim, particle_type* scat, particle_type* electron)
#else

int rotateByRandomPhi(particle_type *prim, particle_type *scat, particle_type *electron)
#endif
{
    // based on t_calc_prim_tracks in SMC code
    // get alpha and ohm
    float alpha = atan2(prim->v, prim->u);
    float ohm = atan2(sqrt(prim->u * prim->u + prim->v * prim->v),
                      sqrt(1.0f - (prim->u * prim->u + prim->v * prim->v)));
    float ca = (float) cos(alpha);
    float sa = (float) sin(alpha);
    float co = (float) cos(ohm);
    float so = (float) sin(ohm);
    double phi = ((double) randomOne()) * TWOPI;

    // Scattered Photon
    if (scat->w < -1.0)
        scat->w = -1.0;
    else if (scat->w > 1.0)
        scat->w = 1.0;

    float theta = acos(scat->w);
    float us = (float) cos(phi) * sin(theta);
    float vs = (float) sin(phi) * sin(theta);
    float ws = scat->w; // = cos(theta)

    scat->u = ca * co * us - sa * vs + ca * so * ws;
    scat->v = co * sa * us + ca * vs + sa * so * ws;
    scat->w = -so * us + co * ws;

    if (fabs(scat->u * scat->u + scat->v * scat->v + scat->w * scat->w - 1.) > DOUBLE_LIMIT) {
        printf("\n ERROR: In rotateByRandomPhi uvw");
        error_part(prim, scat);
        return (FAIL);
    }
    // Scattered Electron (through same phi)
    if (electron->w < -1.0)
        electron->w = -1.0;
    else if (electron->w > 1.0)
        electron->w = 1.0;

    theta = acos(electron->w);
    us = (float) cos(phi) * sin(theta);
    vs = (float) ((float) (sin(phi) * sin(theta)));
    ws = electron->w; // = cos(theta)

    electron->u = ca * co * us - sa * vs + ca * so * ws;
    electron->v = co * sa * us + ca * vs + sa * so * ws;
    electron->w = -so * us + co * ws;

    if (fabs(electron->u * electron->u + electron->v * electron->v + electron->w * electron->w - 1.) > DOUBLE_LIMIT) {
        printf("\n ERROR: In rotateByRandomPhi uvw");
        error_part(prim, electron);
        return (FAIL);
    }

    return (OK);
}
/* ***************************************************************************** */
