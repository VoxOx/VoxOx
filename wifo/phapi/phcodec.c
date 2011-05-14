#include "phglobal.h"
#include "phlog.h"
#include <stdlib.h>
#ifdef OS_POSIX
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <dlfcn.h>
#elif defined(OS_WINDOWS)
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include "msdirent.h"
#endif

#include <phapi-util/phSharedLib.h>

#include "phcodec.h"

//#define FIXED_AMR 1

void ph_media_plugin_codec_init(const char *dirpath);

#ifdef EMBED
//#define ENABLE_AMR_EMBED 1
#ifndef NO_GSM
#define NO_GSM 1
#endif
#endif  //EMBED

//VOXOX CHANGE by ASV 06_22_2009: adding support for ILBC
#include "ilbc/iLBC_define.h"
#include "ilbc/iLBC_encode.h"
#include "ilbc/iLBC_decode.h"

#ifdef PHAPI_VIDEO_SUPPORT
#include "phcodec-h263.h"
#include "phcodec-h263plus.h"
#include "phcodec-h263flv1.h"
#include "phcodec-mpeg4.h"
#include "phcodec-h264.h"
#endif

#ifndef NO_GSM
#define ENABLE_GSM 1
#include "gsm/gsm.h"
#include "gsm/private.h"
#endif

#define ENABLE_G722 1
#if defined(ENABLE_G722)
#include "g722/g722.h"
#endif

#define ENABLE_G726 1
#if defined(ENABLE_G726)

struct g726_state_s;
typedef struct g726_state_s g726_state_t;

g726_state_t *g726_init(g726_state_t *s, int bit_rate, int ext_coding, int packing);
int g726_release(g726_state_t *s);
int g726_decode(g726_state_t *s,
                int16_t amp[],
                const uint8_t g726_data[],
                int g726_bytes);

int g726_encode(g726_state_t *s,
                uint8_t g726_data[],
                const int16_t amp[],
                int len);
#endif

#if defined(WIN32) && !defined(__GNUC__)
# define inline _inline
#endif /* !WIN32 */

/*
 *  PCM - A-Law conversion
 *  Copyright (c) 2000 by Abramo Bagnara <abramo@alsa-project.org>
 *
 *  Wrapper for linphone Codec class by Simon Morlat <simon.morlat@free.fr>
 */

static inline int val_seg(int val)
{
	int r = 0;
	val >>= 7;
	if (val & 0xf0)
	{
		val >>= 4;
		r += 4;
	}
	if (val & 0x0c)
	{
		val >>= 2;
		r += 2;
	}
	if (val & 0x02)
	{
		r += 1;
	}
	return r;
}

/*
 * s16_to_alaw() - Convert a 16-bit linear PCM value to 8-bit A-law
 *
 * s16_to_alaw() accepts an 16-bit integer and encodes it as A-law data.
 *
 *		Linear Input Code	Compressed Code
 *	------------------------	---------------
 *	0000000wxyza			000wxyz
 *	0000001wxyza			001wxyz
 *	000001wxyzab			010wxyz
 *	00001wxyzabc			011wxyz
 *	0001wxyzabcd			100wxyz
 *	001wxyzabcde			101wxyz
 *	01wxyzabcdef			110wxyz
 *	1wxyzabcdefg			111wxyz
 *
 * For further information see John C. Bellamy's Digital Telephony, 1982,
 * John Wiley & Sons, pps 98-111 and 472-476.
 */

static inline unsigned char s16_to_alaw(int pcm_val)
{
	int		mask;
	int		seg;
	unsigned char	aval;

	if (pcm_val >= 0)
	{
		mask = 0xD5;
	}
	else
	{
		mask = 0x55;
		pcm_val = -pcm_val;
		if (pcm_val > 0x7fff)
			pcm_val = 0x7fff;
	}

	if (pcm_val < 256)
	{
		aval = pcm_val >> 4;
	}
	else
	{
		/* Convert the scaled magnitude to segment number. */
		seg = val_seg(pcm_val);
		aval = (seg << 4) | ((pcm_val >> (seg + 3)) & 0x0f);
	}
	return aval ^ mask;
}

