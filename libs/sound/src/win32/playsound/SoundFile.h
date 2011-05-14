/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2005  Wengo
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

#ifndef SOUNDFILE_H
#define SOUNDFILE_H

#include <util/NonCopyable.h>

#include <windows.h>

#include <string>

#include <cstdio>

/**
 * Wrapper around a sound file.
 * A sound file can be a wave file or a raw file.
 *
 * @see WavSoundFile
 * @see RawSoundFile
 * @author Tanguy Krotoff
 */
class SoundFile : NonCopyable {
public:

	SoundFile();

	virtual ~SoundFile() {
		close();
	}

	bool open(const std::string & filename);

	void close();

	long read(char * data, long bufferLength);

	long getLength() const {
		return _soundFileLength;
	}

	virtual WAVEFORMATEX * getFormat() = 0;

protected:

	WAVEFORMATEX _format;

	FILE * _soundFile;

	long _soundFileLength;
};


class WavSoundFile : public SoundFile {
public:

	virtual WAVEFORMATEX * getFormat();
};


class RawSoundFile : public SoundFile {
public:

	virtual WAVEFORMATEX * getFormat();
};

#endif	//SOUNDFILE_H
