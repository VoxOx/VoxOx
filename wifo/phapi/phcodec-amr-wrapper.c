#include <stdlib.h>
#include "phcodec.h"

/* */
#ifndef EMBED

#define ENABLE_AMR_WB 1
#define ENABLE_AMR 1

#ifdef FIXED_AMR
#define ENABLE_AMR_EMBED 1
#undef ENABLE_AMR
#undef ENABLE_AMR_WB
#endif

#endif /* EMBED */

#define DBG(x)


#ifdef ENABLE_AMR_EMBED
#include "amr/amr.h"
#endif

#ifdef ENABLE_AMR_WB
#include "amrwb/dec_if.h"
#include "amrwb/enc_if.h"
#endif

#ifdef ENABLE_AMR
#include "amrnb/interf_dec.h"
#include "amrnb/interf_enc.h"
#include "amrnb/rom_enc.h"
#endif



#if defined(ENABLE_AMR_EMBED) || defined(ENABLE_AMR) || defined(ENABLE_AMR_WB)
typedef struct{
  UWord8 cmr;
  UWord8 toc;
}amr_rtphdr; 

#define AMR_DEFAULT_MODE 7
#define AMR_FRAME_SAMPLES 160
//short amr_block_size[16]={ 12, 13, 15, 17, 19, 20, 26, 31, 5, 0, 0, 0, 0, 0, 0, 0 };
#define AMR_ENCODED_FRAME_SIZE  32+2

#define AMRWB_DEFAULT_MODE 8
#define AMRWB_FRAME_SAMPLES 320
#ifdef IF2
//const UWord8 amrwb_block_size[16]= {18, 23, 33, 37, 41, 47, 51, 59, 61, 6, 6, 0, 0, 0, 1, 1};
#else
//const UWord8 amrwb_block_size[16]= {18, 24, 33, 37, 41, 47, 51, 59, 61, 6, 6, 0, 0, 0, 1, 1};
#endif
#define AMRWB_ENCODED_FRAME_SIZE  62+2

#endif 

#ifdef ENABLE_AMR_EMBED

/* frame size in serial bitstream file (frame type + serial stream + flags) */

#define SERIAL_FRAMESIZE (1+MAX_SERIAL_SIZE+5)
#define MAX_PACKED_SIZE (MAX_SERIAL_SIZE / 8 + 2)


/* static int amr_frame_bits[]={ 95, 103, 118, 134, 148, 159, 204, 244}; */
static int amr_packed_frame_size[]={ 12, 13, 15, 17, 19, 20, 26, 31, 5, 0,0,0,0,0,0,0};

static int ph_amr_encode(void *ctx, const void *src, int srcsize, void *dst, int dstsize);
static int ph_amr_decode(void *ctx, const void *src, int srcsize, void *dst, int dstsize);
static void *ph_amr_enc_init(void *);
static void ph_amr_enc_cleanup(void *ctx);
static void *ph_amr_dec_init(void *);
static void ph_amr_dec_cleanup(void *ctx);

struct amrencoder{
  enum Mode mode;
  enum Mode used_mode;
  enum TXFrameType tx_type;
  Word16 dtx;                     /* enable encoder DTX                */
  Word16 reset_flag;
  Speech_Encode_FrameState *enc;
  sid_syncState *sid_state;
  void *decoder;
};

struct amrdecoder{
  enum Mode mode;
  enum Mode used_mode;
  enum RXFrameType rx_type;
  enum TXFrameType tx_type;
  Word16 reset_flag;
  Word16 reset_flag_old;
  Word16 dtx;                     /* enable encoder DTX                */
  Speech_Decode_FrameState *dec;
  void * encoder;
};

void *ph_amr_enc_init(void *dummy){
  struct amrencoder *amr;
  DBG("AMR EMBED ENCODER INIT\n");
  amr = malloc(sizeof(struct amrencoder));
  if(amr == NULL)
  {
    DBG("MALLOC FAILED!!!!!\n");
    return NULL;
  }
#ifdef AMR_DTX
  amr->dtx = 1;
#else
  amr->dtx = 0;
#endif
  DBG("AMR EMBED ENC initializing...");
  if (Speech_Encode_Frame_init(&amr->enc, amr->dtx, "encoder") || sid_sync_init (&amr->sid_state))
  {
    free(amr);
    DBG("failed\n");
    return NULL;
  }
  DBG("OK\n");
  amr->tx_type = (enum TXFrameType)0;
  amr->reset_flag = 0;
  amr->mode = AMR_DEFAULT_MODE;
  return amr;
}

