/*
   Oct 27, 2015: JVS: 
*/
#ifndef compilerWorkarounds_H
#define compilerWorkarounds_H
//
// Several versions of the SUN compiler (5.8) have not implemented __FUNCTION__
#if (__SUNPRO_CC <= 0x580)
#define __FUNCTION__ __func__
//
#endif 
//#ifdef __SUNPRO_CC
//#define __FUNCTION__ __func__
//#endif
//
#endif
//
