/*
 * rand_source.c
 *
 * implements a random source based on /dev/random
 *
 * David A. McGrew
 * Cisco Systems, Inc.
 */
/*
 *	
 * Copyright(c) 2001-2005 Cisco Systems, Inc.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 *   Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * 
 *   Redistributions in binary form must reproduce the above
 *   copyright notice, this list of conditions and the following
 *   disclaimer in the documentation and/or other materials provided
 *   with the distribution.
 * 
 *   Neither the name of the Cisco Systems, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */


#include "rand_source.h"

#include <fcntl.h>          /* for open()  */
#ifdef WIN32
#define WINVER 0x0400
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wincrypt.h>
#include <io.h>
#define close _close
#define open _open
#define O_RDONLY _O_RDONLY
#else
#include <unistd.h>
#endif


/* global dev_rand_fdes is file descriptor for /dev/random */

int dev_random_fdes = 0;

err_status_t
rand_source_init() {

  /* open /dev/random for reading */
  dev_random_fdes = open("/dev/urandom", O_RDONLY, 0);
  if (dev_random_fdes == -1)
#if DEV_RANDOM == 1
    return err_status_init_fail;
#else
	dev_random_fdes = 0;
#endif

  return err_status_ok;
}

err_status_t
rand_source_get_octet_string(void *dest, int len) {

  /* 
   * read len octets from /dev/random to dest, and
   * check return value to make sure enough octets were
   * written 
   */
  if(dev_random_fdes) {
    if (read(dev_random_fdes, dest, len) != len)
      return err_status_fail;
  } else {
	int* i;
	char* c;
	char* end = (char*)dest + len - 1;
#if defined WIN32
	int rnd;
	HCRYPTPROV hProv=0;
	CryptAcquireContext(&hProv,NULL,NULL, PROV_DH_SCHANNEL , CRYPT_VERIFYCONTEXT);
#endif
	while((char*)dest <= end - sizeof(int) + 1) {
#if defined WIN32
		if (!CryptGenRandom(hProv,4,&rnd)) {
			time_t t; time(&t);
			srand(t^=rand()); rnd=rand();
		};
		*((int*)dest) = rnd;
#else
		*((int*)dest) = rand();
#endif
		i=dest;i++;dest=i;
	}
	while(dest <= end) {
#if defined WIN32
		if (!CryptGenRandom(hProv,4,&rnd)) {
			time_t t; time(&t);srand(t^=rand()); rnd=rand();
		};
		*(((char*)dest)) = (char)rnd;
#else
		*(((char*)dest)) = (char)rand();
#endif
		c=dest;c++;dest=c;
	}
#if defined WIN32
	CryptReleaseContext(hProv,0);
#endif
  }

  return err_status_ok;
}

err_status_t
rand_source_deinit() {

  if (dev_random_fdes == -1)
    return err_status_dealloc_fail;  /* well, we haven't really failed, *
				      * but there is something wrong    */
  if (dev_random_fdes > 0)
    close(dev_random_fdes);  
  
  return err_status_ok;  
}
