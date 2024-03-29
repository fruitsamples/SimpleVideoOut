/*
	File:		CVideoOutput.cpp
	
	Description: An easy to use class which encapsulates the basic set of 
				 methods required to use QuickTime Video Output Components.
				 See CVideoOutput.h for more information.

	Author:		QuickTime DTS
				
	Version:	2.0.2

	Copyright: 	� Copyright 2000 - 2002 Apple Computer, Inc. All rights reserved.
	
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
				
	Change History (most recent first): <5> 06/12/02 don't call SetEchoPort in Begin by default
										<4> 05/27/02 don't leak SoundInfoList handle
										<3> 11/16/01 initial release version 2.0
										<2> 10/11/01 modified to support multiple components
										<1> 1/28/00 initial release
*/

#include "CVideoOutput.h"

using namespace dts;

/*	Check the result of GetError() before using the object.
	
	NOTE: This class will not throw any exceptions but does keep track of errors internally and will
	return errors from most methods. You should call the GetError() method before working with
	the object just to make sure things haven't failed miserably.
*/
CVideoOutput::CVideoOutput( const unsigned char inClientNameStr[], const Movie inMovie ) : mMovie(inMovie), mVOutputComponent(NULL), mVOutputGWorld(NULL),
																							mSoundOutComponent(NULL), mVideoOutputClockInstance(NULL),
																							 mNumberAudioTracks(0), mVideoOutputInUse(false), mCanDoEchoPort(false),
																							  mHasSoundOutput(false), mHasClock(false), rc(noErr)
{	
	// Instantiate the actual QuickTime VO Component object used by this class.
	// We could do this in the ctor init list, but we don't want any uncaught
	// exeptions getting back to the client of this class.
	try {
		CVideoOutputComponentPtr pVOComponent(new CVideoOutputComponent);
		mVOutputComponent = pVOComponent;
	}
	catch ( ... ) {
		rc = badComponentType;
		return;
	}
	
	// We really just need to know the version for the SetMovieVideoOutput call in QT5
	long version;
	if ( (rc = Gestalt( gestaltQuickTime, &version )) != noErr ) return;
	
	mQTVersion = version >> 16;

	BlockMoveData( inClientNameStr, mClientNameStr, inClientNameStr[0]+1 );
	
	for ( UInt8 i = 0; i < kMaxAudioTracks; i++ ) {
		mAudioMediaHandler[i] = NULL;
	}
}

#pragma mark-

/* Open( void )
		Opens an instance of a video output component, registers the client name with the component
		and sets up the display mode.
*/
OSErr CVideoOutput::Open( void )
{
	ComponentInstance theInstance = 0;
 
	if ( mVideoOutputInUse ) { rc = videoOutputInUseErr; goto bail; }
	if ( mVOutputComponent.get() == NULL ) { rc = badComponentType; goto bail; }
	
	// Specifically open a chosen video output component
	mVOutputComponent->OpenComponent();
	
	if (( theInstance = mVOutputComponent->GetComponentInstance() ) == NULL ) { rc = badComponentInstance; goto bail; }
	
	// Register your client name with the Video Output Component
	::QTVideoOutputSetClientName( theInstance, mClientNameStr );
	
	// Set the display mode
	rc = ::QTVideoOutputSetDisplayMode( theInstance, mVOutputComponent->GetDisplayMode() );
	if ( rc ) Close();
	
bail:
	return rc;
}

/* Close( void )
		Closes the component instance and zeros the object. It is also called by the destructor.
*/
void CVideoOutput::Close( void )
{	
	if ( mVideoOutputInUse )
		End();
	
	mVOutputComponent->CloseComponent();
}

#pragma mark-

