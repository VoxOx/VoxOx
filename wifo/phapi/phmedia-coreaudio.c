/*
 * The phmedia-coreaudio  module implements interface to a coreaudio driver
 *
 * Copyright (C) 2006 WENGO SAS
 * @author  Philippe Bernery <philippe.bernery@wengo.fr>
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with dpkg; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <CoreAudio/CoreAudio.h>
#include <AudioToolbox/AudioConverter.h>
#include <AudioUnit/AudioUnit.h>

#include <osip2/osip_mt.h>
#include <osip2/osip.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ortp.h>
#include <telephonyevents.h>
#include <stdlib.h>
#include "phapi.h"
#include "phcall.h"
#include "phmedia.h"
#include "phcodec.h"
#include "tonegen.h"
#include "phmbuf.h"

#include "phmstream.h"
#include "phastream.h"
#include "phaudiodriver.h"
#include "phlog.h"

#include <strings.h>

#define IN_PREFIX "in="
#define OUT_PREFIX "out="

/**
 * Declare the driver to phmedia-audio and initialize it.
 */
void ph_ca_driver_init();

/**
 * Start the stream.
 */
void ca_start(phastream_t *as);

/**
 * Open a device.
 *
 * @param name desired devices. eg: "ca:IN=1 OUT=257"
 * @param rate desired audio rate
 * @param framesize 
 * @param cbk 
 * @return 0 if no error
 */
int ca_open(phastream_t *as, char *name, int rate, int framesize, ph_audio_cbk cbk);

/**
 * Get out data size.
 *
 * @param used
 */
int ca_get_out_space(phastream_t *as, int *used);

/**
 * Get available data.
 */
int ca_get_avail_data(phastream_t *as);

/**
 * Close the stream.
 */
void ca_close(phastream_t *as);

typedef struct _ca_dev {
	char inputID[128];
	char outputID[128];
	AudioUnit outputAU;
	AudioConverterRef inputConverter;
	unsigned inputConverterBufferSize;
	ph_audio_cbk cbk;
	
	char tmpOutputBuffer[32768]; // FIXME: should be set dynamically
	unsigned tmpOutputCount;
	
	char tmpInputBuffer[32768]; // FIXME: should be set dynamically
	unsigned tmpInputCount;
	
	char *currentInputBuffer;
	
	char *convertedInputBuffer; 
	UInt32 convertedInputCount;
	
	unsigned sumDataSize;
} ca_dev;

/**
 * Initialize a audio unit.
 * @param au AudioUnit to set.
 */
static void init_audio_unit(AudioUnit *au);

/**
 * Set Audio device of an audio unit.
 *
 * @param au AudioUnit to set device
 * @param id device id
 */
static void set_audio_unit_device(AudioUnit au, AudioDeviceID id);

/**
 * Set the render callback.
 *
 * @param au AudioUnit to set up
 * @param cbk Callback to use
 * @param data user data for callback
 */
static void set_render_callback(AudioUnit au, AURenderCallback cbk, void *data);

/**
 * Set the intput callback.
 *
 * @param id AudioDevice to set up
 * @param cbk Callback to use
 * @param data user data for callback
 */
static void set_input_callback(AudioDeviceID id, AudioDeviceIOProc cbk, void *data);

/**
 * Open and initialize input device.
 *
 * @param rate desired rate e.g. 8000 Hz
 * @param channels desired amount of channels e.g. 2 (stereo)
 * @param format data format e.g. 16 (16 bits)
 * @param framesize amount of data to send to phapi callback
 * @return -1: error, 0: ok
 */
static int init_input_device(phastream_t *as, float rate, unsigned channels, unsigned format);

/**
 * deinitialize and close input devce
 */
static void clean_input_device(phastream_t *as);

/**
 * Sets the data source of a device.
 *
 * @param audioDeviceId the device ID where to change the source
 * @param isInput the part of the device to change
 * @param dataSourceId the id of the data source to use
 */