/*
 * alaw_to_s16() - Convert an A-law value to 16-bit linear PCM
 *
 */
static inline alaw_to_s16(unsigned char a_val)
{
	int		t;
	int		seg;

	a_val ^= 0x55;
	t = a_val & 0x7f;
	if (t < 16)
	{
		t = (t << 4) + 8;
	}
	else
	{
		seg = (t >> 4) & 0x07;
		t = ((t & 0x0f) << 4) + 0x108;
		t <<= seg -1;
	}
	return ((a_val & 0x80) ? t : -t);
}
/*
 * s16_to_ulaw() - Convert a linear PCM value to u-law
 *
 * In order to simplify the encoding process, the original linear magnitude
 * is biased by adding 33 which shifts the encoding range from (0 - 8158) to
 * (33 - 8191). The result can be seen in the following encoding table:
 *
 *	Biased Linear Input Code	Compressed Code
 *	------------------------	---------------
 *	00000001wxyza			000wxyz
 *	0000001wxyzab			001wxyz
 *	000001wxyzabc			010wxyz
 *	00001wxyzabcd			011wxyz
 *	0001wxyzabcde			100wxyz
 *	001wxyzabcdef			101wxyz
 *	01wxyzabcdefg			110wxyz
 *	1wxyzabcdefgh			111wxyz
 *
 * Each biased linear code has a leading 1 which identifies the segment
 * number. The value of the segment number is equal to 7 minus the number
 * of leading 0's. The quantization interval is directly available as the
 * four bits wxyz.  * The trailing bits (a - h) are ignored.
 *
 * Ordinarily the complement of the resulting code word is used for
 * transmission, and so the code word is complemented before it is returned.
 *
 * For further information see John C. Bellamy's Digital Telephony, 1982,
 * John Wiley & Sons, pps 98-111 and 472-476.
 */

static unsigned char s16_to_ulaw(int pcm_val)	/* 2's complement (16-bit range) */
{
	int mask;
	int seg;
	unsigned char uval;

	if (pcm_val < 0)
	{
		pcm_val = 0x84 - pcm_val;
		mask = 0x7f;
	}
	else
	{
		pcm_val += 0x84;
		mask = 0xff;
	}
	if (pcm_val > 0x7fff)
	{
		pcm_val = 0x7fff;
	}

	/* Convert the scaled magnitude to segment number. */
	seg = val_seg(pcm_val);

	/*
	 * Combine the sign, segment, quantization bits;
	 * and complement the code word.
	 */
	uval = (seg << 4) | ((pcm_val >> (seg + 3)) & 0x0f);
	return uval ^ mask;
}

/*
 * ulaw_to_s16() - Convert a u-law value to 16-bit linear PCM
 *
 * First, a biased linear code is derived from the code word. An unbiased
 * output can then be obtained by subtracting 33 from the biased code.
 *
 * Note that this function expects to be passed the complement of the
 * original code word. This is in keeping with ISDN conventions.
 */
static inline int ulaw_to_s16(unsigned char u_val)
{
  int t;

  /* Complement to obtain normal u-law value. */
  u_val = ~u_val;

  /*
   * Extract and bias the quantization bits. Then
   * shift up by the segment number and subtract out the bias.
   */
  t = ((u_val & 0x0f) << 3) + 0x84;
  t <<= (u_val & 0x70) >> 4;

  return ((u_val & 0x80) ? (0x84 - t) : (t - 0x84));
}

void mulaw_dec(char *mulaw_data /* contains size char */,
	       char *s16_data    /* contains size*2 char */,
	       int size)
{
  int i;
  for(i=0;i<size;i++)
  {
    *((signed short*)s16_data)=ulaw_to_s16( (unsigned char) mulaw_data[i]);
    s16_data+=2;
  }
}

