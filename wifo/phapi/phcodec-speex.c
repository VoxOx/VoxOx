#include <stdlib.h>
#include <speex/speex.h>
#include "phcodec.h"
/* */

#define SPEEX_NB_FRAME_SAMPLES 160
#define  SPEEX_NB_ENCODED_FRAME_SIZE ((492+7)/8)


#define SPEEX_WB_FRAME_SAMPLES (160*2)
#define  SPEEX_WB_ENCODED_FRAME_SIZE ((352+7)/8 + SPEEX_NB_ENCODED_FRAME_SIZE) 

struct speexenc
{
  void *st;
  SpeexBits bits;

};


struct speexdec
{
  void *st;
  SpeexBits bits;
};


void *ph_speexnb_enc_init(void *dummy);
void *ph_speexnb_dec_init(void *dummy);
int ph_speex_encode(void *ctx, const void *src, int srcsize, void *dst, int dstsize);
int ph_speexnb_decode(void *ctx, const void *src, int srcsize, void *dst, int dstsize);
void ph_speex_enc_cleanup(void *ctx);
void ph_speex_dec_cleanup(void *ctx);

static phcodec_t speexnb_codec =
{
  "SPEEX", 8000, SPEEX_NB_ENCODED_FRAME_SIZE, SPEEX_NB_FRAME_SAMPLES*2, 
  ph_speexnb_enc_init, ph_speexnb_dec_init, 
  ph_speex_enc_cleanup, ph_speex_dec_cleanup,  
  ph_speex_encode, ph_speexnb_decode
};



void *ph_speexwb_enc_init(void *dummy);
void *ph_speexwb_dec_init(void *dummy);
int ph_speexwb_decode(void *ctx, const void *src, int srcsize, void *dst, int dstsize);

#ifdef PH_SPEEXWB_REPLACES_AMRWB    
#define SPEEX_PAYLOAD_STRING "AMR-WB"
#elif defined(PH_SPEEXWB_REPLACES_AMRNB)    
#define SPEEX_PAYLOAD_STRING "AMR"
#elif defined(PH_SPEEXWB_REPLACES_G726WB)
#define SPEEX_PAYLOAD_STRING "G726-64wb"
#else    
#define SPEEX_PAYLOAD_STRING "SPEEX"
#endif


static phcodec_t speexwb_codec =
{
  SPEEX_PAYLOAD_STRING, 16000, SPEEX_WB_ENCODED_FRAME_SIZE, SPEEX_WB_FRAME_SAMPLES*2, 
  ph_speexwb_enc_init, ph_speexwb_dec_init, 
  ph_speex_enc_cleanup, ph_speex_dec_cleanup,  
  ph_speex_encode, ph_speexwb_decode
};



void *ph_speex_enc_init(SpeexMode *mode)
{
  struct speexenc *speex;
  int tmp;

  speex = (struct speexenc*) calloc(sizeof(struct speexenc), 1);
  if (!speex)
  {
    return 0;
  }

  speex->st = speex_encoder_init(mode);


  tmp=0;
  speex_encoder_ctl(speex->st, SPEEX_SET_VBR, &tmp);
  tmp=8;
  speex_encoder_ctl(speex->st, SPEEX_SET_QUALITY, &tmp);
  tmp=1;
  speex_encoder_ctl(speex->st, SPEEX_SET_COMPLEXITY, &tmp);

  speex_bits_init(&speex->bits);

  return speex;

}

void *ph_speexnb_enc_init(void *dummy)
{

  return ph_speex_enc_init(speex_lib_get_mode(SPEEX_MODEID_NB));
}


void *ph_speexwb_enc_init(void *dummy)
{

  return ph_speex_enc_init(speex_lib_get_mode(SPEEX_MODEID_WB));
}


void *ph_speex_dec_init(SpeexMode *mode)
{
  struct speexdec *speex;
  int tmp;

  speex = (struct speexdec *)calloc(sizeof(struct speexdec), 1);
  if (!speex)
  {
    return 0;
  }

  speex->st = speex_decoder_init(mode);


  tmp=0;
  speex_decoder_ctl(speex->st, SPEEX_SET_VBR, &tmp);
  tmp=8;
  speex_decoder_ctl(speex->st, SPEEX_SET_QUALITY, &tmp);
  tmp=1;
  speex_decoder_ctl(speex->st, SPEEX_SET_COMPLEXITY, &tmp);

  speex_bits_init(&speex->bits);

  return speex;
}




void *ph_speexnb_dec_init(void *dummy)
{

  return ph_speex_dec_init(speex_lib_get_mode(SPEEX_MODEID_NB));

}


void *ph_speexwb_dec_init(void *dummy)
{

  return ph_speex_dec_init(speex_lib_get_mode(SPEEX_MODEID_WB));

}



int ph_speex_encode(void *ctx, const void *src, int srcsize, void *dst, int dstsize)
{
  struct speexenc *speex = (struct speexenc *)ctx; 
  short *new_speech = (short *)src;
  int count;

  speex_bits_reset(&speex->bits);
  speex_encode_int(speex->st, new_speech, &speex->bits);
  count = speex_bits_write(&speex->bits, dst, dstsize);

  return count;
}



int ph_speexnb_decode(void *ctx, const void *src, int srcsize, void *dst, int dstsize)
{
  int i;
  struct speexdec *speex = (struct speexdec *)ctx; 

  speex_bits_read_from(&speex->bits, src, srcsize);
  speex_decode_int(speex->st, &speex->bits, dst);
  speex_bits_reset(&speex->bits);

  return SPEEX_NB_FRAME_SAMPLES*2;

}

int ph_speexwb_decode(void *ctx, const void *src, int srcsize, void *dst, int dstsize)
{
  int i;
  struct speexdec *speex = (struct speexdec *)ctx; 

  speex_bits_read_from(&speex->bits, src, srcsize);
  speex_decode_int(speex->st, &speex->bits, dst);
  speex_bits_reset(&speex->bits);

  return SPEEX_WB_FRAME_SAMPLES*2;

}


void ph_speex_enc_cleanup(void *ctx)
{
  struct speexenc *speex = (struct speexenc *)ctx; 

  speex_bits_destroy(&speex->bits);
  speex_encoder_destroy(speex->st);
  free(ctx);

}



void ph_speex_dec_cleanup(void *ctx)
{
  struct speexdec *speex = (struct speexdec *)ctx; 

  speex_bits_destroy(&speex->bits);
  speex_decoder_destroy(speex->st);
  free(ctx);

}

#ifdef WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif


DLLEXPORT int ph_codec_plugin_init(void (*codec_register)(phcodec_t *))
{
  codec_register(&speexnb_codec);
  codec_register(&speexwb_codec);

  return 0;
}