void *ph_amr_dec_init(void *dummy){
  struct amrdecoder *amr;
  DBG("AMR EMBED DECODER init\n");
  amr = malloc(sizeof(struct amrdecoder));
  if(amr == NULL)
  {
    DBG("MALLOC FAILED!!!!!\n");
    return NULL;
  }
  DBG("AMR EMBED DEC initializing...\n");
  if (Speech_Decode_Frame_init(&amr->dec, "Decoder"))
  {
    free(amr);
    DBG("failed\n");
    return NULL;
  }
  DBG("OK\n");
  amr->rx_type = (enum RXFrameType)0;
  amr->tx_type = (enum TXFrameType)0;
  amr->reset_flag = 0;
  amr->reset_flag_old = 1;
  return amr;
}

void ph_amr_enc_cleanup(void *ctx){
  struct amrencoder *amr = (struct amrencoder *)ctx; 
  DBG("AMR ENC cleaning\n");
  Speech_Encode_Frame_exit (&amr->enc);
  free(amr);
}

void ph_amr_dec_cleanup(void *ctx){
  struct amrdecoder *amr = (struct amrdecoder *)ctx; 
  DBG("AMR DEC cleaning\n");
  Speech_Decode_Frame_exit (&amr->dec);
  free(amr);
}
/* ENCODE */
int ph_amr_encode(void *ctx, const void *src, int srcsize, void *dst, int dstsize){
  int i;
  struct amrencoder *amr = (struct amrencoder *)ctx; 
  Word16 *new_speech = (Word16 *)src;
  Word16 serial[SERIAL_FRAMESIZE];    /* Output bitstream buffer           */
  UWord8 *packed_bits =  (UWord8 *)dst+2;
  Word16 packed_size;
  static int frame=0;
  amr_rtphdr *hdr = (amr_rtphdr *)dst;

  frame++;

  /* sample must be 13-bit left aligned */
  for (i = 0; i < srcsize; i++)
         new_speech[i] &= 0xFFFC;

  /* zero flags and parameter bits */
  for (i = 0; i < SERIAL_FRAMESIZE; i++)
         serial[i] = 0;

     /* check for homing frame */
  amr->reset_flag = encoder_homing_frame_test(new_speech);
     
     /* encode speech */
  Speech_Encode_Frame(amr->enc, amr->mode, new_speech, &serial[1], &amr->used_mode); 
  sid_sync (amr->sid_state, amr->used_mode, &amr->tx_type);

  if ((frame%10) == 0)
  {
   DBG("\rtxtype=%d mode=%d used_mode=%d", amr->tx_type, amr->mode, amr->used_mode);
  }


  packed_size = PackBits(amr->used_mode, amr->mode, amr->tx_type, &serial[1], packed_bits);
  /* perform homing if homing frame was detected at encoder input */
  if(packed_size != 32)
  {
    DBG("packed size: %d\n", packed_size);
  }

  if (amr->reset_flag != 0)
  {
    Speech_Encode_Frame_reset(amr->enc);
    sid_sync_reset(amr->sid_state);
  }
  hdr->cmr = amr->mode << 4;
  hdr->toc = *packed_bits & 0x07ff;  /* only 1 frame F=0 */
  hdr->toc |= 4;  /*  Q=1 */

  return (packed_size+2);
}
/* DECODE */
int ph_amr_decode(void *ctx, const void *src, int srcsize, void *dst, int dstsize){
  struct amrdecoder *amr = (struct amrdecoder *)ctx; 
  UWord8 toc, q, ft;
  UWord8 *packed_bits = (UWord8 *)src+2;
  Word16 *out = (Word16 *)dst;
  Word16 serial[SERIAL_FRAMESIZE];   /* coded bits                    */
  static int frame=0;
  int i;

  toc = *packed_bits;
  /* read rest of the frame based on ToC byte */
  q  = (toc >> 2) & 0x01;
  ft = (toc >> 3) & 0x0F;
  
  if(ft > 8 && ft < 15)
  {
    /* invalid frame, discard all packet */
    printf ("\rRejected: rxtype=%d mode=%d", amr->rx_type, ft);
    return 0;
  }

  amr->rx_type = UnpackBits(q, ft, packed_bits+1, &amr->mode, &serial[1]);
  frame++;
  if ((frame%10) == 0)
  {
   printf ("\rrxtype=%d mode=%d", amr->rx_type, ft);
  }
  if (amr->rx_type == RX_NO_DATA)
  {
    amr->mode = amr->dec->prev_mode;
  }
  else
  {
    amr->dec->prev_mode = amr->mode;
  }
  
  /* if homed: check if this frame is another homing frame */
  if (amr->reset_flag_old == 1)
  {
    /* only check until end of first subframe */
    amr->reset_flag = decoder_homing_frame_test_first(serial, amr->mode);
  }
  /* produce encoder homing frame if homed & input=decoder homing frame */
  if ((amr->reset_flag != 0) && (amr->reset_flag_old != 0))
  {
    for (i = 0; i < L_FRAME; i++){
      *out++ = EHF_MASK;
    }
  }else{     
    /* decode frame */
    Speech_Decode_Frame(amr->dec, amr->mode, &serial[1], amr->rx_type, out);
  }
  /* if not homed: check whether current frame is a homing frame */
  if (amr->reset_flag_old == 0)
  {
    /* check whole frame */
    amr->reset_flag = decoder_homing_frame_test(&serial[1], amr->mode);
  }
  /* reset decoder if current frame is a homing frame */
  if (amr->reset_flag != 0)
  {
    Speech_Decode_Frame_reset(amr->dec);
  }
  amr->reset_flag_old = amr->reset_flag;

  return AMR_FRAME_SAMPLES*2;
}

