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

/*
 * http://www.planet-source-code.com/vb/scripts/ShowCode.asp?txtCodeId=4422&lngWId=3
 * http://www.ragestorm.net/samples/wavplay.txt
 * http://www.pocketpcdn.com/articles/playsound.html
 */

#include "PlaySoundFile.h"

#include "SoundFile.h"

#include <util/Logger.h>
#include <util/StringList.h>
#include <thread/RecursiveMutex.h>

#include <windows.h>
#include <mmsystem.h>

#include <cstdio>

void stopWaveOutDevice(HWAVEOUT * hWaveOut) {
	if (*hWaveOut != NULL) {
		waveOutReset(*hWaveOut);
		waveOutClose(*hWaveOut);
		*hWaveOut = NULL;
	}
}

bool openWaveOutDevice(HWAVEOUT * hWaveOut, const char * deviceName, WAVEFORMATEX * waveFormat, HANDLE hDoneEvent) {
	WAVEOUTCAPSA outcaps;
	unsigned int nbDevices = waveOutGetNumDevs();
	unsigned int deviceId;
	for (deviceId = 0; deviceId < nbDevices; deviceId++) {
		if (MMSYSERR_NOERROR == waveOutGetDevCapsA(deviceId, &outcaps, sizeof(WAVEOUTCAPSA))) {
			if (deviceName != NULL) {
				if (strncmp(deviceName, outcaps.szPname, strlen(deviceName)) == 0) {
					//The right audio device was found
					break;
				}
			}
		}
	}

	if (deviceId == nbDevices) {
		LOG_ERROR("cannot find the audio device=" + String(deviceName) + ", use the default one instead");

		/*
		* Tries to open the default wave device. WAVE_MAPPER is
		* a constant defined in mmsystem.h, it always points to the
		* default wave device on the system (some people have 2 or
		* more sound cards).
		*
		* From MSDN:
		* waveInOpen()
		* Value: WAVE_MAPPER
		* Meaning: The function selects a waveform-audio input device capable of recording in the specified format.
		* Some multimedia computers have multiple waveform-audio input devices.
		* Unless you know you want to open a specific waveform-audio input device in a system,
		* you should use the WAVE_MAPPER constant for the device identifier when you open a device.
		* The waveInOpen function will choose the device in the system best able to record in the specified data format.
		*
		* What means "best able to record"? We can never be sure about the one that is choosen.
		*/
		//deviceId = WAVE_MAPPER;

		/*
		* Default device Id is 0.
		* It will select the first and default audio device on the system.
		* We have no idea which audio device WAVE_MAPPER will select.
		* It's better to specify it!
		*/
		deviceId = 0;
	}

	LOG_DEBUG("audio device used=" + String(deviceName) + " - " + String::fromNumber(deviceId));

	//Stops wave out
	stopWaveOutDevice(hWaveOut);

	//Opens wave out
	if (MMSYSERR_NOERROR != waveOutOpen(hWaveOut, deviceId, waveFormat, (DWORD) hDoneEvent, 0, CALLBACK_EVENT)) {
		return false;
	}

	return true;
}

static HWAVEOUT _hWaveOut = NULL;
static RecursiveMutex _mutex;

PlaySoundFile::PlaySoundFile() {
	RecursiveMutex::ScopedLock scopedLock(_mutex);

	stop();
	_hWaveOut = NULL;
}

PlaySoundFile::~PlaySoundFile() {
	RecursiveMutex::ScopedLock scopedLock(_mutex);

	_hWaveOut = NULL;
}

bool PlaySoundFile::play(const std::string & filename) {
	SoundFile * soundFile = NULL;

	// pre initialize waveHeader
	WAVEHDR waveHeader;
	ZeroMemory(&waveHeader, sizeof(WAVEHDR));

	//Opens wave file
	std::string::size_type pos = filename.find(".raw");
	if (pos != std::string::npos) {
		soundFile = new RawSoundFile();
	} else {
		soundFile = new WavSoundFile();
	}

	if (!soundFile->open(filename)) {
		LOG_ERROR("can't open file=" + filename);
		if (soundFile) {
			delete soundFile;
		}
		return false;
	}

	// Opens output audio wave device
	HANDLE hDoneEvent = CreateEventA(NULL, FALSE, FALSE, "DONE_EVENT");

	// begin mutexed block
	{
		RecursiveMutex::ScopedLock scopedLock(_mutex);

		if (!openWaveOutDevice(&_hWaveOut, _audioDevice.getData()[0].c_str(), soundFile->getFormat(), hDoneEvent)) {
			LOG_ERROR("can't open wave out device");
			if (soundFile) {
				soundFile->close();
				delete soundFile;
			}
			if (waveHeader.lpData != NULL) {
				delete [] waveHeader.lpData;
			}
			return false;
		}

		// Initializes wave header
		waveHeader.lpData = new char[soundFile->getLength()];
		waveHeader.dwBufferLength = soundFile->getLength();
		waveHeader.dwUser = 0;
		waveHeader.dwFlags = 0;
		waveHeader.dwLoops = 0;
		waveHeader.dwBytesRecorded = 0;
		waveHeader.lpNext = 0;
		waveHeader.reserved = 0;

		// Plays buffer
		soundFile->read(waveHeader.lpData, waveHeader.dwBufferLength);

		MMRESULT mr = waveOutPrepareHeader(_hWaveOut, &waveHeader, sizeof(WAVEHDR));
		if (mr != MMSYSERR_NOERROR) {
			if (soundFile) {
				soundFile->close();
				delete soundFile;
			}
			if (waveHeader.lpData != NULL) {
				delete [] waveHeader.lpData;
			}
			return false;
		}

		mr = waveOutWrite(_hWaveOut, &waveHeader, sizeof(WAVEHDR));
		if (mr != MMSYSERR_NOERROR) {
			if (soundFile) {
				soundFile->close();
				delete soundFile;
			}
			if (waveHeader.lpData != NULL) {
				delete [] waveHeader.lpData;
			}
			return false;
		}
	} // end mutexed block

	//Waits for audio to finish playing
	while (!(waveHeader.dwFlags & WHDR_DONE)) {
		WaitForSingleObject(hDoneEvent, INFINITE);
	}

	// begin mutexed block
	{
		RecursiveMutex::ScopedLock scopedLock(_mutex);

		// Cleans up

		if (!_hWaveOut) {
			if (soundFile) {
				soundFile->close();
				delete soundFile;
			}
			if (waveHeader.lpData != NULL) {
				delete [] waveHeader.lpData;
			}
			return false;
		}

		MMRESULT mr = waveOutUnprepareHeader(_hWaveOut, &waveHeader, sizeof(WAVEHDR));
		if (mr != MMSYSERR_NOERROR) {
			if (soundFile) {
				soundFile->close();
				delete soundFile;
			}
			if (waveHeader.lpData != NULL) {
				delete [] waveHeader.lpData;
			}
			return false;
		}

		mr = waveOutClose(_hWaveOut);
		if (mr != MMSYSERR_NOERROR) {
			if (soundFile) {
				soundFile->close();
				delete soundFile;
			}
			if (waveHeader.lpData != NULL) {
				delete [] waveHeader.lpData;
			}
			return false;
		}
	} // end mutexed block

	delete [] waveHeader.lpData;
	soundFile->close();
	delete soundFile;

	return true;
}

void PlaySoundFile::stop() {
	RecursiveMutex::ScopedLock scopedLock(_mutex);

	if (_hWaveOut) {
		stopWaveOutDevice(&_hWaveOut);
	}
}
