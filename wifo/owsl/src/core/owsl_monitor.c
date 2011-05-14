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

#define OWSL_MONITOR_WAIT_MAX_DELAY 10000 /* us */

typedef struct OWSLMonitorSocket
{
	OWSLSystemSocket system_socket ;
	OWSLMonitorCallback callback_function ;
	void * callback_user_data ;
	OWSLMonitorEvent events ;
	OWSLMonitorEvent once_event ;
} OWSLMonitorSocket ;

static OWList * owsl_monitor_socket_list = NULL ;

typedef struct OWSLMonitorRegister
{
	fd_set read ;
	fd_set write ;
	fd_set error ;
} OWSLMonitorRegister ;

static OWSLMonitorRegister owsl_monitor_register ;
static pthread_mutex_t owsl_monitor_register_mutex ;

static pthread_t owsl_monitor_thread ;

static int
owsl_monitor_register_event_add
(
	OWSLSystemSocket system_socket,
	OWSLMonitorEvent event
)
{
	if (event)
	{
		if (system_socket < 0)
		{
			return -1 ;
		}
		if (pthread_mutex_lock (& owsl_monitor_register_mutex))
		{
			return -1 ;
		}
		if ((event & OWSL_MONITOR_READ) != 0)
		{
			FD_SET (system_socket, & owsl_monitor_register.read) ;
		}
		if ((event & OWSL_MONITOR_WRITE) != 0)
		{
			FD_SET (system_socket, & owsl_monitor_register.write) ;
		}
		if ((event & OWSL_MONITOR_ERROR) != 0)
		{
			FD_SET (system_socket, & owsl_monitor_register.error) ;
		}
		if (pthread_mutex_unlock (& owsl_monitor_register_mutex))
		{
			return -1 ;
		}
	}
	return 0 ;
}

static int
owsl_monitor_register_event_remove
(
	OWSLSystemSocket system_socket,
	OWSLMonitorEvent event
)
{
	if (event)
	{
		if (system_socket < 0)
		{
			return -1 ;
		}
		if (pthread_mutex_lock (& owsl_monitor_register_mutex))
		{
			return -1 ;
		}
		if ((event & OWSL_MONITOR_READ) != 0)
		{
			FD_CLR (system_socket, & owsl_monitor_register.read) ;
		}
		if ((event & OWSL_MONITOR_WRITE) != 0)
		{
			FD_CLR (system_socket, & owsl_monitor_register.write) ;
		}
		if ((event & OWSL_MONITOR_ERROR) != 0)
		{
			FD_CLR (system_socket, & owsl_monitor_register.error) ;
		}
		if (pthread_mutex_unlock (& owsl_monitor_register_mutex))
		{
			return -1 ;
		}
	}
	return 0 ;
}

static int
owsl_monitor_once_event_remove
(
	OWSLMonitorSocket * monitor_socket
)
{
	if (monitor_socket->once_event == 0)
	{
		return 0 ;
	}

	if (owsl_monitor_register_event_add (monitor_socket->system_socket, monitor_socket->events & (~ monitor_socket->once_event)))
	{
		return -1 ;
	}
	if (owsl_monitor_register_event_remove (monitor_socket->system_socket, monitor_socket->once_event & (~ monitor_socket->events)))
	{
		return -1 ;
	}
	monitor_socket->once_event = 0 ;

	return 0 ;
}

