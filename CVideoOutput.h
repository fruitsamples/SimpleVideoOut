/*
	File:		 CVideoOutput.h
	
	Description: CVideoOutput is an easy to use class which encapsulates the basic
				 set of methods required to use QuickTime Video Output Components.
				 
				 The most common use is playing DV streams (.dv files) out to FireWire devices.

	Author:		QuickTime Engineering
				
	Version:	2.0.2

	Copyright: 	� Copyright 2000-2002 Apple Computer, Inc. All rights reserved.
	
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
				
	Change History (most recent first): <4> 06/14/02 Begin now takes a boolean to control setting the echo port
										<3> 11/16/01 initial release version 2.0
										<2> 10/19/01 updated to support multiple components
										<1> 01/28/00 initial release

*/

/*	
	CVideoOutput( const unsigned char inClientNameStr[], const Movie inMovie = NULL )
		To instantiate a CVideoOutput object pass in a client name and a Movie, or don't pass
		in a Movie and call SetMovie() before calling Begin(). inMovie is set to NULL in the interface
		by default.
	
	Open( void )
		Opens an instance of a video output component, registers the client name with the component
		and sets up the display mode.
	
	Begin( Boolean inUseVOsdev = true, Boolean inUseVOClk = true, AudioRate inAudioRate = eAudioRateDefault, Boolean inChangeMovieGWorld = false )
		Gains exclusive access to the hardware, and sets up the sound output and clock associated with the
		video output component. Begin also acquires the GWorld used by the video output component.
		Both the sound and clock parameters are set to 'true' by default, the audio rate is set to "eAudioRateDefault"
		and Begin will not set the Video Output echo port or call SetMovieGWorld by default, allowing the client of this
		class to call SetEchoPort when needed.
	
	SetMovie( const Movie inMovie )
		Set's the Movie to be used by this class. CVideoOutput must have a valid movie before Begin() is called.
		
	SetEchoPort( const CGrafPtr inEchoPort = NULL )
		Allows you to display video both on an external video display and in a window.
		Pass in a CGrafPtr to specify a window to display video sent to the device. When the
		Echo Port is on, the movie is displayed in the window and sent to the video output device.
		If the EchoPort is not supported by the video output component or is turned off,
		the Movie will only directed to the video output components GWorld as you would expect.
		Call CanDoEchoPort() before assuming the video output component supports the echo port.
	
	SetSoundDevice( Boolean inUseVOsdev = true )
		This call will turn on/off the use of the video output components sound device. Passing in 'true'
		will set up the use of the video output components sound device, this is the default setting.
		You should call SetClock() after this call to choose the correct clock.
		
	SetClock( Boolean inUseVOClock = true )
		Allows you to choose which clock to use for audio / video sync. Passing in 'true' will choose
		the video output components clock to synchronize video and sound to the rate of the display.
		Passing in 'false' will use the default QuickTime clock.
		
		NOTE: Setting the Movie master clock to the video output clock MUST be done after
		setting up the sound device or it's gets whacked back to the default QuickTime clock.
		
	End( void )
		Relinquishes exclusive access to the hardware. Also called by Close().
	
	Close( void )
		Closes the component instance and zeros the object. It is also called by the destructor.
				
	GetGWorld( void )
		Returns a pointer to the graphics world used by a video output component.
		NOTE: Do NOT dispose this GWorld!
		
	GetError( void )
		Returns the last return code generated by the system.
		
	SelectVideoOutputComponent( void )
		Calls the CVideoOutputComponents DoSettingsDialog() method. Allows the client of this class to
		select which video output component and mode to use.
	
	Boolean CanDoEchoPort( void )
		Does this component support an EchoPort?
		
	Boolean HasSoundOutput( void )
		Does this component have a sound output component associated with it?
		
	Boolean HasClock( void )
		Does this component have a clock component associated with it?		
*/

#ifndef __CVIDEOOUTPUT_H__
	#define __CVIDEOOUTPUT_H__

#if __APPLE_CC__ || __MACH__
	#include <Carbon/Carbon.h>
	#include <QuickTime/QuickTime.h>
#else
	#include <Carbon.h>
	#include <QuickTimeComponents.h>
	#include <MediaHandlers.h>
#endif
#include <memory>

#include "GetFile.h"
#include "CVideoOutputComponent.h"

namespace dts {

const UInt8  kMaxAudioTracks = 5;
const UInt16 kQTVersion501 = 0x0501;

enum AudioRate {
	eAudioRate48khz = (long)0xBB800000, /* 48000.00000 in fixed-point */
	eAudioRate44khz = (long)0xAC440000, /* 44100.00000 in fixed-point */
	eAudioRate32khz = (long)0x7D000000, /* 32000.00000 in fixed-point */
	eAudioRateDefault = 0L				/* use default sampling rate of the media */
}; 

class CVideoOutput {
	public:
		explicit CVideoOutput( const unsigned char inClientNameStr[], const Movie inMovie = NULL );
		~CVideoOutput() { Close(); }
		
		OSErr Open( void );
		void  Close( void );		
		
		OSErr Begin( Boolean inUseVOsdev = true, Boolean inUseVOClock = true, AudioRate inAudioRate = eAudioRateDefault, Boolean inChangeMovieGWorld = false );
		void  End( void );		
		
		void  SetMovie( const Movie inMovie ) { if ( mVideoOutputInUse == false ) mMovie = inMovie; }
		OSErr SetEchoPort( const CGrafPtr inEchoPort = NULL );
		OSErr SetSoundDevice( Boolean inUseVOsdev = true );
		void  SetClock( Boolean inUseVOClock = true );
		
		const GWorldPtr GetGWorld( void ) const { if ( mVideoOutputInUse == true ) return mVOutputGWorld; else return NULL; }
		OSErr GetError( void ) const { return rc; }
		
		OSErr SelectVideoOutputComponent( void ) { return ( mVOutputComponent->DoSettingsDialog() ); }
	
		Boolean CanDoEchoPort( void ) const { return mCanDoEchoPort; }
		Boolean HasSoundOutput( void ) const { return mHasSoundOutput; }
		Boolean HasClock( void ) const { return mHasClock; }

	private:
		// nope
		CVideoOutput( const CVideoOutput &inVOObject );
		CVideoOutput operator=( CVideoOutput inVOObject );
		
	private:
		Str255					 mClientNameStr;
		Movie					 mMovie;
		CVideoOutputComponentPtr mVOutputComponent;	// auto_ptr object, deletion will be handled for us
		GWorldPtr				 mVOutputGWorld;
		Component				 mSoundOutComponent;
		ComponentInstance		 mVideoOutputClockInstance;
		MediaHandler 			 mAudioMediaHandler[kMaxAudioTracks];
		UInt8					 mNumberAudioTracks;
		Boolean					 mVideoOutputInUse;
		Boolean					 mCanDoEchoPort;
		Boolean					 mHasSoundOutput;
		Boolean					 mHasClock;
		UInt16					 mQTVersion;
		ComponentResult			 rc;
};

} // namespace

#endif // __CVIDEOOUTPUT_H__