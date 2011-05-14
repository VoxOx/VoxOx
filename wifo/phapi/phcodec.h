#ifndef __PHCODEC_H__
#define __PHCODEC_H__



#ifdef __cplusplus
extern "C"
{
#endif

enum phcodec_types 
{
  PH_CODEC_TYPE_AUDIO=0,
  PH_CODEC_TYPE_VIDEO,
  PH_CODEC_TYPE_OTHER
};

struct phcodec
{
  const char *mime;                  /** mime string describing the codec */
  int   clockrate; /** sampling rate of the decoded signal */
  int   encoded_framesize;
  int   decoded_framesize; /** size in bytes of a 20ms frame of samples */
  void  *(*encoder_init)(void *);
  void  *(*decoder_init)(void *);
  void  (*encoder_cleanup)(void *ctx);
  void  (*decoder_cleanup)(void *ctx);
  int   (*encode)(void *ctx, const void *src, int srcsize, void *dst, int dstsize);
  int   (*decode)(void *ctx, const void *src, int srcsize, void *dst, int dstsize);
  void (*tie)(void *enc, void *dec); 

  int   codec_type;
  int   priority;
  struct phcodec *next;
};

typedef struct phcodec phcodec_t;


extern phcodec_t *ph_codec_list;

void ph_media_codecs_init(const char *pluginpath);

typedef   int (*ph_codec_plugin_init_type)(void (*codec_register_function)(phcodec_t *));

/*******************************************************************************************
 *										Codec MIME constants							   *
 *******************************************************************************************/

#ifdef USE_FLV1_OVER_H261
#define CODEC_FLV1_MIME_STRING "H261"
#else
#define CODEC_FLV1_MIME_STRING "H263-FLV1"
#endif

#define CODEC_H264_MIME_STRING "H264"
#define CODEC_H263_MIME_STRING "H263"
#define CODEC_H263P_MIME_STRING "H263-1998"

#ifdef __cplusplus
}
#endif

#endif