static void *
owsl_monitor_loop
(
	void * dummy
)
{
	OWSLMonitorRegister temporary_register ;
	struct timeval timeout ;
	OWSLSystemSocket socket_max ;
	int fd_count ;
	OWListIterator * socket_list_iterator ;
	OWSLMonitorSocket * monitor_socket ;
	OWSLMonitorEvent event = 0 ;

	/* set thread name in debug */
	OW_SET_THREAD_NAME ("OWSL monitor") ;

#define OWSL_MONITOR_LOOP_CHECK(return_code) \
	if ((return_code) != 0) \
	{ \
		/* what is the best error handling in a such loop ? */ \
		msleep (timeout.tv_usec / 1000) ; \
		continue ; \
	}

	while (1)
	{
		/* let a chance that the thread be killed  */
		pthread_testcancel () ;
		/* let a chance that the thread be preempted  */
		sched_yield () ;

		/* get highest socket + 1 */
		socket_max = 0 ;
		socket_list_iterator = owlist_iterator_new (owsl_monitor_socket_list, OWLIST_READ) ;
		OWSL_MONITOR_LOOP_CHECK (socket_list_iterator == NULL)
		while (owlist_iterator_next (socket_list_iterator) == 0)
		{
			monitor_socket = owlist_iterator_get (socket_list_iterator) ;
			socket_max = OW_MAX (socket_max, monitor_socket->system_socket) ;
		}
		OWSL_MONITOR_LOOP_CHECK (owlist_iterator_free (socket_list_iterator))
		socket_max ++ ;

		/* timeout must be set before each call to select */
		timeout.tv_sec = OWSL_MONITOR_WAIT_MAX_DELAY / 1000000 ;
		timeout.tv_usec = OWSL_MONITOR_WAIT_MAX_DELAY % 1000000 ;

		/* restore temporary register */
		OWSL_MONITOR_LOOP_CHECK (pthread_mutex_lock (& owsl_monitor_register_mutex))
		memcpy (& temporary_register, & owsl_monitor_register, sizeof (OWSLMonitorRegister)) ;
		OWSL_MONITOR_LOOP_CHECK (pthread_mutex_unlock (& owsl_monitor_register_mutex))

		/* wait & check sockets */
		fd_count = select ((int) socket_max, & temporary_register.read, & temporary_register.write, & temporary_register.error, & timeout) ;
		OWSL_MONITOR_LOOP_CHECK (fd_count < 0)

		/* call callbacks to manage sockets that are ready */
		if (fd_count > 0)
		{
			socket_list_iterator = owlist_iterator_new (owsl_monitor_socket_list, OWLIST_READ) ;
			OWSL_MONITOR_LOOP_CHECK (socket_list_iterator == NULL)
			while (owlist_iterator_next (socket_list_iterator) == 0 && fd_count > 0)
			{
				monitor_socket = owlist_iterator_get (socket_list_iterator) ;
				if (FD_ISSET (monitor_socket->system_socket, & temporary_register.read))
				{
					event |= OWSL_MONITOR_READ ;
				}
				if (FD_ISSET (monitor_socket->system_socket, & temporary_register.write))
				{
					event |= OWSL_MONITOR_WRITE ;
				}
				if (FD_ISSET (monitor_socket->system_socket, & temporary_register.error))
				{
					event |= OWSL_MONITOR_ERROR ;
				}
				if (event)
				{
					if (monitor_socket->once_event != 0)
					{
						OWSL_MONITOR_LOOP_CHECK (owsl_monitor_once_event_remove (monitor_socket))
						event |= OWSL_MONITOR_ONCE ;
					}
					monitor_socket->callback_function (monitor_socket->system_socket, event, monitor_socket->callback_user_data) ;
					event = 0 ;
					fd_count -- ;
				}
			}
			OWSL_MONITOR_LOOP_CHECK (owlist_iterator_free (socket_list_iterator))
		}
	}

	return NULL ;
}

int
owsl_monitor_start
(void)
{
	/* initialize monitor socket list */
	owsl_monitor_socket_list = owlist_new () ;
	if (owsl_monitor_socket_list == NULL)
	{
		return -1 ;
	}

	/* initialize monitor register */
	FD_ZERO (& owsl_monitor_register.read) ;
	FD_ZERO (& owsl_monitor_register.write) ;
	FD_ZERO (& owsl_monitor_register.error) ;

	/* initialize monitor register mutex */
	if (pthread_mutex_init (& owsl_monitor_register_mutex, NULL))
	{
		owlist_free (owsl_monitor_socket_list) ;
		owsl_monitor_socket_list = NULL ;
		return -1 ;
	}

	/* start monitor thread */
	if (pthread_create (& owsl_monitor_thread, NULL, owsl_monitor_loop, NULL))
	{
		owlist_free (owsl_monitor_socket_list) ;
		owsl_monitor_socket_list = NULL ;
		return -1 ;
	}
	/* let cpu resource to start the new thread */
	sched_yield () ;

	return 0 ;
}

