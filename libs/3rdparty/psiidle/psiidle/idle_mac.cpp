/*
 * idle_mac.cpp - detect desktop idle time
 * Copyright (C) 2003  Tarkvara Design Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "idle.h"
#include <Carbon/Carbon.h>

static EventLoopTimerRef mTimerRef = 0;
static int mSecondsIdle = 0;

// Why does Apple have to make this so complicated?
static OSStatus LoadFrameworkBundle(CFStringRef framework, CFBundleRef *bundlePtr) 
{
	OSStatus  err;
	FSRef   frameworksFolderRef;
	CFURLRef baseURL;
	CFURLRef bundleURL;
	
	if (bundlePtr == nil) return -1;
	
	*bundlePtr = nil;
	
	baseURL = nil;
	bundleURL = nil;
	
	err = FSFindFolder(kOnAppropriateDisk, kFrameworksFolderType, true, &frameworksFolderRef);
	if (err == noErr) 
	{
		baseURL = CFURLCreateFromFSRef(kCFAllocatorSystemDefault, &frameworksFolderRef);
		if (baseURL == nil) 
			err = coreFoundationUnknownErr;
	}
	if (err == noErr) 
	{
		bundleURL = CFURLCreateCopyAppendingPathComponent(kCFAllocatorSystemDefault, baseURL, framework, false);
		if (bundleURL == nil) 
			err = coreFoundationUnknownErr;
	}
	if (err == noErr) 
	{
		*bundlePtr = CFBundleCreate(kCFAllocatorSystemDefault, bundleURL);
		if (*bundlePtr == nil) 
			err = coreFoundationUnknownErr;
	}
	if (err == noErr) 
	{
		if (!CFBundleLoadExecutable(*bundlePtr)) 
			err = coreFoundationUnknownErr;
	}
	
	// Clean up.
	if ((err != noErr) && (*bundlePtr != nil))
	{
		CFRelease(*bundlePtr);
		*bundlePtr = nil;
	}
	
	if (bundleURL != nil)
		CFRelease(bundleURL);
	
	if (baseURL != nil)
		CFRelease(baseURL);
	
	return err;
}

static pascal void IdleTimerAction(EventLoopTimerRef, EventLoopIdleTimerMessage inState, void* inUserData)
{
	switch (inState)
	{
		case kEventLoopIdleTimerStarted:
		case kEventLoopIdleTimerStopped:
			// Get invoked with this constant at the start of the idle period,
			// or whenever user activity cancels the idle.
			mSecondsIdle = 0;
			break;
		case kEventLoopIdleTimerIdling:
			// Called every time the timer fires (i.e. every second).
			mSecondsIdle++;
			break;
	}
}

// Typedef for the function we're getting back from CFBundleGetFunctionPointerForName.
typedef OSStatus (*InstallEventLoopIdleTimerPtr)(EventLoopRef inEventLoop,
												 EventTimerInterval   inFireDelay,
												 EventTimerInterval   inInterval,
												 EventLoopIdleTimerUPP    inTimerProc,
												 void *               inTimerData,
												 EventLoopTimerRef *  outTimer);


IdlePlatform::IdlePlatform() 
{
	// May already be init'ed.
	if (mTimerRef == 0) 
	{
		// According to the docs, InstallEventLoopIdleTimer is new in 10.2.
		// According to the headers, it has been around since 10.0.
		// One of them is lying.  We'll play it safe and weak-link the function.
		
		// Load the "Carbon.framework" bundle.
		CFBundleRef carbonBundle;
		if (LoadFrameworkBundle( CFSTR("Carbon.framework"), &carbonBundle ) == noErr) 
		{
			// Load the Mach-O function pointers for the routine we will be using.
			InstallEventLoopIdleTimerPtr myInstallEventLoopIdleTimer = (InstallEventLoopIdleTimerPtr)CFBundleGetFunctionPointerForName(carbonBundle, CFSTR("InstallEventLoopIdleTimer"));
			if (myInstallEventLoopIdleTimer != 0) 
			{
				EventLoopIdleTimerUPP timerUPP = NewEventLoopIdleTimerUPP(IdleTimerAction);
				(*myInstallEventLoopIdleTimer)(GetMainEventLoop(), kEventDurationSecond, kEventDurationSecond, timerUPP, 0, &mTimerRef);
			}
		}
	}
}

IdlePlatform::~IdlePlatform()
{
	RemoveEventLoopTimer(mTimerRef);
	mTimerRef = 0;
}

bool IdlePlatform::init()
{
	return true; // VOXOX -ASV- 08-07-2009: we burn true to mantain the structure consistent with idle_win32.cpp
}

int IdlePlatform::secondsIdle()
{
	return mSecondsIdle;
}