static void set_data_source(AudioDeviceID audioDeviceId, int isInput, UInt32 dataSourceId);

/**
 * Open and initialize output device.
 *
 * @param rate desired rate e.g. 8000 Hz
 * @param channels desired amount of channels e.g. 2 (stereo)
 * @param format data format e.g. 16 (16 bits)
 * @param framesize amount of data to send to phapi callback
 * @return -1: error, 0: ok
 */
static int init_output_device(phastream_t *as, float rate, unsigned channels, unsigned format);

/**
 * Set format of data that will be played.
 *
 * @param au AudioUnit to set
 * @param rate desired rate e.g. 8000 Hz
 * @param channels desired amount of channels e.g. 2 (stereo)
 * @param format data format e.g. 16 (16 bits) 
 */
static void set_played_format(AudioUnit au, float rate, unsigned channels, unsigned format);

/**
 * Set format of data that will be recorded.
 *
 * @param rate desired rate e.g. 8000 Hz
 * @param channels desired amount of channels e.g. 2 (stereo)
 * @param format data format e.g. 16 (16 bits) 
 */
static void set_recorded_format(phastream_t *as, float rate, unsigned channels, unsigned format);

/**
 * Gets the AudioDeviceID part of the device id.
 *
 * @param deviceId the device id to work on
 * @return the AudioDeviceID contained in deviceId
 */
static AudioDeviceID get_audiodeviceid(const char * deviceId);

/**
 * Gets the DataSource id part of the device id.
 *
 * @param deviceId the device id to work on
 * @return the DataSource id contained in deviceId
 */
static UInt32 get_datasourceid(const char * deviceId);

/**
 * Output callback.
 */
static OSStatus output_renderer(void *inRefCon,
	AudioUnitRenderActionFlags *ioActionFlags,
	const AudioTimeStamp *inTimeStamp,
	UInt32 inBusNumber, 
	UInt32 inNumberFrames, 
	AudioBufferList *ioData);

/**
 * Input callback.
 */
static OSStatus input_proc(AudioDeviceID device,
	const AudioTimeStamp *currentTime,
	const AudioBufferList *inputData,
	const AudioTimeStamp *inputTime,
	AudioBufferList *outputData,
	const AudioTimeStamp *outputTime,
	void *context);

/**
 * Set devices in cadev from name.
 *
 * name follows the scheme: "ca:IN=256:12345678913:1 OUT=256:65498712365:0"
 *
 * @param cadev struct to fill in
 * @param name name of the device to find
 */
static void parse_device(ca_dev *cadev, const char *name);

/**
 * Procedure used by input audio converter to provide data.
 *
 * @see Apple documentation
 */
static OSStatus buffer_data_proc(AudioConverterRef inAudioConverter, UInt32 *ioDataSize, void **outData, void *context);

struct ph_audio_driver ph_ca_audio_driver = {
	"ca",
	PH_SNDDRVR_PLAY_CALLBACK | PH_SNDDRVR_REC_CALLBACK,
	0,
	ca_start,
	ca_open,
	0,
	0,
	ca_get_out_space,
	ca_get_avail_data,
	ca_close
};



