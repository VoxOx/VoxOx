#ifndef WIN32
#include <sys/types.h>
#include <unistd.h>
#define OPENMODE O_RDONLY
#else
#include <io.h>
#define OPENMODE (O_RDONLY|O_BINARY)
#ifndef __MINGW32__
typedef long off_t;
#endif
#define SEEK_END 2
#define SEEK_SET 0
#define SEEK_CUR 1
#endif

#include <fcntl.h>
#include <stdlib.h>

#include "phmbuf.h"
#include "wav.h"

#define DONT(x)
#define DO(x) x

#define SATURATE(x) ((x > 0x7fff) ? 0x7fff : ((x < ~0x7fff) ? ~0x7fff : x))

ph_mediabuf_t *
ph_mediabuf_new(int size)
{
  ph_mediabuf_t  *mb;

  mb = (ph_mediabuf_t *) malloc(sizeof(*mb) + (size + 1) );

  if (!mb)
  {
    return 0;
  }

  DONT(printf("allocated mediabuf = %08x size=%d\n", mb, size));
  
  mb->buf = (short *) (mb + 1);
  mb->size = size/2;
  mb->next = 0;
  return mb;
}


void ph_mediabuf_free(ph_mediabuf_t *mb)
{
  DONT(printf("ph_mediabuf_free(%08x)\n",mb));
  if (mb)
  {
    free(mb);
  }
}

void ph_mediabuf_init(ph_mediabuf_t *mb, void *buf, int size)
{
  mb->buf = buf;
  mb->size = size/2;
  mb->next = 0;
}

void ph_mediabuf_cleanup(ph_mediabuf_t *mb)
{
  if (mb->buf)
  {
    free(mb->buf);
  }
}



int ph_mediabuf_mixaudio(ph_mediabuf_t *mb, short *mix, int samples)
{
	short *src = mb->buf + mb->next;
	int  len;
	int  xlen;
	int avail = mb->size - mb->next;

	if (samples > avail)
	{
		len = avail;
	}
	else
	{
		len = samples;
	}

	xlen = len;

	while(len--)
	{
		*mix = (*mix + *src) >> 1;
		mix++;
		src++;
	}

	mb->next += xlen;

	return xlen;
}

void ph_mediabuf_mixmedia(ph_mediabuf_t *dmb, ph_mediabuf_t *smb1)
{
	int tmp;
	short *src1 = smb1->buf;
	short *dst = dmb->buf;
	short *dend;

	if (smb1->next < dmb->next)
	{
		dend = dmb->buf + smb1->next;
	}
	else
	{
		dend = dmb->buf + dmb->next;
	}

	while(dst < dend)
	{
		tmp = (int)*src1++ + (int)*dst;
		tmp = SATURATE(tmp);
		*dst++ = (short) tmp;
	}

}
void ph_mediabuf_mixmedia2(ph_mediabuf_t *dmb, ph_mediabuf_t *smb1, ph_mediabuf_t *smb2, int framesize)
{
	int tmp;
	short *src1 = smb1->buf;
	short *send1 = smb1->buf + smb1->next;
	short *src2 = smb2->buf;
	short *send2 = smb2->buf + smb2->next;
	short *dst = dmb->buf;
	short *dend = dst + framesize;
	int s1len = smb1->next;
	int s2len = smb2->next;

	// mixes the 2 buffers until framesize or starvation
	while((dst < dend) && s1len && s2len )
	{
		tmp = (int)*src1++ + (int)*src2++;
		tmp = SATURATE(tmp);
		*dst++ = (short) tmp;
		s1len--; 
		s2len--;
	}

	// starvation was due to source 2
	while((src1 < send1) && (dst < dend))
	{
		*dst++ = *src1++;
	}

	// starvation was due to source 1
	while((src2 < send2) && (dst < dend))
	{
		*dst++ = *src2++;
	}

	dmb->next = dst - dmb->buf;
}




#define ABUFLEN (4*1024)
#define BAD_HDR -1
#define BAD_FMT -2
#define NO_MEM  -3