void mulaw_enc(char *s16_data    /* contains pcm_size char */,
	       char *mulaw_data  /* contains pcm_size/2 char */,
	       int pcm_size)
{
  int i;
  int limit = pcm_size/2;
  for(i=0;i<limit;i++)
  {
    mulaw_data[i]=s16_to_ulaw( *((signed short*)s16_data) );
    s16_data+=2;
  }
}

void alaw_dec(char *alaw_data   /* contains size char */,
	      char *s16_data    /* contains size*2 char */,
	      int size)
{
  int i;
  for(i=0;i<size;i++)
  {
    ((signed short*)s16_data)[i]=alaw_to_s16( (unsigned char) alaw_data[i]);
  }
}

void alaw_enc(char *s16_data   /* contains 320 char */,
	      char *alaw_data  /* contains 160 char */,
	      int pcm_size)
{
  int i;
  int limit = pcm_size/2;
  for(i=0;i<limit;i++)
  {
    alaw_data[i]=s16_to_alaw( *((signed short*)s16_data) );
    s16_data+=2;
  }
}


static int pcmu_encode(void *ctx, const void *src, int srcsize, void *dst, int dstsize);
static int pcmu_decode(void *ctx, const void *src, int srcsize, void *dst, int dstsize);
static int pcma_encode(void *ctx, const void *src, int srcsize, void *dst, int dstsize);
static int pcma_decode(void *ctx, const void *src, int srcsize, void *dst, int dstsize);

static int pcmu_encode(void *ctx, const void *src, int srcsize, void *dst, int dstsize)
{
  mulaw_enc((char *) src, (char *) dst, srcsize);
  return srcsize/2;
}

static int pcmu_decode(void *ctx, const void *src, int srcsize, void *dst, int dstsize)
{
  mulaw_dec((char *) src, (char *) dst, srcsize);
  return srcsize*2;
}

static int pcma_encode(void *ctx, const void *src, int srcsize, void *dst, int dstsize)
{
  alaw_enc((char *) src, (char *) dst, srcsize);
  return srcsize/2;
}

static int pcma_decode(void *ctx, const void *src, int srcsize, void *dst, int dstsize)
{
  alaw_dec((char *) src, (char *) dst, srcsize);
  return srcsize*2;
}

static phcodec_t pcmu_codec = 
{
  "PCMU", 8000, 160, 320, 0, 0, 0, 0, 
  pcmu_encode, pcmu_decode
};

static phcodec_t pcma_codec =
{
  "PCMA", 8000, 160, 320, 0, 0, 0, 0, 
  pcma_encode, pcma_decode
};


#ifdef ENABLE_GSM

#define GSM_ENCODED_FRAME_SIZE 33
#define GSM_FRAME_SAMPLES 160
#define GSM_FRAME_DURATION 20

static int ph_gsm_encode(void *ctx, const void *src, int srcsize, void *dst, int dstsize);
static int ph_gsm_decode(void *ctx, const void *src, int srcsize, void *dst, int dstsize);
static void *ph_gsm_init(void *);
static void ph_gsm_cleanup(void *ctx);

static void *
ph_gsm_init(void *dummy)
{
  return gsm_create();
}

static void 
ph_gsm_cleanup(void *ctx)
{
  gsm_destroy(ctx);
}


int ph_gsm_encode(void *ctx, const void *src, int srcsize, void *dst, int dstsize)
{
  gsm_encode(ctx, src, dst);
  return GSM_ENCODED_FRAME_SIZE;
}

int ph_gsm_decode(void *ctx, const void *src, int srcsize, void *dst, int dstsize)
{
  gsm_decode(ctx, src, dst);
  return GSM_FRAME_SAMPLES*2;
}

static phcodec_t gsm_codec =
{
  "GSM", 8000, GSM_ENCODED_FRAME_SIZE, GSM_FRAME_SAMPLES*2, ph_gsm_init, ph_gsm_init, ph_gsm_cleanup, ph_gsm_cleanup, 
  ph_gsm_encode, ph_gsm_decode
};