static OSStatus output_renderer(void *inRefCon,
	AudioUnitRenderActionFlags *ioActionFlags,
	const AudioTimeStamp *inTimeStamp,
	UInt32 inBusNumber,
	UInt32 inNumberFrames,
	AudioBufferList *ioData)
{
	phastream_t *as = ((phastream_t *)inRefCon);
	ca_dev *cadev = (ca_dev *) as->drvinfo;
	int needMore;
	int outCount = inNumberFrames * sizeof(short);
	char *playBuf = (char *) ioData->mBuffers[0].mData;
	unsigned decodedFrameSize = ph_astream_decoded_framesize_get(as);

	if (as->actual_rate != as->clock_rate) {
		decodedFrameSize *= 2;
	}

	/* do we have some data left from previous callback? */
	if (cadev->tmpOutputCount) {
		int xsize = (cadev->tmpOutputCount > outCount) ? outCount : cadev->tmpOutputCount;
		int newtmpOutputCount;
		
		memcpy(playBuf, cadev->tmpOutputBuffer, xsize);
		
		outCount -= xsize;
		newtmpOutputCount = cadev->tmpOutputCount - xsize;
		cadev->tmpOutputCount = newtmpOutputCount;
		if (newtmpOutputCount) {
			memcpy(cadev->tmpOutputBuffer, cadev->tmpOutputBuffer+xsize, newtmpOutputCount);
		}
		
		playBuf += xsize;
	}

	/* did we fill the playback buffer completely? */
	if (!outCount)
	{
		return noErr;
	}

	needMore = outCount;
	while (needMore > decodedFrameSize) {
		int chunkSize = decodedFrameSize;
		
		cadev->cbk(as, (void *) 0, 0,  playBuf, &chunkSize);
		
		needMore -= chunkSize;
		playBuf += chunkSize;
		
		if (chunkSize != decodedFrameSize)
		{
			break;
		}
	}

	if (needMore >= decodedFrameSize) {
		memset(playBuf, 0, needMore);
	}  else if (needMore) {
		/* 
		we still need some data to fill the buffer, but the amount needed 
		 is LESS than complete decoded audio frame 
		 */
		int chunkSize = decodedFrameSize;
		
		cadev->cbk(as, (void *) 0, 0,  cadev->tmpOutputBuffer,  &chunkSize);
		cadev->tmpOutputCount = chunkSize;
		DBG_DYNA_AUDIO_DRV("**CoreAudio: chunkSize: %d, needMore: %d\n", chunkSize, needMore);

		/* did we got some more data? */ 
		if (chunkSize) {
			int xsize = (chunkSize > needMore) ? needMore : chunkSize;
			int newtmpOutputCount;

			memcpy(playBuf, cadev->tmpOutputBuffer, xsize);

			needMore -= xsize;
			newtmpOutputCount = cadev->tmpOutputCount - xsize;
			cadev->tmpOutputCount = newtmpOutputCount;
			if (newtmpOutputCount) {
				memcpy(cadev->tmpOutputBuffer, cadev->tmpOutputBuffer + xsize, newtmpOutputCount);
			}

			playBuf += xsize;
		}
		
		/* if we still need more data fill it with zeroes */
		if (needMore > 0)
		{
			memset(playBuf, 0, needMore);
		}
	}
	
	return noErr;	
}


static OSStatus input_proc(AudioDeviceID device,
	const AudioTimeStamp *currentTime,
	const AudioBufferList *inputData,
	const AudioTimeStamp *inputTime,
	AudioBufferList *outputData,
	const AudioTimeStamp *outputTime,
	void *context)
{
	OSStatus err = noErr;
	phastream_t *as = ((phastream_t *) context);
	ca_dev *cadev = (ca_dev *) as->drvinfo;
	unsigned decodedFrameSize = ph_astream_decoded_framesize_get(as);

	if (as->actual_rate != as->clock_rate) {
		decodedFrameSize *= 2;
	}

	DBG_DYNA_AUDIO_DRV("**CoreAudio: available input data: %d\n",
		inputData->mBuffers[0].mDataByteSize);

	DBG_DYNA_AUDIO_DRV("**CoreAudio: phapi framesize:%d, input converter buffer size: %d\n",
		decodedFrameSize, cadev->inputConverterBufferSize);
	
	memcpy(cadev->tmpInputBuffer + cadev->tmpInputCount, 
		inputData->mBuffers[0].mData, inputData->mBuffers[0].mDataByteSize);
	cadev->tmpInputCount += inputData->mBuffers[0].mDataByteSize;
	
	while (cadev->tmpInputCount >= cadev->inputConverterBufferSize) {
		cadev->convertedInputCount = decodedFrameSize;
		unsigned savedtmpInputCount = cadev->tmpInputCount;
		cadev->currentInputBuffer = cadev->tmpInputBuffer;
		cadev->sumDataSize = 0;
		
		err = AudioConverterFillBuffer(cadev->inputConverter, buffer_data_proc,
			cadev, &cadev->convertedInputCount, cadev->convertedInputBuffer);
		if (err != noErr) {
			DBG_DYNA_AUDIO_DRV("!!CoreAudio: error while converting\n");
		}
		DBG_DYNA_AUDIO_DRV("**CoreAudio: converted data: %d\n", cadev->convertedInputCount);
		
		// Send converted data to phapi
		cadev->cbk(as, cadev->convertedInputBuffer, cadev->convertedInputCount, NULL, 0);
		
		unsigned usedData = cadev->sumDataSize;
		if (usedData != 0) {
			unsigned remainingData = savedtmpInputCount - usedData;
			memcpy(cadev->tmpInputBuffer, cadev->tmpInputBuffer + usedData, remainingData);
			cadev->tmpInputCount = remainingData;
		}
		DBG_DYNA_AUDIO_DRV("**CoreAudio: used data: %d\n", usedData);
	}
	
	return noErr;
}


