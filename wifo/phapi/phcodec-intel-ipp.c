#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
/* */

#define G729_FRAME_SAMPLES 160
#define G729_ENCODED_FRAME_SIZE (20)



#include <usc.h>
//


#define ERROR printf

struct stream
{
  USC_Handle  handle;
  int nBanks;
  USC_MemBank*  banks;
  USC_CodecInfo pInfo;  
};



extern USC_Fxns USC_G729I_Fxns;
USC_Fxns *fns = &USC_G729I_Fxns; 



static void no_printf(const char *x, ...) { }


static int 
stream_alloc(USC_CodecInfo *info, struct stream *md, const char *name)  
{
  
  int i;

  if (USC_NoError != fns->std.NumAlloc(&info->params, &md->nBanks))
    {
      ERROR("g729_stream_alloc: can't query memory reqirements for %s\n", name);
      return -1;
    }

  /* allocate memory for memory bank table */
  md->banks  =  (USC_MemBank*)malloc(sizeof(USC_MemBank)*md->nBanks);
  
  /* Query how big has to be each block */
  if (USC_NoError != fns->std.MemAlloc(&info->params, md->banks))
    {
      ERROR("g729_stream_alloc: can't query memory bank size for %s\n", name);
      return -1;
    }
 

    /* allocate memory for each block */
  for(i=0; i<md->nBanks; i++)
    {
        md->banks[i].pMem = (unsigned char*)malloc(md->banks[i].nbytes);
    }
   
  return 0;
}


static void
stream_free(struct stream *st)
{
  int i;

  for(i = 0; i < st->nBanks; i++)
    free(st->banks[i].pMem);

  free(st->banks);

}


static int
stream_create(USC_CodecInfo *info, struct stream *st, const char *name)
{
   if (stream_alloc(info, st, name))
      {
	return -1;
      }

    /* Create encoder instance */
    if(USC_NoError != fns->std.Init(&info->params, st->banks, &st->handle))
      {
	ERROR("g729_stream_create: can't intialize stream %s\n", name);
	stream_free(st);
	return -1;
      }

    return 0;
}


static void
stream_destroy(struct stream *st)
{
  stream_free(st);
  free(st);
}




void *ph_g729_enc_init(void *dummy);
void *ph_g729_dec_init(void *dummy);
int ph_g729_encode(void *ctx, const void *src, int srcsize, void *dst, int dstsize);
int ph_g729_decode(void *ctx, const void *src, int srcsize, void *dst, int dstsize);
void ph_g729_enc_cleanup(void *ctx);
void ph_g729_dec_cleanup(void *ctx);


#ifndef TEST
#include "phcodec.h"

static phcodec_t g729_codec =
{
  "G729", 8000, G729_ENCODED_FRAME_SIZE, G729_FRAME_SAMPLES*2, 
  ph_g729_enc_init, ph_g729_dec_init, 
  ph_g729_enc_cleanup, ph_g729_dec_cleanup,  
  ph_g729_encode, ph_g729_decode
};
#endif


void *ph_g729_enc_init(void *dummy)
{
    USC_CodecInfo pInfo;  
    struct stream *st;


    if (USC_NoError != fns->std.GetInfo((USC_Handle)NULL, &pInfo))
      {
	ERROR("g729: Can't query codec info\n");
	return (0);
      }
  

    pInfo.params.direction = 0;             /* Direction: encode */
    pInfo.params.modes.vad = 0;         /* Suppress a silence compression */
    pInfo.params.law = 0;                    /* Linear PCM input */
    pInfo.params.modes.bitrate = 8000; // pInfo.pRateTbl[pInfo.nRates-1].bitrate;     /*Set highest bitrate*/


    st = calloc(sizeof(struct stream), 1);

    if (stream_create(&pInfo, st, "encoder"))
      {
	free(st);
	return 0;
      }
     
    st->pInfo = pInfo;

    return st;
}



void *ph_g729_dec_init(void *dummy)
{
    USC_CodecInfo pInfo;  
    struct stream *st;


    if (USC_NoError != fns->std.GetInfo((USC_Handle)NULL, &pInfo))
      {
	ERROR("g729: Can't query codec info\n");
	return (0);
      }
  

    pInfo.params.direction = 1;             /* Direction: encode */
    pInfo.params.modes.vad = 0;         /* Suppress a silence compression */
    pInfo.params.law = 0;                    /* Linear PCM input */
    pInfo.params.modes.bitrate = 8000; // pInfo.pRateTbl[pInfo.nRates-1].bitrate;     /*Set highest bitrate*/


    st = calloc(sizeof(struct stream), 1);

    if (stream_create(&pInfo, st, "decoder"))
      {
	free(st);
	return 0;
      }
     
    st->pInfo = pInfo;

    return st;

}





