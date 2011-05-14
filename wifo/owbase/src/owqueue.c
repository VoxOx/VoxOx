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

#include "owqueue.h"

#include <stdlib.h>
#include <memory.h>
#include <pthread.h>
#ifndef OS_WINDOWS
	#include <errno.h>
#endif

struct OWQueue
{
	/* data buffer */
	char * data_buffer ;                   /** used as a circular buffer for fifo memory */
	int data_buffer_size ;                 /** data length cannot exceed (data_buffer_size - 1) */
	int data_buffer_read_front_index ;     /** pointer on the byte before the data */
	int data_buffer_write_index ;          /** pointer on the byte after the data */

	/* info buffer */
	OWQueuePacketMode packet_mode ;        /** if packet mode is disabled, there is no info_buffer */
	char * info_buffer ;                   /** used as a circular buffer for fifo memory */
	int info_buffer_size ;                 /** infos length cannot exceed (info_buffer_size - 1) */
	int info_buffer_read_front_index ;     /** pointer on the byte before the first info */
	int info_buffer_write_index ;          /** pointer on the byte after the last info */
	int info_element_usable_size ;         /** size allocated for user in info_buffer */
	int info_element_real_size ;           /** size of an element of info_buffer */

	/* limits */
	int packet_max_nb ;                    /** maximum number of packets in the queue (= info_buffer_size - 1 / info_element_size) */
	int packet_min_size ;                  /** minimum size of a splitted packet in multi-packet mode */

	/* transaction */
	int transaction_read_length ;          /** bytes read in the current transaction */
	int transaction_write_length ;         /** bytes written in the current transaction */

	/* state and callback */
	int is_empty ;                         /** boolean */
	int space_needed ;                     /** size needed by a not blocking write which failed */
	pthread_mutex_t state_mutex ;          /** used for modification of is_empty and space_needed */
	OWQueueCallback callback_function ;    /** called at each event */
	const void * callback_user_data ;      /** used by callback_function */

	/* thread synchronization */
	OWQueueBlockingMode blocking_mode ;    /** wait & lock mode */
	pthread_mutex_t read_mutex ;           /** used for modification of read_front_index */
	pthread_mutex_t write_mutex ;          /** used for modification of write_index */
	pthread_mutex_t not_full_mutex ;       /** used with not_full_condition */
	pthread_mutex_t not_empty_mutex ;      /** used with not_empty_condition */
	pthread_cond_t not_full_condition ;    /** signaled after reading */
	pthread_cond_t not_empty_condition ;   /** signaled after writing */
} ;