static OSStatus buffer_data_proc(AudioConverterRef inAudioConverter, UInt32 *ioDataSize, void **outData, void *context) {
	ca_dev *cadev = (ca_dev *) context;

	DBG_DYNA_AUDIO_DRV("**CoreAudio: buffer size: %u, ioDataSize: %d\n", cadev->tmpInputCount, *ioDataSize);

	cadev->sumDataSize += *ioDataSize;
	
	unsigned enough = (cadev->tmpInputCount < *ioDataSize) ? 0 : 1;
	
	if (!enough) {
		*ioDataSize = 0;
		*outData = 0;
	} else {		
		*outData = cadev->currentInputBuffer;
		cadev->tmpInputCount -= *ioDataSize;
		cadev->currentInputBuffer += *ioDataSize;		
	}
		
	return kAudioHardwareNoError;
}


static void defaultInputDevice(char * deviceId) {
	OSStatus err = noErr;
	AudioDeviceID device;
	UInt32 dataSourceId = 0;
	UInt32 size = sizeof(AudioDeviceID);

	err = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultInputDevice, &size, &device);
	if (err != noErr) {
		DBG_DYNA_AUDIO_DRV("!!CoreAudio: can't get default input device\n");
		return;
	}

	size = sizeof(UInt32);
	err = AudioDeviceGetProperty(device, 0, 1, kAudioDevicePropertyDataSource, &size, &dataSourceId);
	if (err != noErr) {
		DBG_DYNA_AUDIO_DRV("**CoreAudio: can't get default input data source. No data source on this device");
	}

	snprintf(deviceId, 128, "%lu:%lu:%d", device, dataSourceId, 1);
}


static void defaultOutputDevice(char * deviceId) {
	OSStatus err = noErr;
	AudioDeviceID device;
	UInt32 dataSourceId = 0;
	UInt32 size = sizeof(AudioDeviceID);

	err = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice, &size, &device);
	if (err != noErr) {
		DBG_DYNA_AUDIO_DRV("!!CoreAudio: can't get default output device\n");
		return;
	}

	size = sizeof(UInt32);
	err = AudioDeviceGetProperty(device, 0, 0, kAudioDevicePropertyDataSource, &size, &dataSourceId);
	if (err != noErr) {
		DBG_DYNA_AUDIO_DRV("**CoreAudio: can't get default output data source. No data source on this device");
	}

	snprintf(deviceId, 128, "%lu:%lu:%d", device, dataSourceId, 1);
}


