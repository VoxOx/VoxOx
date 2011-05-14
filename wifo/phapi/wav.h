#ifndef __WAV_H__
#define __WAV_H__

typedef struct _waveaudiofmt {
	unsigned short	format_tag;
	unsigned short	channels;			/* 1 = mono, 2 = stereo */
	unsigned long	samplerate;			/* typically: 44100, 32000, 22050, 11025 or 8000*/
	unsigned long	bytes_per_second;	/* SamplesPerSec * BlockAlign*/
	unsigned short	blockalign;			/* Channels * (BitsPerSample / 8)*/
	unsigned short	bits_per_sample;	/* 16 or 8 */
} WAVEAUDIOFORMAT;

typedef struct _riffchunk {
	char info[4];
	unsigned long length;
} RIFF_CHUNK;


int wav_read_header(int fd, WAVEAUDIOFORMAT  *fmt);

#if 0
int wav_create_header(int fd);
int wav_update_header(int fd);
#endif

#endif
