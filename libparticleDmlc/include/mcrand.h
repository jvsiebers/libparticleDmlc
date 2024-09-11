#ifndef MCRAND_H
#define MCRAND_H
/*=========================================================================== 

    File:     mcrand.h

    Date:     26-Mar-1999

    Purpose:  Header file for Nova MC random number generation.

 ============================================================================

    Programmer:   James C. Satterthwaite, Ph.D.
                  satter@pc016.radonc.nemc.org
                  617-636-0612

 ===========================================================================*/

#include "mcrng.h"


/* Declaration of inline static functions:
==========================================*/

inline static double RandomNumber(struct Rng* const rng);


/* Definition of inline static functions:
==========================================*/

inline static double RandomNumber(struct Rng* const rng)
/* Implementation of the RANMAR random number generator. 
 * References:
 *     F. James, Comp. Phys. Comm. 60 (1990), 329-44.
 *     F. James, "A review of pseudo-random number generators,"
 *         CERN Report SOFTWR 88-20 (1988).
 *     G. Marsaglia and A. Zaman, "Toward a universal random number
 *         generator," Florida State University Report FSU-SCRI-87-50 (1987).
 */
{
    static const double decrementOfDecrement =  7654321.0 / 16777216.0;
    static const double incrementOfDecrement = 16777213.0 / 16777216.0;
    static       double decrement            =   362436.0 / 16777216.0;
    double gen;
    double rnd;

    /* Compute a new generator: */
    gen = *(rng->pIGen) - *(rng->pJGen);
    if (gen < 0.0) gen += 1.0;

    /* Recompute the decrement: */
    decrement -= decrementOfDecrement;
    if (decrement < 0.0) decrement += incrementOfDecrement;

    /* Decrement the generator to obtain the random number: */
    rnd = gen - decrement;
    if (rnd < 0.0) rnd += 1.0;

    /* Save the generator and move the pointers for use during
     * the next function call: */
    *(rng->pIGen) = gen;
    if (--(rng->pIGen) < rng->genArray) rng->pIGen = rng->p96Gen;
    if (--(rng->pJGen) < rng->genArray) rng->pJGen = rng->p96Gen;

    return rnd;
}

/*==========================================================================*/
#endif /* MCRAND_H */
