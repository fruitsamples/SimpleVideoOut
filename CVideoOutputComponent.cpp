/*
	File:		 CVideoOutputComponent.cpp
	
	Description: A wrapper class for Video Output Components, used and instantiated
	             by the CVideoOutput class. Abstracts the setting up of QuickTime Video
	             Output Components and provides a UI for choosing a specific component and mode.

	Author:		QuickTime DTS

	Version:	2.0.6

	Copyright: 	� Copyright 2001-2005 Apple Computer, Inc. All rights reserved.
	
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
				
	Change History (most recent first): <5> 07/29/05 added endian macros for mode data
										<4> 10/02/04 check return code from QTVideoOutputGetDisplayModeList
										<3> 09/26/02 for CW 8.2 MSL remove calls to num2dec and dec2str
										<2> 06/ 4/02 don't crash if VOut has a bad mode list
										<1> 10/19/01 initial release
*/

#include "CVideoOutputComponent.h"
#include <cstring>
#include <cstdio>

using namespace dts;

#ifdef _CSTD
	using _CSTD::strncpy;
	using _CSTD::sprintf;
#endif

CVideoOutputComponent::CVideoOutputComponent() throw(ComponentResult): mComponent(0), mComponentInstance(0), mComponentList(NULL), mTotalNumOfComponents(0),
															            mWhichComponentIndex(1), mWhichModeIndex(1), mDialogRef(NULL)
{
	ComponentDescription cd = {QTVideoOutputComponentType, 0, 0, 0L, kQTVideoOutputDontDisplayToUser};
	QTAtomContainer 	 modeListAtomContainer = NULL;
	UInt8 				 componentIndex = 0;
	ComponentResult		 rc = badComponentType;
	
	mComponentList = (ComponentListPtr)::NewPtrClear(sizeof(ComponentListRecord) * kMaxNumberOfComponents);
	if (NULL == mComponentList) throw (rc = ::MemError());
	
	while ((mComponent = ::FindNextComponent(mComponent, &cd))) {
		if (componentIndex < kMaxNumberOfComponents) {
			ComponentDescription cInfo;
			Handle hComponentName = ::NewHandle(0);
			if (NULL == hComponentName) throw (rc = ::MemError());
			
			::GetComponentInfo(mComponent, &cInfo, hComponentName, NULL, NULL);
			mComponentList[componentIndex].hName = hComponentName;
			mComponentList[componentIndex].subType = cInfo.componentSubType;
			
			rc = ::QTVideoOutputGetDisplayModeList((ComponentInstance)mComponent, &modeListAtomContainer);
			if (noErr == rc && NULL != modeListAtomContainer) {
				mComponentList[componentIndex].numberOfModes =::QTCountChildrenOfType(modeListAtomContainer, kParentAtomIsContainer, kQTVODisplayModeItem); 
				mComponentList[componentIndex].pDisplayModeList = GetFirstLevelAtoms(modeListAtomContainer, mComponentList[componentIndex].numberOfModes);
				::QTDisposeAtomContainer(modeListAtomContainer);
				modeListAtomContainer = NULL;
			} else {
				// This VOut component has no mode list !! BAD !!
				mComponentList[componentIndex].numberOfModes = 0;
				mComponentList[componentIndex].pDisplayModeList = (DisplayModeAtomPtr)NewPtrClear(sizeof(DisplayModeAtomRecord));
				mComponentList[componentIndex].pDisplayModeList->pixelType = 'BAD ';
			}
			componentIndex++;
		}
    }
    
    mTotalNumOfComponents = componentIndex;
    
    // We have at least one Video Output Component available
    // Select the first one as a default
    try { 
	    if (mTotalNumOfComponents) {
			cd.componentSubType = mComponentList[mWhichComponentIndex-1].subType;
			
			mComponent = ::FindNextComponent(0, &cd);
			if (0 == mComponent) throw (rc);
	   	} else {
			throw (rc);
		}
	}
	catch (ComponentResult) {
		this->~CVideoOutputComponent();
		throw;
	}	
}

CVideoOutputComponent::~CVideoOutputComponent()
{
	UInt8 componentIndex;
	
	CloseComponent();
	
	if (mComponentList) {
	    for (componentIndex = 0; componentIndex < mTotalNumOfComponents; componentIndex++) {
	    	if (mComponentList[componentIndex].hName)
	    		::DisposeHandle(mComponentList[componentIndex].hName);
	    	if (mComponentList[componentIndex].pDisplayModeList)
	    		::DisposePtr((Ptr)mComponentList[componentIndex].pDisplayModeList);
	    }
	    
	    ::DisposePtr((Ptr)mComponentList);
	}
}

#pragma mark-