OWQueue *
owqueue_new
(
	int usable_size,
	OWQueuePacketMode packet_mode,
	int packet_max_nb,
	int info_element_size
)
{
	OWQueue * queue ;
	const int data_buffer_size = usable_size + 1 ;
	int info_buffer_size, info_element_real_size ;

	if (packet_mode != OWQUEUE_NO_PACKET)
	{
		info_element_real_size = sizeof (int) + info_element_size ;   /* length of packet is stored before user info */
		info_buffer_size = info_element_real_size * packet_max_nb + 1 ;
		/* check oversize */
		if ((info_buffer_size - 1) / info_element_real_size != packet_max_nb)
		{
			return NULL ;
		}
	}
	else
	{
		info_element_size = 0 ;
		info_element_real_size = 0 ;
		info_buffer_size = 0 ;
		packet_max_nb = 0 ;
	}

	queue = malloc (sizeof (OWQueue) + data_buffer_size + info_buffer_size) ;
	if (queue != NULL)
	{
		queue->data_buffer = (char *) queue + sizeof (OWQueue) ;
		queue->data_buffer_size = data_buffer_size ;
		queue->data_buffer_read_front_index = usable_size ;
		queue->data_buffer_write_index = 0 ;
		queue->packet_mode = packet_mode ;
		if (packet_mode != OWQUEUE_NO_PACKET)
		{
			queue->info_buffer = (char *) queue + sizeof (OWQueue) + data_buffer_size ;
		}
		else
		{
			queue->info_buffer = NULL ;
		}
		queue->info_buffer_size = info_buffer_size ;
		queue->info_buffer_read_front_index = info_buffer_size - 1 ;
		queue->info_buffer_write_index = 0 ;
		queue->info_element_usable_size = info_element_size ;
		queue->info_element_real_size = info_element_real_size ;
		queue->packet_max_nb = packet_max_nb ;
		queue->packet_min_size = OWQUEUE_DEFAULT_PACKET_MIN_SIZE ;
		queue->transaction_read_length = 0 ;
		queue->transaction_write_length = 0 ;
		queue->is_empty = 1 ;
		queue->space_needed = 0 ;
		if (pthread_mutex_init (& queue->state_mutex, NULL))
		{
			free (queue) ;
			return NULL ;
		}
		queue->callback_function = NULL ;
		queue->callback_user_data = NULL ;
		queue->blocking_mode = OWQUEUE_BLOCKING ;
		if (pthread_mutex_init (& queue->read_mutex, NULL))
		{
			pthread_mutex_destroy (& queue->state_mutex) ;
			free (queue) ;
			return NULL ;
		}
		if (pthread_mutex_init (& queue->write_mutex, NULL))
		{
			pthread_mutex_destroy (& queue->read_mutex) ;
			free (queue) ;
			return NULL ;
		}
		if (pthread_mutex_init (& queue->not_full_mutex, NULL))
		{
			pthread_mutex_destroy (& queue->write_mutex) ;
			pthread_mutex_destroy (& queue->read_mutex) ;
			pthread_mutex_destroy (& queue->state_mutex) ;
			free (queue) ;
			return NULL ;
		}
		if (pthread_mutex_init (& queue->not_empty_mutex, NULL))
		{
			pthread_mutex_destroy (& queue->not_full_mutex) ;
			pthread_mutex_destroy (& queue->write_mutex) ;
			pthread_mutex_destroy (& queue->read_mutex) ;
			pthread_mutex_destroy (& queue->state_mutex) ;
			free (queue) ;
			return NULL ;
		}
		if (pthread_cond_init (& queue->not_full_condition, NULL))
		{
			pthread_mutex_destroy (& queue->not_empty_mutex) ;
			pthread_mutex_destroy (& queue->not_full_mutex) ;
			pthread_mutex_destroy (& queue->write_mutex) ;
			pthread_mutex_destroy (& queue->read_mutex) ;
			pthread_mutex_destroy (& queue->state_mutex) ;
			free (queue) ;
			return NULL ;
		}
		if (pthread_cond_init (& queue->not_empty_condition, NULL))
		{
			pthread_cond_destroy (& queue->not_full_condition) ;
			pthread_mutex_destroy (& queue->not_empty_mutex) ;
			pthread_mutex_destroy (& queue->not_full_mutex) ;
			pthread_mutex_destroy (& queue->write_mutex) ;
			pthread_mutex_destroy (& queue->read_mutex) ;
			pthread_mutex_destroy (& queue->state_mutex) ;
			free (queue) ;
			return NULL ;
		}
	}

	return queue ;
}

int
owqueue_free
(
	OWQueue * queue
)
{
	int return_code = 0 ;
	return_code |= pthread_mutex_destroy (& queue->state_mutex) ;
	return_code |= pthread_cond_destroy (& queue->not_empty_condition) ;
	return_code |= pthread_cond_destroy (& queue->not_full_condition) ;
	return_code |= pthread_mutex_destroy (& queue->not_empty_mutex) ;
	return_code |= pthread_mutex_destroy (& queue->not_full_mutex) ;
	return_code |= pthread_mutex_destroy (& queue->write_mutex) ;
	return_code |= pthread_mutex_destroy (& queue->read_mutex) ;
	free (queue) ;
	return return_code ;
}