static void parse_device(ca_dev *cadev, const char *name) {
	char buf[256], deviceId[128];
	char *input, *output, *tmp, *buffer = buf;

	strncpy(buffer, name, sizeof(buf));

	DBG_DYNA_AUDIO_DRV("**CoreAudio: parsing %s\n", name);

	if (strncasecmp(buffer, "ca:", 3) == 0) {
		buffer += 3;
	}

	if ((input = strcasestr(buffer, IN_PREFIX))
		&& (tmp = strchr(input + 3, ' '))) {
		strncpy(cadev->inputID, input + 3, tmp - (input + 3));
	}

	if (strlen(cadev->inputID) < 6) { /* 6 because minimum string can be: "1:0:2\0" */
		memset(deviceId, 0, sizeof(deviceId));
		defaultInputDevice(deviceId);
		strncpy(cadev->inputID, deviceId, sizeof(cadev->inputID));
	}

	if ((output = strcasestr(buffer, OUT_PREFIX))) {
		strncpy(cadev->outputID, output + 4, strlen(output + 4));
	}

	if (strlen(cadev->outputID) < 6) {
		memset(deviceId, 0, sizeof(deviceId));
		defaultOutputDevice(deviceId);
		strncpy(cadev->outputID, deviceId, sizeof(cadev->inputID));
	}

	DBG_DYNA_AUDIO_DRV("**CoreAudio: using devices in=%s out=%s\n",
		cadev->inputID, cadev->outputID);
}

static int colon_pos(const char * str, unsigned whichone) {
	size_t size = strlen(str);
	int result = -1;
	unsigned cur = 0, i;

	for (i = 0; i < size; i++) {
		if (str[i] == ':') {
			cur++;
			if (cur == whichone) {
				result = i;
				break;
			}
		}
	}

	return result;
}

static AudioDeviceID get_audiodeviceid(const char * deviceId) {
	AudioDeviceID result = 0;
	char tmp[128];
	int pos1 = -1;

	if (deviceId) {
		DBG_DYNA_AUDIO_DRV("**CoreAudio: deviceId in get_audiodeviceid => %s\n", deviceId);
		pos1 = colon_pos(deviceId, 1);
		if (pos1 != -1) {
			strncpy(tmp, deviceId, pos1);
			result = atoi(tmp);
		}
	}

	return result;
}

static UInt32 get_datasourceid(const char * deviceId) {
	AudioDeviceID result = 0;
	char tmp[128];
	int pos1 = -1, pos2 = -1;

	if (deviceId) {
		DBG_DYNA_AUDIO_DRV("**CoreAudio: deviceId in get_datasourceid => %s\n", deviceId);
		pos1 = colon_pos(deviceId, 1);
		if (pos1 != -1) {
			pos2 = colon_pos(deviceId, 2);
			if (pos2 != -1) {
				strncpy(tmp, deviceId + pos1 + 1, pos2 - pos1 - 1);
				result = atoi(tmp);
			}
		}
	}

	return result;
}

static void init_audio_unit(AudioUnit *au) {
	OSStatus err = noErr;
	ComponentDescription desc;
	desc.componentType = kAudioUnitType_Output;
	desc.componentSubType = kAudioUnitSubType_HALOutput;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;
	desc.componentFlags = 0;
	desc.componentFlagsMask = 0;
	
	Component comp = FindNextComponent(NULL, &desc);
	if (comp == NULL) {
		DBG_DYNA_AUDIO_DRV("!!CoreAudio: can't audio component\n");
	}
	
	err = OpenAComponent(comp, au);
	if (err != noErr) {
		DBG_DYNA_AUDIO_DRV("!!CoreAudio: can't open audio component\n");
	}
}

static void set_audio_unit_device(AudioUnit au, AudioDeviceID id) {
	OSStatus err = noErr;

	err = AudioUnitSetProperty(au,
		kAudioOutputUnitProperty_CurrentDevice,
		kAudioUnitScope_Global,
		0,
		&id,
		sizeof(id));

	if (err != noErr) {
		DBG_DYNA_AUDIO_DRV("!!CoreAudio: can't set device #%d\n", id);
	}
}

