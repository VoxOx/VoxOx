/*
 * phresample Audio data resampling
 *
 * Copyright (C) 2006, 2007 WENGO SAS
 * Copyright (C) 2004  Vadim Lebedev  <vadim@mbdsys.com>
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

#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifdef PH_USE_RESAMPLE
#include <samplerate.h>
#endif

#include "phlog.h"

void *ph_resample_init(void);
void ph_resample_cleanup(void *p);
void ph_downsample(void *rctx, void *framebuf, int framesize);
void ph_upsample(void *rctx, void *dbuf, void *framebuf, int framesize);
void * ph_resample_mic_init0(int clockrate, int actual_clockrate);
void * ph_resample_spk_init0(int clockrate, int actual_clockrate);
void ph_resample_audio0(void *ctx, void *inbuf, int inbsize, void *outbuf, int *outbsize);
void ph_resample_cleanup0(void* resample_audiodrv_ctx);

/**
 * file structure :
 *  - context structure , common for all resamplers
 *  - code
 *    - direct optimized code for /2 or *2 resampling (in the IF clause of PH_OPTIMIZE_RESAMPLE)
 *    - fidlib implementation (in the ELSE clause of PH_OPTIMIZE_RESAMPLE)
 *    - samplerate implementation (in the IF clause of PH_USE_RESAMPLE)
 */

#define PH_OPTIMIZE_RESAMPLE 1

#ifdef _MSC_VER
#define INLINE __inline
#else
#define INLINE __inline__
#endif

struct phresamplectx
{
  double     upsamplebuf[4];
  double     downsamplebuf[4];
#ifdef PH_USE_RESAMPLE
  /** lib samplerate structures for filters */
  SRC_STATE* src_state;
  SRC_DATA* src_data;
#endif
};


#if defined(PH_OPTIMIZE_RESAMPLE)

// Filter descriptions:
//   LpBu4/3600 == Lowpass Butterworth filter, order 4, -3.01dB frequency
//     3600
//
static INLINE double
dofilter(register double val, double buf[4]) {
   register double tmp, fir, iir;
   tmp= buf[0]; memmove(buf, buf+1, 3*sizeof(double));
   iir= val * 0.06750480601637321;
   iir -= 0.4514083390923062*tmp; fir= tmp;
   iir -= -0.2270502870808351*buf[0]; fir += buf[0]+buf[0];
   fir += iir;
   tmp= buf[1]; buf[1]= iir; val= fir;
   iir= val;
   iir -= 0.04574887683193848*tmp; fir= tmp;
   iir -= -0.1635911661136266*buf[2]; fir += buf[2]+buf[2];
   fir += iir;
   buf[3]= iir; val= fir;
   return val;
}

void *ph_resample_init(void)
{
  struct phresamplectx *ctx = calloc(sizeof(struct phresamplectx), 1);
  return ctx;
}

void ph_resample_cleanup(void *p)
{
  struct phresamplectx *ctx = (struct phresamplectx *)p;
  free(ctx);
}

#define SATURATE(x) ((x > 0x7fff) ? 0x7fff : ((x < ~0x7fff) ? ~0x7fff : x))

/**
 * @brief in-place downsample of a buffer by a factor of 2
 *         we first pass the signal through low-pass filter with cutoff freq 3700 Hz
 *         and then decimate by factor of 2 
 */
void ph_downsample(void *rctx, void *framebuf, int framesize)
{
  short *sp = (short *) framebuf; // 'narrow' buffer
  short *dp = (short *) framebuf; // 'wide' buffer
  double tmp;
  struct phresamplectx *ctx = (struct phresamplectx *)rctx;
  double *fbuf = ctx->downsamplebuf;

  framesize = (framesize / sizeof(short)) / 2;

  while( framesize-- )
    {
      int itmp;
      tmp = dofilter((double) *sp++, fbuf);
      itmp =   (int) (tmp + 0.5);
      *dp++ = SATURATE(itmp);
      dofilter((double) *sp++, fbuf);
    }
}


/**
 * @brief upsample of a buffer by a factor of 2
 *        we insert 0 between each sample and then pass  the signal through low-pass filter with cutoff freq 3700 Hz
 */
