/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2006  Wengo SAS
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
 * @author David Ferlier <david.ferlier@wengo.fr>
 *
 * Functions to init timer library, and manipulate implementations.
 *
 */

#include <wtimer.h>
#include <stdio.h>
#include <string.h>

#define MAX_IMPLS   256

static int libtimer_inited = 0;

struct timer_runtime {
    struct timer_impl *timer_impls[MAX_IMPLS];
    int    nr_impls;
};

static struct timer_runtime timer_runtime_config;

void timer_register_impl(struct timer_impl *t_impl) {
    if (timer_runtime_config.nr_impls > MAX_IMPLS) {
        return;
    }
    timer_runtime_config.timer_impls[timer_runtime_config.nr_impls++] = t_impl;
}

#if defined(TIMER_ENABLE_CLOCK_GETTIME_IMPL)
extern struct timer_impl clock_gettime_impl;
#endif

#if defined(TIMER_ENABLE_WINMM_IMPL)
extern struct timer_impl winmm_impl;
#endif

void timer_init() {

    if (libtimer_inited == 1) {
        return;
    }

    timer_runtime_config.nr_impls = 0;
    memset(&timer_runtime_config.timer_impls, 0, MAX_IMPLS);

#ifdef TIMER_ENABLE_CLOCK_GETTIME_IMPL
    timer_register_impl(&clock_gettime_impl);
#endif

#ifdef TIMER_ENABLE_WINMM_IMPL
    timer_register_impl(&winmm_impl);
#endif

    libtimer_inited = 1;
}

struct timer_impl *timer_impl_getbyname(char *impl_name) {
    int it;
    struct timer_impl *tim;

    for (it = 0; it < MAX_IMPLS - 1; it += 1) {
        tim = timer_runtime_config.timer_impls[it];
        if (tim && tim->timer_impl_name && strncmp(impl_name,
                    tim->timer_impl_name, strlen(impl_name)) == 0) {
            return tim;
        }
    }

    return NULL;
}

struct timer_impl *timer_impl_getfirst() {
    int it;
    struct timer_impl *tim;

    for (it = 0; it < MAX_IMPLS - 1; it += 1) {
        tim = timer_runtime_config.timer_impls[it];
        if (tim) {
            return tim;
        }
    }

    return NULL;
}
