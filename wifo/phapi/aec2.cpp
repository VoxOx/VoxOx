/* aec2.cpp
 *
 * Copyright (C) DFS Deutsche Flugsicherung (2004, 2005). 
 * All Rights Reserved.
 *
 * Acoustic Echo Cancellation NLMS-pw algorithm
 *
 * Version 0.3 filter created with www.dsptutor.freeuk.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "aec2.h"

#define UNROLLPTR 
#define LOOPSTEP 4
#if 0
#define LOOPSTEP 2
#endif

#ifdef UNROLLPTR
REAL dotp(REAL a[], REAL b[])
{
  REAL sum0 = 0.0, sum1 = 0.0;
#if LOOPSTEP == 4
  REAL sum2 = 0.0, sum3 = 0.0;
#endif
  REAL *enda = a + NLMS_LEN;

  for (; a < enda; a += LOOPSTEP, b += LOOPSTEP) {
    // optimize: partial loop unrolling
    sum0 += a[0] * b[0];
    sum1 += a[1] * b[1];
#if LOOPSTEP == 4
    sum2 += a[2] * b[2];
    sum3 += a[3] * b[3];
#endif
  }

#if LOOPSTEP == 4
  return sum0 + sum1 + sum2 + sum3;
#else
  return sum0 + sum1;
#endif
}
#else
/* Vector Dot Product */
REAL dotp(REAL a[], REAL b[])
{
  REAL sum0 = 0.0, sum1 = 0.0;
#if LOOPSTEP == 4
  REAL sum2 = 0.0, sum3 = 0.0;
#endif
  int j;

  for (j = 0; j < NLMS_LEN; j += LOOPSTEP) {
    // optimize: partial loop unrolling
    sum0 += a[j] * b[j];
    sum1 += a[j + 1] * b[j + 1];
#if LOOPSTEP == 4
    sum2 += a[j+2] * b[j+2];
    sum3 += a[j + 3] * b[j + 3];
#endif
  }

#if LOOPSTEP == 4
  return sum0 + sum1 + sum2 + sum3;
#else
  return sum0 + sum1;
#endif
}
#endif  /* UNROLLPTR */



/* Exponential Smoothing or IIR Infinite Impulse Response Filter */
class IIR_HP {
  REAL x;

public:
   IIR_HP() {
    x = 0.0f;
  };
  REAL highpass(REAL in) {
    const REAL a0 = 0.01f;      /* controls Transfer Frequency */
    /* Highpass = Signal - Lowpass. Lowpass = Exponential Smoothing */
    x += a0 * (in - x);
    return in - x;
  };
};

/* 13 taps FIR Finite Impulse Response filter
 * Coefficients calculated with
 * www.dsptutor.freeuk.com/KaiserFilterDesign/KaiserFilterDesign.html
 */
class FIR_HP13 {
  REAL z[14];

public:
   FIR_HP13() {
    memset(this, 0, sizeof(FIR_HP13));
  };
  REAL highpass(REAL in) {
    const REAL a[14] = {
      // Kaiser Window FIR Filter, Filter type: High pass
      // Passband: 300.0 - 4000.0 Hz, Order: 12
      // Transition band: 100.0 Hz, Stopband attenuation: 10.0 dB
      -0.043183226f, -0.046636667f, -0.049576525f, -0.051936015f,
      -0.053661242f, -0.054712527f, 0.82598513f, -0.054712527f,
      -0.053661242f, -0.051936015f, -0.049576525f, -0.046636667f,
      -0.043183226f, 0.0f
    };
    memmove(z + 1, z, 13 * sizeof(REAL));
    z[0] = in;
    REAL sum0 = 0.0, sum1 = 0.0;
    int j;

    for (j = 0; j < 14; j += 2) {
      // optimize: partial loop unrolling
      sum0 += a[j] * z[j];
      sum1 += a[j + 1] * z[j + 1];
    }
    return sum0 + sum1;
  }
};

/* Recursive single pole IIR Infinite Impulse response filter
 * Coefficients calculated with
 * http://www.dsptutor.freeuk.com/IIRFilterDesign/IIRFiltDes102.html
 */
