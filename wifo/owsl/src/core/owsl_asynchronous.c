/*
 * Open Wengo Socket Library
 * Copyright (C) 2007  Wengo
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

#include "owsl_internal.h"

#include <owlist.h>
#include <owlibc.h>

#include <stdlib.h>
#include <memory.h>
#include <limits.h>

/* callback mechanism */
typedef struct OWSLAsynchronousEvent
{
	OWSLSocketInfo * socket ;
	OWSLEvent event ;
} OWSLAsynchronousEvent ;
static const int owsl_asynchronous_event_size = sizeof (OWSLAsynchronousEvent) ;

static OWQueue * owsl_asynchronous_event_queue = NULL ;   /* in blocking mode */
#define OWSL_ASYNCHRONOUS_PENDING_MAX 5

static pthread_t owsl_asynchronous_thread ;
static int owsl_asynchronous_thread_run = 0 ;

/* select mechanism */
static int owsl_select_enabled ;

typedef struct OWSLSelectHandle
{
	pthread_cond_t event ;
	fd_set * read_sockets ;
	fd_set * write_sockets ;
	fd_set * error_sockets ;
} OWSLSelectHandle ;

/* each waiting select call registers its sockets to be signaled */
static OWList * owsl_select_handle_list = NULL ;
static pthread_mutex_t owsl_select_mutex ;

static OWSLSelectHandle *
owsl_select_handle_new
(
	fd_set * read_sockets,
	fd_set * write_sockets,
	fd_set * error_sockets
)
{
	OWSLSelectHandle * select_handle = malloc (sizeof (OWSLSelectHandle)) ;
	if (select_handle != NULL)
	{
		if (pthread_cond_init (& select_handle->event, NULL))
		{
			free (select_handle) ;
			return NULL ;
		}
		select_handle->read_sockets = read_sockets ;
		select_handle->write_sockets = write_sockets ;
		select_handle->error_sockets = error_sockets ;
		if (owlist_add (owsl_select_handle_list, select_handle, NULL))
		{
			pthread_cond_destroy (& select_handle->event) ;
			free (select_handle) ;
			return NULL ;
		}
	}
	return select_handle ;
}

static int
owsl_select_handle_free_without_remove
(
	OWSLSelectHandle * select_handle
)
{
	int return_code = pthread_cond_destroy (& select_handle->event) ;
	free (select_handle) ;
	return return_code ;
}

static int
owsl_select_handle_free
(
	OWSLSelectHandle * select_handle
)
{
	int return_code = 0 ;
	return_code |= (owlist_remove (owsl_select_handle_list, select_handle, NULL) == NULL) ;
	return_code |= owsl_select_handle_free_without_remove (select_handle) ;
	return return_code ;
}

static void *
owsl_asynchronous_loop
(
	void * dummy
)
{
	OWSLAsynchronousEvent event ;

	/* set thread name in debug */
	OW_SET_THREAD_NAME ("OWSL asynchronous") ;

	while (owsl_asynchronous_thread_run != 0)
	{
		if
		(
			owqueue_read (owsl_asynchronous_event_queue, & event, owsl_asynchronous_event_size, NULL, 0) == owsl_asynchronous_event_size
			&&
			event.socket->callback_function != NULL
		)
		{
			event.socket->callback_function (event.socket->socket, event.event, event.socket->callback_user_data) ;
		}
	}

	return NULL ;
}