int
owsl_monitor_stop
(void)
{
	int return_code = 0 ;

	/* kill monitor thread */
	if (pthread_cancel (owsl_monitor_thread) == 0)
	{
		return_code |= pthread_join (owsl_monitor_thread, NULL) ;
	}
	else
	{
		return_code = -1 ;
	}

	/* free monitor register mutex */
	return_code |= pthread_mutex_destroy (& owsl_monitor_register_mutex) ;

	/* free monitor socket list */
	return_code |= owlist_free_all (owsl_monitor_socket_list, free) ;
	owsl_monitor_socket_list = NULL ;

	return return_code ;
}

static OWSLMonitorSocket *
owsl_monitor_socket_get
(
	OWSLSystemSocket system_socket
)
{
	OWSLMonitorSocket * monitor_socket = NULL ;
	OWListIterator * socket_list_iterator ;
	int found = 0 ;

	socket_list_iterator = owlist_iterator_new (owsl_monitor_socket_list, OWLIST_READ) ;
	if (socket_list_iterator == NULL)
	{
		return NULL ;
	}
	while (owlist_iterator_next (socket_list_iterator) == 0)
	{
		monitor_socket = owlist_iterator_get (socket_list_iterator) ;
		if (monitor_socket->system_socket == system_socket)
		{
			found = 1 ;
			break ;
		}
	}
	if (owlist_iterator_free (socket_list_iterator))
	{
		return NULL ;
	}
	if (! found)
	{
		return NULL ;
	}

	return monitor_socket ;
}

static int
owsl_monitor_socket_compare
(
	const void * void1,
	const void * void2
)
{
	const OWSLMonitorSocket * socket1 = void1 ;
	const OWSLMonitorSocket * socket2 = void2 ;
	if (socket1->system_socket < socket2->system_socket)
	{
		return -1 ;
	}
	else if (socket1->system_socket == socket2->system_socket)
	{
		return 0 ;
	}
	else
	{
		return 1 ;
	}
}

int
owsl_monitor_socket_add
(
	OWSLSystemSocket system_socket,
	OWSLMonitorCallback callback_function,
	void * callback_user_data
)
{
	OWSLMonitorSocket * owsl_monitor_socket ;

	/* create socket */
	owsl_monitor_socket = malloc (sizeof (OWSLMonitorSocket)) ;
	if (owsl_monitor_socket == NULL)
	{
		return -1 ;
	}
	owsl_monitor_socket->system_socket = system_socket ;
	owsl_monitor_socket->callback_function = callback_function ;
	owsl_monitor_socket->callback_user_data = callback_user_data ;
	owsl_monitor_socket->events = 0 ;
	owsl_monitor_socket->once_event = 0 ;

	/* add new socket to the list */
	if (owlist_add (owsl_monitor_socket_list, owsl_monitor_socket, owsl_monitor_socket_compare))
	{
		free (owsl_monitor_socket) ;
		return -1 ;
	}

	/* register new socket for errors */
	if (owsl_monitor_event_add (system_socket, OWSL_MONITOR_ERROR))
	{
		free (owsl_monitor_socket) ;
		return -1 ;
	}

	return 0 ;
}

