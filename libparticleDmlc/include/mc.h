#ifndef MC_H
#define MC_H
/*=========================================================================== 

    File:     mc.h

    Date:     07-May-1999

    Purpose:  General header file for Nova MC.

 ============================================================================

    Programmer:   James C. Satterthwaite, Ph.D.
                  satter@pc016.radonc.nemc.org
                  617-636-0612

 ===========================================================================
    Modification History:
         Jan 7, 2000:pjk: uncommented then recommended 
                          EGS4_PATH_LENGTH_CORRECTION- worse results for
                          18 MeV electron beam in water
 ===========================================================================*/



/* User options:
====================================================================== */

#define SINGLE_PRECISION

#define inline inline

#define DEBUG
/*
#define RAYLEIGH_SCATTERING
#define SAUTER_PHOTOELECTRON_ANGLE
#define KOCH_MOTZ_BREMSSTRAHLUNG_ANGLE
#define MOTZ_OLSEN_KOCH_PAIR_ANGLE
#define BREMSSTRAHLUNG_SPLITTING
#define EGS4_PATH_LENGTH_CORRECTION
*/

#define BETHE_HEITLER_THRESHOLD          (4.14)  /* Set no lower than 4.14! */
#define NEGATIVE_STEP_WARNING_THRESHOLD  (-5.0e-4)



/* Standard header files:
====================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <time.h>


/* Implementation of chosen numerical precision:
====================================================================== */
 
typedef int Int;
typedef unsigned short UShrt;
typedef unsigned long  ULong;
 
/* Define appropriate typedef and macro substitutions: */
#if defined(SINGLE_PRECISION)
    typedef  float      Float;
    #define  FLOAT_EPS  FLT_EPSILON
    #define  FLOAT_MIN  FLT_MIN
    #define  FLOAT_MAX  FLT_MAX
    #define  qFLTq      " %e"
    #define  qcFLTq     ",%e"
#else
    /* Default to double precision: */
    typedef  double     Float;
    #define  FLOAT_EPS  DBL_EPSILON
    #define  FLOAT_MIN  DBL_MIN
    #define  FLOAT_MAX  DBL_MAX
    #define  qFLTq      " %le"
    #define  qcFLTq     ",%le"
#endif
 
#ifndef FILENAME_MAX
    #define FILENAME_MAX  (256)
#endif


/* Miscellaneous definitions:
====================================================================== */

/* Enumerations:
------------------------------------- */
enum {NO, YES, PENDING};
enum {X, Y, Z};


/* Symbolic constants:
------------------------------------- */
#ifdef PI
#undef PI       /* Suppresses compiler warnings about redefinition of PI */
#endif
#define  ZERO   (0.0)
#define  PI     (3.14159265358979324)
#define  TWOPI  (6.28318530717958648)
#define  PI5D2  (7.85398163397448310)
#define  EULER  (2.71828182845904523536)

/* Note: The source for the following constants is:
 *  E. Richard Cohen and Barry N. Taylor, "The Fundamental Physical
 *  Constants," Physics Today, August 1998 Buyers' Guide, pp. BG7-11. */
#define  JOULE_PER_MEV (1.60217733e-13)
#define  EMASS         (0.51099906)
#define  TWICE_EMASS   (1.02199812)
#define  EMASS_SQUARED (0.26112004)

#define  PEGSLINE_MAX  (81)   /* Maximum length of a PEGS data file line */
#define  MED_NAME_MAX  (24)   /* Maximum length of a medium name */
#define  VACUUM        (-1)   /* Alias for nonexistent medium array index */
#define  THE_VOID      (-1)   /* Alias for nonexistent region array index */


/* Computation macros:
------------------------------------- */
/* The following simple operations are in the form of macros because
 * type-checking by the compiler would be a hindrance rather than a help.
 * Expressions involving mathematical operations should not be passed
 * as arguments to these macros, because the expressions would here be
 * evaluated more than once. On the other hand, compilers seem to do better
 * with nested constructions such as d = MAX(c, MAX(b, a)) than with
 * unnested constructions such as d = MAX(b, a), d = MAX(c, d). */
#define  MAX(a, b)  ((a) > (b) ? (a) :  (b))
#ifndef MIN
#define  MIN(a, b)  ((a) < (b) ? (a) :  (b))
#endif
#define  ABS(a)     ((a) >= 0  ? (a) : -(a))
#define  SQUARE(a)  ((a)*(a))


