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
 * Timer implementation using timeSetTime
 *
 */

#include <wtimer.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <windows.h>

struct winmm_metadata {
    timer_delay_t delay;
    unsigned int timerId;
    unsigned int usage_counter;
};

w_timer_t *winmm_timer_create () {
    w_timer_t *ti;

    ti = (w_timer_t *) malloc (sizeof(w_timer_t));
    memset(ti, 0, sizeof(w_timer_t));
    ti->impl_data = malloc (sizeof(struct winmm_metadata));
    return ti;
}

void winmm_timer_set_delay(w_timer_t *ti, timer_delay_t delay) {
    struct winmm_metadata *ptm = (struct clock_gettime_medata *)
                                       ti->impl_data;
    ptm->delay = delay;
}

void winmm_timer_set_callback(w_timer_t *ti, void (*callback)(void *)) {
    ti->callback = callback;
}

void CALLBACK winmm_internal_callback(UINT uID,
				      UINT uMsg,
				      DWORD dwUser,
				      DWORD dw1,
				      DWORD dw2) {

	w_timer_t *ti = (w_timer_t *) dwUser;
    struct winmm_metadata *ptm = (struct clock_gettime_medata *)
                                       ti->impl_data;
    ptm->usage_counter++;
	ti->callback(ti->userdata);
    ptm->usage_counter--;
}

int winmm_timer_start(w_timer_t *ti) {
    struct winmm_metadata *ptm = (struct clock_gettime_medata *)
                                       ti->impl_data;
    ptm->usage_counter = 0;
    ptm->timerId = timeSetEvent(ptm->delay, 10,
            (LPTIMECALLBACK)winmm_internal_callback, (DWORD) ti,
            TIME_CALLBACK_FUNCTION | TIME_PERIODIC);
}

int winmm_timer_stop(w_timer_t *ti) {
    struct winmm_metadata *ptm = (struct clock_gettime_medata *)
                                       ti->impl_data;
    timeKillEvent(ptm->timerId);
    
    // make sure all executing callbacks are finished
    Sleep(2*ptm->delay);
    assert(!ptm->usage_counter);
}

void winmm_timer_set_userdata(w_timer_t *ti, void *userdata) {
    ti->userdata = userdata;
}

void winmm_timer_destroy(w_timer_t *ti) {
    free(ti->impl_data);
}

struct timer_impl winmm_impl = {
    "winmm",
    winmm_timer_create,
    winmm_timer_set_delay,
    winmm_timer_set_callback,
    winmm_timer_set_userdata,
    winmm_timer_start,
    winmm_timer_stop,
    winmm_timer_destroy,
};