OSErr CVideoOutputComponent::OpenComponent(void)
{
	OSErr err = noErr;
	
	CloseComponent();
	
	mComponentInstance = ::OpenComponent(mComponent);
		
	if (NULL == mComponentInstance)
		err = badComponentType;
		
	return err;
}

void CVideoOutputComponent::CloseComponent(void)
{
	if (mComponentInstance) {
		::CloseComponent(mComponentInstance);
		mComponentInstance = NULL;
	}
}

OSErr CVideoOutputComponent::DoSettingsDialog(void)
{
	ControlRef	  componentListControlRef = NULL,
			      modeListControlRef = NULL,
			      okButtonControlRef = NULL;
	MenuRef		  componentListMenuRef = NULL,
				  modeListMenuRef = NULL;
    EventTypeSpec theEventTypes[] = {{kEventClassCommand, kEventCommandProcess}};
    GrafPtr		  savedPort;
	OSErr 		  err;
    
	EventHandlerUPP theEventHandlerUPP = ::NewEventHandlerUPP(dts::DialogEventHandler);
	
	mDialogRef = ::GetNewDialog(kComponentDialogResource, NULL, (WindowPtr)-1);
	if (NULL == mDialogRef) { err = ::ResError(); goto bail; }
	
	::GetPort(&savedPort);
	::SetPortDialogPort(mDialogRef);

	err = ::GetDialogItemAsControl(mDialogRef, kComponentListPopUpDialogItem, &componentListControlRef);
	if (err) goto bail;
	
	::SetControlMaximum(componentListControlRef, kMaxNumberOfComponents);
	componentListMenuRef = ::GetControlPopupMenuHandle(componentListControlRef);
	if (NULL == componentListMenuRef) { err = menuNotFoundErr; goto bail; }
	
	err = ::GetDialogItemAsControl(mDialogRef, kModeListPopUpDialogItem, &modeListControlRef);
	if (err) goto bail;
	
	::SetControlMaximum(modeListControlRef, kMaxNumberOfComponents);
	modeListMenuRef = ::GetControlPopupMenuHandle(modeListControlRef);
	if (NULL == modeListMenuRef) { err = menuNotFoundErr; goto bail; }
	
	err = ::GetDialogItemAsControl(mDialogRef, kOKButtonItem, &okButtonControlRef);
	if (err) goto bail;
	
	::SetControlCommandID(componentListControlRef, kCommandComponentListPopUp);
    ::SetControlCommandID(modeListControlRef, kCommandModeListPopUp);
    ::SetControlCommandID(okButtonControlRef, kHICommandOK);
	
	Str255 thePStr;
    UInt8  componentIndex, modeIndex;
    for (componentIndex = 0; componentIndex < mTotalNumOfComponents; componentIndex++) {
		::AppendMenu(componentListMenuRef, (unsigned char *)*mComponentList[componentIndex].hName);
	}
	
	for (modeIndex = 0; modeIndex < mComponentList[mWhichComponentIndex-1].numberOfModes; modeIndex++) {
		::c2pstrcpy(thePStr, (mComponentList[mWhichComponentIndex-1]).pDisplayModeList[modeIndex].name);		
		::AppendMenu(modeListMenuRef, thePStr);
	}
    
    ::SetControlValue(componentListControlRef, mWhichComponentIndex);
    ::SetControlValue(modeListControlRef, mWhichModeIndex);
    ::SetWindowDefaultButton(::GetDialogWindow(mDialogRef), okButtonControlRef);
    
    ::InstallStandardEventHandler(::GetWindowEventTarget(::GetDialogWindow(mDialogRef)));
    ::InstallWindowEventHandler(::GetDialogWindow(mDialogRef), theEventHandlerUPP, GetEventTypeCount(theEventTypes), theEventTypes, this, NULL);
    
    UpdateDialogTextItems();
    ::TransitionWindow(GetDialogWindow(mDialogRef), kWindowZoomTransitionEffect, kWindowShowTransitionAction, NULL);
		
	::RunAppModalLoopForWindow(GetDialogWindow(mDialogRef));
	
	// Apply the new selection
    { // gcc complains without this in brackets
	ComponentDescription cd = {QTVideoOutputComponentType, mComponentList[mWhichComponentIndex-1].subType, 0, 0L, kQTVideoOutputDontDisplayToUser};
	mComponent = ::FindNextComponent(0, &cd);
	if (0 == mComponent) err = badComponentType;
    }
  
bail:
	if (mDialogRef) {
		::TransitionWindow(GetDialogWindow(mDialogRef), kWindowZoomTransitionEffect, kWindowHideTransitionAction, NULL);
		::DisposeDialog(mDialogRef);
	}	
	mDialogRef = NULL;
	
	if (theEventHandlerUPP) ::DisposeEventHandlerUPP(theEventHandlerUPP);
	
	::SetPort(savedPort);
	
	return err; 
}

#pragma mark-

