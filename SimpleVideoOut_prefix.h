/* Metrowerks Standard Library
 * Copyright � 1995-2001 Metrowerks Corporation.  All rights reserved.
 *
 * $Date: 2004/10/05 00:04:53 $
 * $Revision: 1.1 $
 */

#ifndef _MSL_ANSI_PREFIX_MAC_H
#define _MSL_ANSI_PREFIX_MAC_H

#include <os_enum.h>
#define __dest_os __mac_os

/*
	JWW - You can change the define of _MWMT to control the behavior of how memory is allocated
	at the system level.  When defined to 0, the traditional NewPtr/DisposePtr toolbox calls are
	used to request memory from the system.  When defined to 1, the multiprocessing
	MPAllocateAligned/MPFree toolbox calls are used to request memory.  Note that the OS must
	have MP 2.0 or later in order for MPAllocateAligned to be present.
*/

#ifndef _MWMT
	#define _MWMT 0				/*- JWW 010426 -*/
#endif

#ifndef _MSL_IMP_EXP    		/*- cc 000315 -*/  
	#define _MSL_IMP_EXP 
#endif 

#define _POSIX

/*
	JWW - You can comment out either of the following two defines to limit the MSL library
	to using only one style of the file system APIs or the other.
	
	When _MSL_USE_OLD_FILE_APIS is 1 (but _MSL_USE_NEW_FILE_APIS is 0), MSL operates exactly
	the same way it always has since MSL first shipped.
	
	When _MSL_USE_NEW_FILE_APIS is 1 (but _MSL_USE_OLD_FILE_APIS is 0), MSL uses the new
	calls introduced in OS 9 to access the file system.  This means you get access to filenames
	longer than 32 characters and files greater than 2GB.  You must be careful to not use this
	configuration on a system which does not support the new APIs since no test is done to
	see if the file system routines are actually present before using them.
	
	When both _MSL_USE_NEW_FILE_APIS and _MSL_USE_OLD_FILE_APIS are 1, MSL tests the system
	to determine if the enhanced file system APIs are available, and if so it uses them.  If not,
	it falls back to the traditional method of accessing files.  This increases the library size
	since twice the amount of file system code is necessary, but you get the safety of knowing
	your code will operate properly on older systems.
	
	It is an error for both _MSL_USE_NEW_FILE_APIS and _MSL_USE_OLD_FILE_APIS to be 0.
*/

#ifndef _MSL_USE_OLD_FILE_APIS
	#define _MSL_USE_OLD_FILE_APIS 1
#endif

#ifndef _MSL_USE_NEW_FILE_APIS
	#define _MSL_USE_NEW_FILE_APIS 1
#endif

#if _MSL_USE_NEW_FILE_APIS && (!defined(__POWERPC__))
	/* JWW - _MSL_USE_NEW_FILE_APIS cannot be used with 68K targets */
	#undef _MSL_USE_NEW_FILE_APIS
	#define _MSL_USE_NEW_FILE_APIS 0
#endif

#if _MSL_USE_OLD_FILE_APIS && _MSL_USE_NEW_FILE_APIS
	#define _MSL_USE_OLD_AND_NEW_FILE_APIS 1
#elif _MSL_USE_OLD_FILE_APIS || _MSL_USE_NEW_FILE_APIS
	#define _MSL_USE_OLD_AND_NEW_FILE_APIS 0
#else
	#error At least one of _MSL_USE_OLD_FILE_APIS or _MSL_USE_NEW_FILE_APIS must be on!
#endif

#define __MSL_LONGLONG_SUPPORT__
/* #define _MSL_MALLOC_0_RETURNS_NON_NULL */
/*
	Turn on _MSL_OS_DIRECT_MALLOC for a malloc alternative that simply goes
	straight to the OS with	no pooling.  Recompile the C lib when flipping
	this switch.  This will typically cause poorer performance, but may be of
	help when debugging memory problems. */
/* #define _MSL_OS_DIRECT_MALLOC */
/* #define _MSL_PRO4_MALLOC */

/* Turn on and off namespace std here */
#if defined(__cplusplus) && __embedded_cplusplus == 0
    #define _MSL_USING_NAMESPACE
	/* Turn on support for wchar_t as a built in type */
	/* #pragma wchar_type on */   /*  vss  not implemented yet  */
#endif

