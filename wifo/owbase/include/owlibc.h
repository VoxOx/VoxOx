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

#ifndef _OWLIBC_HEADER_
#define _OWLIBC_HEADER_

#include "owcommon.h"

#ifdef OS_WINDOWS

	/* disable secure compilation warnings */
	#if defined CC_MSVC || defined CC_WINCE
		#define _CRT_SECURE_NO_WARNINGS
	#endif /* CC_MSVC || defined CC_WINCE */

	/* string functions replacement */
	#define strcasecmp _stricmp
	#define strncasecmp _strnicmp
	#define strdup _strdup
	#define snprintf _snprintf
	#define vsnprintf _vsnprintf

	/* gettimeofday without time zone*/
	#ifndef __GNUC__
		#define EPOCHFILETIME (116444736000000000i64)
	#else
		#define EPOCHFILETIME (116444736000000000LL)
	#endif
	#define gettimeofday(tv,tz) \
	{ \
		FILETIME ft ; \
		LARGE_INTEGER li ; \
		__int64 t ; \
		GetSystemTimeAsFileTime (& ft) ; \
		li.LowPart = ft.dwLowDateTime ; \
		li.HighPart = ft.dwHighDateTime ; \
		t = li.QuadPart ;      /* In 100-nanosecond intervals */ \
		t -= EPOCHFILETIME ;   /* Offset to the Epoch time */ \
		t /= 10 ;              /* In microseconds */ \
		(tv)->tv_sec  = (long) (t / 1000000) ; \
		(tv)->tv_usec = (long) (t % 1000000) ; \
	}

	/* sleep in ms (portable max = 999) */
	#define msleep(ms) Sleep ((DWORD) ms)

#else /* OS_POSIX */

	#include <unistd.h>

	/* sleep in ms (portable max = 999) */
	#define msleep(ms) usleep ((useconds_t) ms * 1000)

#endif

#endif /* _OWLIBC_HEADER_ */
