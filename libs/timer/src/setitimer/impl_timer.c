/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2005  Wengo
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

/*
 * @author David Ferlier
 *
 * Timer implementation for POSIX system. It uses setitimer.
 *
 * NOTE: IT ONLY SUPPORTS ONE TIMER AND SHOULD BE CONSIDERATE UNUSABLE
 * (unless you know you'll only use one timer)
 *
 */

#include <wtimer.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>

struct posix_timer_metadata {
    struct itimerval itval;
    struct sigaction sa;
    int which_id;
    int running;
    pthread_t w_timer_thread;
};

w_timer_t *timer_create () {
    w_timer_t *ti;

    ti = (w_timer_t *) malloc (sizeof(w_timer_t));
    memset(ti, 0, sizeof(w_timer_t));
    ti->impl_data = malloc (sizeof(struct posix_timer_metadata));

    return ti;
}

void timer_set_delay(w_timer_t *ti, timer_delay_t delay) {
    struct posix_timer_metadata *ptm = (struct posix_timer_medata *)
                                       ti->impl_data;
    ptm->itval.it_value.tv_sec = 0;
    ptm->itval.it_value.tv_usec = delay * 1000;

    ptm->itval.it_interval.tv_sec = 0;
    ptm->itval.it_interval.tv_usec = delay * 1000;
}

void timer_set_callback(w_timer_t *ti, void (*callback)(void *)) {
    ti->callback = callback;
}

void timer_handler(int signum) {
}

void timer_thread(void *parg) {
    w_timer_t *ti = (w_timer_t *) parg;
    struct posix_timer_metadata *ptm = (struct posix_timer_medata *)
                                       ti->impl_data;
    sigset_t set, oldset;

    sigprocmask(SIG_UNBLOCK, NULL, &oldset);

    sigfillset(&set);
    sigdelset(&set, SIGALRM);

    while (ptm->running) {
        memset(&ptm->sa, 0, sizeof(ptm->sa));
        ptm->sa.sa_handler = &timer_handler;
        sigaction (SIGALRM, &ptm->sa, NULL);

        sigsuspend(&set);
        if (ti->callback) {
            ti->callback(ti->userdata);
        }
        sigprocmask(SIG_SETMASK, &oldset, NULL);
    }
}

int timer_start(w_timer_t *ti) {
    struct posix_timer_metadata *ptm = (struct posix_timer_medata *)
                                       ti->impl_data;

    setitimer (ITIMER_REAL, &ptm->itval, NULL);

    ptm->running = 1;
    pthread_create(&ptm->w_timer_thread, NULL, timer_thread, ti);
}

int timer_stop(w_timer_t *ti) {
    struct posix_timer_metadata *ptm = (struct posix_timer_medata *)
                                       ti->impl_data;

    ptm->running = 0;
    pthread_join(&ptm->w_timer_thread, NULL);
}

void timer_set_userdata(w_timer_t *ti, void *userdata) {
    ti->userdata = userdata;
}