static void set_render_callback(AudioUnit au, AURenderCallback cbk, void *data) {
	OSStatus err = noErr;
	AURenderCallbackStruct input;
	input.inputProc = cbk;
	input.inputProcRefCon = data;
	
	err = AudioUnitSetProperty(au,
	   kAudioUnitProperty_SetRenderCallback,
	   kAudioUnitScope_Input,
	   0,
	   &input,
	   sizeof(input));
	
	if (err != noErr) {
		DBG_DYNA_AUDIO_DRV("!!CoreAudio: can't set render callback\n");
	}
}

static void set_input_callback(AudioDeviceID id, AudioDeviceIOProc cbk, void *data) {
	OSStatus err = noErr;

	err = AudioDeviceAddIOProc(id, cbk, data);
	if (err != noErr) {
		DBG_DYNA_AUDIO_DRV("!!CoreAudio: can't set input callback\n");
	}	
}

static void set_data_source(AudioDeviceID audioDeviceId, int isInput, UInt32 dataSourceId) {
	UInt32 size = sizeof(UInt32);
	OSStatus status = noErr;

	status = AudioDeviceSetProperty(audioDeviceId, NULL, 0, isInput,
		kAudioDevicePropertyDataSource, size, &dataSourceId);
	if (status) {
		DBG_DYNA_AUDIO_DRV("!!CoreAudio: can't set data source\n");
		return;
	}
}

static int init_output_device(phastream_t *as, float rate, unsigned channels, unsigned format) {
	OSStatus err = noErr;
	ca_dev *cadev = (ca_dev *) as->drvinfo;

	set_data_source(get_audiodeviceid(cadev->outputID), 0, get_datasourceid(cadev->outputID));

	init_audio_unit(&cadev->outputAU);
	set_played_format(cadev->outputAU, rate, channels, format);
	set_audio_unit_device(cadev->outputAU, get_audiodeviceid(cadev->outputID));
	set_render_callback(cadev->outputAU, output_renderer, as);

	err = AudioUnitInitialize(cadev->outputAU);
	if (err) {
		DBG_DYNA_AUDIO_DRV ("!!CoreAudio: AudioUnitInitialize-SF=%4.4s, %ld\n", (char*)&err, err);
		return -1;
	}
	
	return 0; //OK
}

static int init_input_device(phastream_t *as, float rate, unsigned channels, unsigned format) {
	ca_dev *cadev = (ca_dev *) as->drvinfo;

	set_data_source(get_audiodeviceid(cadev->inputID), 1, get_datasourceid(cadev->inputID));
	set_recorded_format(as, rate, channels, format);
	set_input_callback(get_audiodeviceid(cadev->inputID), input_proc, as);
	return 0; // OK
}

static void clean_input_device(phastream_t *as) {

	ca_dev *cadev = (ca_dev *) as->drvinfo;

	verify_noerr(AudioDeviceRemoveIOProc(get_audiodeviceid(cadev->inputID), input_proc));

}