void ph_upsample(void *rctx, void *dbuf, void *framebuf, int framesize)
{
  short *sp = (short *) framebuf; // 'narrow' buffer
  short *dp = (short *) dbuf; // 'wide' buffer
  double tmp;
  struct phresamplectx *ctx = (struct phresamplectx *)rctx;
  double *fbuf = ctx->upsamplebuf;

  framesize = framesize / sizeof(short);

  while( framesize-- )
  {
    int itmp;
    tmp = dofilter((double) *sp++, fbuf);
    itmp = (int) (tmp*2.0 + 0.5);
    *dp++ = SATURATE(itmp);
    tmp = dofilter(0.0, fbuf);
    itmp = (int) (tmp*2.0 + 0.5);
    *dp++ = SATURATE(itmp);
  }
}

#else /* PH_OPTIMIZE_RESAMPLE */

#include "fidlib.h"

struct phresamplectx
{
  FidFilter *filt;
  FidFunc    *funcp;
  FidRun     *run;
  void       *upsamplebuf;
  void       *downsamplebuf;
};

void *ph_resample_init()
{
  struct phresamplectx *ctx = calloc(sizeof(struct phresamplectx), 1);

  if ( ctx ) {
	  ctx->filt = fid_design("LpBu4", 16000, 3600, 0, 0, 0);
	  ctx->run = fid_run_new(ctx->filt, &ctx->funcp);
	  ctx->upsamplebuf = fid_run_newbuf(ctx->run);
	  ctx->downsamplebuf = fid_run_newbuf(ctx->run);
  }

  return ctx;
}

void ph_resample_cleanup(void *p)
{
  struct phresamplectx *ctx = (struct phresamplectx *)p;

  if (!p) {
    return;
  }

  fid_run_freebuf(ctx->downsamplebuf);
  fid_run_freebuf(ctx->upsamplebuf);
  fid_run_free(ctx->run);
  free(ctx->filt);
  free(ctx);
}

/**
 * @brief in-place downsample of a buffer by a factor of 2
 *         we first pass the signal through low-pass filter with cutoff freq 3700 Hz
 *         and then decimate by factor of 2 
 */
void ph_downsample(void *rctx, void *framebuf, int framesize)
{
  short *sp = (short *) framebuf; // 'narrow' buffer
  short *dp = (short *) framebuf; // 'wide' buffer
  double tmp;
  struct phresamplectx *ctx = (struct phresamplectx *)rctx;
  void *fbuf = ctx->downsamplebuf;
  FidRun *run = ctx->run;
  FidFunc *funcp = ctx->funcp;

  framesize = (framesize / sizeof(short)) / 2;

  while( framesize-- )
  {
    tmp = funcp(fbuf, (double) *sp++);
    *dp++ = (short) (tmp + 0.5);
    funcp(fbuf, (double) *sp++);
  }
}

/**
 * @brief upsample of a buffer by a factor of 2
 *        we insert 0 between each sample and then pass  the signal through low-pass filter with cutoff freq 3700 Hz
 */
void ph_upsample(void *rctx, void *dbuf, void *framebuf, int framesize)
{
  short *sp = (short *) framebuf; // 'narrow' buffer
  short *dp = (short *) dbuf; // 'wide' buffer
  double tmp;
  struct phresamplectx *ctx = (struct phresamplectx *)rctx;
  void *fbuf = ctx->upsamplebuf;
  FidRun *run = ctx->run;
  FidFunc *funcp = ctx->funcp;

  framesize = framesize / sizeof(short);

  while( framesize-- )
  {
    tmp = funcp(fbuf, (double) *sp++);
    *dp++ = (short) (tmp + 0.5);
    tmp = funcp(fbuf, 0.0);
    *dp++ = (short) (tmp + 0.5);
  }
}

#endif


#ifdef PH_USE_RESAMPLE

void * ph_resample_mic_init0(int clockrate, int actual_clockrate)
{
  struct phresamplectx *ctx_mic = calloc(1, sizeof(struct phresamplectx));
  double recRatio = 0;
  int expected_clockrate = clockrate;
  int libsamplerate_error = 0;

#ifdef PH_FORCE_16KHZ
  expected_clockrate = 16000;
#endif

  recRatio = ((double)(1.0 * expected_clockrate) / (double)(actual_clockrate));
  ctx_mic->src_data = calloc(1, sizeof(SRC_DATA));
  ctx_mic->src_data->src_ratio = recRatio;
  ctx_mic->src_state = src_new(SRC_LINEAR, 1, &libsamplerate_error);
  if( libsamplerate_error )
  {
    DBG_DYNA_AUDIO_RESAMPLE("RESAMPLE: error in libsamplerate: %s\n", src_strerror(libsamplerate_error));
  }

  DBG_DYNA_AUDIO_RESAMPLE("RESAMPLE: ph_resample_mic_init1: expected = %d actual = %d rec=%f\n",
    expected_clockrate, actual_clockrate, recRatio);

  return  ctx_mic;
}