int
owsl_asynchronous_initialize
(void)
{
	owsl_select_enabled = 0 ;
	owsl_select_handle_list = owlist_new () ;
	if (owsl_select_handle_list == NULL)
	{
		return -1 ;
	}

	if (pthread_mutex_init (& owsl_select_mutex, NULL))
	{
		owlist_free (owsl_select_handle_list) ;
		owsl_select_handle_list = NULL ;
		return -1 ;
	}

	owsl_asynchronous_event_queue = owqueue_new
	(
		sizeof (OWSLAsynchronousEvent) * OWSL_ASYNCHRONOUS_PENDING_MAX,   /* usable size */
		OWQUEUE_PACKET,                                                   /* mode */
		OWSL_ASYNCHRONOUS_PENDING_MAX,                                    /* packet maximum */
		0                                                                 /* packet info size */
	) ;
	if (owsl_asynchronous_event_queue == NULL)
	{
		pthread_mutex_destroy (& owsl_select_mutex) ;
		owlist_free (owsl_select_handle_list) ;
		owsl_select_handle_list = NULL ;
		return -1 ;
	}

	/* start thread */
	owsl_asynchronous_thread_run = 1 ;
	if (pthread_create (& owsl_asynchronous_thread, NULL, owsl_asynchronous_loop, NULL))
	{
		owsl_asynchronous_thread_run = 0 ;
		owqueue_free (owsl_asynchronous_event_queue) ;
		owsl_asynchronous_event_queue = NULL ;
		pthread_mutex_destroy (& owsl_select_mutex) ;
		owlist_free (owsl_select_handle_list) ;
		owsl_select_handle_list = NULL ;
		return -1 ;
	}
	/* let cpu resource to start the new thread */
	sched_yield () ;

	return 0 ;
}

int
owsl_asynchronous_terminate
(void)
{
	int return_code = 0 ;

	/* kill thread */
	/* HACK: on Mac OS X, the thread cannot be cancelled while waiting in pthread_cond_wait()
	 * That's why we simulate something to read on the queue to unblock it. */
	owsl_asynchronous_thread_run = 0 ;
	//if (pthread_cancel (owsl_asynchronous_thread) == 0)
	if (owqueue_write (owsl_asynchronous_event_queue, "end", 3, NULL, OWQUEUE_NON_BLOCKING))
	{
		return_code |= pthread_join (owsl_asynchronous_thread, NULL) ;
	}
	else
	{
		return_code = -1 ;
	}

	return_code |= owqueue_free (owsl_asynchronous_event_queue) ;
	owsl_asynchronous_event_queue = NULL ;

	return_code |= pthread_mutex_destroy (& owsl_select_mutex) ;

	return_code |= owlist_free_all (owsl_select_handle_list, (void (*) (void *)) owsl_select_handle_free_without_remove) ;
	owsl_select_handle_list = NULL ;

	return return_code ;
}

static int
owsl_signal_select
(
	OWSLSocket socket,
	OWSLEvent event
)
{
	OWListIterator * iterator ;
	OWSLSelectHandle * select_handle ;

	if (pthread_mutex_lock (& owsl_select_mutex))
	{
		return -1 ;
	}

	iterator = owlist_iterator_new (owsl_select_handle_list, OWLIST_READ) ;
	if (iterator == NULL)
	{
		pthread_mutex_unlock (& owsl_select_mutex) ;
		return -1 ;
	}
	while (owlist_iterator_next (iterator) == 0)
	{
		select_handle = owlist_iterator_get (iterator) ;
		if (((event & OWSL_EVENT_READ) != 0 && select_handle->read_sockets != NULL && FD_ISSET (socket, select_handle->read_sockets)) ||
			((event & OWSL_EVENT_WRITE) != 0 && select_handle->write_sockets != NULL && FD_ISSET (socket, select_handle->write_sockets)) ||
			((event & OWSL_EVENT_ERROR) != 0 && select_handle->error_sockets != NULL && FD_ISSET (socket, select_handle->error_sockets)))
		{
			pthread_cond_signal (& select_handle->event) ;
		}
	}
	if (owlist_iterator_free (iterator))
	{
		pthread_mutex_unlock (& owsl_select_mutex) ;
		return -1 ;
	}

	if (pthread_mutex_unlock (& owsl_select_mutex))
	{
		return -1 ;
	}

	return 0 ;
}

