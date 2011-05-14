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

#ifndef MACSOUND_H
#define MACSOUND_H

#include <sound/ISound.h>

#include <util/NonCopyable.h>

#include <string>

#include <Cocoa/Cocoa.h>

class MacSound;

/**
 * @author Philippe Bernery
 */
@interface MacSoundObjC : NSObject
{
	/** The NSSound object that will play the file. */
	NSSound * _sound;

	/** Times that the sound must be played. */
	int _loops;

	/**
	 * True when stop has been called to avoid the sound to loop
	 * infinitely.
	 */
	BOOL _stopped;

	/**
	 * Autorelease pool.
	 * FIXME: The autorelease pool is created here because
	 * Cocoa is currently used only in this class (2006/04/05).
	 */
	NSAutoreleasePool *_pool;

	/**
	 * MacSound instance.
	 */
	MacSound * _macSound;
}

- (id)initWithFilename:(NSString *)filename andMacSoundInstance:(MacSound *)macSound;
- (void)dealloc;
- (void)play;
- (void)stop;
- (void)sound:(NSSound *)sound didFinishPlaying:(BOOL)aBool;
- (void)setLoops:(int)loops;

@end

/**
 * C++ Wrapper for NSSound.
 *
 * @author Philippe Bernery
 */
class MacSound : public ISound {
	friend class MacSoundObjC;
public:

	/**
	 * Constructs a Sound that can quickly play the sound in a file named filename.
	 *
	 * @param filename sound file
	 */
	MacSound(const std::string & filename);

	~MacSound();

	void setLoops(int loops);

	virtual bool setWaveOutDevice(const AudioDevice & device);

	void play();

	void stop();

	/** If called, MacSound will release itself at end of play. */
	void releaseAtEnd();

	/**
	 * DO NOT USE!!! Used internally by MacSoundObjC. Can't be private.
	 */
	void stopped();

private:

	/** True if MacSound must release itself at end of play. */
	bool _mustReleaseAtEnd;

	/** MacSoundObjC object. */
	MacSoundObjC * _macSoundObjCPrivate;
};

#endif	//MACSOUND_H
