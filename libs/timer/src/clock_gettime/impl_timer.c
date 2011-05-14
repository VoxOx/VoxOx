/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2007  Wengo
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/**
 * @author David Ferlier
 * @author Mathieu Stute
 *
 * Timer implementation for POSIX systems. It uses pthreads + clock_gettime
 */

#include <wtimer.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

#define TIMER_DEBUG

#define CLOCKS_PER_NANOSEC  (CLOCKS_PER_SEC / 1000000000.)

#ifndef TIMEVAL_TO_TIMESPEC
#define TIMEVAL_TO_TIMESPEC(tv, ts) {   \
  (ts)->tv_sec = (tv)->tv_sec;          \
  (ts)->tv_nsec = (tv)->tv_usec * 1000; \
}
#endif /*TIMEVAL_TO_TIMESPEC*/

struct clock_gettime_metadata {
    struct sigaction sa;
//    struct timeval ts;
    struct timespec ts;

    int running;
    pthread_t w_timer_thread;
    pthread_mutex_t w_timer_mutex;
};

w_timer_t *cgt_timer_create () {
    w_timer_t *ti;

    ti = (w_timer_t *) malloc (sizeof(w_timer_t));
    memset(ti, 0, sizeof(w_timer_t));
    ti->impl_data = malloc (sizeof(struct clock_gettime_metadata));

    return ti;
}

void cgt_timer_set_delay(w_timer_t *ti, timer_delay_t delay) {
    struct clock_gettime_metadata *ptm = (struct clock_gettime_metadata *)
                                       ti->impl_data;

    ptm->ts.tv_sec = 0;
    ptm->ts.tv_nsec = delay * 1000 * 1000;
}

void cgt_timer_set_callback(w_timer_t *ti, void (*callback)(void *)) {
    assert(ti != 0);
    ti->callback = callback;
}

int
timeval_substract(struct timeval *result, struct timeval *x, struct timeval *y)
{
  /* Perform the carry for the later subtraction by updating y. */
  if (x->tv_usec < y->tv_usec)
  {
    int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
    y->tv_usec -= 1000000 * nsec;
    y->tv_sec += nsec;
  }
  if (x->tv_usec - y->tv_usec > 1000000)
  {
    int nsec = (x->tv_usec - y->tv_usec) / 1000000;
    y->tv_usec += 1000000 * nsec;
    y->tv_sec -= nsec;
  }

  /* Compute the time remaining to wait. tv_usec is certainly positive. */
  result->tv_sec = x->tv_sec - y->tv_sec;
  result->tv_usec = x->tv_usec - y->tv_usec;

  /* Return 1 if result is negative. */
  return x->tv_sec < y->tv_sec;
}

void * cgt_timer_thread(void *parg) {
    w_timer_t *ti = (w_timer_t *) parg;
    struct clock_gettime_metadata *ptm =
        (struct clock_gettime_metadata *)ti->impl_data;

    struct timeval elapsed_time, sleeptime, start_time, end_time, loop_time;
    struct timespec sleepns;

    loop_time.tv_sec = ptm->ts.tv_sec;
    loop_time.tv_usec = ptm->ts.tv_nsec / 1000;

    while (ptm->running) {

        gettimeofday(&start_time, 0);

        if (ti->callback) {
            ti->callback(ti->userdata);
        }

        gettimeofday(&end_time, 0);

        timeval_substract(&elapsed_time, &end_time, &start_time);
        if (!timeval_substract(&sleeptime, &loop_time, &elapsed_time))
        {
            TIMEVAL_TO_TIMESPEC(&sleeptime, &sleepns);
#ifdef WIN32
            Sleep(sleepns.tv_sec * 1000 + sleepns.tv_nsec / 1000000);
#else
            nanosleep(&sleepns, 0);
#endif
        }
    }
    return 0;
}

int cgt_timer_start(w_timer_t *ti) {
    int err = 0;
    struct clock_gettime_metadata *ptm = (struct clock_gettime_metadata *)
                                       ti->impl_data;
    ptm->running = 1;
    err = pthread_create(&ptm->w_timer_thread, NULL, cgt_timer_thread, ti);
#ifdef TIMER_DEBUG
    printf("Timer: pthread_create: %s\n", strerror(err));
#endif
    return err;
}

int cgt_timer_stop(w_timer_t *ti) {
    struct clock_gettime_metadata *ptm = (struct clock_gettime_metadata *)
                                       ti->impl_data;
    ptm->running = 0;
    //pthread_join(&ptm->w_timer_thread, NULL);
    usleep(2*ptm->ts.tv_nsec / 1000);
    return 0;
}

void cgt_timer_set_userdata(w_timer_t *ti, void *userdata) {
    ti->userdata = userdata;
}

void cgt_timer_destroy(w_timer_t *ti) {
    free(ti->impl_data);
}

struct timer_impl clock_gettime_impl = {
    "clock_gettime",
    cgt_timer_create,
    cgt_timer_set_delay,
    cgt_timer_set_callback,
    cgt_timer_set_userdata,
    cgt_timer_start,
    cgt_timer_stop,
    cgt_timer_destroy,
};
