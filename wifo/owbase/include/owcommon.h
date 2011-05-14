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

#ifndef _OWCOMMON_HEADER_
#define _OWCOMMON_HEADER_


/**
 * OS and compilers detection via defines (preprocessor).
 *
 * Most of this defines are from Qt (Trolltech).
 * In each category (OS_ and CC_) one define exludes the others.
 * Warning: if you want to add a define to this file, the order is very important.
 *
 * Operating systems:
 * - OS_WINDOWS
 *   - OS_WIN32
 *   - OS_WIN64
 *   - OS_WINCE_POCKETPC (Windows CE for PocketPC)
 *   - OS_WINCE_SMARTPHONE (Windows CE for smartphone)
 * - OS_POSIX
 *   - OS_MACOSX
 *   - OS_LINUX
 *   - OS_HURD
 *   - OS_BSD
 *     - OS_FREEBSD
 *     - OS_NETBSD
 *     - OS_OPENBSD
 *
 * Compilers:
 * - CC_MSVC (Microsoft Visual C++)
 *   - CC_MSVC6 (Visual C++ 6)
 *   - CC_MSVC7 (Visual C++ .NET)
 *   - CC_MSVC71 (Visual C++ 2003)
 *   - CC_MSVC8 (Visual C++ 2005)
 * - CC_WINCE (Microsoft Visual C++ Embedded for Windows CE)
 *   - CC_WINCE1 (Windows CE 1.x)
 *   - CC_WINCE2 (Windows CE 2.x)
 *   - CC_WINCE3 (Windows CE 3.x)
 *   - CC_WINCE4 (Windows CE 4.x)
 *   - CC_WINCE5 (Windows CE 5.x)
 * - CC_GCC
 *   - CC_MINGW (Native GCC under Windows)
 *   - CC_GCC3 (GNU GCC 3.x)
 *   - CC_GCC4 (GNU GCC 4.x)
 * - CC_INTEL (Intel C++)
 * - CC_BORLAND (Borland C++)
 *
 * @file global.h
 * @see qglobal.h from Qt4
 * @author Tanguy Krotoff
 */


/* OS */

#if defined(__APPLE__) && (defined(__GNUC__) || defined(__xlC__) || defined(__xlc__))
	#define OS_MACOSX
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
	#define OS_WIN32
#elif defined(WIN64) || defined(_WIN64) || defined(__WIN64__)
	#define OS_WIN64
#elif defined(__linux__) || defined(__linux)
	#define OS_LINUX
#elif defined(__FreeBSD__) || defined(__DragonFly__)
	#define OS_FREEBSD
#elif defined(__NetBSD__)
	#define OS_NETBSD
#elif defined(__OpenBSD__)
	#define OS_OPENBSD
#elif defined(__GNU_HURD__)
	#define OS_HURD
#elif defined(WIN32_PLATFORM_PSPC)
	#define OS_WINCE_POCKETPC
#elif defined(WIN32_PLATFORM_WFSP)
	#define OS_WINCE_SMARTPHONE
#else
	#error This OS has not been tested
#endif

#if defined(OS_WIN32) || defined(OS_WIN64) || defined(OS_WINCE_POCKETPC) || defined(OS_WINCE_SMARTPHONE)
	#define OS_WINDOWS
#endif

#if defined(OS_FREEBSD) || defined(OS_NETBSD) || defined(OS_OPENBSD)
	#define OS_BSD
#endif

#if defined(OS_MACOSX) || defined(OS_LINUX) || defined(OS_HURD) || defined(OS_BSD)
	#define OS_POSIX
#endif


/* COMPILERS */

#if defined(__INTEL_COMPILER)
	#define CC_INTEL
#elif defined(_MSC_VER)
	#define CC_MSVC
	#if _MSC_VER <= 1200
		#define CC_MSVC6
	#elif _MSC_VER <= 1300
		#define CC_MSVC7
	#elif _MSC_VER <= 1310
		#define CC_MSVC71
	#elif _MSC_VER <= 1400
		#define CC_MSVC8
	#endif
#elif defined(_WIN32_WCE)
	#define CC_WINCE
	#if _WIN32_WCE <= 101
		#define CC_WINCE1
	#elif _WIN32_WCE <= 211
		#define CC_WINCE2
	#elif _WIN32_WCE <= 300
		#define CC_WINCE3
	#elif _WIN32_WCE <= 400
		#define CC_WINCE4
	#elif _WIN32_WCE <= 500
		#define CC_WINCE5
	#endif
#elif defined(__GNUC__)
	#define CC_GCC
	#if __GNUC__ == 3
		#define CC_GCC3
	#elif __GNUC__ == 4
		#define CC_GCC4
	#endif
	#if defined(__MINGW32__)
		#define CC_MINGW
	#endif
#elif defined(__BORLANDC__) || defined(__TURBOC__)
	#define CC_BORLAND
#else
	#error This compiler has not been tested
#endif


/* PREPROCESSOR */

#if !defined(OS_WINDOWS) && !defined(OS_MACOSX)
	#undef _XOPEN_SOURCE
	#define _XOPEN_SOURCE 500
#endif

#define OW_STRINGIZE(str) #str
#define OW_STRINGIZE_PASS2(str) OW_STRINGIZE (str)

/* #pragma MESSAGE (...) clickable in Visual C++ */
#ifdef CC_MSVC
	#define OW_MESSAGE(msg) message (__FILE__ "(" OW_STRINGIZE_PASS2 (__LINE__) ") : " msg)
#endif


/* MACROS */

#define OW_MIN(a,b) (a < b ? a : b)
#define OW_MAX(a,b) (a > b ? a : b)

#define OW_MEMCPY(data_dst,len_dst,data_src,len_src)  \
{                                                     \
	if ((len_dst) != NULL)                            \
	{                                                 \
		int min_len = OW_MIN (* (len_dst), len_src) ; \
		if ((data_dst) != NULL)                       \
		{                                             \
			memcpy (data_dst, data_src, min_len) ;    \
		}                                             \
		* (len_dst) = min_len ;                       \
	}                                                 \
}

#if defined _DEBUG && defined CC_MSVC
#define OW_SET_THREAD_NAME(n)                \
{                                            \
	struct                                   \
	{                                        \
		DWORD type ;                         \
		LPCSTR name ;                        \
		DWORD thread_id ;                    \
		DWORD flags ;                        \
	} info ;                                 \
	info.type = 0x1000 ;                     \
	info.name = n ;                          \
	info.thread_id = GetCurrentThreadId () ; \
	info.flags = 0 ;                         \
	__try                                    \
	{                                        \
		RaiseException                       \
		(                                    \
			0x406D1388,                      \
			0,                               \
			sizeof (info) / sizeof (DWORD),  \
			(void *) & info                  \
		) ;                                  \
	}                                        \
	__except (EXCEPTION_CONTINUE_EXECUTION)  \
	{                                        \
	}                                        \
}
#else
#define OW_SET_THREAD_NAME(n)
#endif

#ifdef OS_WINDOWS
	#define OW_SET_ERROR(e) SetLastError (e)
	#define OW_GET_ERROR GetLastError ()
#else
	#define OW_SET_ERROR(e) (errno = e)
	#define OW_GET_ERROR errno
#endif


#endif /* _OWCOMMON_HEADER_ */