void
owqueue_blocking_mode_set
(
	OWQueue * queue,
	OWQueueBlockingMode mode
)
{
	queue->blocking_mode = mode ;
	return ;
}

void
owqueue_packet_min_size_set
(
	OWQueue * queue,
	int packet_min_size
)
{
	queue->packet_min_size = packet_min_size ;
	return ;
}

void
owqueue_callback_set
(
	OWQueue * queue,
	OWQueueCallback function,
	const void * user_data
)
{
	queue->callback_function = function ;
	queue->callback_user_data = user_data ;
	return ;
}

int
owqueue_is_empty
(
	OWQueue * queue
)
{
	return (queue->data_buffer_read_front_index + 1) % queue->data_buffer_size == queue->data_buffer_write_index ;
}

int
owqueue_is_full
(
	OWQueue * queue
)
{
	return queue->data_buffer_read_front_index == queue->data_buffer_write_index
	    || queue->info_buffer_read_front_index == queue->info_buffer_write_index
		|| queue->space_needed > 0 ;
}

static int
owqueue_get_data_length
(
	OWQueue * queue
)
{
	int queue_read_index = queue->data_buffer_read_front_index + 1 ;
	/* queue->data_buffer_write_index may change in another thread, then a snapshot is needed */
	int queue_data_buffer_write_index_snapshot = queue->data_buffer_write_index ;
	if (queue->data_buffer_read_front_index < queue_data_buffer_write_index_snapshot)
	{
		return queue_data_buffer_write_index_snapshot - queue_read_index ;
	}
	else
	{
		return queue_data_buffer_write_index_snapshot + queue->data_buffer_size - queue_read_index ;
	}
}

static int
owqueue_get_available_space
(
	OWQueue * queue
)
{
	/* queue->data_buffer_read_front_index may change in another thread, then a snapshot is needed */
	int queue_data_buffer_read_front_index_snapshot = queue->data_buffer_read_front_index ;
	/* compute the available space in the queue */
	if (queue_data_buffer_read_front_index_snapshot < queue->data_buffer_write_index)
	{
		return queue_data_buffer_read_front_index_snapshot + queue->data_buffer_size - queue->data_buffer_write_index ;
	}
	else
	{
		return queue_data_buffer_read_front_index_snapshot - queue->data_buffer_write_index ;
	}
}

static int
owqueue_begin_critical_section
(
	OWQueueBlockingMode blocking_mode,
	pthread_mutex_t * mutex
)
{
	if (blocking_mode == OWQUEUE_BLOCKING)
	{
		if (pthread_mutex_lock (mutex))
		{
			return -1 ;
		}
	}
	else
	{
		int return_code = pthread_mutex_trylock (mutex) ;
		if (return_code)
		{
			if (return_code == EBUSY)
			{
				return 1 ;
			}
			return -1 ;
		}
	}
	return 0 ;
}

static int
owqueue_end_critical_section
(
	pthread_mutex_t * mutex
)
{
	if (pthread_mutex_unlock (mutex))
	{
		return -1 ;
	}
	return 0 ;
}

static int
owqueue_check_wait_condition
(
	OWQueue * queue,
	OWQueueBlockingMode blocking_mode,
	pthread_mutex_t * mutex,
	pthread_cond_t * needed_condition,
	int (* test_negative_condition) (OWQueue *),
	int force_wait
)
{
	if (blocking_mode == OWQUEUE_BLOCKING)
	{
		if (pthread_mutex_lock (mutex))
		{
			return -1 ;
		}
		while (test_negative_condition (queue) || force_wait != 0)
		{
			force_wait = 0 ;
			/* wait for the needed_condition */
			if (pthread_cond_wait (needed_condition, mutex))
			{
				return -1 ;
			}
		}
		if (pthread_mutex_unlock (mutex))
		{
			return -1 ;
		}
	}
	else
	{
		if (test_negative_condition (queue))
		{
			return 1 ;
		}
	}
	return 0 ;
}

