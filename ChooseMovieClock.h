/*
	File:		 ChooseMovieClock.h
	
	Description: Exposes ChooseMovieClock API currently not available in QuickTime 5.0.2
	             Call with m set to the movie and flags set to 0

	Author:		QuickTime Engineering

	Copyright: 	� Copyright 2001-2002 Apple Computer, Inc. All rights reserved.
	
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple�s
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Computer, Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.

				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.

				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
				
	Change History (most recent first): <2> 09/27/02 updated for CW8 not to be included on 10.2+
										<1> 10/19/01 initial release
*/

/*

 *** The ChooseMovieClock API ***

ChooseMovieClock will assign the default QuickTime Clock to a Movie.
When you want to reset a Movie's Clock, use the ChooseMovieClock API.

EXTERN_API( void ) ChooseMovieClock( Movie m, long flags );

m		The movie for this operation. Your application obtains this movie identifier from
		such functions as NewMovie, NewMovieFromFile, and NewMovieFromHandle.
flags 	Currently not used and must be set to 0

*/

/*  The QuickTime 6 SDK, a version of Universal Interfaces with the QT 6 APIs
    and the QuickTime Framework for QT 6 will make this .h file redundant --
    but until then, it's required.
*/
#ifndef AVAILABLE_MAC_OS_X_VERSION_10_2_AND_LATER
#ifndef __CHOOSEMOVIECLOCK__
#define __CHOOSEMOVIECLOCK__
#if __APPLE_CC__
#ifdef __cplusplus
	extern "C" {
#endif
#endif

// Call with m set to the Movie and flags set to 0
EXTERN_API( void ) ChooseMovieClock( Movie m, long flags );

#if __APPLE_CC__
#ifdef __cplusplus
	}
#endif
#endif
#endif
#endif