static void set_recorded_format(phastream_t *as, float rate, unsigned channels, unsigned format) {
	ca_dev *cadev = (ca_dev *) as->drvinfo;
	OSStatus err = noErr;
	unsigned decodedFrameSize = ph_astream_decoded_framesize_get(as);
	AudioStreamBasicDescription imgFmt, devFmt;
	UInt32 propsize = sizeof(devFmt);
	UInt32 formatFlags = kLinearPCMFormatFlagIsSignedInteger |
		kLinearPCMFormatFlagIsPacked |
		kLinearPCMFormatFlagIsNonInterleaved;

#if defined(__BIG_ENDIAN__)
	formatFlags |= kLinearPCMFormatFlagIsBigEndian;
#endif

	err = AudioDeviceGetProperty(get_audiodeviceid(cadev->inputID), 0, 1, kAudioDevicePropertyStreamFormat, &propsize, &devFmt);
	if (err != noErr) {
		DBG_DYNA_AUDIO_DRV("!!CoreAudio: can't get device info\n");
		return;
	}

	DBG_DYNA_AUDIO_DRV("**CoreAudio: Input format:\n\t");
	DBG_DYNA_AUDIO_DRV("...SampleRate=%f,\n", devFmt.mSampleRate);
	DBG_DYNA_AUDIO_DRV("...BytesPerPacket=%ld,\n", devFmt.mBytesPerPacket);
	DBG_DYNA_AUDIO_DRV("...FramesPerPacket=%ld,\n", devFmt.mFramesPerPacket);
	DBG_DYNA_AUDIO_DRV("...BytesPerFrame=%ld,\n", devFmt.mBytesPerFrame);
	DBG_DYNA_AUDIO_DRV("...BitsPerChannel=%ld,\n", devFmt.mBitsPerChannel);
	DBG_DYNA_AUDIO_DRV("...ChannelsPerFrame=%ld\n", devFmt.mChannelsPerFrame);

	imgFmt.mSampleRate = rate;
	imgFmt.mFormatID = kAudioFormatLinearPCM;
	imgFmt.mFormatFlags = formatFlags;
	imgFmt.mBytesPerPacket = 2;
	imgFmt.mFramesPerPacket = 1;
	imgFmt.mBytesPerFrame = 2;
	imgFmt.mChannelsPerFrame = channels;
	imgFmt.mBitsPerChannel = format;

	err = AudioConverterNew(&devFmt, &imgFmt, &cadev->inputConverter);
	if (err != noErr) {
		DBG_DYNA_AUDIO_DRV("!!CoreAudio: can't create audio converter for input\n");
		return;
	}

	if (as->actual_rate != as->clock_rate) {
		decodedFrameSize *= 2;
	}

	propsize = sizeof(unsigned);
	cadev->inputConverterBufferSize = decodedFrameSize;
	if ((cadev->convertedInputBuffer = malloc(sizeof(char) * decodedFrameSize)) == NULL) {
		DBG_DYNA_AUDIO_DRV("!!CoreAudio: can't allocate enough memory for cadev->convertedInputBuffer\n");
		return;
	}

	err = AudioConverterGetProperty(cadev->inputConverter, kAudioConverterPropertyCalculateInputBufferSize,
		&propsize, &cadev->inputConverterBufferSize);
	if (err != noErr) {
		DBG_DYNA_AUDIO_DRV("!!CoreAudio: can't get input converter buffer size\n");
		return;
	}
}

static void set_played_format(AudioUnit au, float rate, unsigned channels, unsigned format) {
	OSStatus err = noErr;
	UInt32 formatFlags = kLinearPCMFormatFlagIsSignedInteger | //FIXME: need to be tested for more portability
		kLinearPCMFormatFlagIsPacked |
		kLinearPCMFormatFlagIsNonInterleaved;

#if defined(__BIG_ENDIAN__)
	formatFlags |= kLinearPCMFormatFlagIsBigEndian;
#endif
	
	AudioStreamBasicDescription streamFormat;
	streamFormat.mSampleRate = rate;
	streamFormat.mFormatID = kAudioFormatLinearPCM;
	streamFormat.mFormatFlags = formatFlags;
	streamFormat.mBytesPerPacket = 2;
	streamFormat.mBytesPerFrame = 2;
	streamFormat.mFramesPerPacket = 1;
	streamFormat.mChannelsPerFrame = channels;
	streamFormat.mBitsPerChannel = format;

	DBG_DYNA_AUDIO_DRV("phad_coreaudio: set_played_format: Rendering source:\n\t");
	DBG_DYNA_AUDIO_DRV("SampleRate=%f,", streamFormat.mSampleRate);
	DBG_DYNA_AUDIO_DRV("BytesPerPacket=%ld,", streamFormat.mBytesPerPacket);
	DBG_DYNA_AUDIO_DRV("FramesPerPacket=%ld,", streamFormat.mFramesPerPacket);
	DBG_DYNA_AUDIO_DRV("BytesPerFrame=%ld,", streamFormat.mBytesPerFrame);
	DBG_DYNA_AUDIO_DRV("BitsPerChannel=%ld,", streamFormat.mBitsPerChannel);
	DBG_DYNA_AUDIO_DRV("ChannelsPerFrame=%ld\n", streamFormat.mChannelsPerFrame);

	err = AudioUnitSetProperty(au,
		kAudioUnitProperty_StreamFormat,
		kAudioUnitScope_Input,
		0,
		&streamFormat,
		sizeof(AudioStreamBasicDescription));

	if (err) { 
		DBG_DYNA_AUDIO_DRV("!!CoreAudio: AudioUnitSetProperty-SF=%4.4s, %ld\n", (char*)&err, err);
		return;
	}
}

