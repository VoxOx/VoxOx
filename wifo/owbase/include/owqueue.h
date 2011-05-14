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

#ifndef _OWQUEUE_HEADER_
#define _OWQUEUE_HEADER_

#include "owcommon.h"

#ifdef OS_WINDOWS
	#if defined BUILD_OWBASE_DLL
		#define OWQUEUE_FUNC_DEF __declspec(dllexport)
	#elif ! defined BUILD_OWBASE_STATIC
		#define OWQUEUE_FUNC_DEF __declspec(dllimport)
	#endif
#endif
#ifndef OWQUEUE_FUNC_DEF
	#define OWQUEUE_FUNC_DEF
#endif /* OWQUEUE_FUNC_DEF */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * The OWQueue is a FIFO packet memory implemented as a circular buffer that can be read and written simultaneously.
 * Then, the data length cannot be compute.
 * To distinguish the full and empty states, the read index (called read_front_index) must point before the data.
 * In this scheme, the queue is empty if the read index is just before the write index.
 * It is full if the write index point the same byte as read index.
 */

#define OWQUEUE_DATA_BUFFER_USABLE_SIZE(queue) (queue->data_buffer_size - 1)
#define OWQUEUE_DEFAULT_PACKET_MIN_SIZE 30

typedef struct OWQueue OWQueue ;

typedef enum OWQueuePacketMode
{
	OWQUEUE_NO_PACKET,     /** bytes are independents, no info is stored */
	OWQUEUE_PACKET,        /** data to write are a packet, read only a packet at once */
	OWQUEUE_MULTI_PACKET   /** data to write can be split if there is not enough space, read many packets (with the same info) at once if possible */
} OWQueuePacketMode ;

typedef enum OWQueueBlockingMode
{
	OWQUEUE_BLOCKING         = 1 << 0,   /** read waits if queue is empty or being read, write waits if queue is full or being written */
	OWQUEUE_NON_BLOCKING     = 1 << 1    /** read and write do not suspend the execution */
} OWQueueBlockingMode ;
typedef enum OWQueueTransactionMode
{
	OWQUEUE_PEEK             = 1 << 2,   /** read/write does not modify the queue (as a rollbacked transaction) */
	OWQUEUE_TRANSACTION_OPEN = 1 << 3    /** read/write begins a transaction which must be ended (allow commit/rollback) */
} OWQueueTransactionMode ;

typedef enum OWQueueEvent
{
	OWQUEUE_EMPTY           = 1 << 0,   /** no data after a read */
	OWQUEUE_NOT_EMPTY       = 1 << 1,   /** write in an empty queue */
	OWQUEUE_SPACE_NEEDED    = 1 << 2,   /** not blocking write failed because of space */
	OWQUEUE_SPACE_AVAILABLE = 1 << 3    /** sufficient space for last failed write is freed by a read */
} OWQueueEvent ;
typedef void (* OWQueueCallback) (OWQueue * queue, OWQueueEvent event, void * user_data) ;

/**
 * Create a queue.
 * By default, blocking_mode = OWQUEUE_BLOCKING.
 *
 * @param usable_size:       data buffer will have a size equals to size + 1
 * @param packet_mode:       behaviour of the queue
 * @param packet_max_nb:     maximum number of packets in the queue (ignored if packet mode is disabled)
 * @param info_element_size: size of info data for each packet (ignored if packet mode is disabled)
 *
 * @return pointer on the new queue
 * @return NULL if there is not enough memory
 */
OWQUEUE_FUNC_DEF OWQueue *
owqueue_new
(
	int usable_size,
	OWQueuePacketMode packet_mode,
	int packet_max_nb,
	int info_element_size
) ;

/**
 * Destroy a queue.
 */
OWQUEUE_FUNC_DEF int
owqueue_free
(
	OWQueue * queue
) ;

/**
 * Set blocking mode.
 */
OWQUEUE_FUNC_DEF void
owqueue_blocking_mode_set
(
	OWQueue * queue,
	OWQueueBlockingMode mode
) ;

/**
 * Set packet minimum size for multi-packet mode.
 */
OWQUEUE_FUNC_DEF void
owqueue_packet_min_size_set
(
	OWQueue * queue,
	int packet_min_size
) ;

/**
 * Set callback.
 *
 * @warning callback should be short and must not call owqueue read/write functions
 */
OWQUEUE_FUNC_DEF void
owqueue_callback_set
(
	OWQueue * queue,
	OWQueueCallback function,
	const void * user_data
) ;

/**
 * @return a not null value if the queue is empty
 * @return 0 otherwise
 */
OWQUEUE_FUNC_DEF int
owqueue_is_empty
(
	OWQueue * queue
) ;

/**
 * @return a not null value if the queue is full
 * @return 0 otherwise
 */
OWQUEUE_FUNC_DEF int
owqueue_is_full
(
	OWQueue * queue
) ;

/**
 * Get data in circular fifo memory.
 * In packet mode, if the packet length is bigger than the input size, nothing is read.
 *
 * @param transaction can be NULL or must be later used to call owqueue_read_transaction_close
 * @param mode OWQueueBlockingMode can be 0 (queue mode is used) / OWQueueTransactionMode
 *
 * @return length of read data
 * @return negative value if there is an error
 */
OWQUEUE_FUNC_DEF int
owqueue_read
(
	OWQueue * queue,
	void * buffer,
	int size,
	void * info,
	int mode
) ;

/**
 * Push data in circular fifo memory.
 *
 * @param transaction can be NULL or must be later used to call owqueue_write_transaction_close
 * @param mode OWQueueBlockingMode can be 0 (queue mode is used) / OWQueueTransactionMode
 *
 * @return length of written data
 * @return negative value if there is an error
 */
OWQUEUE_FUNC_DEF int
owqueue_write
(
	OWQueue * queue,
	const void * data,
	int length,
	const void * info,
	int mode
) ;

/**
 * Close read transaction.
 *
 * @warning do not close if owqueue_read returns a value <= 0
 *
 * @param length represents read bytes that will be removed from the queue
 */
OWQUEUE_FUNC_DEF int
owqueue_read_transaction_close
(
	OWQueue * queue,
	int length
) ;

/**
 * Close write transaction.
 * In packet mode, length must be equal to transaction_write_length.
 * In multi-packet mode, length must be greater than packet_min_size.
 *
 * @warning do not close if owqueue_write returns a value <= 0
 *
 * @param length represents written bytes that will be saved for a future read
 */
OWQUEUE_FUNC_DEF int
owqueue_write_transaction_close
(
	OWQueue * queue,
	int length
) ;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _OW_QUEUE_HEADER_ */