class IIR1 {
  REAL x, y;

public:
   IIR1() {
    memset(this, 0, sizeof(IIR1));
  };
  REAL highpass(REAL in) {
    // Chebyshev IIR filter, Filter type: HP
    // Passband: 3700 - 4000.0 Hz
    // Passband ripple: 1.5 dB, Order: 1
    const REAL a0 = 0.105831884f;
    const REAL a1 = -0.105831884;
    const REAL b1 = 0.78833646f;
    REAL out = a0 * in + a1 * x + b1 * y;
    x = in;
    y = out;
    return out;
  }
};

/* Recursive two pole IIR Infinite Impulse Response filter
 * Coefficients calculated with
 * http://www.dsptutor.freeuk.com/IIRFilterDesign/IIRFiltDes102.html
 */
class IIR2 {
  REAL x[2], y[2];

public:
   IIR2() {
    memset(this, 0, sizeof(IIR2));
  };
  REAL highpass(REAL in) {
    // Butterworth IIR filter, Filter type: HP
    // Passband: 2000 - 4000.0 Hz, Order: 2
    const REAL a[] = { 0.29289323f, -0.58578646f, 0.29289323f };
    const REAL b[] = { 1.3007072E-16f, 0.17157288f };
    REAL out =
      a[0] * in + a[1] * x[0] + a[2] * x[1] - b[0] * y[0] - b[1] * y[1];

    x[1] = x[0];
    x[0] = in;
    y[1] = y[0];
    y[0] = out;
    return out;
  }
};


class AEC: public AEC_Base {
  // Time domain Filters
  IIR_HP hp00, hp1;             // DC-level remove Highpass)
  FIR_HP13 hp0;                 // 300Hz cut-off Highpass
  IIR1 Fx, Fe;                  // pre-whitening Highpass for x, e

  // Geigel DTD (Double Talk Detector)
  REAL max_max_x;               // max(|x[0]|, .. |x[L-1]|)
  int hangover;
  // optimize: less calculations for max()
  REAL max_x[NLMS_LEN / DTD_LEN];
  int dtdCnt;
  int dtdNdx;

  // NLMS-pw
  REAL x[NLMS_LEN + NLMS_EXT];  // tap delayed loudspeaker signal
  REAL xf[NLMS_LEN + NLMS_EXT]; // pre-whitening tap delayed signal
  REAL w[NLMS_LEN];             // tap weights
  int j;                        // optimize: less memory copies
  int lastupdate;               // optimize: iterative dotp(x,x)
  double dotp_xf_xf;            // double to avoid loss of precision
  REAL s0avg;

public:
   AEC();

/* Geigel Double-Talk Detector
 *
 * in d: microphone sample (PCM as REALing point value)
 * in x: loudspeaker sample (PCM as REALing point value)
 * return: 0 for no talking, 1 for talking
 */
  int dtd(REAL d, REAL x);

/* Normalized Least Mean Square Algorithm pre-whitening (NLMS-pw)
 * The LMS algorithm was developed by Bernard Widrow
 * book: Widrow/Stearns, Adaptive Signal Processing, Prentice-Hall, 1985
 *
 * in mic: microphone sample (PCM as REALing point value)
 * in spk: loudspeaker sample (PCM as REALing point value)
 * in update: 0 for convolve only, 1 for convolve and update 
 * return: echo cancelled microphone sample
 */
  REAL nlms_pw(REAL mic, REAL spk, int update);

/* Acoustic Echo Cancellation and Suppression of one sample
 * in   d:  microphone signal with echo
 * in   x:  loudspeaker signal
 * return:  echo cancelled microphone signal
 */
  int doAEC(int d, int x);

  REAL getambient() const {
    return s0avg;
  };
  void setambient(REAL Min_xf) {
    dotp_xf_xf = NLMS_LEN * Min_xf * Min_xf;
  };
};



AEC::AEC()
{
  max_max_x = 0.0f;
  hangover = 0;
  memset(max_x, 0, sizeof(max_x));
  dtdCnt = dtdNdx = 0;

  memset(x, 0, sizeof(x));
  memset(xf, 0, sizeof(xf));
  memset(w, 0, sizeof(w));
  j = NLMS_EXT;
  lastupdate = 0;
  s0avg = M80dB_PCM;
  setambient(Min_xf);
}

