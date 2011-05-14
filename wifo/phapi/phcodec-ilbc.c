#include <stdlib.h>
#include "ilbc/iLBC_define.h"
#include "ilbc/iLBC_encode.h"
#include "ilbc/iLBC_decode.h"
#include "phcodec.h"

#include "phmedia.h"
/* */

static int ph_ilbc_encode(void *ctx, const void *src, int srcsize, void *dst, int dstsize);
static int ph_ilbc_decode(void *ctx, const void *src, int srcsize, void *dst, int dstsize);
static void *ph_ilbc_dec_init(void *dummy);
static void *ph_ilbc_enc_init(void *dummy);
static void ph_ilbc_dec_cleanup(void *ctx);
static void ph_ilbc_enc_cleanup(void *ctx);

static int ph_ilbc_get_frame_size(void *dummy)
{
	int packetization = 20;
	char* pszPayloadMode = NULL;
	struct ph_media_payload_s* p_media_payload = NULL;
	
	p_media_payload = (struct ph_media_payload_s*)dummy;
	
	if (p_media_payload != NULL)
	{
		if ( (p_media_payload->fmtp != NULL) && 
				(strstr(p_media_payload->fmtp, "111") != NULL) && 
				((pszPayloadMode = strstr(p_media_payload->fmtp, "mode=")) != NULL) )
		{
			pszPayloadMode += 5;
			packetization = atoi(pszPayloadMode);
			
			// Update payload's "ptime" with the desired "mode" that have priority.
			p_media_payload->ptime = packetization;
			p_media_payload->mode = packetization;
		}
		else if (p_media_payload->ptime != 0)
		{
			packetization = p_media_payload->ptime;
		}
	}
	
	return packetization;
}

static void *ph_ilbc_dec_init(void *dummy)
{
    iLBC_Dec_Inst_t *ctx;
	int packetization;

    ctx = malloc(sizeof(*ctx));

	packetization = ph_ilbc_get_frame_size(dummy);
	initDecode(ctx, packetization, 1);

    return ctx;
}

static void *ph_ilbc_enc_init(void *dummy)
{
    iLBC_Enc_Inst_t *ctx;
	int packetization;

    ctx = malloc(sizeof(*ctx));

	packetization = ph_ilbc_get_frame_size(dummy);
	initEncode(ctx, packetization);

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
  "iLBC", 8000, NO_OF_BYTES_30MS, BLOCKL_30MS*2, ph_ilbc_enc_init, ph_ilbc_dec_init, ph_ilbc_enc_cleanup, ph_ilbc_dec_cleanup, 
  ph_ilbc_encode, ph_ilbc_decode
};


#ifdef WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif


DLLEXPORT int ph_codec_plugin_init(void (*codec_register)(phcodec_t *))
{
  codec_register(&ilbc_codec);

  return 0;
}
