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

#include "SoundFile.h"

SoundFile::SoundFile() {
	_soundFile = NULL;
}

bool SoundFile::open(const std::string & filename) {
	if (filename.empty()) {
		return false;
	}

	LockResource(0);

	//Open file
	_soundFile = fopen(filename.c_str(), "rb");
	if (!_soundFile) {
		return false;
	}
	return true;
}

void SoundFile::close() {
	if (_soundFile) {
		fclose(_soundFile);
	}
}

long SoundFile::read(char * data, long bufferLength) {
	return fread(data, 1, bufferLength, _soundFile);
}

WAVEFORMATEX * RawSoundFile::getFormat() {
	//Set file length
	fseek(_soundFile, 0, SEEK_END);
	_soundFileLength = ftell(_soundFile);
	fseek(_soundFile, 0, SEEK_SET);

	//Set the format attribute members
	_format.nSamplesPerSec = 16000;
	_format.wBitsPerSample = 16;
	_format.nChannels = 1;
	_format.nBlockAlign = (_format.wBitsPerSample * _format.nChannels) >> 3;
	_format.nAvgBytesPerSec = _format.nBlockAlign * _format.nSamplesPerSec;
	_format.wFormatTag = WAVE_FORMAT_PCM;

	return &_format;
}

WAVEFORMATEX * WavSoundFile::getFormat() {
	static const int WF_OFFSET_FORMATTAG = 20;
	static const int WF_OFFSET_CHANNELS = 22;
	static const int WF_OFFSET_SAMPLESPERSEC = 24;
	static const int WF_OFFSET_AVGBYTESPERSEC = 28;
	static const int WF_OFFSET_BLOCKALIGN = 32;
	static const int WF_OFFSET_BITSPERSAMPLE = 34;
	static const int WF_OFFSET_DATASIZE = 40;
	static const int WF_OFFSET_DATA = 44;
	static const int WF_HEADER_SIZE = WF_OFFSET_DATA;

	BYTE aHeader[WF_HEADER_SIZE];

	//Set file length
	fseek(_soundFile, 0, SEEK_END);
	_soundFileLength = ftell(_soundFile) - WF_HEADER_SIZE;
	fseek(_soundFile, 0, SEEK_SET);

	//Set the format attribute members
	fread(aHeader, 1, WF_HEADER_SIZE, _soundFile);
	_format.nSamplesPerSec = *((DWORD*) (aHeader + WF_OFFSET_SAMPLESPERSEC));
	_format.wBitsPerSample = *((WORD*) (aHeader + WF_OFFSET_BITSPERSAMPLE));
	_format.nChannels = *((WORD*) (aHeader + WF_OFFSET_CHANNELS));
	_format.nBlockAlign = *((WORD*) (aHeader + WF_OFFSET_BLOCKALIGN));
	_format.nAvgBytesPerSec = *((DWORD*) (aHeader + WF_OFFSET_AVGBYTESPERSEC));
	_format.wFormatTag = *((WORD*) (aHeader + WF_OFFSET_FORMATTAG));

	return &_format;
}
