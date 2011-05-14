/*
 * Compander effect
 *
 * Written by Nick Bailey (nick@bailey-family.org.uk or
 *                         n.bailey@elec.gla.ac.uk)
 *
 * Copyright 1999 Chris Bagwell And Nick Bailey
 * This source code is freely redistributable and may be used for
 * any purpose.  This copyright notice must be maintained. 
 * Chris Bagwell And Nick Bailey are not responsible for 
 * the consequences of using this software.
 */

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "sox.h"

/*
 * Compressor/expander effect for dsp.
 *
 * Flow diagram for one channel:
 *
 *		 ------------	   ---------------
 *		|	     |	  |		  |	---
 * ibuff ---+---| integrator |--->| transfer func |--->|   |
 *	    |	|	     |	  |		  |    |   |
 *	    |	 ------------	   ---------------     |   |  * gain
 *	    |					       | * |----------->obuff
 *	    |	    -------			       |   |
 *	    |	   |	   |			       |   |
 *	    +----->| delay |-------------------------->|   |
 *		   |	   |			        ---
 *		    -------
 *
 * Usage:
 *   compand attack1,decay1[,attack2,decay2...]
 *                  in-dB1,out-dB1[,in-dB2,out-dB2...]
 *                 [ gain [ initial-volume [ delay ] ] ] 
 *
 * Note: clipping can occur if the transfer function pushes things too
 * close to 0 dB.  In that case, use a negative gain, or reduce the
 * output level of the transfer function.
 */

/*
 * Process options
 *
 * Don't do initialization now.
 * The 'info' fields are not yet filled in.
 */
int st_compand_getopts(compand_t l, int n, char **argv) 
{

    if (n < 2 || n > 5)
    {
      st_fail("Wrong number of arguments for the compander effect\n"
	   "Use: {<attack_time>,<decay_time>}+ {<dB_in>,<db_out>}+ "
	   "[<dB_postamp> [<initial-volume> [<delay_time]]]\n"
	   "where {}+ means `one or more in a comma-separated, "
	   "white-space-free list'\n"
	   "and [] indications possible omission.  dB values are floating\n"
	   "point or `-inf'; times are in seconds.");
      return (ST_EOF);
    }
    else { /* Right no. of args, but are they well formed? */
      char *s;
      int rates, tfers, i, commas;

      /* Start by checking the attack and decay rates */

      for (s = argv[0], commas = 0; *s; ++s)
	if (*s == ',') ++commas;

      if (commas % 2 == 0) /* There must be an even number of
			      attack/decay parameters */
      {
	st_fail("compander: Odd number of attack & decay rate parameters");
	return (ST_EOF);
      }

      rates = 1 + commas/2;
      if ((l->attackRate = malloc(sizeof(double) * rates)) == NULL ||
	  (l->decayRate  = malloc(sizeof(double) * rates)) == NULL ||
	  (l->volume     = malloc(sizeof(double) * rates)) == NULL)
      {
	st_fail("Out of memory");
	return (ST_EOF);
      }
      l->expectedChannels = rates;
      l->delay_buf = NULL;

      /* Now tokenise the rates string and set up these arrays.  Keep
	 them in seconds at the moment: we don't know the sample rate yet. */

      s = strtok(argv[0], ","); i = 0;
      do {
	l->attackRate[i] = atof(s); s = strtok(NULL, ",");
	l->decayRate[i]  = atof(s); s = strtok(NULL, ",");
	++i;
      } while (s != NULL);

      /* Same business, but this time for the transfer function */

      for (s = argv[1], commas = 0; *s; ++s)
	if (*s == ',') ++commas;

      if (commas % 2 == 0) /* There must be an even number of
			      transfer parameters */
      {
	st_fail("compander: Odd number of transfer function parameters\n"
	     "Each input value in dB must have a corresponding output value");
	return (ST_EOF);
      }

      tfers = 3 + commas/2; /* 0, 0 at start; 1, 1 at end */
      if ((l->transferIns  = malloc(sizeof(double) * tfers)) == NULL ||
	  (l->transferOuts = malloc(sizeof(double) * tfers)) == NULL)
      {
	st_fail("Out of memory");
	return (ST_EOF);
      }
      l->transferPoints = tfers;
      l->transferIns[0] = 0.0; l->transferOuts[0] = 0.0;
      l->transferIns[tfers-1] = 1.0; l->transferOuts[tfers-1] = 1.0;
      s = strtok(argv[1], ","); i = 1;
      do {
	if (!strcmp(s, "-inf"))
	{
	  st_fail("Input signals of zero level must always generate zero output");
	  return (ST_EOF);
	}
	l->transferIns[i]  = pow(10.0, atof(s)/20.0);
	if (l->transferIns[i] > 1.0)
	{
	  st_fail("dB values are relative to maximum input, and, ipso facto, "
	       "cannot exceed 0");
	  return (ST_EOF);
	}
	if (l->transferIns[i] == 1.0) /* Final point was explicit */
	  --(l->transferPoints);
	if (i > 0 && l->transferIns[i] <= l->transferIns[i-1])
	{
	  st_fail("Transfer function points don't have strictly ascending "
	       "input amplitude");
	  return (ST_EOF);
	}
	s = strtok(NULL, ",");
	l->transferOuts[i] = strcmp(s, "-inf") ?
	                       pow(10.0, atof(s)/20.0) : 0;
	s = strtok(NULL, ",");
	++i;
      } while (s != NULL);
      
      /* If there is a postprocessor gain, store it */
      if (n >= 3) l->outgain = pow(10.0, atof(argv[2])/20.0);
      else l->outgain = 1.0;

      /* Set the initial "volume" to be attibuted to the input channels.
	 Unless specified, choose 1.0 (maximum) otherwise clipping will
	 result if the user has seleced a long attack time */
      for (i = 0; i < l->expectedChannels; ++i) {
	double v = n>=4 ? pow(10.0, atof(argv[3])/20) : 1.0;
	l->volume[i] = v;

      /* If there is a delay, store it. */
      if (n >= 5) l->delay = atof(argv[4]);
      else l->delay = 0.0;
      }
    }
    return (ST_SUCCESS);
}


