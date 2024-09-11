#ifndef MCRNG_H
#define MCRNG_H
/*=========================================================================== 

    File:     mcrng.h

    Date:     26-Mar-1999

    Purpose:  Header file for Nova MC random number generator (RNG)
              structure. Contains source code for random number
              function itself (inline static).

 ============================================================================

    Programmer:   James C. Satterthwaite, Ph.D.
                  satter@pc016.radonc.nemc.org
                  617-636-0612

 ===========================================================================*/


/* Structure declaration:
======================================== */
struct Rng {
    double  genArray[97];
    double* p96Gen;   /* Pointer to last element of generator array */
    double* pIGen;
    double* pJGen;
};


/* Constructor and destructor functions:
======================================== */

struct Rng* NewRng(struct File* const pFile);


void FreeRng(struct Rng** ppRng);


/*==========================================================================*/
#endif /* MCRNG_H */