REAL AEC::nlms_pw(REAL mic, REAL spk, int update)
{
  REAL d = mic;                 // desired signal
  x[j] = spk;
  xf[j] = Fx.highpass(spk);     // pre-whitening of x

  // calculate error value 
  // (mic signal - estimated mic signal from spk signal)
  REAL e = d - dotp(w, x + j);
  REAL ef = Fe.highpass(e);     // pre-whitening of e
  // optimize: iterative dotp(xf, xf)
  dotp_xf_xf +=
    (xf[j] * xf[j] - xf[j + NLMS_LEN - 1] * xf[j + NLMS_LEN - 1]);
  if (update) {
    // calculate variable step size
    REAL mikro_ef = Stepsize * ef / dotp_xf_xf;
#ifdef UNROLLPTR
	REAL *wp = w, *xfp = xf+j, *endw = w + NLMS_LEN;
    for (; wp < endw; wp += LOOPSTEP, xfp += LOOPSTEP) {
      // optimize: partial loop unrolling
      wp[0] += mikro_ef * xfp[0];
      wp[1] += mikro_ef * xfp[1];
#if LOOPSTEP == 4
      wp[2] += mikro_ef * xfp[2];
      wp[3] += mikro_ef * xfp[3];
#endif
    }

#else
    // update tap weights (filter learning)
    int i;

    for (i = 0; i < NLMS_LEN; i += LOOPSTEP) {
      // optimize: partial loop unrolling
      w[i] += mikro_ef * xf[i + j];
      w[i + 1] += mikro_ef * xf[i + j + 1];
#if LOOPSTEP == 4
      w[i+2] += mikro_ef * xf[i + j + 2];
      w[i + 3] += mikro_ef * xf[i + j + 3];
#endif
    }
#endif  /* UNROLLPTR */
  }

  if (--j < 0) {
    // optimize: decrease number of memory copies
    j = NLMS_EXT;
    memmove(x + j + 1, x, (NLMS_LEN - 1) * sizeof(REAL));
    memmove(xf + j + 1, xf, (NLMS_LEN - 1) * sizeof(REAL));
  }

  return e;
}


int AEC::dtd(REAL d, REAL x)
{
  // optimized implementation of max(|x[0]|, |x[1]|, .., |x[L-1]|):
  // calculate max of block (DTD_LEN values)
  x = fabsf(x);
  if (x > max_x[dtdNdx]) {
    max_x[dtdNdx] = x;
    if (x > max_max_x) {
      max_max_x = x;
    }
  }
  if (++dtdCnt >= DTD_LEN) {
    dtdCnt = 0;
    // calculate max of max
    max_max_x = 0.0f;
    for (int i = 0; i < NLMS_LEN / DTD_LEN; ++i) {
      if (max_x[i] > max_max_x) {
        max_max_x = max_x[i];
      }
    }
    // rotate Ndx
    if (++dtdNdx >= NLMS_LEN / DTD_LEN)
	{
      dtdNdx = 0;
	}
    max_x[dtdNdx] = 0.0f;
  }
  // The Geigel DTD algorithm with Hangover timer Thold
  if (fabsf(d) >= GeigelThreshold * max_max_x) {
    hangover = Thold;
  }

  if (hangover)
  {
    --hangover;
  }

  return (hangover > 0);
}


int AEC::doAEC(int d, int x)
{
  REAL s0 = (REAL) d;
  REAL s1 = (REAL) x;

  // Mic Highpass Filter - to remove DC
  // s0 = hp00.highpass(s0);

  // Mic Highpass Filter - telephone users are used to 300Hz cut-off
  s0 = hp0.highpass(s0);

  // ambient mic level estimation
  s0avg += 1e-4f * (fabsf(s0) - s0avg);

  // Spk Highpass Filter - to remove DC
  s1 = hp1.highpass(s1);

  // Double Talk Detector
  int update = !dtd(s0, s1);

  // Acoustic Echo Cancellation
  s0 = nlms_pw(s0, s1, update);

  // Acoustic Echo Suppression
  if (update) {
    // Non Linear Processor (NLP): attenuate low volumes
    s0 *= NLPAttenuation;
  }
  // Saturation
  if (s0 > MAXPCM) {
    return (int) MAXPCM;
  } else if (s0 < -MAXPCM) {
    return (int) -MAXPCM;
  } else {
    return (int) (s0+0.5f);
  }
}

extern "C"
{
#include "fidlib.h"
}