/*
 * Prepare processing.
 * Do all initializations.
 */
int st_compand_start(compand_t *lH, char **opts, int nopts)
{
  int i;
  compand_t l;

  *lH = malloc(sizeof (struct compand));
  l = *lH;

  st_compand_getopts(l, nopts, opts);

# ifdef DEBUG
  {
    fprintf(stderr, "Starting compand effect\n");
    fprintf(stderr, "\nRate %ld, size %d, encoding %d, output gain %g.\n",
	   ST_SAMPLE_RATE, effp->outinfo.size, effp->outinfo.encoding,
	   l->outgain);
    fprintf(stderr, "%d input channel(s) expected: actually %d\n",
	   l->expectedChannels, ST_CHANNELS);
    fprintf(stderr, "\nAttack and decay rates\n"
	     "======================\n");
    for (i = 0; i < l->expectedChannels; ++i)
      fprintf(stderr, "Channel %d: attack = %-12g decay = %-12g\n",
	     i, l->attackRate[i], l->decayRate[i]);
    fprintf(stderr, "\nTransfer function (linear values)\n"
	     "=================  =============\n");
    for (i = 0; i < l->transferPoints; ++i)
      fprintf(stderr, "%12g -> %-12g\n",
	     l->transferIns[i], l->transferOuts[i]);
  }
# endif
  
  /* Convert attack and decay rates using number of samples */

  for (i = 0; i < l->expectedChannels; ++i) {
    if (l->attackRate[i] > 1.0/ST_SAMPLE_RATE)
      l->attackRate[i] = 1.0 -
	exp(-1.0/(ST_SAMPLE_RATE * l->attackRate[i]));
    else
      l->attackRate[i] = 1.0;
    if (l->decayRate[i] > 1.0/ST_SAMPLE_RATE)
      l->decayRate[i] = 1.0 -
	exp(-1.0/(ST_SAMPLE_RATE * l->decayRate[i]));
    else
      l->decayRate[i] = 1.0;
  }

  /* Allocate the delay buffer */
  l->delay_buf_size = (int) (l->delay * ST_SAMPLE_RATE * ST_CHANNELS);
  if (l->delay_buf_size > 0
   && (l->delay_buf = malloc(sizeof(long) * l->delay_buf_size)) == NULL) {
    st_fail("Out of memory");
    return (ST_EOF);
  }
  for (i = 0;  i < l->delay_buf_size;  i++)
    l->delay_buf[i] = 0;
  l->delay_buf_ptr = 0;
  l->delay_buf_cnt = 0;
  l->delay_buf_full= 0;

  return (ST_SUCCESS);
}

/*
 * Update a volume value using the given sample
 * value, the attack rate and decay rate
 */

static void doVolume(double *v, double samp, compand_t l, int chan)
{
  double s = samp/ST_SAMPLE_MAX;
  double delta = s - *v;

  if (delta > 0.0) /* increase volume according to attack rate */
    *v += delta * l->attackRate[chan];
  else             /* reduce volume according to decay rate */
    *v += delta * l->decayRate[chan];
}

