#ifndef WIN32
#include <sys/types.h>
#include <unistd.h>
#define OPENMODE O_RDONLY
#else
#include <io.h>
#ifndef __MINGW32__
typedef long off_t;
#endif
#define SEEK_END 2
#define SEEK_SET 0
#define SEEK_CUR 1
#endif

#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

#include "wav.h"


/* returns the length of the audio data ready to be read or -1 in case of error */
int wav_read_header(int fd,  WAVEAUDIOFORMAT *fmt)
{
  /* Reads a wave header from a file to the WAVEAUDIOFORMAT structure */

  WAVEAUDIOFORMAT format;
  RIFF_CHUNK riff_chunk;
  char wavestr[4];
  
  int riff_length;
  
  if (sizeof(riff_chunk) != read(fd, &riff_chunk, sizeof(riff_chunk)))
  {
    return -1;
  }
  
  if (strncmp(riff_chunk.info, "RIFF", 4) != 0)
    {
      return -1;
    }
  
  riff_length = riff_chunk.length;
  
  if (4 != read(fd, wavestr, 4))
  {
    return -1;
  }
  
  
  if (strncmp(wavestr, "WAVE", 4) != 0)
    {
      return -1;
    }
  
  while(1)
    {
      
      if (sizeof(riff_chunk) != read(fd, &riff_chunk, sizeof(riff_chunk)))
        {
          return -1;
        }
      
      if (!strncmp(riff_chunk.info, "fmt ", 4))
	  {
        break;
	  }
      
      lseek(fd, riff_chunk.length, SEEK_CUR);
      
    }
  
  
  if (riff_chunk.length != 16)
    {
      return -1;
    }
  
  if (sizeof(format) != read(fd, &format, sizeof(format)))
    {
      return -1;
    }
  
  
  if (format.format_tag != 1)
    {
      return -1;
    }
  
  
  while(1)
    {
      if (sizeof(riff_chunk) != read(fd, &riff_chunk, sizeof(riff_chunk)))
        {
          return -1;
        }
      
      if (!strncmp(riff_chunk.info, "data", 4))
	  {
        break;
	  }
      
      lseek(fd, riff_chunk.length, SEEK_CUR);
      
    }
  
  *fmt = format;
  return riff_chunk.length;
}

#if 0
int wav_create_header(int fd)
{
	int length;

	/* Use a zero length for the chunk sizes for now, then modify when finished */
	WAVEAUDIOFORMAT format;

	format.format_tag = 1;
	format.channels = 2;
	format.samplerate = 44100;
	format.bytes_per_second = 176400;
	format.blockalign = 4;
	format.bits_per_sample = 16;

	fseek(file, 0, SEEK_SET);

	fwrite("RIFF\0\0\0\0WAVEfmt ", sizeof(char), 16, file); /* Write RIFF, WAVE, and fmt  headers */

	length = 16;
	fwrite(&length, 1, sizeof(long), file); /* Length of Format (always 16) */
	fwrite(&format, 1, sizeof(format), file);
	
	fwrite("data\0\0\0\0", sizeof(char), 8, file); /* Write data chunk */

	return 0;
}

/* Update the RIFF structure with proper values. CreateWavHeader must be called first */
int wav_update_header(int fd)
{
  unsigned long filelen, riff_length, data_length;

	/* Get the length of the file */

  fseek( file, 0, SEEK_END );
  filelen = ftell( file );
  
  riff_length = filelen - 8;
  data_length = filelen - 44;
  
  fseek(file, 4, SEEK_SET);
  fwrite(&riff_length, 1, sizeof(long), file);
  
  fseek(file, 40, SEEK_SET);
  fwrite(&data_length, 1, sizeof(long), file);
  
  /* reset file position for appending data */
  fseek(file, 0, SEEK_END);
  
  return 0;
}

#endif