#endif

#ifdef ENABLE_G722
static int ph_g722_encode(void *ctx, const void *src, int srcsize, void *dst, int dstsize);
static int ph_g722_decode(void *ctx, const void *src, int srcsize, void *dst, int dstsize);
static void *ph_g722_encode_init(void *);
static void *ph_g722_decode_init(void *);
static void ph_g722_encode_cleanup(void *ctx);
static void ph_g722_decode_cleanup(void *ctx);

static void *
ph_g722_encode_init(void *dummy)
{
    return g722_encode_init(0, 64000, 0);
}

static void *
ph_g722_decode_init(void *dummy)
{
    return g722_decode_init(0, 64000, 0);
}

static void 
ph_g722_encode_cleanup(void *ctx)
{
  g722_encode_release((g722_encode_state_t *)ctx);
}

static void 
ph_g722_decode_cleanup(void *ctx)
{
  g722_decode_release((g722_decode_state_t *)ctx);
}

int ph_g722_encode(void *ctx, const void *src, int srcsize, void *dst, int dstsize)
{
    return g722_encode((g722_encode_state_t *)ctx, dst, src, srcsize/2);
}

int ph_g722_decode(void *ctx, const void *src, int srcsize, void *dst, int dstsize)
{
    return 2*g722_decode((g722_decode_state_t *)ctx, dst, src, srcsize);
}

static phcodec_t g722_codec =
{
  "G722", 16000, 160, 320*2, ph_g722_encode_init, ph_g722_decode_init, ph_g722_encode_cleanup, ph_g722_decode_cleanup, 
  ph_g722_encode, ph_g722_decode
};
#endif

#ifdef ENABLE_G726

static int ph_g726_encode(void *ctx, const void *src, int srcsize, void *dst, int dstsize);
static int ph_g726_decode(void *ctx, const void *src, int srcsize, void *dst, int dstsize);
static void *ph_g726_encode_init(void *dummy);
static void *ph_g726_decode_init(void *dummy);
static void ph_g726_encode_cleanup(void *ctx);
static void ph_g726_decode_cleanup(void *ctx);

static void *ph_g726_encode_init(void *dummy)
{
	return g726_init(0, 32000, 0, 2);
}

static void *ph_g726_decode_init(void *dummy)
{
	return g726_init(0, 32000, 0, 2);
}

static void ph_g726_encode_cleanup(void *ctx)
{
	g726_release((g726_state_t *)ctx);
}

static void ph_g726_decode_cleanup(void *ctx)
{
	g726_release((g726_state_t *)ctx);
}


int ph_g726_encode(void *ctx, const void *src, int srcsize, void *dst, int dstsize)
{
	return g726_encode((g726_state_t *)ctx, dst, src, srcsize/2);
}

int ph_g726_decode(void *ctx, const void *src, int srcsize, void *dst, int dstsize)
{
	return 2*g726_decode((g726_state_t*)ctx, dst, src, srcsize);
}

static phcodec_t g726_codec =
{
  "G726-32", 8000, 80, 320, ph_g726_encode_init, ph_g726_decode_init, ph_g726_encode_cleanup, ph_g726_decode_cleanup, 
  ph_g726_encode, ph_g726_decode
};

#endif


//VOXOX CHANGE by ASV 06_22_2009: adding support for ILBC
static int ph_ilbc_encode(void *ctx, const void *src, int srcsize, void *dst, int dstsize);
static int ph_ilbc_decode(void *ctx, const void *src, int srcsize, void *dst, int dstsize);
static void *ph_ilbc_dec_init(void *dummy);
static void *ph_ilbc_enc_init(void *dummy);
static void ph_ilbc_dec_cleanup(void *ctx);
static void ph_ilbc_enc_cleanup(void *ctx);

static void *ph_ilbc_dec_init(void *dummy)
{
    iLBC_Dec_Inst_t *ctx;

    ctx = malloc(sizeof(*ctx));

    initDecode(ctx, 20, 1);

    return ctx;
}