static int
owsl_select_test
(
	OWSLSocket socket_max,
	fd_set * input_read_sockets,
	fd_set * input_write_sockets,
	fd_set * input_error_sockets,
	fd_set * output_read_sockets,
	fd_set * output_write_sockets,
	fd_set * output_error_sockets
)
{
	int count = 0 ;
	OWSLSocket socket ;
	OWSLSocketInfo * socket_info ;
	int has_event ;

	for (socket = 1 ; socket < socket_max && socket <= OWSL_SOCKET_MAX ; socket ++)
	{
		socket_info = owsl_socket_info_get (socket) ;
		if (socket_info != NULL)
		{
			has_event = 0 ;
			if (output_read_sockets != NULL && FD_ISSET (socket, input_read_sockets) && owsl_socket_is_readable (socket_info))
			{
				FD_SET (socket, output_read_sockets) ;
				has_event = 1 ;
			}
			if (output_write_sockets != NULL && FD_ISSET (socket, input_write_sockets) && owsl_socket_is_writable (socket_info))
			{
				FD_SET (socket, output_write_sockets) ;
				has_event = 1 ;
			}
			if (output_error_sockets != NULL && FD_ISSET (socket, input_error_sockets) && owsl_socket_has_error (socket_info))
			{
				FD_SET (socket, output_error_sockets) ;
				has_event = 1 ;
			}
			if (has_event)
			{
				count ++ ;
			}
		}
	}

	return count ;
}

static int
owsl_select_wait
(
	OWSLSelectHandle * select_handle,
	const struct timeval * timeout
)
{
	int return_code ;
	struct timespec absolute_timeout ;
	struct timeval now ;

	if (timeout == NULL)
	{
		/* timeout must be infinite */
		absolute_timeout.tv_sec = LONG_MAX ;
		absolute_timeout.tv_nsec = LONG_MAX ;
	}
	else
	{
		gettimeofday (& now, NULL) ;
		absolute_timeout.tv_sec = now.tv_sec + timeout->tv_sec ;
		absolute_timeout.tv_nsec = (now.tv_usec + timeout->tv_usec) * 1000 ;
	}

	return_code = pthread_cond_timedwait (& select_handle->event, & owsl_select_mutex, & absolute_timeout) ;
	if (return_code == 0)
	{
		return_code = 1 ;
	}
	else if (return_code == ETIMEDOUT)
	{
		return_code = 0 ;
	}
	else
	{
		return -1 ;
	}

	return return_code ;
}