void ph_ca_driver_init() {
	DBG_DYNA_AUDIO_DRV("** Register and initialize ca audio driver\n");
	ph_register_audio_driver(&ph_ca_audio_driver);
}

void ca_start(phastream_t *as) {
	OSStatus err;
	ca_dev *cadev = (ca_dev *) as->drvinfo;

	
	DBG_DYNA_AUDIO_DRV("** Starting audio stream\n");
	printf("** Starting audio stream\n"); // power pc hack 1/2
		
	err = noErr;
	verify_noerr(err = AudioOutputUnitStart (cadev->outputAU));
	
	err = noErr;
	verify_noerr(err = AudioDeviceStart(get_audiodeviceid(cadev->inputID), input_proc));

}

int ca_open(phastream_t *as, char *name, int rate, int framesize, ph_audio_cbk cbk) {
	int errInput, errOutput;
	
	ca_dev *cadev = (ca_dev *) calloc(1, sizeof(ca_dev));

	DBG_DYNA_AUDIO_DRV("phad_coreaudio: ca_open: Opening device %s with rate: %d, framesize: %d, and callback: %p\n",
		  name, rate, framesize, cbk);

	cadev->cbk = cbk;

	as->drvinfo = cadev;

	parse_device(cadev, name);

	as->actual_rate = rate;
	errInput = init_input_device(as, rate, 1, 16); //FIXME: channels and format should be given by phapi
	errOutput = init_output_device(as, rate, 1, 16);

	if (errOutput < 0)
	{
		DBG_DYNA_AUDIO_DRV("phad_coreaudio: could not init output device");
		clean_input_device(as);
		return -PH_NORESOURCES;
	}

	DBG_DYNA_AUDIO_DRV("**CoreAudio: actual_rate: %d, clock_rate: %d\n", as->actual_rate, rate);

	return 0;
}

int ca_get_out_space(phastream_t *as, int *used) {
	*used = 320;

	DBG_DYNA_AUDIO_DRV("** Out space used: %d\n", *used);

	return *used;
}

int ca_get_avail_data(phastream_t *as) {
	DBG_DYNA_AUDIO_DRV("** Available data: 0\n");
	return 320;
}

void ca_close(phastream_t *as) {
	ca_dev *cadev = (ca_dev *) as->drvinfo;

	DBG_DYNA_AUDIO_DRV("** Closing audio stream\n");


	verify_noerr(AudioDeviceStop(get_audiodeviceid(cadev->inputID), input_proc));

	clean_input_device(as);

	verify_noerr(AudioOutputUnitStop(cadev->outputAU));
	verify_noerr(AudioUnitUninitialize (cadev->outputAU));

	printf("phad_coreaudio:ca_close:cleaning cadev\n"); // power pc hack 2/2
	
	if (cadev) {
		if (cadev->convertedInputBuffer) {
			free(cadev->convertedInputBuffer);
		}
		cadev->convertedInputBuffer = NULL;
		if (cadev->inputConverter) {
			AudioConverterDispose(cadev->inputConverter);
			cadev->inputConverter = NULL;
		}
		free(cadev);
		as->drvinfo = 0;
	}
}