static int
owqueue_signal_condition
(
	OWQueue * queue,
	pthread_mutex_t * mutex,
	pthread_cond_t * condition
)
{
	if (pthread_mutex_lock (mutex))
	{
		return -1 ;
	}
	/* wake up threads that are waiting for the condition (even in mode OWQUEUE_NON_BLOCKING because of possible blocking calls) */
	if (pthread_cond_broadcast (condition))
	{
		return -1 ;
	}
	if (pthread_mutex_unlock (mutex))
	{
		return -1 ;
	}
	return 0 ;
}

static int
owqueue_signal_empty
(
	OWQueue * queue
)
{
	if (pthread_mutex_lock (& queue->state_mutex))
	{
		return -1 ;
	}
	if (owqueue_is_empty (queue))
	{
		queue->is_empty = 1 ;
		if (queue->callback_function != NULL)
		{
			queue->callback_function (queue, OWQUEUE_EMPTY, (void *) queue->callback_user_data) ;
		}
	}
	if (pthread_mutex_unlock (& queue->state_mutex))
	{
		return -1 ;
	}
	return 0 ;
}

static int
owqueue_signal_space_needed
(
	OWQueue * queue,
	int length
)
{
	if (pthread_mutex_lock (& queue->state_mutex))
	{
		return -1 ;
	}
	switch (queue->packet_mode)
	{
		case OWQUEUE_NO_PACKET :
		{
			queue->space_needed = 1 ;
			break ;
		}
		case OWQUEUE_PACKET :
		{
			queue->space_needed = length ;
			break ;
		}
		case OWQUEUE_MULTI_PACKET :
		{
			queue->space_needed = queue->packet_min_size ;
			break ;
		}
	}
	if (owqueue_get_available_space (queue) < queue->space_needed)
	{
		if (queue->callback_function != NULL)
		{
			queue->callback_function (queue, OWQUEUE_SPACE_NEEDED, (void *) queue->callback_user_data) ;
		}
	}
	else
	{
		queue->space_needed = 0 ;
	}
	if (pthread_mutex_unlock (& queue->state_mutex))
	{
		return -1 ;
	}
	return 0 ;
}

static int
owqueue_signal_read
(
	OWQueue * queue
)
{
	OWQueueEvent event = 0 ;
	if (pthread_mutex_lock (& queue->state_mutex))
	{
		return -1 ;
	}
	if (owqueue_is_empty (queue))
	{
		queue->is_empty = 1 ;
		event |= OWQUEUE_EMPTY ;
	}
	if (queue->space_needed <= owqueue_get_available_space (queue))
	{
		queue->space_needed = 0 ;
		event |= OWQUEUE_SPACE_AVAILABLE ;
	}
	if (event != 0 && queue->callback_function != NULL)
	{
		queue->callback_function (queue, event, (void *) queue->callback_user_data) ;
	}
	if (pthread_mutex_unlock (& queue->state_mutex))
	{
		return -1 ;
	}
	return 0 ;
}

static int
owqueue_signal_write
(
	OWQueue * queue
)
{
	OWQueueEvent event = 0 ;
	if (pthread_mutex_lock (& queue->state_mutex))
	{
		return -1 ;
	}
	if (queue->is_empty)
	{
		/* was empty before this write */
		queue->is_empty = 0 ;
		event |= OWQUEUE_NOT_EMPTY ;
	}
	else if (queue->space_needed)
	{
		/* if write succeed and there was space needed, assume that last failed write is forgotten */
		queue->space_needed = 0 ;
		event |= OWQUEUE_SPACE_AVAILABLE ;
	}
	if (owqueue_is_full (queue))
	{
		if (queue->packet_mode == OWQUEUE_MULTI_PACKET)
		{
			queue->space_needed = queue->packet_min_size ;
		}
		else
		{
			queue->space_needed = 1 ;
		}
		event |= OWQUEUE_SPACE_NEEDED ;
	}
	if (event != 0 && queue->callback_function != NULL)
	{
		queue->callback_function (queue, event, (void *) queue->callback_user_data) ;
	}
	if (pthread_mutex_unlock (& queue->state_mutex))
	{
		return -1 ;
	}
	return 0 ;
}

