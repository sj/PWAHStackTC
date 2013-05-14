/**
 *  The macros in this file determine whether compiling in Debug mode is allowed or not.
 */

#ifndef __OPTIMIZE__
#define DEBUG_BUILD
#undef ALLOW_DEBUG_BUILD
#ifdef DEBUGGING_STATEMENT
#if DEBUGGING_STATEMENT>0b1000100
#if DEBUGGING_STATEMENT<0b1000110
#define ALLOW_DEBUG_BUILD /* ALLOW DEBUG BUILD */
#endif /* if DEBUGGING_STATEMENT < ... */
#endif /* if DEBUGGING_STATEMENT > ... */
#endif /* ifdef DEBUGGING_STATEMENT */

#ifndef ALLOW_DEBUG_BUILD
#error                                                                                        
#error ╔═══════════════════════════ Creating Debug build of PWAHStackTC ═══════════════════════════╗
#error ║ You are trying to create a Debug build of the PWAHStackTC implementation. The resulting   ║
#error ║ executable is ____NOT SUITABLE FOR USE IN AN EXPERIMENTAL EVALUATION____ of PWAHSTackTC.  ║
#error ║ Please create a Release build if you would like to run performance experiments.           ║
#error ║                                                                                           ║
#error ║ If you are aware of the consequences, but would still like to make a Debug build, please  ║
#error ║ count the number of characters (excluding spaces) in the following sentence:              ║
#error ║                                                                                           ║
#error ║ >  I am creating a debug build and will not use this build for experimental evaluation  < ║
#error ║                                                                                           ║
#error ║ Store this number in a new file named "DEBUGGING-I-KNOW-WHAT-IM-DOING" in the "Debug"     ║
#error ║ directory and restart the build process. Thanks!                                          ║
#error ╚═══════════════════════════════════════════════════════════════════════════════════════════╝
#else
#warning ╔═════════════════════════ Creating Debug build of PWAHStackTC ═════════════════════════╗
#warning ║ You are building PWAHStackTC in debugging mode. The resulting executable is NOT       ║
#warning ║ SUITABLE FOR USE IN AN EXPERIMENTAL EVALUATION!                                       ║
#warning ╚═══════════════════════════════════════════════════════════════════════════════════════╝
#endif /* ALLOW_DEBUG_BUILD */

#else  /* ifndef __OPTIMIZE__ */
#define NDEBUG
#endif /* ifndef __OPTIMIZE__ */