class FidlibFilter
{
  FidFilter *filt;
  FidFunc    *funcp;
  FidRun     *run;
  void       *fbuf;

  
public:
  FidlibFilter(const char *spec, int f0, int f1)
  {

    filt = fid_design((char*)spec, f0, f1, 0,0,0);
    run = fid_run_new(filt, &funcp);
    fbuf = fid_run_newbuf(run);
  }

  double highpass(REAL in)
  {
    return funcp(fbuf, (double) in);
  }

  ~FidlibFilter()
  {
    fid_run_freebuf(fbuf);
    fid_run_free(run);
    free(filt);
  }

};


class IIR_HP16KHZ: public FidlibFilter
{
public:
  IIR_HP16KHZ() : FidlibFilter("HpBe1", 16000, 10) { }
};

class FIR_HP13_16KHZ: public FidlibFilter
{
public:
  FIR_HP13_16KHZ() : FidlibFilter("HpBq1/0.7", 16000, 300) { }
};


class IIR1_16KHZ: public FidlibFilter
{
public:
  IIR1_16KHZ() : FidlibFilter("HpBe1", 16000, 3700) { }
};





class AEC16KHZ: public AEC_Base {
  // Time domain Filters
  IIR_HP16KHZ hp00, hp1;             // DC-level remove Highpass)
  FIR_HP13_16KHZ hp0;                 // 300Hz cut-off Highpass
  IIR1_16KHZ Fx, Fe;                  // pre-whitening Highpass for x, e

  // Geigel DTD (Double Talk Detector)
  REAL max_max_x;               // max(|x[0]|, .. |x[L-1]|)
  int hangover;
  // optimize: less calculations for max()
  REAL max_x[NLMS_LEN / DTD_LEN];
  int dtdCnt;
  int dtdNdx;

  // NLMS-pw
  REAL x[NLMS_LEN + NLMS_EXT];  // tap delayed loudspeaker signal
  REAL xf[NLMS_LEN + NLMS_EXT]; // pre-whitening tap delayed signal
  REAL w[NLMS_LEN];             // tap weights
  int j;                        // optimize: less memory copies
  int lastupdate;               // optimize: iterative dotp(x,x)
  double dotp_xf_xf;            // double to avoid loss of precision
  REAL s0avg;

public:
   AEC16KHZ();

/* Geigel Double-Talk Detector
 *
 * in d: microphone sample (PCM as REALing point value)
 * in x: loudspeaker sample (PCM as REALing point value)
 * return: 0 for no talking, 1 for talking
 */
  int dtd(REAL d, REAL x);

/* Normalized Least Mean Square Algorithm pre-whitening (NLMS-pw)
 * The LMS algorithm was developed by Bernard Widrow
 * book: Widrow/Stearns, Adaptive Signal Processing, Prentice-Hall, 1985
 *
 * in mic: microphone sample (PCM as REALing point value)
 * in spk: loudspeaker sample (PCM as REALing point value)
 * in update: 0 for convolve only, 1 for convolve and update 
 * return: echo cancelled microphone sample
 */
  REAL nlms_pw(REAL mic, REAL spk, int update);

/* Acoustic Echo Cancellation and Suppression of one sample
 * in   d:  microphone signal with echo
 * in   x:  loudspeaker signal
 * return:  echo cancelled microphone signal
 */
  int doAEC(int d, int x);

  REAL getambient() const {
    return s0avg;
  };
  void setambient(REAL Min_xf) {
    dotp_xf_xf = NLMS_LEN * Min_xf * Min_xf;
  };
};




AEC16KHZ::AEC16KHZ()
{
  max_max_x = 0.0f;
  hangover = 0;
  memset(max_x, 0, sizeof(max_x));
  dtdCnt = dtdNdx = 0;

  memset(x, 0, sizeof(x));
  memset(xf, 0, sizeof(xf));
  memset(w, 0, sizeof(w));
  j = NLMS_EXT;
  lastupdate = 0;
  s0avg = M80dB_PCM;
  setambient(Min_xf);
}