int
owsl_select
(
	OWSLSocket socket_max,
	fd_set * read_sockets,
	fd_set * write_sockets,
	fd_set * error_sockets,
	struct timeval * timeout
)
{
	int return_code ;
	fd_set * input_read_sockets = NULL ;
	fd_set * input_write_sockets = NULL ;
	fd_set * input_error_sockets = NULL ;
	int mutex_locked = 0 ;
	OWSLSelectHandle * select_handle = NULL ;

	/* allow signal to select handle if it is the first select call */
	if (! owsl_select_enabled)
	{
		owsl_select_enabled = 1 ;
	}

	/* save input parameters and reset them to use them as output */
	if (read_sockets != NULL)
	{
		input_read_sockets = malloc (sizeof (fd_set)) ;
		if (input_read_sockets == NULL)
		{
			return_code = -1 ;
			goto end ;
		}
		memcpy (input_read_sockets, read_sockets, sizeof (fd_set)) ;
		FD_ZERO (read_sockets) ;
	}
	if (write_sockets != NULL)
	{
		input_write_sockets = malloc (sizeof (fd_set)) ;
		if (input_write_sockets == NULL)
		{
			return_code = -1 ;
			goto end ;
		}
		memcpy (input_write_sockets, write_sockets, sizeof (fd_set)) ;
		FD_ZERO (write_sockets) ;
	}
	if (error_sockets != NULL)
	{
		input_error_sockets = malloc (sizeof (fd_set)) ;
		if (input_error_sockets == NULL)
		{
			return_code = -1 ;
			goto end ;
		}
		memcpy (input_error_sockets, error_sockets, sizeof (fd_set)) ;
		FD_ZERO (error_sockets) ;
	}

	/* begin of critical section */
	if (pthread_mutex_lock (& owsl_select_mutex))
	{
		return_code = -1 ;
		goto end ;
	}
	mutex_locked = 1 ;

	/* test sockets, set ouput parameters and get event count */
	return_code = owsl_select_test (socket_max, input_read_sockets, input_write_sockets, input_error_sockets, read_sockets, write_sockets, error_sockets) ;
	if (return_code < 0)
	{
		return_code = -1 ;
		goto end ;
	}

	/* if there was no event on tested sockets, then wait */
	if (return_code == 0)
	{
		/* create an object to be signaled on */
		select_handle = owsl_select_handle_new (input_read_sockets, input_write_sockets, input_error_sockets) ;
		if (select_handle == NULL)
		{
			return_code = -1 ;
			goto end ;
		}

		/* wait on the created object */
		return_code = owsl_select_wait (select_handle, timeout) ;
		if (return_code < 0)
		{
			return_code = -1 ;
			goto end ;
		}

		/* if there was at least one event, then test sockets, set ouput parameters and get event count */
		if (return_code > 0)
		{
			return_code = owsl_select_test (socket_max, input_read_sockets, input_write_sockets, input_error_sockets, read_sockets, write_sockets, error_sockets) ;
			if (return_code < 0)
			{
				return_code = -1 ;
				goto end ;
			}
		}
	}

end :
	/* end of critical section */
	if (mutex_locked && pthread_mutex_unlock (& owsl_select_mutex))
	{
		return_code = -1 ;
	}

	/* free object created for wait */
	if (select_handle != NULL && owsl_select_handle_free (select_handle))
	{
		return_code = -1 ;
	}

	/* free input parameters */
	if (input_read_sockets != NULL)
	{
		free (input_read_sockets) ;
	}
	if (input_write_sockets != NULL)
	{
		free (input_write_sockets) ;
	}
	if (input_write_sockets != NULL)
	{
		free (input_write_sockets) ;
	}

	return return_code ;
}

int
owsl_signal
(
	OWSLSocketInfo * socket,
	OWSLEvent event
)
{
	int return_code = 0 ;

	if ((event & OWSL_EVENT_ERROR) && socket->error == 0)
	{
		socket->error = 1 ;
	}

	if (owsl_select_enabled)
	{
		return_code |= owsl_signal_select (socket->socket, event) ;
	}

	return return_code ;
}

int
owsl_callback
(
	OWSLSocketInfo * socket,
	OWSLEvent event
)
{
	int return_code = 0 ;

	if ((event & OWSL_EVENT_ERROR) && socket->error == 0)
	{
		socket->error = 1 ;
	}

	if (socket->callback_function != NULL)
	{
		OWSLAsynchronousEvent saved_event ;
		saved_event.socket = socket ;
		saved_event.event = event ;
		if (owqueue_write (owsl_asynchronous_event_queue, & saved_event, owsl_asynchronous_event_size, NULL, 0) != owsl_asynchronous_event_size)
		{
			return_code = -1 ;
		}
	}

	return return_code ;
}

int
owsl_notify
(
	OWSLSocketInfo * socket,
	OWSLEvent event
)
{
	int return_code = 0 ;
	return_code |= owsl_signal (socket, event) ;
	return_code |= owsl_callback (socket, event) ;
	return return_code ;
}

int
owsl_callback_set
(
	OWSLSocket socket,
	OWSLCallback callback_function,
	void * callback_user_data
)
{
	OWSLSocketInfo * socket_info = owsl_socket_info_get (socket) ;
	if (socket_info == NULL)
	{
		return -1 ;
	}

	if (callback_function != NULL && socket_info->blocking_mode == OWSL_BLOCKING)
	{
		return -1 ;
	}

	socket_info->callback_function = callback_function ;
	socket_info->callback_user_data = callback_user_data ;

	return 0 ;
}