static void
owqueue_shift_circular_buffer_index
(
	int * index,
	int offset,
	int size
)
{
	if (size > 0)
	{
		* index = (* index + offset) % size ;
	}
	return ;
}

static void
owqueue_read_circular_buffer_without_commit
(
	const void * source,
	int source_size,
	int source_read_front_index,
	void * destination,
	int length
)
{
	int chunk1_length, chunk2_length ;

	/* read all the data or until the end of the circular buffer */
	chunk1_length = source_size - source_read_front_index - 1 ;
	if (chunk1_length > 0)
	{
		if (chunk1_length > length)
		{
			chunk1_length = length ;
		}
		memcpy (destination, (char *) source + source_read_front_index + 1, chunk1_length) ;
	}

	/* read the second chunk at the beginning of the circular buffer if needed */
	chunk2_length = length - chunk1_length ;
	if (chunk2_length > 0)
	{
		memcpy ((char *) destination + chunk1_length, source, chunk2_length) ;
	}

	return ;
}

static void
owqueue_write_circular_buffer_without_commit
(
	void * destination,
	int destination_size,
	int destination_write_index,
	const void * source,
	int length
)
{
	int chunk1_length, chunk2_length ;

	/* write all the data or until the end of the circular buffer */
	chunk1_length = destination_size - destination_write_index ;
	if (chunk1_length > length)
	{
		chunk1_length = length ;
	}
	memcpy ((char *) destination + destination_write_index, source, chunk1_length) ;

	/* write the second chunk at the beginning of the circular buffer if needed */
	chunk2_length = length - chunk1_length ;
	if (chunk2_length > 0)
	{
		memcpy (destination, (char *) source + chunk1_length, chunk2_length) ;
	}

	return ;
}

static void
owqueue_read_circular_buffer
(
	const void * source,
	int source_size,
	int * source_read_front_index,
	void * destination,
	int length
)
{
	owqueue_read_circular_buffer_without_commit (source, source_size, * source_read_front_index, destination, length) ;
	owqueue_shift_circular_buffer_index (source_read_front_index, length, source_size) ;
	return ;
}

static void
owqueue_write_circular_buffer
(
	void * destination,
	int destination_size,
	int * destination_write_index,
	const void * source,
	int length
)
{
	owqueue_write_circular_buffer_without_commit (destination, destination_size, * destination_write_index, source, length) ;
	owqueue_shift_circular_buffer_index (destination_write_index, length, destination_size) ;
	return ;
}

