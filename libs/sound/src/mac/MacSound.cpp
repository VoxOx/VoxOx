/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2006  Wengo
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "MacSound.h"

@implementation MacSoundObjC

- (id)initWithFilename:(NSString *)filename andMacSoundInstance:(MacSound *)macSound
{
	[super init];

	_pool = [[NSAutoreleasePool alloc] init];

	_sound = [[NSSound alloc] initWithContentsOfFile:filename byReference:YES];
	[_sound setDelegate:self];

	_loops = 0;
	_stopped = NO;
	_macSound = macSound;

	return self;
}

- (void)dealloc
{
	[_sound stop];
	[_sound release];
	[_pool release];
	[super dealloc];
}

- (void)play
{
	[_sound play];
}

- (void)stop
{
	_stopped = YES;
	[_sound stop];
}

- (void)sound:(NSSound *)sound didFinishPlaying:(BOOL)aBool
{
	if (!_stopped) {
		if (_loops > 0) {
			_loops--;
			[_sound play];
		} else if (_loops == -1) {
			[_sound play];
		} else {
			_stopped = YES;
		}
	} 
	
	if (_stopped) {
		_macSound->stopped();
	}
}

- (void)setLoops:(int)loops
{
	_loops = loops;
}

@end

MacSound::MacSound(const std::string & filename) {
	NSString * pathName = NULL;

	pathName = [NSString stringWithUTF8String:filename.c_str()];
	_macSoundObjCPrivate = [[MacSoundObjC alloc] initWithFilename:pathName andMacSoundInstance:this];

	_mustReleaseAtEnd = false;
}

MacSound::~MacSound() {
	[_macSoundObjCPrivate release];
}

void MacSound::setLoops(int loops) {
	[_macSoundObjCPrivate setLoops:loops];
}

bool MacSound::setWaveOutDevice(const AudioDevice & device) {
	//TODO: set the wave out device
	return true;
}

void MacSound::play() {
	[_macSoundObjCPrivate play];
}

void MacSound::stop() {
	[_macSoundObjCPrivate stop];
}

void MacSound::releaseAtEnd() {
	_mustReleaseAtEnd = true;
}

void MacSound::stopped() {
	if (_mustReleaseAtEnd) {
		delete this;
	}
}
