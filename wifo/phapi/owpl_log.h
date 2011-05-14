/*
 * PhApi, a voice over Internet library
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
#ifndef OWPL_LOG_H
#define OWPL_LOG_H

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
	#if defined(BUILD_PHAPI_DLL)
		#define MY_DLLEXPORT __declspec(dllexport)
	#elif defined(PHAPI_DLL)
		#define MY_DLLEXPORT __declspec(dllimport)
	#endif
#endif

#ifndef MY_DLLEXPORT
	#define MY_DLLEXPORT
#endif

typedef enum OWPL_LOG_LEVEL_ {
	OWPL_LOG_LEVEL_DEBUG,
	OWPL_LOG_LEVEL_INFO,
	OWPL_LOG_LEVEL_WARN,
	OWPL_LOG_LEVEL_ERROR
} OWPL_LOG_LEVEL;


/**
 * Signature for log function.  Application developers should not block the
 * calling thread.
 *
 * @param level The log level
 * @param message The log message
 */
typedef void (*OWPL_LOG_FUNCTION)(OWPL_LOG_LEVEL level, const char * message);


MY_DLLEXPORT void
owplSetLogFunction(OWPL_LOG_FUNCTION function);

MY_DLLEXPORT void
owplLogMessageAP(OWPL_LOG_LEVEL level, const char* format, va_list ap);

MY_DLLEXPORT void
owplLogMessage(OWPL_LOG_LEVEL level, const char* format, ...);

MY_DLLEXPORT void
owplLogDebug(const char* format, ...);

MY_DLLEXPORT void
owplLogInfo(const char* format, ...);

MY_DLLEXPORT void
owplLogWarn(const char* format, ...);

MY_DLLEXPORT void
owplLogError(const char* format, ...);

#ifdef __cplusplus
}
#endif

#endif /* OWPL_LOG_H */