int
owsl_monitor_socket_remove
(
	OWSLSystemSocket system_socket
)
{
	OWListIterator * socket_list_iterator ;
	OWSLMonitorSocket * monitor_socket = NULL ;
	int found = 0, return_code = 0 ;

	/* nothing to do if monitor is killed */
	if (owsl_monitor_socket_list == NULL)
	{
		return 0 ;
	}

	/* unregister socket */
	if (owsl_monitor_register_event_remove (system_socket, OWSL_MONITOR_READ | OWSL_MONITOR_WRITE | OWSL_MONITOR_ERROR))
	{
		return -1 ;
	}

	/* search socket in the list */
	socket_list_iterator = owlist_iterator_new (owsl_monitor_socket_list, OWLIST_WRITE) ;
	if (socket_list_iterator == NULL)
	{
		return -1 ;
	}
	while (owlist_iterator_next (socket_list_iterator) == 0)
	{
		monitor_socket = owlist_iterator_get (socket_list_iterator) ;
		if (monitor_socket->system_socket == system_socket)
		{
			found = 1 ;
			break ;
		}
	}
	/* remove socket */
	if (found)
	{
		if (owlist_iterator_remove (socket_list_iterator))
		{
			return_code = -1 ;
		}
	}
	else
	{
		return_code = -1 ;
	}
	if (owlist_iterator_free (socket_list_iterator))
	{
		return_code = -1 ;
	}

	/* free socket */
	free (monitor_socket) ;

	return return_code ;
}

int
owsl_monitor_event_add
(
	OWSLSystemSocket system_socket,
	OWSLMonitorEvent event
)
{
	OWSLMonitorSocket * monitor_socket ;
	OWSLMonitorEvent previous_event ;

	if ((event & (~ OWSL_MONITOR_ONCE)) == 0)
	{
		return 0 ;
	}

	/* search socket in the list */
	monitor_socket = owsl_monitor_socket_get (system_socket) ;
	if (monitor_socket == NULL)
	{
		return -1 ;
	}

	/* manage once event */
	if (event & OWSL_MONITOR_ONCE)
	{
		/* get previous */
		if (monitor_socket->once_event != 0)
		{
			previous_event = monitor_socket->once_event ;
		}
		else
		{
			previous_event = monitor_socket->events ;
		}

		/* update once event */
		monitor_socket->once_event |= (event | OWSL_MONITOR_ERROR) ^ OWSL_MONITOR_ONCE ;

		/* remove previous */
		event = previous_event & (~ monitor_socket->once_event) ;
		if (owsl_monitor_register_event_remove (system_socket, event))
		{
			return -1 ;
		}

		/* prepare to add */
		event = monitor_socket->once_event & (~ previous_event) ;
	}
	else
	{
		/* get previous */
		if (monitor_socket->once_event == 0)
		{
			previous_event = monitor_socket->events ;
		}

		/* update events */
		monitor_socket->events |= event ;

		/* prepare to add */
		if (monitor_socket->once_event != 0)
		{
			return 0 ;
		}
		event &= ~ previous_event ;
	}

	/* add to the register */
	if (owsl_monitor_register_event_add (system_socket, event))
	{
		return -1 ;
	}

	return 0 ;
}

int
owsl_monitor_event_remove
(
	OWSLSystemSocket system_socket,
	OWSLMonitorEvent event
)
{
	OWSLMonitorSocket * monitor_socket ;
	OWSLMonitorEvent previous_event ;

	if ((event & (~ OWSL_MONITOR_ONCE)) == 0)
	{
		return 0 ;
	}

	/* search socket in the list */
	monitor_socket = owsl_monitor_socket_get (system_socket) ;
	if (monitor_socket == NULL)
	{
		return -1 ;
	}

	/* manage once event */
	if (event & OWSL_MONITOR_ONCE)
	{
		if (monitor_socket->once_event == 0)
		{
			return 0 ;
		}

		event ^= OWSL_MONITOR_ONCE ;

		if (monitor_socket->once_event == event)
		{
			return owsl_monitor_once_event_remove (monitor_socket) ;
		}

		/* get previous */
		previous_event = monitor_socket->once_event ;

		/* update once event */
		monitor_socket->once_event &= ~ event ;

		/* prepare to remove */
		event = previous_event & (~ monitor_socket->once_event) ;
	}
	else
	{
		/* get previous */
		if (monitor_socket->once_event == 0)
		{
			previous_event = monitor_socket->events ;
		}

		/* update events */
		monitor_socket->events &= ~ event ;

		/* prepare to remove */
		if (monitor_socket->once_event != 0)
		{
			return 0 ;
		}
		event &= previous_event ;
	}

	/* remove from the register */
	if (owsl_monitor_register_event_remove (system_socket, event))
	{
		return -1 ;
	}

	return 0 ;
}
