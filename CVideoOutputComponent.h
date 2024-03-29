/*
	File:		 CVideoOutputComponent.h
	
	Description: A wrapper class for Video Output Components, used and instantiated
	             by the CVideoOutput class. Abstracts the setting up of QuickTime Video
	             Output Components and provides a UI for choosing a specific component and mode.

	Author:		QuickTime DTS
				
	Version:	2.0.5

	Copyright: 	� Copyright 2001 Apple Computer, Inc. All rights reserved.
	
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
				
	Change History (most recent first): <1> 11/19/01 initial release

*/

#ifndef __CVIDEOOUTPUTCOMPONENT_H__
	#define __CVIDEOOUTPUTCOMPONENT_H__

#if __APPLE_CC__ || __MACH__
	#include <Carbon/Carbon.h>
	#include <QuickTime/QuickTime.h>
#else
	#include <Carbon.h>
	#include <QuickTimeComponents.h>
#endif
#include <memory>

#include "GetFile.h"

namespace dts {

const UInt8  kMaxNumberOfComponents = 10;
const UInt16 kComponentDialogResource = 5000;

const UInt8 kOKButtonItem = 1;
const UInt8 kComponentListPopUpDialogItem = 2;
const UInt8 kModeListPopUpDialogItem = 3;
const UInt8 kModeWidthItem = 4;
const UInt8 kModeHeightItem = 5;
const UInt8 kModeHResItem = 6;
const UInt8 kModeVRestItem = 7;
const UInt8 kModeRefreshRateItem = 8;
const UInt8 kModePixelTypeItem = 9;

const UInt32 kCommandComponentListPopUp = FOUR_CHAR_CODE('Itm1');
const UInt32 kCommandModeListPopUp = FOUR_CHAR_CODE('Itm2');

/*typedef struct {
	OSType					codecType; 		// specifies the type of compressed data
									   		// that the decompressor can decompress
	DecompressorComponent	codecComponent;	// optional - a decompressor component can be used to decompress
											// the data specified by the corresponding codecType
	Boolean					continuous;		// optional - specifies whether the resulting video display will be continuous
											// true = data will be displayed without any visual gaps between successive images
											// false = data will be displayed, but there may be a visual gap (such as a black screen)
											// between the display of images - if this atom doesn't exist you should not make any
											// assumptions about the performance of the decompressor
} DecompressorAtomRecord, *DecompressorAtomPtr;*/

typedef struct {
	long	width, height;	// pixels of the display
	Fixed	hRes, vRes; 	// pixels per inch
	Fixed	refreshRate;	// refresh rate
	OSType	pixelType;		// type of pixel used by display format:
							// 		1,2,4,8,16,24,32 :: standard bit depths
							//		33,34,36,40 :: gray-scale pixel depths
							//      of 1,2,4,8
	char	name[50];		// name of display mode
	// DecompressorAtomPtr pDecompressorAtom; // not used in this sample		
} DisplayModeAtomRecord, *DisplayModeAtomPtr;

typedef struct {
	Handle				hName;
	OSType		  		subType;
	UInt8				numberOfModes;
	DisplayModeAtomPtr	pDisplayModeList;
} ComponentListRecord, *ComponentListPtr;

class CVideoOutputComponent {
	public:
		CVideoOutputComponent() throw(ComponentResult);
		~CVideoOutputComponent();		
		
		OSErr OpenComponent(void);
		void  CloseComponent(void);
		
		OSErr DoSettingsDialog(void);

		const QTVideoOutputComponent GetComponentInstance(void) const { return mComponentInstance; }
		UInt8 GetDisplayMode(void) const { return mWhichModeIndex; }
		
	private:
		void UpdateModeListPopUp(UInt8 inValue);
		void UpdateDialogTextItems(void);
		void SetText(UInt8 inItem, Str255 inString);
		DisplayModeAtomPtr GetFirstLevelAtoms(QTAtomContainer container, UInt8 inNumberOfModes);
		void GetSecondLevelAtoms(DisplayModeAtomPtr myPtr, QTAtom parentAtom, QTAtomContainer container);

		friend pascal OSStatus DialogEventHandler(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
		
		// nope
		CVideoOutputComponent(const CVideoOutputComponent &inVOObject);
		CVideoOutputComponent operator=(CVideoOutputComponent inVOObject);

	private:
		Component				mComponent;
		QTVideoOutputComponent  mComponentInstance;
		ComponentListPtr		mComponentList;
		UInt8					mTotalNumOfComponents;
		UInt8					mWhichComponentIndex;
		UInt8					mWhichModeIndex;
		DialogRef				mDialogRef;
};

typedef std::auto_ptr<CVideoOutputComponent> CVideoOutputComponentPtr;

pascal OSStatus DialogEventHandler(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);

} // namespace

#endif // __CVIDEOOUTPUTCOMPONENT_H__