int ph_g729_encode(void *ctx, const void *src, int srcsize, void *dst, int dstsize)
{
    div_t blocks;
    struct stream  *codec = (struct stream *) ctx;
    int out_size = 0;
    int err;
    unsigned char *in_buf = (unsigned char *) src;
    unsigned char *out_buf = (unsigned char *) dst;
    

    if (!ctx)
	{
      return 0;
	}
    
    blocks = div(srcsize, codec->pInfo.framesize);

    if (blocks.rem)
      {
	ERROR("pcm16_2_G729: number of blocks should be integral (block size = %d)\n", codec->pInfo.framesize);
	return -1;
      }

   

    while(srcsize >= codec->pInfo.framesize) 
      {
        USC_PCMStream in;
        USC_Bitstream out;
   

        /* Set input stream params */
        in.bitrate = codec->pInfo.params.modes.bitrate;
        in.nbytes = srcsize;
        in.pBuffer = in_buf;
        in.pcmType = codec->pInfo.pcmType;

        /* Set output buffer */
        out.pBuffer = out_buf;

        /* Encode a frame  */
	err = fns->Encode (codec->handle, &in, &out);
        if (USC_NoError != err)
	  {
	    ERROR("pcm16_2_G729: error %d encoding\n", err);
	    return -1;
	  }
    
	srcsize -= in.nbytes;
	in_buf += in.nbytes;

	out_buf += out.nbytes;
	out_size += out.nbytes;
      }

    return out_size;
}









int ph_g729_decode(void *ctx, const void *src, int srcsize, void *dst, int dstsize)
{
    div_t blocks;
    unsigned int out_size = 0;
    int frameSize = 0;
    int x;
    struct stream *codec = (struct stream *) ctx;
    int err;
    unsigned char *in_buf = (unsigned char *) src;
    unsigned char *out_buf = (unsigned char *) dst;
    int origsize = srcsize;

    if (!ctx)
	{
      return 0;
	}

    for(x = 0; x < origsize; x += frameSize) 
      {
        USC_PCMStream out;
        USC_Bitstream in;
	
	
	in.pBuffer = in_buf;
	in.nbytes = srcsize;
	in.bitrate = codec->pInfo.params.modes.bitrate;
	in.frametype = 3;

	out.pcmType = codec->pInfo.pcmType;
	out.pBuffer = out_buf;


	err = fns->Decode (codec->handle, &in, &out);
	if (USC_NoError != err)
	  {
	    ERROR("ph_g729_decode: error %d decoding data\n", err);
	    break;
	  }

	in_buf += in.nbytes;
	frameSize = in.nbytes;
	srcsize -= in.nbytes;

	out_buf += out.nbytes;
	out_size += out.nbytes;
      }

    return out_size;

}



void ph_g729_enc_cleanup(void *ctx)
{
  struct stream *codec = (struct stream *) ctx;

  stream_destroy(codec);
}



void ph_g729_dec_cleanup(void *ctx)
{
  struct stream *codec = (struct stream *) ctx;

  stream_destroy(codec);
}

#ifndef TEST
#ifdef WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif



DLLEXPORT int ph_codec_plugin_init(void (*codec_register)(phcodec_t *))
{
  codec_register(&g729_codec);

  return 0;
}

#endif

#ifdef TEST
int main(int argc, char *argv[])
{
	int ifile, ofile;
	void *codec;
	char ibuf[1024];
	char obuf[1024];
	int rn, cn, wn;
	
	ifile = open(argv[2], O_RDONLY|O_BINARY);
	ofile = open(argv[3], O_WRONLY|O_CREAT|O_BINARY|O_TRUNC, _S_IREAD|_S_IWRITE);
	if (ofile < 0)
    {
            perror("output file");
            exit(1);
    }
    
	if (!strcmp(argv[1], "-d"))
	{
		codec = ph_g729_dec_init(0);
		while(1)
		{


			rn = read(ifile, ibuf, G729_ENCODED_FRAME_SIZE);
			if (!rn)
			{
				break;
			}
			cn = ph_g729_decode(codec, ibuf, rn, obuf, sizeof(obuf));
			wn = write(ofile, obuf, cn);
		}
		ph_g729_dec_cleanup(codec);
	}
	else if (!strcmp(argv[1], "-e"))
	{
		codec = ph_g729_enc_init(0);
		while(1)
		{


			rn = read(ifile, ibuf, G729_FRAME_SAMPLES*2);
           // printf("read %d bytes\n", rn);
			if (!rn)
			{
				break;
			}
			cn = ph_g729_encode(codec, ibuf, rn, obuf, sizeof(obuf));
			wn = write(ofile, obuf, cn);
            // printf("wrote %d bytes\n", wn);
            if (wn < 0)
            {
                perror("writing file");
                exit(1);
            }

		}
		ph_g729_enc_cleanup(codec);
	}

	close(ifile);
	close(ofile);

	exit(0);
}


#endif
	

			
			