REAL AEC16KHZ::nlms_pw(REAL mic, REAL spk, int update)
{
  REAL d = mic;                 // desired signal
  x[j] = spk;
  xf[j] = Fx.highpass(spk);     // pre-whitening of x

  // calculate error value 
  // (mic signal - estimated mic signal from spk signal)
  REAL e = d - dotp(w, x + j);
  REAL ef = Fe.highpass(e);     // pre-whitening of e
  // optimize: iterative dotp(xf, xf)
  dotp_xf_xf +=
    (xf[j] * xf[j] - xf[j + NLMS_LEN - 1] * xf[j + NLMS_LEN - 1]);
  if (update) {
    // calculate variable step size
    REAL mikro_ef = Stepsize * ef / dotp_xf_xf;
#ifdef UNROLLPTR
	REAL *wp = w, *xfp = xf+j, *endw = w + NLMS_LEN;
    for (; wp < endw; wp += LOOPSTEP, xfp += LOOPSTEP) {
      // optimize: partial loop unrolling
      wp[0] += mikro_ef * xfp[0];
      wp[1] += mikro_ef * xfp[1];
#if LOOPSTEP == 4
      wp[2] += mikro_ef * xfp[2];
      wp[3] += mikro_ef * xfp[3];
#endif
    }
#else
    // update tap weights (filter learning)
    int i;
    for (i = 0; i < NLMS_LEN; i += 2) {
      // optimize: partial loop unrolling
      w[i] += mikro_ef * xf[i + j];
      w[i + 1] += mikro_ef * xf[i + j + 1];
    }
#endif /* UNROLLPTR */

  }
  if (--j < 0) {
    // optimize: decrease number of memory copies
    j = NLMS_EXT;
    memmove(x + j + 1, x, (NLMS_LEN - 1) * sizeof(REAL));
    memmove(xf + j + 1, xf, (NLMS_LEN - 1) * sizeof(REAL));
  }

  return e;
}


int AEC16KHZ::dtd(REAL d, REAL x)
{
  // optimized implementation of max(|x[0]|, |x[1]|, .., |x[L-1]|):
  // calculate max of block (DTD_LEN values)
  x = fabsf(x);
  if (x > max_x[dtdNdx]) {
    max_x[dtdNdx] = x;
    if (x > max_max_x) {
      max_max_x = x;
    }
  }
  if (++dtdCnt >= DTD_LEN) {
    dtdCnt = 0;
    // calculate max of max
    max_max_x = 0.0f;
    for (int i = 0; i < NLMS_LEN / DTD_LEN; ++i) {
      if (max_x[i] > max_max_x) {
        max_max_x = max_x[i];
      }
    }
    // rotate Ndx
    if (++dtdNdx >= NLMS_LEN / DTD_LEN)
	{
      dtdNdx = 0;
	}
    max_x[dtdNdx] = 0.0f;
  }
  // The Geigel DTD algorithm with Hangover timer Thold
  if (fabsf(d) >= GeigelThreshold * max_max_x) {
    hangover = Thold;
  }

  if (hangover)
  {
    --hangover;
  }

  return (hangover > 0);
}


int AEC16KHZ::doAEC(int d, int x)
{
  REAL s0 = (REAL) d;
  REAL s1 = (REAL) x;

  // Mic Highpass Filter - to remove DC
  // s0 = hp00.highpass(s0);

  // Mic Highpass Filter - telephone users are used to 300Hz cut-off
  s0 = hp0.highpass(s0);

  // ambient mic level estimation
  s0avg += 1e-4f * (fabsf(s0) - s0avg);

  // Spk Highpass Filter - to remove DC
  s1 = hp1.highpass(s1);

  // Double Talk Detector
  int update = !dtd(s0, s1);

  // Acoustic Echo Cancellation
  s0 = nlms_pw(s0, s1, update);

  // Acoustic Echo Suppression
  if (update) {
    // Non Linear Processor (NLP): attenuate low volumes
    s0 *= NLPAttenuation;
  }
  // Saturation
  if (s0 > MAXPCM) {
    return (int) MAXPCM;
  } else if (s0 < -MAXPCM) {
    return (int) -MAXPCM;
  } else {
    return (int) (s0+0.5f);
  }
}








extern "C" {

void *create_AEC(int boost, int samplingfreq){
  AEC_Base *aec;
  if (samplingfreq == 16000)
  {
    aec =  new AEC16KHZ();
  }
  else
  {
    aec =  new AEC();
  }

  aec->setambient(M55dB_PCM);
  return aec;
}

short do_AEC(void *ec, short ref, short mic){
  if(ec)
  {
    return((short)(((class AEC_Base *)ec)->doAEC((int)mic, (int)ref)));
	}

  return mic;
}

void kill_AEC(void *ec){
  delete (class AEC_Base *)ec;
}

}
