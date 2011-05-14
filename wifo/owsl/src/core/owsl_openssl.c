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

#include <openssl/ssl.h>
#include <pthread.h>

static pthread_mutex_t owsl_openssl_user_mutex ;
static size_t owsl_openssl_user_count ;
static pthread_mutex_t * owsl_openssl_mutex_array = NULL ;
static size_t owsl_openssl_mutex_size ;

struct CRYPTO_dynlock_value
{
	pthread_mutex_t mutex ;
} ;

int
owsl_openssl_wrapper_initialize
(void)
{
	if (pthread_mutex_init (& owsl_openssl_user_mutex, NULL))
	{
		return -1 ;
	}
	owsl_openssl_user_count = 0 ;
	return 0 ;
}

int
owsl_openssl_wrapper_terminate
(void)
{
	int return_code = 0 ;
	return_code |= pthread_mutex_destroy (& owsl_openssl_user_mutex) ;
	return return_code ;
}

static int
owsl_openssl_prng_seed
(void)
{
	/* TODO: PRNG should be seeded */
	/* On Win32 with specific device: CryptGetDefaultProvider, CryptAcquireContext, CryptGenRandom, CryptReleaseContext */
	return 0 ;
}

static void
owsl_openssl_locking_callback
(
	int action,
	int index,
	const char * file,
	int line
)
{
	if (action & CRYPTO_LOCK)
	{
		pthread_mutex_lock (& owsl_openssl_mutex_array [index]) ;
	}
	else
	{
		pthread_mutex_unlock (& owsl_openssl_mutex_array [index]) ;
	}
	return ;
}

#ifndef OS_WINDOWS
static unsigned long
owsl_openssl_thread_id_callback
(void)
{
	return (unsigned long) pthread_self () ;
}
#endif

static struct CRYPTO_dynlock_value *
owsl_openssl_dynlock_create_callback
(
	const char * file,
	int line
)
{
	struct CRYPTO_dynlock_value * dynlock ;
	dynlock = malloc (sizeof (struct CRYPTO_dynlock_value)) ;
	if (dynlock == NULL)
	{
		return NULL ;
	}
	pthread_mutex_init (& dynlock->mutex, NULL) ;
	return dynlock ;
}

static void
owsl_openssl_dynlock_destroy_callback
(
	struct CRYPTO_dynlock_value * dynlock,
	const char * file,
	int line
)
{
	pthread_mutex_destroy (& dynlock->mutex) ;
	free (dynlock) ;
	return ;
}

static void
owsl_openssl_dynlock_lock_callback
(
	int action,
	struct CRYPTO_dynlock_value * dynlock,
	const char * file,
	int line
)
{
	if (action & CRYPTO_LOCK)
	{
		pthread_mutex_lock (& dynlock->mutex) ;
	}
	else
	{
		pthread_mutex_unlock (& dynlock->mutex) ;
	}
}

static int
owsl_openssl_lock_initialize
(void)
{
	size_t mutex_index ;

	owsl_openssl_mutex_size = CRYPTO_num_locks () ;

	owsl_openssl_mutex_array = malloc (owsl_openssl_mutex_size * sizeof (pthread_mutex_t)) ;
	if (owsl_openssl_mutex_array == NULL)
	{
		return -1 ;
	}

	for (mutex_index = 0 ; mutex_index < owsl_openssl_mutex_size ; mutex_index ++)
	{
		pthread_mutex_init (& owsl_openssl_mutex_array [mutex_index], NULL) ;
	}

	CRYPTO_set_locking_callback (owsl_openssl_locking_callback) ;

	CRYPTO_set_dynlock_create_callback (owsl_openssl_dynlock_create_callback) ;
	CRYPTO_set_dynlock_destroy_callback (owsl_openssl_dynlock_destroy_callback) ;
	CRYPTO_set_dynlock_lock_callback (owsl_openssl_dynlock_lock_callback) ;

	return 0 ;
}

static int
owsl_openssl_lock_terminate
(void)
{
	int return_code = 0 ;
	size_t mutex_index ;

	if (owsl_openssl_mutex_array == NULL)
	{
		return -1 ;
	}

	CRYPTO_set_locking_callback (NULL) ;

	CRYPTO_set_dynlock_create_callback (NULL) ;
	CRYPTO_set_dynlock_lock_callback (NULL) ;
	CRYPTO_set_dynlock_destroy_callback (NULL) ;

	for (mutex_index = 0 ; mutex_index < owsl_openssl_mutex_size ; mutex_index ++)
	{
		pthread_mutex_destroy (& owsl_openssl_mutex_array [mutex_index]) ;
	}

	free (owsl_openssl_mutex_array) ;
	owsl_openssl_mutex_array = NULL ;

	return return_code ;
}

int
owsl_openssl_initialize
(void)
{
	if (pthread_mutex_lock (& owsl_openssl_user_mutex))
	{
		return -1 ;
	}

	if (owsl_openssl_user_count == 0)
	{
		SSL_library_init () ;

		if (owsl_openssl_prng_seed ())
		{
			return -1 ;
		}

#ifndef OS_WINDOWS
		/* OpenSSL doesn't need it on Windows */
		CRYPTO_set_id_callback (owsl_openssl_thread_id_callback) ;
#endif

		if (owsl_openssl_lock_initialize ())
		{
			return -1 ;
		}
	}

	owsl_openssl_user_count ++ ;

	if (pthread_mutex_unlock (& owsl_openssl_user_mutex))
	{
		return -1 ;
	}

	return 0 ;
}

int
owsl_openssl_terminate
(void)
{
	int return_code = 0 ;

	if (pthread_mutex_lock (& owsl_openssl_user_mutex))
	{
		return -1 ;
	}

	owsl_openssl_user_count -- ;

	if (owsl_openssl_user_count == 0)
	{
		CRYPTO_set_id_callback (NULL) ;

		return_code |= owsl_openssl_lock_terminate () ;
	}

	if (pthread_mutex_unlock (& owsl_openssl_user_mutex))
	{
		return -1 ;
	}

	return return_code ;
}