/*
 * Processed signed long samples from ibuf to obuf.
 * Return number of samples processed.
 */
int st_compand_flow(compand_t l, st_sample_t *ibuf, st_sample_t *obuf, 
                    st_size_t *isamp, st_size_t *osamp)
{
  int len =  (*isamp > *osamp) ? *osamp : *isamp;
  int filechans = ST_CHANNELS;
  int idone,odone;
  long checkbuf; //if st_sample_t of type int32_t

  for (idone = 0,odone = 0; idone < len; ibuf += filechans) {
    int chan;

    /* Maintain the volume fields by simulating a leaky pump circuit */

    for (chan = 0; chan < filechans; ++chan) {
      if (l->expectedChannels == 1 && filechans > 1) {
	/* User is expecting same compander for all channels */
	int i;
	double maxsamp = 0.0;
	for (i = 0; i < filechans; ++i) {
	  double rect = fabs(ibuf[i]);
	  if (rect > maxsamp) maxsamp = rect;
	}
	doVolume(&l->volume[0], maxsamp, l, 0);
	break;
      } else
	doVolume(&l->volume[chan], fabs(ibuf[chan]), l, chan);
    }

    /* Volume memory is updated: perform compand */

    for (chan = 0; chan < filechans; ++chan) {
      double v = l->expectedChannels > 1 ?
	l->volume[chan] : l->volume[0];
      double outv;
      int piece;

      for (piece = 1 /* yes, 1 */;
	   piece < l->transferPoints;
	   ++piece)
	if (v >= l->transferIns[piece - 1] &&
	    v < l->transferIns[piece])
	  break;

      outv = l->transferOuts[piece-1] +
	(l->transferOuts[piece] - l->transferOuts[piece-1]) *
	(v - l->transferIns[piece-1]) /
	(l->transferIns[piece] - l->transferIns[piece-1]);

      if (l->delay_buf_size <= 0)
      {
        checkbuf = (long int) (ibuf[chan]*(outv/v)*l->outgain);
        if(checkbuf > ST_SAMPLE_MAX)
         obuf[odone] = ST_SAMPLE_MAX;
        else if(checkbuf < ST_SAMPLE_MIN)
         obuf[odone] = ST_SAMPLE_MIN;
        else
         obuf[odone] = (st_sample_t) checkbuf;

        idone++;
        odone++;
      }
      else
      {
	if (l->delay_buf_cnt >= l->delay_buf_size)
        {
            l->delay_buf_full=1; //delay buffer is now definetly full
	        checkbuf = (long int) (l->delay_buf[l->delay_buf_ptr]*(outv/v)*l->outgain);
            if(checkbuf > ST_SAMPLE_MAX)
             obuf[odone] = ST_SAMPLE_MAX;
            else if(checkbuf < ST_SAMPLE_MIN)
             obuf[odone] = ST_SAMPLE_MIN;
            else
             obuf[odone] = (st_sample_t) checkbuf;

            odone++;
            idone++;
        }
	else
        {
	    l->delay_buf_cnt++;
            idone++; //no "odone++" because we did not fill obuf[...]
        }
        l->delay_buf[l->delay_buf_ptr++] = ibuf[chan];
        l->delay_buf_ptr %= l->delay_buf_size;
      }
    }
  }

  *isamp = idone; *osamp = odone;
  return (ST_SUCCESS);
}

/*
 * Drain out compander delay lines.
 */
int st_compand_drain(compand_t l, st_sample_t *obuf, st_size_t *osamp)
{
  int done;

  /*
   * Drain out delay samples.  Note that this loop does all channels.
   */
  if(l->delay_buf_full==0) l->delay_buf_ptr=0;
  for (done = 0;  done < (int) *osamp  &&  l->delay_buf_cnt > 0;  done++) {
    obuf[done] = l->delay_buf[l->delay_buf_ptr++];
    l->delay_buf_ptr %= l->delay_buf_size;
    l->delay_buf_cnt--;
  }

  /* tell caller number of samples played */
  *osamp = done;
  return (ST_SUCCESS);
}


/*
 * Clean up compander effect.
 */
int st_compand_stop(compand_t l)
{

  free((char *) l->delay_buf);
  free((char *) l->transferOuts);
  free((char *) l->transferIns);
  free((char *) l->volume);
  free((char *) l->decayRate);
  free((char *) l->attackRate);

  l->delay_buf = NULL;
  l->transferOuts = NULL;
  l->transferIns = NULL;
  l->volume = NULL;
  l->decayRate = NULL;
  l->attackRate = NULL;

  return (ST_SUCCESS);
}
