/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2007  Wengo
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

/**
 * @author Mathieu Stute
 * @author Aurelien Gateau
 */

#ifndef UNIXSOUNDTREAD_H
#define UNIXSOUNDTREAD_H

#include <sound/ISound.h>
#include <sound/AudioDevice.h>
#include <thread/Thread.h>

#include <sndfile.h>
#include <portaudio.h>

struct SoundFileData;

class UnixSoundThread : public ISound, public Thread {
public:

	UnixSoundThread(const std::string & filename);

	virtual ~UnixSoundThread();

	void setLoops(int loops);

	void play();

	void stop();

	bool setWaveOutDevice(const AudioDevice & device);

protected:

	void run();

private:

	bool startPlaying(SoundFileData*);

	void computeOutDevice();

	std::string _filename;

	int _loops;

	int _paDeviceIndex;
};

#endif	//UNIXSOUNDTREAD_H