static void *ph_ilbc_enc_init(void *dummy)
{
    iLBC_Enc_Inst_t *ctx;

    ctx = malloc(sizeof(*ctx));

    initEncode(ctx, 20);

    return ctx;
}

static void ph_ilbc_dec_cleanup(void *ctx)
{
  free(ctx);
}

static void ph_ilbc_enc_cleanup(void *ctx)
{
  free(ctx);
}

static int ph_ilbc_decode(void *ctx, const void *src, int srcsize, void *dst, int dstsize)
{
  int k;
  float decflt[BLOCKL_MAX], tmp;
  short *decshrt = (short*) dst;
  iLBC_Dec_Inst_t *dec = (iLBC_Dec_Inst_t *) ctx;

  iLBC_decode(decflt, (unsigned char *)src, dec, 1); 

  for (k=0; k< dec->blockl; k++)
  {
      tmp=decflt[k];
      if (tmp<MIN_SAMPLE)
      {
        tmp=MIN_SAMPLE;
      }
      else if (tmp>MAX_SAMPLE)
      {
        tmp=MAX_SAMPLE;
      }
      decshrt[k] = (short) tmp;
  }
  return (dec->blockl*2); 
}

static int ph_ilbc_encode(void *ctx, const void *src, int srcsize, void *dst, int dstsize)
{
  float tmp[BLOCKL_MAX];
  short *indata = (short *) src;
  int k;
  iLBC_Enc_Inst_t *enc = (iLBC_Enc_Inst_t *) ctx;

  for (k=0; k<enc->blockl; k++)
  {
    tmp[k] = (float)indata[k];
  }

  /* do the actual encoding */
  iLBC_encode((unsigned char *)dst, tmp, enc);

  return (enc->no_of_bytes);
}

static phcodec_t ilbc_codec =
{
  "ILBC", 8000, NO_OF_BYTES_20MS, BLOCKL_20MS*2, ph_ilbc_enc_init, ph_ilbc_dec_init, ph_ilbc_enc_cleanup, ph_ilbc_dec_cleanup, 
  ph_ilbc_encode, ph_ilbc_decode
};
//VOXOX CHANGE by ASV 06_22_2009 - end

phcodec_t *ph_codec_list;

#ifdef PHAPI_VIDEO_SUPPORT

// HACK: Define H263 codec in SDP but use h263p encoder and decoder (a trick to bypass cirpack)
// TO remove the hack, just use these commented lines to replace the definition of h263_codec
/*
static phcodec_t h263_codec = 
{
  CODEC_H263_MIME_STRING, 90000, 160, 320, h263_encoder_init, h263_decoder_init, h263_encoder_cleanup,
  h263_decoder_cleanup, h263_encode, h263_decode, 0, 0
};
*/

static phcodec_t h263_codec = 
{
  CODEC_H263_MIME_STRING, 90000, 160, 320, h263p_encoder_init, h263_decoder_init, h263p_encoder_cleanup,
  h263_decoder_cleanup, h263p_encode, h263_decode, 0, 0
};


static phcodec_t h263p_codec = 
{
  CODEC_H263P_MIME_STRING, 90000, 160, 320, h263p_encoder_init, h263p_decoder_init, h263p_encoder_cleanup,
  h263p_decoder_cleanup, h263p_encode, h263p_decode, 0, 0
};

static phcodec_t h263flv1_codec = 
{
  CODEC_FLV1_MIME_STRING, 90000, 160, 320, h263flv1_encoder_init, h263flv1_decoder_init, h263flv1_encoder_cleanup,
  h263flv1_decoder_cleanup, h263flv1_encode, h263flv1_decode, 0, 0
};

static phcodec_t mpeg4_codec = 
{
  "MP4V-ES", 90000, 160, 320, mpeg4_encoder_init, mpeg4_decoder_init, mpeg4_encoder_cleanup,
  mpeg4_decoder_cleanup, mpeg4_encode, mpeg4_decode, 0, 0
};