/* Begin( Boolean inUseVOsdev = true, Boolean inUseVOClk = true, AudioRate inAudioRate = eAudioRateDefault )
		Gains exclusive access to the hardware, and sets up the sound output and clock associated with the
		video output component. Begin also acquires the GWorld used by the video output component.
		Both the sound and clock parameters are set to 'true' by default, and the
		audio rate is set to "eAudioRateDefault".
*/
OSErr CVideoOutput::Begin( Boolean inUseVOsdev, Boolean inUseVOClock, AudioRate inAudioRate, Boolean inChangeMovieGWorld )
{
	ComponentInstance theInstance = 0;
	UnsignedFixed	  theSampleRate = inAudioRate;
	
	if ( mMovie == NULL ) { rc = paramErr; goto bail; }
	if ( mVideoOutputInUse ) { rc = videoOutputInUseErr; goto bail; }
	if (( theInstance = mVOutputComponent->GetComponentInstance() ) == NULL ) { rc = badComponentInstance; goto bail; }
	
	// Find out how many tracks the movie contains, then for each track find out
	// which contain a sound media type and finally grab the media handler for those tracks
  {	// gcc complains without this in brackets
	long theTrackCount = ::GetMovieTrackCount( mMovie );
	for ( UInt8 i = 1; i < theTrackCount + 1; i++) {
		OSType aMediaType;
		
		Track aTrack = ::GetMovieIndTrack( mMovie, i );
		Media aMedia = ::GetTrackMedia( aTrack );		
		::GetMediaHandlerDescription( aMedia, &aMediaType, NULL, NULL );	
		
		if ( aMediaType == SoundMediaType ) {
			mAudioMediaHandler[mNumberAudioTracks] = ::GetMediaHandler( aMedia );
			mNumberAudioTracks++;
			
			// When the default audio sample rate has been requested, get the sample rate
			// the sound was originally captured at - this becomes the sample rate
			// passed to the output component
			// If there are multiple audio tracks pick the higest rate
			if ( inAudioRate == eAudioRateDefault ) {
				SoundDescriptionHandle hSoundDesc = (SoundDescriptionHandle)::NewHandle(0);
				if ( hSoundDesc ) {
					::GetMediaSampleDescription( aMedia, 1, (SampleDescriptionHandle)hSoundDesc );
					if ( theSampleRate < (**hSoundDesc).sampleRate )
						theSampleRate = (**hSoundDesc).sampleRate;
					::DisposeHandle( (Handle)hSoundDesc );
				}
			}
		}
		
		if ( mNumberAudioTracks == kMaxAudioTracks ) break;
	}
  }
	// Gain exclusive access to the video output hardware
	rc = ::QTVideoOutputBegin( theInstance );
	if ( rc ) goto bail;
	
	mVideoOutputInUse = true;
	
	// Does this Video Output Component implement an EchoPort?
	if ( ::ComponentFunctionImplemented( theInstance, kQTVideoOutputSetEchoPortSelect ) )
		mCanDoEchoPort = true;
	
	// Does this Video Output Component have a Sound Output Component associated with it?
	if ( ::ComponentFunctionImplemented( theInstance, kQTVideoOutputGetIndSoundOutputSelect ) ) {
		// Get the first sound output component associated with the video output component
		::QTVideoOutputGetIndSoundOutput( theInstance, 1, &mSoundOutComponent );
		if ( mSoundOutComponent ) {
			mHasSoundOutput = true;
			
			// Does the output component actually supports the chosen audio sample rate?
			// If it does just go ahead and use it. If not, the sample rate will be changed
			// to a valid rate
			SoundInfoList theInfoList;
			rc = ::GetSoundOutputInfo( mSoundOutComponent, siSampleRateAvailable, &theInfoList);
			if ( rc ) { Close(); goto bail; }
			
			UnsignedFixedPtr pRates = reinterpret_cast<UnsignedFixedPtr>( *(theInfoList.infoHandle) );
			UnsignedFixed tempRate = 0;
			for ( UInt8 i = 0; i < theInfoList.count; i++ ) {				
				tempRate = pRates[i];
				if ( tempRate == theSampleRate ) break;
			}
			DisposeHandle( theInfoList.infoHandle );
			
			if ( tempRate != theSampleRate )
				theSampleRate = tempRate;

			rc = ::SetSoundOutputInfo( mSoundOutComponent, siSampleRate, (void *)theSampleRate );
			if ( rc ) { Close(); goto bail; }
		}
	}
	
	// Does this Video Output Component have a Clock Component associated with it?
	if ( ::ComponentFunctionImplemented( theInstance, kQTVideoOutputGetClockSelect ) ) {
		// Get an instance of the clock component associated with the video output component - used to
		// synchronize video and sound to the rate of the display
		::QTVideoOutputGetClock( theInstance, &mVideoOutputClockInstance );
		if ( mVideoOutputClockInstance )
			mHasClock = true;
	}
	
	if ( mQTVersion >= kQTVersion501 ) {
		// Indicates to the ICM the video output component being used with the given movie
		// You should make this call so the ICM can keep track of the video output in use
		::SetMovieVideoOutput( mMovie, theInstance );
	}
	
	// Get a pointer to the graphics world used by a video output component
	rc = ::QTVideoOutputGetGWorld( theInstance, &mVOutputGWorld );
	if ( rc ) goto bail;
	
	// Set up the sound device
	SetSoundDevice( inUseVOsdev );
	
	// Set up the clock - needs to be called after setting up the sdev
	SetClock( inUseVOClock );
	
	// Don't call SetEchoPort by default as it could cause some problems when using the standard movie
	// controller and a video output component which doesn't support an echo port (rare but still...).
	// We just force the client of this class to call SetEchoPort() when they want, instead
	// of doing it for them, unless they specifically tell us that it's ok to call it.
	if ( inChangeMovieGWorld ) {
		// Set up the Movie GWorld and initially turn off the EchoPort.
		SetEchoPort( NULL );
	}
	
bail:	
	return rc;
}