static phcodec_t amr_codec =
{
  "AMR", AMR_ENCODED_FRAME_SIZE, AMR_FRAME_SAMPLES*2, ph_amr_enc_init, ph_amr_dec_init, ph_amr_enc_cleanup, ph_amr_dec_cleanup, 
  ph_amr_encode, ph_amr_decode
};

#else

#ifdef ENABLE_AMR


static int ph_amr_encode(void *ctx, const void *src, int srcsize, void *dst, int dstsize);
static int ph_amr_decode(void *ctx, const void *src, int srcsize, void *dst, int dstsize);
static void *ph_amr_enc_init(void *dummy);
static void ph_amr_enc_cleanup(void *ctx);
static void *ph_amr_dec_init(void *dummy);
static void ph_amr_dec_cleanup(void *ctx);

static int ph_amr_encode(void *ctx, const void *src, int srcsize, void *dst, int dstsize)
{
  Word16 *inp = (Word16 *)src;
  UWord8 *out = (UWord8 *)dst+2;  /* reserve place for Payload header and TOC in octet aligned mode */
  Word16 mode = AMR_DEFAULT_MODE;
  Word16 coded_size;
  amr_rtphdr *hdr = (amr_rtphdr *)dst;

  coded_size = Encoder_Interface_Encode(ctx, mode, inp, out, 0);
  hdr->cmr = mode << 4;
  hdr->toc = *out & 0x07ff;  /* only 1 frame F=0 */
  hdr->toc |= 4;  /*  Q=1 */
  return coded_size+2;
}
static int ph_amr_decode(void *ctx, const void *src, int srcsize, void *dst, int dstsize)
{
  Word16 *out = (Word16 *)dst;
  UWord8 *inp = (UWord8 *)src + 2;
  Word16 mode;
  amr_rtphdr *hdr = (amr_rtphdr *)src;

  Decoder_Interface_Decode(ctx, inp,  out, 0);
  return AMR_FRAME_SAMPLES*2;
}