static int
owqueue_transaction_read
(
	OWQueue * queue,
	void * buffer,
	int size,
	void * info,
	OWQueueBlockingMode blocking_mode
)
{
	int return_code ;
	int length ;
	int local_data_buffer_read_front_index = queue->data_buffer_read_front_index ;
	int local_info_buffer_read_front_index = queue->info_buffer_read_front_index ;

	/* begin critical section */
	return_code = owqueue_begin_critical_section (blocking_mode, & queue->read_mutex) ;
	if (return_code)
	{
		if (return_code > 0)
		{
			return_code = 0 ;   /* abort in non blocking mode */
		}
		return return_code ;
	}
	/* from this point, mutex must be unlocked before return or in the transaction close */

	/* check if queue is not empty */
	return_code = owqueue_check_wait_condition (queue, blocking_mode, & queue->not_empty_mutex, & queue->not_empty_condition, owqueue_is_empty, 0) ;
	if (return_code)
	{
		if (return_code > 0)
		{
			owqueue_signal_empty (queue) ;
			return_code = 0 ;   /* abort in non blocking mode */
		}
		owqueue_end_critical_section (& queue->read_mutex) ;
		return return_code ;
	}

	/* read info and get data length */
	if (queue->packet_mode == OWQUEUE_NO_PACKET)
	{
		/* compute the data length of the queue */
		length = owqueue_get_data_length (queue) ;
		/* get the maximum length to read */
		if (length > size)
		{
			length = size ;
		}
	}
/* TODO: improve MULTI_PACKET by reading many packets at once */
	else if (queue->packet_mode == OWQUEUE_PACKET || queue->packet_mode == OWQUEUE_MULTI_PACKET)
	{
		/* read data length in info buffer without commit */
		owqueue_read_circular_buffer_without_commit (queue->info_buffer, queue->info_buffer_size, local_info_buffer_read_front_index, (void *) & length, sizeof (int)) ;
		/* check if packet can be read */
		if (length > size)
		{
			owqueue_end_critical_section (& queue->read_mutex) ;
			return -1 ;
		}
		/* data can be read, commit data length read by shifting info index */
		owqueue_shift_circular_buffer_index (& local_info_buffer_read_front_index, sizeof (int), queue->info_buffer_size) ;
		/* read packet info if exists */
		if (queue->info_element_usable_size > 0)
		{
			/* read info */
			if (info != NULL)
			{
				owqueue_read_circular_buffer_without_commit (queue->info_buffer, queue->info_buffer_size, local_info_buffer_read_front_index, info, queue->info_element_usable_size) ;
			}
			owqueue_shift_circular_buffer_index (& local_info_buffer_read_front_index, queue->info_element_usable_size, queue->info_buffer_size) ;
		}
	}

	/* copy queue data to buffer */
	owqueue_read_circular_buffer (queue->data_buffer, queue->data_buffer_size, & local_data_buffer_read_front_index, buffer, length) ;

	if (length <= 0)
	{
		owqueue_end_critical_section (& queue->read_mutex) ;
	}

	queue->transaction_read_length = length ;
	return length ;
}

static int
owqueue_transaction_write
(
	OWQueue * queue,
	const void * data,
	int length,
	const void * info,
	OWQueueBlockingMode blocking_mode
)
{
	int return_code ;
	int max_length = length ;
	int queue_available, queue_needed_space = 0 ;
	int local_data_buffer_write_index = queue->data_buffer_write_index ;
	int local_info_buffer_write_index = queue->info_buffer_write_index ;

	/* begin critical section */
	return_code = owqueue_begin_critical_section (blocking_mode, & queue->write_mutex) ;
	if (return_code)
	{
		if (return_code > 0)
		{
			return_code = 0 ;   /* abort in non blocking mode */
		}
		return return_code ;
	}
	/* from this point, mutex must be unlocked before return or in the transaction close */

	do   /* loop in blocking mode */
	{
		/* check if queue is not full */
		return_code = owqueue_check_wait_condition (queue, blocking_mode, & queue->not_full_mutex, & queue->not_full_condition, owqueue_is_full, queue_needed_space) ;
		if (return_code)
		{
			if (return_code > 0)
			{
				owqueue_signal_space_needed (queue, length) ;
				return_code = 0 ;   /* abort in non blocking mode */
			}
			owqueue_end_critical_section (& queue->write_mutex) ;
			return return_code ;
		}

		/* compute the available space in the queue */
		queue_available = owqueue_get_available_space (queue) ;

		/* set maximum data length */
		if (queue->packet_mode == OWQUEUE_NO_PACKET || queue->packet_mode == OWQUEUE_MULTI_PACKET)
		{
			/* get the maximum length to write */
			length = max_length ;   /* reset to max length at each loop iteration */
			if (length > queue_available)
			{
				length = queue_available ;
			}
		}

		/* check if packet can be written */
		if (queue->packet_mode == OWQUEUE_PACKET)
		{
			queue_needed_space = length - queue_available ;
		}
		else if (queue->packet_mode == OWQUEUE_MULTI_PACKET)
		{
			queue_needed_space = queue->packet_min_size - length ;
		}

		if (queue->packet_mode == OWQUEUE_PACKET || queue->packet_mode == OWQUEUE_MULTI_PACKET)
		{
			if (queue_needed_space > 0)
			{
				if (blocking_mode != OWQUEUE_BLOCKING)
				{
					owqueue_signal_space_needed (queue, length) ;
					owqueue_end_critical_section (& queue->write_mutex) ;
					return 0 ;
				}
			}
			else
			{
				/* write data length in info buffer */
				owqueue_write_circular_buffer_without_commit (queue->info_buffer, queue->info_buffer_size, local_info_buffer_write_index, (void *) & length, sizeof (int)) ;
				/* data can be written, commit data length write by shifting info index */
				owqueue_shift_circular_buffer_index (& local_info_buffer_write_index, sizeof (int), queue->info_buffer_size) ;
				/* write packet info if exists */
				if (queue->info_element_usable_size > 0)
				{
					if (info != NULL)
					{
						/* write info */
						owqueue_write_circular_buffer (queue->info_buffer, queue->info_buffer_size, & local_info_buffer_write_index, info, queue->info_element_usable_size) ;
					}
					else
					{
						/* write zeros */
						char * info_zeros = malloc (queue->info_element_usable_size) ;
						if (info_zeros == NULL)
						{
							owqueue_end_critical_section (& queue->write_mutex) ;
							return -1 ;
						}
						memset (info_zeros, 0, queue->info_element_usable_size) ;
						owqueue_write_circular_buffer (queue->info_buffer, queue->info_buffer_size, & local_info_buffer_write_index, info_zeros, queue->info_element_usable_size) ;
					}
				}
			}
		}
	}
	while (queue_needed_space > 0) ;

	/* copy data to queue */
	owqueue_write_circular_buffer (queue->data_buffer, queue->data_buffer_size, & local_data_buffer_write_index, data, length) ;

	if (length <= 0)
	{
		owqueue_end_critical_section (& queue->write_mutex) ;
	}

	queue->transaction_write_length = length ;
	return length ;
}