int ph_mediabuf_loadwavffile(int fd,  int samplerate, ph_mediabuf_t **pmb)
{
  WAVEAUDIOFORMAT  wfmt;
  int filedatalen;
  ph_mediabuf_t *mb = 0;
  char *audiodata = 0;
  int targetlen;
  int errcode;


  filedatalen = wav_read_header(fd, &wfmt);

  if (filedatalen <= 0)
  {
    return BAD_HDR;
  }

  if (wfmt.channels != 1 && wfmt.bits_per_sample != 16)
  {
    return BAD_FMT;
  }


  if (wfmt.samplerate != 16000 && wfmt.samplerate != 8000)
  {
    return BAD_FMT;
  }

  if (samplerate == wfmt.samplerate)
    {
      mb = ph_mediabuf_new((int) filedatalen);
      if (!mb)
	  {
		return NO_MEM;
	  }

      if (filedatalen != read(fd, (char *) (mb->buf),  filedatalen))
	{
	  errcode = BAD_FMT; goto err;
	}

      *pmb = mb;
      return filedatalen;

    }


  targetlen = (samplerate == 8000) ? filedatalen/2 : filedatalen*2;
  mb = ph_mediabuf_new(targetlen);
  if (!mb)
    {
      errcode = NO_MEM; goto err;
    }
  
  if (!(audiodata = malloc(filedatalen))) 
    {
      errcode = NO_MEM; goto err;
    }


  if (filedatalen != read(fd, audiodata, filedatalen))
    {
      errcode = BAD_FMT; goto err;
    }

  if (samplerate == 8000)
    {
      short *src, *dst;
      int nsamples;

      nsamples = targetlen/2;
      

      src = (short *) audiodata;
      dst = (short*) mb->buf;
      
      while(nsamples--)
	{
	  *dst++ = *src++;
	  src++;
	}
      
    }
  else
    {
      short *src, *dst;
      int nsamples;

      nsamples = targetlen/2;

      src = (short *) audiodata;
      dst = (short*) mb->buf;
      
      while(nsamples--)
	{
	  *dst++ = *src;
	  *dst++ = *src++;
	}
	  
    }


  *pmb = mb;
  free(audiodata);
  return targetlen;


err:

  if (audiodata)
  {
    free(audiodata);
  }
  
  if (mb)
  {
    ph_mediabuf_free(mb);
  }

  *pmb = 0;
  return errcode;


}


ph_mediabuf_t *
ph_mediabuf_load(const char *filename, int samplerate)
{
  int  fd;
  off_t flen;
  ph_mediabuf_t *mb;
  int err;


  if (16000 != samplerate && samplerate != 8000)
  {
    return 0;
  }

  fd = open(filename, OPENMODE);
  
  if (fd == -1)
  {
    return 0;
  }

  err = ph_mediabuf_loadwavffile(fd, samplerate, &mb);
  if (err > 0 || err != BAD_HDR)
    {
      close(fd);
      return (err > 0) ? mb : 0;
    }


  flen = lseek(fd, 0, SEEK_END);

  lseek(fd, 0, SEEK_SET);



  if (samplerate == PH_MEDIA_NATIVE_FILE_RATE)
    {
      mb = ph_mediabuf_new((int) flen);
      if (mb)
	  {
		read(fd, (char *) (mb->buf),  flen);
	  }
      close(fd);
      return mb;
    }

  
  if (PH_MEDIA_NATIVE_FILE_RATE == 16000 && samplerate == 8000)
  {
      short buf[256];
      int blen;
      short *dst;
      
      flen /= 2;
      mb = ph_mediabuf_new((int) flen);

      if (!mb)
	{
	  close(fd);
	  return 0;
	}

      dst = mb->buf;

      while(flen > 0)
	{
	  blen = read(fd, (char *) buf, sizeof(buf));
	  if (blen >= 0)
	    {
	      int i;
	      blen /= 2;

	      for(i = 0; i < blen; i += 2)
		*dst++ = buf[i];
	    }
	  else
	  {
	    break;
	  }

	  flen -= blen;
	}

      close(fd);
      return mb;
  }

  return mb;

}







