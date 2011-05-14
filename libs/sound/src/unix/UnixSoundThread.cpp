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

#include "UnixSoundThread.h"

#include <thread/ThreadEvent.h>
#include <util/Logger.h>
#include <util/String.h>


struct SoundFileData {
	SNDFILE *sndFile;
	SF_INFO sfInfo;
	int position;
	int loops;
	PaStream *stream;
	UnixSoundThread * unixSoundThread;
};


int audioCallback(const void *input, void *output, unsigned long frameCount,
			 const PaStreamCallbackTimeInfo *paTimeInfo,
			 PaStreamCallbackFlags statusFlags,
			 void *userData);

UnixSoundThread::UnixSoundThread(const std::string & filename)
	: _filename(filename), _loops(1) {
	_paDeviceIndex = -1;
}

UnixSoundThread::~UnixSoundThread() {
}

bool UnixSoundThread::setWaveOutDevice(const AudioDevice & device) {
	_paDeviceIndex = String(device.getData()[1]).toInteger();
	return true;
}

void UnixSoundThread::run() {
	SoundFileData data;
	if (!startPlaying(&data)) {
		return;
	}

	runEvents();

	Pa_StopStream(data.stream);
	Pa_CloseStream(data.stream);
	sf_close(data.sndFile);
	Pa_Terminate();
}

void UnixSoundThread::play() {
	// Stop any playing sound, but temporarily disable autoDelete, otherwise we
	// will be deleted when join() is called.
	bool autoDelete = getAutoDelete();
	if (autoDelete) {
		setAutoDelete(false);
	}
	terminate();
	join();

	// Reactive autoDelete if necessary
	if (autoDelete) {
		setAutoDelete(true);
	}

	// Start playing
	start();
}

void UnixSoundThread::stop() {
	terminate();
}

void UnixSoundThread::setLoops(int loops) {
	_loops = loops;
}

void UnixSoundThread::computeOutDevice() {
	if (_paDeviceIndex == -1) {
		_paDeviceIndex = Pa_GetDefaultOutputDevice();
	}
}


void streamFinishedCallback(void *userData) {
	SoundFileData *data = (SoundFileData *)userData;
	data->unixSoundThread->stop();
}


bool UnixSoundThread::startPlaying(SoundFileData* data) {
	if (_filename.size() == 0) {
		return false;
	}

	PaStream *stream;
	PaError error;
	PaStreamParameters outputParameters;

	// initialize our data structure
	data->sndFile = sf_open(_filename.c_str(), SFM_READ, &data->sfInfo);
	if (!data->sndFile) {
		LOG_WARN("sndfile error opening file:" + _filename);
		return false;
	}
	data->position = 0;
	data->loops = _loops;
	data->stream = NULL;
	data->unixSoundThread = this;
	////

	// start portaudio
	Pa_Initialize();

	// compute the output device
	computeOutDevice();

	// set the output device
	outputParameters.device = _paDeviceIndex;
	// use the same number of channels as our sound file
	outputParameters.channelCount = data->sfInfo.channels;
	// 32bit int format
	outputParameters.sampleFormat = paInt32;
	// 200 ms ought to satisfy even the worst sound card
	//outputParameters.suggestedLatency = 0.2;
	const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(outputParameters.device);
	if (!deviceInfo) {
		LOG_WARN("could not get sound card device info");
		sf_close(data->sndFile);
		return false;
	}
	outputParameters.suggestedLatency = deviceInfo->defaultLowInputLatency;
	// no api specific data
	outputParameters.hostApiSpecificStreamInfo = 0;

	// try to open the output with same sample rate as the sound file
	// and we let portaudio choose the buffersize
	error = Pa_OpenStream(&stream, 0, &outputParameters,
							data->sfInfo.samplerate,
							paFramesPerBufferUnspecified,
							paNoFlag, audioCallback, data);

	if (error) {
		LOG_DEBUG("error opening output stream: " + String::fromNumber(error));
		sf_close(data->sndFile);
		return false;
	}
	data->stream = stream;
	Pa_SetStreamFinishedCallback(stream, streamFinishedCallback);

	// when we start the stream, the callback starts getting called
	Pa_StartStream(stream);

	return true;
}

int audioCallback(const void */*input*/,
				void *output,
				unsigned long frameCount,
				const PaStreamCallbackTimeInfo */*paTimeInfo*/,
				PaStreamCallbackFlags /*statusFlags*/,
				void *userData)
{
	SoundFileData *data = (SoundFileData *)userData;
	int *cursor; /* current pointer into the output */
	int thisSize = frameCount;
	int thisRead;
	int endOfFile = 0;

	// set the output cursor to the beginning
	cursor = (int *)output;
	while (thisSize > 0) {
		// seek to our current file position
		sf_seek(data->sndFile, data->position, SEEK_SET);

		// are we going to read past the end of the file?
		if (thisSize > (data->sfInfo.frames - data->position)) {
			//if we are, only read to the end of the file
			thisRead = data->sfInfo.frames - data->position;

			data->loops--;
			if (data->loops) {
				data->position = 0;
			} else {
				endOfFile = 1;
			}

		} else {
			// otherwise, we'll just fill up the rest of the output buffer
			thisRead = thisSize;
			// and increment the file position
			data->position += thisRead;
		}

		// since our output format and channel interleaving is the same as
		// sf_readf_int's requirements we'll just read straight 
		// into the output buffer
		sf_readf_int(data->sndFile, cursor, thisRead);
		// increment the output cursor
		cursor += thisRead;
		// decrement the number of samples left to process
		thisSize -= thisRead;
	}

	if (endOfFile) {
		return paComplete;
	}
	return paContinue;
}
