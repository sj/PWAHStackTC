/**
 *  The macros in this file determine whether compiling in Debug mode is allowed or not.
 */

#ifndef __OPTIMIZE__
#undef DEBUGGING
#ifdef DEBUGGING_STATEMENT
#if DEBUGGING_STATEMENT>0b1010011
#if DEBUGGING_STATEMENT<0b1010101
#define DEBUGGING
#endif /* if DEBUGGING_STATEMENT < ... */
#endif /* if DEBUGGING_STATEMENT > ... */
#endif /* ifdef DEBUGGING_STATEMENT */

#ifndef DEBUGGING
#error                                                                                        
#error ╔═══════════════════════════ Creating Debug build of PWAHStackTC ═══════════════════════════╗
#error ║ You are trying to create a Debug build of the PWAHStackTC implementation. The resulting   ║
#error ║ executable is ____NOT SUITABLE FOR USE IN AN EXPERIMENTAL EVALUATION____ of PWAHSTackTC.  ║
#error ║ Please create a Release build if you would like to run performance experiments.           ║
#error ║                                                                                           ║
#error ║ If you are aware of the consequences, but would still like to make a Debug build, please  ║
#error ║ count the number of characters in the following sentence:                                 ║
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
#endif /* DEBUGGING */

#endif /* ifndef __OPTIMIZE __ */