static void *ph_amr_enc_init(void *dummy)
{
  Word16 dtx;
  void *enc;
#ifdef AMR_DTX
  dtx = 1;
#else
  dtx = 0;
#endif
  DBG("AMR ENC initializing...");
  enc = Encoder_Interface_init(dtx);
  if (enc == NULL)
  {
    DBG("failed\n");
    return NULL;
  }else{
    DBG("OK\n");
    return enc;
  }
  //  return (Encoder_Interface_init(dtx));
}
static void ph_amr_enc_cleanup(void *ctx)
{
  Encoder_Interface_exit(ctx);
}
static void *ph_amr_dec_init(void *dummy)
{
  void *dec;
  DBG("AMR DEC initializing...");
  dec = Decoder_Interface_init();
  if (dec == NULL)
  {
    DBG("failed\n");
    return NULL;
  }else{
    DBG("OK\n");
    return dec;
  } 
  // return (Decoder_Interface_init());
}
static void ph_amr_dec_cleanup(void *ctx)
{
  Decoder_Interface_exit(ctx);
}
static phcodec_t amr_codec =
{
  "AMR", 8000,
   AMR_ENCODED_FRAME_SIZE, AMR_FRAME_SAMPLES*2, ph_amr_enc_init, ph_amr_dec_init, ph_amr_enc_cleanup, ph_amr_dec_cleanup, ph_amr_encode, ph_amr_decode
};
#endif
#endif


#ifdef ENABLE_AMR_WB

extern const UWord8 block_size[];

static int ph_amrwb_encode(void *ctx, const void *src, int srcsize, void *dst, int dstsize);
static int ph_amrwb_decode(void *ctx, const void *src, int srcsize, void *dst, int dstsize);
static void *ph_amrwb_enc_init(void *);
static void ph_amrwb_enc_cleanup(void *ctx);
static void *ph_amrwb_dec_init(void *);
static void ph_amrwb_dec_cleanup(void *ctx);

int enc_cnt, dec_cnt;

static int ph_amrwb_encode(void *ctx, const void *src, int srcsize, void *dst, int dstsize)
{
  Word16 *inp = (Word16 *)src;
  UWord8 *out = (UWord8 *)dst+2;
  Word16 mode = AMRWB_DEFAULT_MODE;
  Word16 allow_dtx;
  Word16 coded_size;
  amr_rtphdr *hdr = (amr_rtphdr *)dst;

#ifdef AMR_DTX
  allow_dtx = 1;
#else
  allow_dtx = 0;
#endif

  coded_size = E_IF_encode(ctx, mode, inp, out, allow_dtx);
  hdr->cmr = mode << 4;
  hdr->toc = *out & 0x07ff;  /* only 1 frame F=0 */
  hdr->toc |= 4;  /*  Q=1 */
  return coded_size+2;
}
static int ph_amrwb_decode(void *ctx, const void *src, int srcsize, void *dst, int dstsize)
{
  Word16 *out = (Word16 *)dst;
  UWord8 *inp = (UWord8 *)src+2;
  amr_rtphdr *hdr = (amr_rtphdr *)src;

  D_IF_decode( ctx, inp,  out, _good_frame);
  return(AMRWB_FRAME_SAMPLES*2);
}

static void *ph_amrwb_enc_init(void *dummy)
{
  DBG("AMR-WB ENC initializing...");
  return (E_IF_init());
}
static void ph_amrwb_enc_cleanup(void *ctx)
{
  DBG("AMR-WB ENC cleanup");
  E_IF_exit(ctx);
}
static void *ph_amrwb_dec_init(void *dummy)
{
  DBG("AMR-WB DEC initializing...");
  return (D_IF_init());
}
static void ph_amrwb_dec_cleanup(void *ctx)
{
  DBG("AMR-WB DEC cleanup");
  D_IF_exit(ctx);
}


static phcodec_t amr_wb_codec =
{
  "AMR-WB", 16000, 
  AMRWB_ENCODED_FRAME_SIZE, AMRWB_FRAME_SAMPLES*2, 
  ph_amrwb_enc_init, ph_amrwb_dec_init, ph_amrwb_enc_cleanup, ph_amrwb_dec_cleanup,  ph_amrwb_encode, ph_amrwb_decode
};
#endif


#ifdef WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif


DLLEXPORT int ph_codec_plugin_init(void (*codec_register)(phcodec_t *))
{
#ifndef PH_SPEEXWB_REPLACES_AMRWB
  codec_register(&amr_wb_codec);
#endif

#ifndef PH_SPEEXWB_REPLACES_AMRNB
 codec_register(&amr_codec);
#endif
  return 0;
}


#ifdef WIN32
int ph_codec_amr_codecs_init(void (*codec_register)(phcodec_t *))
{
  ph_codec_plugin_init(codec_register);
}
#endif