static phcodec_t h264_codec = 
{
  CODEC_H264_MIME_STRING, 90000, 160, 320, h264_encoder_init, h264_decoder_init, h264_encoder_cleanup, h264_decoder_cleanup,
  h264_encode, h264_decode, 0, 0
};
#endif


static phcodec_t *codec_table[] = 
{
	&pcmu_codec, &pcma_codec,

	&ilbc_codec,

#ifdef ENABLE_GSM
	&gsm_codec,
#endif
#ifdef ENABLE_G722
	&g722_codec,
#endif
#ifdef ENABLE_G726
	&g726_codec,
#endif
#ifdef PHAPI_VIDEO_SUPPORT
	&h263p_codec,
	&h263flv1_codec,
	&h263_codec,
	&mpeg4_codec,
	&h264_codec,
#endif
	0
};


void ph_media_register_codec(phcodec_t *codec)
{
  phcodec_t *last = ph_codec_list;

  DBG_CODEC_LOOKUP("ph_media_register_codec(%s)\n", codec->mime);

  if (!last)
  {
    ph_codec_list = codec;
    return;
  }

  while(last->next)
  {
    last = last->next;
  }

  last->next = codec;
}

const char entry_point_name[] = "ph_codec_plugin_init";

#ifdef OS_WINDOWS

#define HLIB HINSTANCE
#define snprintf _snprintf
struct stat { int x; };
#endif


/*
  scan the given directory path for a shared library files and load them 
 */
void ph_media_plugin_codec_init(const char *dirpath)
{
  DIR *dir;
  struct dirent *entry;
#ifdef OS_POSIX
  struct stat st;
#endif
  char   modulename[512];
  ph_codec_plugin_init_type  plugin_init;
  HLIB lib;
  char *fpath = getenv("PH_FORCE_CODEC_PATH");

  if (fpath)
  {
      dirpath = fpath;
  }

  if (!dirpath || !dirpath[0])
  {
    dirpath = getenv("PH_CODEC_PATH");
  }

  if (!dirpath)
  {
    dirpath = "./phapi-plugins";
  }

  DBG_CODEC_LOOKUP("looking for dynamic codecs in : %s\n", dirpath, 0, 0);

  dir = opendir(dirpath);
  if (!dir)
  {
    DBG_CODEC_LOOKUP("pay attention: path does not exist\n", 0, 0, 0);
    return;
  }

  while(0 != (entry = readdir(dir)))
  {
    if (!ph_is_shared_lib(entry->d_name)) 
    {
      continue;
    }

    snprintf(modulename, sizeof(modulename), "%s/%s", dirpath, entry->d_name);

#ifdef OS_POSIX
    stat(modulename, &st);

    if (!(st.st_mode & S_IFREG))
    {
      continue;
    }
#endif
    lib = dlopen(modulename, RTLD_NOW);
    if (!lib)
    {
      continue;
    }

    plugin_init = (ph_codec_plugin_init_type) dlsym(lib, entry_point_name);
    if (plugin_init)
    {
      if (plugin_init(ph_media_register_codec))
      {
        DBG_CODEC_LOOKUP("registering dynamic codecs from : %s\n", entry->d_name, 0, 0);
        dlclose(lib);
      }
    }
    else
    {
      dlclose(lib);
    }
  }

  closedir(dir);
}


void ph_media_codecs_init(const char *pluginpath)
{
  phcodec_t *codec;
  int i = 0;

#ifdef PH_VIDEO_SUPPORT
  ph_avcodec_wrapper_init();
#endif

  while(0 != (codec= codec_table[i++]))
  {
    DBG_CODEC_LOOKUP("setup codec in phcodec: \"%s/%d\"\n", codec->mime, codec->clockrate, 0);
    codec->next = codec_table[i];
  }

  ph_codec_list = codec_table[0];
  ph_media_plugin_codec_init(pluginpath);
}