int
owqueue_read_transaction_close
(
	OWQueue * queue,
	int length
)
{
	int return_code = 0 ;

	if
	(
		queue == NULL
		||
		queue->transaction_read_length == 0
		||
		length > queue->transaction_read_length
		||
		(queue->packet_mode == OWQUEUE_PACKET && length != queue->transaction_read_length)
	)
	{
		return -1 ;
	}
/* TODO: improve MULTI_PACKET by handling reading many packets at once */

	/* commit read */
	if (length > 0)
	{
		owqueue_shift_circular_buffer_index (& queue->data_buffer_read_front_index, length, queue->data_buffer_size) ;
		if (queue->packet_mode == OWQUEUE_MULTI_PACKET && length < queue->transaction_read_length)
		{
			/* modify data length in info buffer */
			int new_length ;
			owqueue_read_circular_buffer_without_commit (queue->info_buffer, queue->info_buffer_size, queue->info_buffer_read_front_index, & new_length, sizeof (int)) ;
			new_length -= length ;
			owqueue_write_circular_buffer_without_commit (queue->info_buffer, queue->info_buffer_size, queue->info_buffer_write_index, (void *) & length, sizeof (int)) ;
		}
		else
		{
			owqueue_shift_circular_buffer_index (& queue->info_buffer_read_front_index, queue->info_element_real_size, queue->info_buffer_size) ;
		}

		/* signal to threads waiting for writing */
		return_code |= owqueue_signal_condition (queue, & queue->not_full_mutex, & queue->not_full_condition) ;

		/* signal callback and update variables */
		return_code |= owqueue_signal_read (queue) ;
	}

	/* end transaction */
	queue->transaction_read_length = 0 ;

	/* end critical section */
	return_code |= owqueue_end_critical_section (& queue->read_mutex) ;

	return return_code ;
}