/* Memory allocation macro:
------------------------------------- */
#define  ALLOCATE(len, type)  ((type *) calloc((size_t) len, sizeof(type)))


/* Input macros:
------------------------------------- */
/* The following macros provide rich diagnostics during input while keeping
 * the input routines as short as possible. The preprocessor's quotation
 * operator (#) is useful here, and therefore these macros have not been
 * turned into inline functions. */
#define READ(funcName, file, format, var)                                   \
    if (fscanf(file->stream, format, &var) != 1)                            \
    return Message(eREAD, funcName, #var, file->name);
#define READ_I(funcName, file, format, var, i)                              \
    if (fscanf(file->stream, format, &var[i]) != 1)                         \
    return Message(eREAD_I, funcName, #var, i, file->name);
#define READN(funcName, file, format, var)                                  \
    if (fscanf(file->stream, format, &var) != 1) {                          \
        Message(eREAD, funcName, #var, file->name);                         \
        return NULL;                                                        \
    }
#define READN_I(funcName, file, format, var, i)                             \
    if (fscanf(file->stream, format, &var[i]) != 1) {                       \
        Message(eREAD_I, funcName, #var, i, file->name);                    \
        return NULL;                                                        \
    }


/*  Data for Watch():
========================================= */

enum {NO_WATCH, WATCH_EVENTS, WATCH_ALL};
enum {  /* watchpoint indices */
    BEFORE_STEP,  EVENT_CUTOFF,  TRANSPORT_CUTOFF, ENTERING_VOID,
    USER_DISCARD, DURING_PHOTO, SCORING_CUTOFF,
    BEFORE_SHOWER,     AFTER_DISCARD,     NEW_MEDIUM,      AFTER_STEP,
    TIME_TO_INTERACT, AFTER_SHOWER,
    BEFORE_BREMS,    AFTER_BREMS,    BEFORE_MOLLER,   AFTER_MOLLER,
    BEFORE_BHABHA,   AFTER_BHABHA,   BEFORE_ANNIHF,   AFTER_ANNIHF,
    BEFORE_ANNIHR,   AFTER_ANNIHR,   BEFORE_PAIR,     AFTER_PAIR,
    BEFORE_COMPTON,  AFTER_COMPTON,  BEFORE_PHOTO,    AFTER_PHOTO,
    BEFORE_RAYLEIGH, AFTER_RAYLEIGH,
    N_WATCHPOINTS
};
/* Note: The enumeration begins at zero, so the final dummy element,
 * N_WATCHPOINTS, is automatically set to the number of elements in the
 * doWatch array. The dummy element SCORING_CUTOFF comes at the end of
 * the watchpoints at which energy deposition scoring is usually done. */


/* Note: The following array is used only in Watch(), but putting it
 * here, below the enumeration, facilitates program maintenance. */
#define INSERT_WATCH_STRINGS_HERE                                           \
static char* watchString[] = {                                              \
    "          Before step              :",                                 \
    " Discard: KE below event cutoff    :",                                 \
    " Discard: KE below transport cutoff:",                                 \
    " Discard: Particle enters the void: ",                                 \
    " Discard: requested by user        :",                                 \
    "      Photon energy deposited      :",                                 \
    " WARNING: Dummy watchpoint index!!!:",                                 \
    " Incident particle initiates shower:",                                 \
    "          Now on top of stack      :",                                 \
    "      Now in different medium      :",                                 \
    "          After step               :",                                 \
    "          Time to interact         :",                                 \
    "End of history%10ld  ****************************************\n\n",    \
    " Bremsstrahlung about to occur     :",                                 \
    "      Resulting photon or electron :",                                 \
    " Moller scattering about to occur  :",                                 \
    "           Resulting electron      :",                                 \
    " Bhabha scattering about to occur  :",                                 \
    "    Resulting electron or positron :",                                 \
    " Positron about to decay in flight :",                                 \
    "           Resulting photon        :",                                 \
    "   Positron about to decay at rest :",                                 \
    "           Resulting photon        :",                                 \
    " Pair production about to occur    :",                                 \
    "    Resulting electron or positron :",                                 \
    " Compton scattering about to occur :",                                 \
    "      Resulting electron or photon :",                                 \
    "Photoelectric effect about to occur:",                                 \
    "      Resulting electron or photon :",                                 \
    " Rayleigh scattering about to occur:",                                 \
    "       Photon with new direction   :",                                 \
    " WARNING: Dummy watchpoint index!!!:"                                  \
}


/*==========================================================================*/
#endif /* MC_H */
