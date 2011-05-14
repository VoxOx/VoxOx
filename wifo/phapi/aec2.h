/* aec.h
 *
 * Copyright (C) DFS Deutsche Flugsicherung (2004, 2005). 
 * All Rights Reserved.
 *
 * Acoustic Echo Cancellation NLMS-pw algorithm
 *
 * Version 0.3 filter created with www.dsptutor.freeuk.com
 */

#ifndef _AEC_H                  /* include only once */

// use double if your CPU does software-emulation of float
typedef float REAL;

/* dB Values */
const REAL M0dB = 1.0f;
const REAL M3dB = 0.71f;
const REAL M6dB = 0.50f;
const REAL M9dB = 0.35f;
const REAL M12dB = 0.25f;
const REAL M18dB = 0.125f;
const REAL M24dB = 0.063f;

/* dB values for 16bit PCM */
/* MxdB_PCM = 32767 * 10 ^(x / 20) */
const REAL M10dB_PCM = 10362.0f;
const REAL M20dB_PCM = 3277.0f;
const REAL M25dB_PCM = 1843.0f;
const REAL M30dB_PCM = 1026.0f;
const REAL M35dB_PCM = 583.0f;
const REAL M40dB_PCM = 328.0f;
const REAL M45dB_PCM = 184.0f;
const REAL M50dB_PCM = 104.0f;
const REAL M55dB_PCM = 58.0f;
const REAL M60dB_PCM = 33.0f;
const REAL M65dB_PCM = 18.0f;
const REAL M70dB_PCM = 10.0f;
const REAL M75dB_PCM = 6.0f;
const REAL M80dB_PCM = 3.0f;
const REAL M85dB_PCM = 2.0f;
const REAL M90dB_PCM = 1.0f;

const REAL MAXPCM = 32767.0f;

/* Design constants (Change to fine tune the algorithms */

/* The following values are for hardware AEC and studio quality 
 * microphone */

/* maximum NLMS filter length in taps. A longer filter length gives 
 * better Echo Cancellation, but slower convergence speed and
 * needs more CPU power (Order of NLMS is linear) */
#define NLMS_LEN  (240*8)

/* convergence speed. Range: >0 to <1 (0.2 to 0.7). Larger values give
 * more AEC in lower frequencies, but less AEC in higher frequencies. */
const REAL Stepsize = 0.4f;

/* minimum energy in xf. Range: M70dB_PCM to M50dB_PCM. Should be equal
 * to microphone ambient Noise level */
const REAL Min_xf = M75dB_PCM;

/* Double Talk Detector Speaker/Microphone Threshold. Range <=1
 * Large value (M0dB) is good for Single-Talk Echo cancellation, 
 * small value (M12dB) is good for Doulbe-Talk AEC */
const REAL GeigelThreshold = M6dB;

/* Double Talk Detector hangover in taps. Not relevant for Single-Talk 
 * AEC */
const int Thold = 30 * 8;

/* for Non Linear Processor. Range >0 to 1. Large value (M0dB) is good
 * for Double-Talk, small value (M12dB) is good for Single-Talk */
const REAL NLPAttenuation = M6dB;

/* Below this line there are no more design constants */



// Extention in taps to reduce mem copies
#define NLMS_EXT  (10*8)

// block size in taps to optimize DTD calculation 
#define DTD_LEN   16

class AEC_Base
{
 public:
  virtual ~AEC_Base() { }
  
  virtual int doAEC(int d, int x) = 0;
  virtual REAL getambient() const = 0;

  virtual void setambient(REAL Min_xf) = 0;
};

 


#define _AEC_H
#endif