int
owqueue_write_transaction_close
(
	OWQueue * queue,
	int length
)
{
	int return_code = 0 ;

	if
	(
		queue == NULL
		||
		queue->transaction_write_length == 0
		||
		length > queue->transaction_write_length
		||
		(queue->packet_mode == OWQUEUE_PACKET && length != queue->transaction_write_length)
		||
		(queue->packet_mode == OWQUEUE_MULTI_PACKET && length < queue->packet_min_size)
	)
	{
		return -1 ;
	}

	/* commit write */
	if (length > 0)
	{
		if (queue->packet_mode == OWQUEUE_MULTI_PACKET && length < queue->transaction_write_length)
		{
			/* modify data length in info buffer */
			owqueue_write_circular_buffer_without_commit (queue->info_buffer, queue->info_buffer_size, queue->info_buffer_write_index, (void *) & length, sizeof (int)) ;
		}
		owqueue_shift_circular_buffer_index (& queue->data_buffer_write_index, length, queue->data_buffer_size) ;
		owqueue_shift_circular_buffer_index (& queue->info_buffer_write_index, queue->info_element_real_size, queue->info_buffer_size) ;

		/* signal to threads waiting for reading */
		return_code |= owqueue_signal_condition (queue, & queue->not_empty_mutex, & queue->not_empty_condition) ;

		/* signal callback and update variables */
		return_code |= owqueue_signal_write (queue) ;
	}

	/* end transaction */
	queue->transaction_write_length = 0 ;

	/* end critical section */
	return_code |= owqueue_end_critical_section (& queue->write_mutex) ;

	return return_code ;
}

int
owqueue_read
(
	OWQueue * queue,
	void * buffer,
	int size,
	void * info,
	int mode
)
{
	OWQueueBlockingMode blocking_mode ;
	int read_bytes, remove_length ;

	if (queue == NULL)
	{
		return -1 ;
	}
	if (buffer == NULL || size == 0)
	{
		return 0 ;
	}

	if ((mode & OWQUEUE_BLOCKING) != 0)
	{
		blocking_mode = OWQUEUE_BLOCKING ;
	}
	else if ((mode & OWQUEUE_NON_BLOCKING) != 0)
	{
		blocking_mode = OWQUEUE_NON_BLOCKING ;
	}
	else
	{
		blocking_mode = queue->blocking_mode ;
	}

	read_bytes = owqueue_transaction_read (queue, buffer, size, info, blocking_mode) ;
	if ((mode & OWQUEUE_TRANSACTION_OPEN) == 0 && read_bytes > 0)
	{
		if ((mode & OWQUEUE_PEEK) != 0)
		{
			remove_length = 0 ;
		}
		else
		{
			remove_length = read_bytes ;
		}
		if (owqueue_read_transaction_close (queue, remove_length))
		{
			return -1 ;
		}
	}

	return read_bytes ;
}

int
owqueue_write
(
	OWQueue * queue,
	const void * data,
	int length,
	const void * info,
	int mode
)
{
	OWQueueBlockingMode blocking_mode ;
	int written_bytes, add_length ;

	if (queue == NULL)
	{
		return -1 ;
	}
	if (data == NULL || length == 0)
	{
		return 0 ;
	}

	if ((mode & OWQUEUE_BLOCKING) != 0)
	{
		blocking_mode = OWQUEUE_BLOCKING ;
	}
	else if ((mode & OWQUEUE_NON_BLOCKING) != 0)
	{
		blocking_mode = OWQUEUE_NON_BLOCKING ;
	}
	else
	{
		blocking_mode = queue->blocking_mode ;
	}

	written_bytes = owqueue_transaction_write (queue, data, length, info, blocking_mode) ;
	if ((mode & OWQUEUE_TRANSACTION_OPEN) == 0 && written_bytes > 0)
	{
		if ((mode & OWQUEUE_PEEK) != 0)
		{
			add_length = 0 ;
		}
		else
		{
			add_length = written_bytes ;
		}
		if (owqueue_write_transaction_close (queue, add_length))
		{
			return -1 ;
		}
	}

	return written_bytes ;
}