void CVideoOutputComponent::UpdateModeListPopUp(UInt8 inValue)
{
	ControlRef modeListControlRef;
	MenuRef	   modeListMenuRef;
	UInt8	   modeIndex;
	Str255 	   thePStr;

	::GetDialogItemAsControl(mDialogRef, kModeListPopUpDialogItem, &modeListControlRef);	
	modeListMenuRef = ::GetControlPopupMenuHandle(modeListControlRef);
	::SetControlValue(modeListControlRef, 1);
	
	for (modeIndex = 0; modeIndex < mComponentList[mWhichComponentIndex-1].numberOfModes; modeIndex++) {
		::DeleteMenuItem(modeListMenuRef, 1);
	}
	
	for (modeIndex = 0; modeIndex < mComponentList[inValue-1].numberOfModes; modeIndex++) {
		::c2pstrcpy(thePStr, (mComponentList[inValue-1]).pDisplayModeList[modeIndex].name);		
		::AppendMenu(modeListMenuRef, thePStr);
	}
	
	::DrawOneControl(modeListControlRef);
}

void CVideoOutputComponent::UpdateDialogTextItems(void)
{
	Str255  thePStr;
	char    tempStr[8];
	double  tempDbl;
	
    DisplayModeAtomPtr pModeListData = mComponentList[mWhichComponentIndex-1].pDisplayModeList;
    
	// Display atom info in fields
	NumToString(pModeListData[mWhichModeIndex-1].width, thePStr);
	SetText(kModeWidthItem, thePStr);

	NumToString(pModeListData[mWhichModeIndex-1].height, thePStr);
	SetText(kModeHeightItem, thePStr);

	NumToString(Fix2Long(pModeListData[mWhichModeIndex-1].hRes), thePStr);
	SetText(kModeHResItem, thePStr);

	NumToString(Fix2Long(pModeListData[mWhichModeIndex-1].vRes), thePStr);
	SetText(kModeVRestItem, thePStr);
	
	tempDbl = Fix2X(pModeListData[mWhichModeIndex-1].refreshRate);
	sprintf(tempStr, "%.2lf", tempDbl);
	CopyCStringToPascal(tempStr, thePStr);
	SetText(kModeRefreshRateItem, thePStr);

	// Values of 1, 2, 4, 8, 16, 24 and 32 specify standard Mac OS RGB pixel formats
	// with corresponding bit depths. 
    // Values of 33, 34, 36 and 40 specify standard Mac OS gray-scale pixel formats with
    // depths of 1, 2, 4, and 8 bits per pixel. 
    // Other pixel formats are specified by four-character codes.
	if (pModeListData[mWhichModeIndex-1].pixelType <= 40) {
		NumToString(pModeListData[mWhichModeIndex-1].pixelType, thePStr);
		SetText(kModePixelTypeItem, thePStr);
	} else {
		char thePixelType[5] = { 0x04 };
		*(OSType *)&thePixelType[1] = EndianU32_NtoB(pModeListData[mWhichModeIndex-1].pixelType);
		SetText(kModePixelTypeItem, (unsigned char *)&thePixelType[0]);
	}
}

void CVideoOutputComponent::SetText(UInt8 inItem, Str255 inString)
{
	ControlRef theControl;

	::GetDialogItemAsControl(mDialogRef, inItem, &theControl);
	::SetDialogItemText((Handle)theControl, inString);
}

// First level:
//		Children of the container are of type:
//		kQTVODisplayModeItem
// At the root of the QT atom container returned by QTVideoOutputGetDisplayModeList()
// are one or more atoms of type kQTVODisplayModeItem
DisplayModeAtomPtr CVideoOutputComponent::GetFirstLevelAtoms(QTAtomContainer container, UInt8 inNumberOfModes)
{
	DisplayModeAtomPtr pModeListData = NULL;
	QTAtom 			   atomDisplay = 0, nextAtomDisplay = 0;
	UInt8			   i = 0;
	QTAtomType		   type;
	QTAtomID		   id;
	OSErr			   err;
	
	// Allocate display structs to hold info found in container
	pModeListData = (DisplayModeAtomPtr)NewPtrClear(sizeof(DisplayModeAtomRecord) * inNumberOfModes);
	
	while (i < inNumberOfModes) {
	
		err = QTNextChildAnyType(container, kParentAtomIsContainer, atomDisplay, &nextAtomDisplay);
										
		// Make sure its a display atom
		err = QTGetAtomTypeAndID(container, nextAtomDisplay, &type, &id);
		if (type != kQTVODisplayModeItem) continue;														
		
		// Get children of this display atom										
		atomDisplay = nextAtomDisplay;										
		GetSecondLevelAtoms(&pModeListData[i], atomDisplay, container);
		i++;
	}
	
	return pModeListData;
}

