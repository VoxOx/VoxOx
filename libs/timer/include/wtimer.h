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

#ifndef _WENGO_TIMER_H_
#define _WENGO_TIMER_H_

/*
 * @author David Ferlier
 *
 * Simple & portable library for timers
 *
 */

typedef int timer_delay_t;

/*
 * @brief Portable timer structure
 *
 * @param callback A callback function called when the timer expires
 * @param impl_data private data for timer implementation
 *
 */

struct timer {
    void (*callback)(void *userdata);
    void *impl_data;
    void *userdata;
};

typedef struct timer w_timer_t;

/*
 * @brief A timer implementation structure
 *
 * This structure holds the functions a timer API has to implement.
 *
 */

struct timer_impl {
    char * timer_impl_name;
    w_timer_t * (*timer_create)();
    void (*timer_set_delay)(w_timer_t *, timer_delay_t delay);
    void (*timer_set_callback)(w_timer_t *, void (*callback)(void *));
    void (*timer_set_userdata)(w_timer_t *, void *userdata);
    int  (*timer_start)(w_timer_t *);
    int  (*timer_stop)(w_timer_t *);
    int  (*timer_destroy)(w_timer_t *);
};

/*
 * @brief Initialize the timer library
 *
 * This function registers the different timer implementations available
 *
 */

void timer_init();

/*
 * @brief Get a timer implementation by its name
 *
 */

struct timer_impl *timer_impl_getbyname(char *impl_name);

/*
 * @brief Get the first timer implementation available
 *
 */

struct timer_impl *timer_impl_getfirst();

#endif