/*- hh 980217 

	__ANSI_OVERLOAD__ controls whether or not the prototypes in the C++ standard
	section 26.5 get added to <cmath> and <math.h> or not.  If __ANSI_OVERLOAD__
	is defined, and a C++ compiler is used, then these functions are available,
	otherwise not.
	
	There is one exception to the above rule:  double abs(double); is available
	in <cmath> and <math.h> if the C++ compiler is used.  __ANSI_OVERLOAD__ has
	no effect on the availability of this one function.

	There is no need to recompile the C or C++ libs when this switch is flipped.

	If _MSL_INTEGRAL_MATH is defined then in addition to the prototypes added by
	__ANSI_OVERLOAD__, there are also non-standard integral versions of these
	prototypes added as well.  This is to allow client code to put integral arguments
	into math functions, and avoid ambiguous call errors.
*/

#define __ANSI_OVERLOAD__  /*- hh 990201 -*/
#define _MSL_INTEGRAL_MATH

/*
For MacHeaders
*/

/*if you are using PP or custom precompiled headers please 
	1) set MSL_USE_PRECOMPILED_HEADERS to 0
	2) make sure OLDROUTINENAMES, OLDROUTINELOCATIONS are set to false before 
	   MacHeaders is precompiled or parts of Universal Headers are included
*/

/*- hh 980727 -*/

#ifndef OLDROUTINENAMES
	#define OLDROUTINENAMES 0
#endif

#ifndef OLDROUTINELOCATIONS
	#define OLDROUTINELOCATIONS 0
#endif

/*
#define _MWMT 1
*/

#ifndef	MSL_USE_PRECOMPILED_HEADERS 
#define MSL_USE_PRECOMPILED_HEADERS	0	/*Set to have ansi_prefix include some form of MacHeaders*/
#endif 

#if (MSL_USE_PRECOMPILED_HEADERS == 1 )
	#ifndef USE_PRECOMPILED_MAC_HEADERS
		#define	USE_PRECOMPILED_MAC_HEADERS	1  /*Set to 0 if you don't want to use precompiled MacHeaders*/
	#endif

	#if (USE_PRECOMPILED_MAC_HEADERS != 1 )
		#include 	<MacHeaders.c>
	#else
		#if __POWERPC__
			#ifdef __cplusplus
				#include <MacHeadersPPC++>
			#else
				#include <MacHeadersPPC>
			#endif
		#endif
	#endif

#endif /*MSL_USE_PRECOMPILED_HEADERS*/

/*
For ZoneRanger
	If you want malloc to register its pools with ZoneRanger, add ZoneRanger.c
	to your project.  ZoneRanger support is no longer part of MSL.
*/

/*
For DebugNew

NB: this assumes that the only placement versions of new are in the following files:
	mmemory.h, new.h, and bstring.h. DebugNew does not currently work with the 
	placement versions of operator new

1) 	add DebugNew.cp to your project

2) 	add this to New.cp
	#ifdef DebugNew_H
	#undef new
	#endif

3) to zap memory, add gDebugNewFlags |= dnDontFreeBlocks; right before your problem code

4) put DebugNewForgetLeaks() underneath this

5) put DebugNewReportLeaks() at the end of your problem code

6) uncomment lines below

7) run
*/

/*to activate debug new, uncomment the following lines*/
/*
#define DEBUG_NEW	2
#include <DebugNew.h>
#ifdef DebugNew_H
	#define new NEW
#endif
*/

#endif /*	_MSL_ANSI_PREFIX_MAC_H	  */

/*#pragma once on*/
/* Switching this pragma on, can improve compilation speed but it may cause problems with the
   use of assert.h, which generally depends upon being actually included more than once during
   compilation. */

/* Change record:
 * mm  970110 Changed wrapper for long long support
 * hh  980727 Wrapped OLDROUTINENAMES and OLDROUTINELOCATIONS to prevent changing previously defined values.
 * mf  980811 commented out #define __ANSI_OVERLOAD__ 
 * hh  990201 turned __ANSI_OVERLOAD__ on because we now have foo(int) support
 * hh  990227 Added flag for malloc - ZoneRanger cooperation
 * hh  000302 Moved the namespace flag to here from mslGlobals.h
 * cc  000315 added _MSL_IMP_EXP
 * JWW 001030 Added _MSL_USE_OLD_FILE_APIS and _MSL_USE_NEW_FILE_APIS definitions
 * JWW 010426 Added _MWMT for using Multiprocessing for obtaining memory
 */