/* End( void )
		Relinquishes exclusive access to the hardware. Also called by Close().
*/
void CVideoOutput::End( void )
{
	if ( mVideoOutputInUse ) {
		// Because the video output component disposes of the instance of the clock component which was returned to us
		// by the QTVideoOutputGetClock call in the Begin() method, we need to reset the clock for the movie to the default
		// QuickTime clock before calling QTVideoOutputEnd()

		SetSoundDevice( false );
		SetClock( false );
		
		if ( mQTVersion >= kQTVersion501 ) {
			// Set the vout parameter to NULL as soon as the video out component is no longer in use
			::SetMovieVideoOutput( mMovie, NULL );
		}
		
		::QTVideoOutputEnd( mVOutputComponent->GetComponentInstance() );
		
		mVideoOutputInUse = false;
	}
	
	// If the video output was in use, after the call to ::QTVideoOutputEnd() mVOutputGWorld is
	// no longer valid as the video output component automatically disposes of the
	// graphics world. If you need to use the GWorld after calling End(), you can call
	// CVideoOutput::GetGWorld() again but ONLY after the next time you call Begin() or it
	// will return NULL.
    // Egon's important safety tip - You must not call DisposeGWorld to dispose of the
    // graphics world used by a video output component...that would be bad.
	mVOutputGWorld = NULL;

	mSoundOutComponent = NULL;
	mVideoOutputClockInstance = NULL;
	
	mNumberAudioTracks = 0;
	
	mCanDoEchoPort = false;
	mHasSoundOutput = false;
	mHasClock = false;

	for ( UInt8 i = 0; i < kMaxAudioTracks; i++) {
		mAudioMediaHandler[i] = NULL;
	}
}

#pragma mark-

/* SetEchoPort( const CGrafPtr inEchoPort = NULL )
		Allows you to display video both on an external video display and in a window.
		Pass in a CGrafPtr to specify a window to display video sent to the device. When the
		Echo Port is on, the movie is displayed in the window and sent to the video output device.
		If the EchoPort is not supported by the video output component or is turned off,
		the Movie will be directed to the video output components GWorld as you would expect.
		Call CanDoEchoPort() before assuming the video output component supports the echo port.
*/
OSErr CVideoOutput::SetEchoPort( const CGrafPtr inEchoPort )
{
	ComponentInstance theInstance = 0;
	
	if ( mVideoOutputInUse == false ) return videoOutputInUseErr;
		
	if (( theInstance = mVOutputComponent->GetComponentInstance() ) == NULL ) { rc = badComponentInstance; goto bail; }
	
	if ( mCanDoEchoPort ) {
		if ( inEchoPort == NULL ) {
			// Turn off Echo Port
			rc = ::QTVideoOutputSetEchoPort( theInstance, (CGrafPtr)NULL );
			if ( rc == noErr ) {
				if ( mVideoOutputInUse ) {
					::SetMovieGWorld( mMovie, mVOutputGWorld, NULL );
				}
			}
		} else {
			// Turn on Echo Port		
			rc = ::QTVideoOutputSetEchoPort( theInstance, inEchoPort );
			if ( rc == noErr ) {
				if ( mVideoOutputInUse ) {						
					::SetMovieGWorld( mMovie, inEchoPort, NULL);
				}
			}
		}
	} else {
		// The Echo Port isn't supported by this component but
		// we still need to set the Movie GWorld correctly
		if ( inEchoPort == NULL ) {	
			::SetMovieGWorld( mMovie, mVOutputGWorld, NULL );
		} else {
			::SetMovieGWorld( mMovie, inEchoPort, NULL );
		}
	}

bail:
	return rc;
}

/* SetSoundDevice( Boolean inUseVOsdev = true )
		This call will turn on/off the use of the video output components sound device. Passing in 'true'
		will set up the use of the video output components sound device, this is the default setting.
		You should call SetClock() after this call to choose the correct clock.
*/
OSErr CVideoOutput::SetSoundDevice( Boolean inUseVOsdev )
{			
	if ( mVideoOutputInUse == false ) return videoOutputInUseErr;
	
	if ( mHasSoundOutput ) {
		if ( inUseVOsdev == true ) {
			for ( UInt8 i = 0;i < mNumberAudioTracks; i++ ) {
				rc = ::MediaSetSoundOutputComponent( mAudioMediaHandler[i], mSoundOutComponent );
				if ( rc ) goto bail;
			}
		} else {	
			for ( UInt8 i = 0;i < mNumberAudioTracks; i++ ) {
				rc = ::MediaSetSoundOutputComponent( mAudioMediaHandler[i], NULL );
				if( rc ) goto bail;
			}
		}
	}

bail:
	return rc;
}

/* SetClock( Boolean inUseVOClock = true )
		Allows you to choose which clock to use for audio / video sync. Passing in 'true' will choose
		the video output components clock to synchronize video and sound to the rate of the display.
		Passing in 'false' will use the default QuickTime clock.
		
		NOTE: Setting the Movie master clock to the video output clock MUST be done after
		setting up the sound device or it's gets whacked back to the default QuickTime clock.
*/
void CVideoOutput::SetClock( Boolean inUseVOClock )
{
	if ( mHasClock ) {
		if ( inUseVOClock == true ) {
			::SetMovieMasterClock( mMovie, (Component)mVideoOutputClockInstance, NULL );
		} else {
			::ChooseMovieClock( mMovie, 0 );
		}
	}
}