// Second level:
//		Children of the kQTVODisplayModeItem atom are of type:
// 		kQTVODimensions
//		kQTVOResolution
//		kQTVORefreshRate
//		kQTVOPixelType
//		kQTVOName
// 		kQTVODecompressors atom(s)
void CVideoOutputComponent::GetSecondLevelAtoms(DisplayModeAtomPtr myPtr, QTAtom parentAtom, QTAtomContainer container)
{
	QTAtom atom;
	long   dataSize, *dataPtr;
	OSErr  err;
						
	// ******************* kQTVODimensions
	atom = QTFindChildByID(container, parentAtom, kQTVODimensions, 1, NULL);
	err = QTGetAtomDataPtr(container, atom, &dataSize, (Ptr *)&dataPtr);
	if (noErr == err) {
		myPtr->width  = EndianS32_BtoN(dataPtr[0]);
		myPtr->height = EndianS32_BtoN(dataPtr[1]);
	}		

	// ******************* kQTVOResolution
	atom = QTFindChildByID( container, parentAtom, kQTVOResolution, 1, NULL);
	err = QTGetAtomDataPtr( container, atom, &dataSize, (Ptr *)&dataPtr);
	if (noErr == err) {
		myPtr->hRes = EndianS32_BtoN(dataPtr[0]);
		myPtr->vRes = EndianS32_BtoN(dataPtr[1]);
	}		

	// ******************* kQTVORefreshRate
	atom = QTFindChildByID(container, parentAtom, kQTVORefreshRate, 1, NULL);
	err = QTGetAtomDataPtr(container, atom, &dataSize, (Ptr *)&dataPtr);
	if (noErr == err) {
		myPtr->refreshRate = EndianS32_BtoN(dataPtr[0]);
	}

	// ******************* kQTVOPixelType
	atom = QTFindChildByID(container, parentAtom, kQTVOPixelType, 1, NULL);
	err = QTGetAtomDataPtr(container, atom, &dataSize, (Ptr *)&dataPtr);
	if (noErr == err) {
		myPtr->pixelType = EndianU32_BtoN(dataPtr[0]);
	}			

	// ******************* kQTVOName
	atom = QTFindChildByID(container, parentAtom, kQTVOName, 1, NULL);
	err = QTGetAtomDataPtr(container, atom, &dataSize, (Ptr*)&dataPtr);
	if (noErr == err) {
		strncpy((char *)myPtr->name, (char *)dataPtr, dataSize);
	}
	myPtr->name[dataSize] = '\0';

	// ******************* kQTVODecompressors
	// We don't care about the kQTVODecompressors atom(s) in this sample, if we did we
	// would need to go down another level and muck about in much the same fashion.
	// Because kQTVODecompressors atoms are not required to have consecutive IDs, use
	// QTFindChildByIndex to iterate through the decompressors.
	
	// Third level:
	//		Children of the kQTVODecompressors atom are of type:
	//		kQTVODecompressorType
	//		kQTVODecompressorComponent
	//		kQTVODecompressorContinuous
}

pascal OSStatus dts::DialogEventHandler(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
#pragma unused(inHandlerCallRef)

	UInt32	 theEventKind = GetEventKind(inEvent);
	
	OSStatus status	= eventNotHandledErr;
	
	CVideoOutputComponent *pUserData = (CVideoOutputComponent *)inUserData;
	if (NULL == pUserData) return status;
	
	if (kEventCommandProcess == theEventKind) {
		HICommand  theCommand;
		ControlRef theControlRef;
		::GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &theCommand);
		
		switch (theCommand.commandID) {
		case kCommandComponentListPopUp:
		{ // gcc complains without this in brackets
			::GetDialogItemAsControl(pUserData->mDialogRef, kComponentListPopUpDialogItem, &theControlRef);
			UInt8 theNewValue = ::GetControlValue(theControlRef);
			
			// If a new component was chosen from the list, update the mode list popup for that component
			if (theNewValue != pUserData->mWhichComponentIndex) {
				HICommand updateCommand = { 0, kCommandModeListPopUp };
				
				pUserData->UpdateModeListPopUp(theNewValue);
				pUserData->mWhichComponentIndex = theNewValue;
				::ProcessHICommand(&updateCommand);
			}
			status = noErr;
			break;
		}
		case kCommandModeListPopUp:
			::GetDialogItemAsControl(pUserData->mDialogRef, kModeListPopUpDialogItem, &theControlRef);
			pUserData->mWhichModeIndex = ::GetControlValue(theControlRef);
			pUserData->UpdateDialogTextItems(); 
			status = noErr;
			break;
		case kHICommandOK:
			::QuitAppModalLoopForWindow(::GetDialogWindow(pUserData->mDialogRef));
			status = noErr;
			break;
		default:
			break;
		}
	}
	
	return status;
}