void * ph_resample_spk_init0(int clockrate, int actual_clockrate)
{
  struct phresamplectx *ctx_spk = calloc(1, sizeof(struct phresamplectx));
  double playRatio = 0;
  int expected_clockrate = clockrate;
  int libsamplerate_error = 0;

#ifdef PH_FORCE_16KHZ
  expected_clockrate = 16000;
#endif

  playRatio = (1.0 * actual_clockrate) / expected_clockrate;
  ctx_spk->src_data = calloc(1, sizeof(SRC_DATA));
  ctx_spk->src_data->src_ratio = playRatio;
  ctx_spk->src_state = src_new(SRC_LINEAR, 1, &libsamplerate_error);
  if( libsamplerate_error )
  {
    DBG_DYNA_AUDIO_RESAMPLE("RESAMPLE: error in libsamplerate: %s\n", src_strerror(libsamplerate_error));
  }

  DBG_DYNA_AUDIO_RESAMPLE("RESAMPLE: ph_resample_spk_init1: expected = %d actual = %d play=%f\n",
    expected_clockrate, actual_clockrate, playRatio);

  return ctx_spk;
}


void ph_resample_cleanup0(void* resample_audiodrv_ctx)
{
  struct phresamplectx *ctx = (struct phresamplectx *)resample_audiodrv_ctx;

  if (ctx)
  {
    src_delete(ctx->src_state);
    free(ctx);
  }
}

void ph_resample_audio0(void *ctx, void *inbuf, int inbsize/* #chars */, void *outbuf, int *outbsize)
{
  float finbuf[2048];
  float foutbuf[2048];
  struct phresamplectx *ctx_filter;
  int errorCode = 0;
  //outbsize is the excepted frame size after resampling
  int postsampling_framesize = *outbsize;
  int output_pin_final_frame_size = 0;

  if (!ctx)
  {
    return;
  }
  ctx_filter = (struct phresamplectx *)ctx;

  if (!postsampling_framesize)
  {
    /* #chars */
    postsampling_framesize = (int)(inbsize*ctx_filter->src_data->src_ratio);
  }

  //pointer to the input data samples
  ctx_filter->src_data->data_in = finbuf;
  //number of frames of data pointed to by data_in
  ctx_filter->src_data->input_frames = (long)inbsize / 2;

  //pointer to the output data samples
  ctx_filter->src_data->data_out = foutbuf;
  //Maximum number of frames pointer to by data_out
  ctx_filter->src_data->output_frames = 2048.0;

  //more data are comming
  ctx_filter->src_data->end_of_input = 0;

  //convert buffer from short to float
  src_short_to_float_array((short *)inbuf, finbuf, (int) ctx_filter->src_data->input_frames);

  //process resampling
  errorCode = src_process(ctx_filter->src_state, ctx_filter->src_data);
  if( errorCode )
  {
    DBG_DYNA_AUDIO_RESAMPLE("RESAMPLE: error in libresample: %s\n", src_strerror(errorCode));
    return;
  }

  if (ctx_filter->src_data->output_frames_gen > postsampling_framesize / 2)
  {
    /* #frames */
    output_pin_final_frame_size = postsampling_framesize / 2;
  } else {
    output_pin_final_frame_size = ctx_filter->src_data->output_frames_gen;
  }
  //convert buffer from float to short
  src_float_to_short_array(foutbuf, (short *)outbuf, (int) output_pin_final_frame_size);

  *outbsize =  output_pin_final_frame_size * 2;

  //hack: if resample filter is in acquisition mode (we hope it is only for the first samples),
  //it does not had out enough samples
  if (*outbsize != postsampling_framesize)
  {
    DBG_DYNA_AUDIO_RESAMPLE("RESAMPLE: acquisition mode: dropping !: generated: %d , wished: %d\n",
      *outbsize, postsampling_framesize);

    *outbsize = 0;
  }

  DBG_DYNA_AUDIO_RESAMPLE("RESAMPLE: ratio=%f in=%d out=%d input_frames_used: %d\n",
    ctx_filter->src_data->src_ratio,
    ctx_filter->src_data->input_frames,
    ctx_filter->src_data->output_frames_gen,
    ctx_filter->src_data->input_frames_used);
}

#endif /* PH_USE_RESAMPLE */
