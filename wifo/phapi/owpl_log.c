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
#include "owpl_log.h"

#include <stdio.h>
#include <stdarg.h>

#ifdef _MSC_VER
	#define vsnprintf _vsnprintf
#endif

static OWPL_LOG_FUNCTION owplLogFunction = NULL;

void
owplSetLogFunction(OWPL_LOG_FUNCTION function)
{
	owplLogFunction = function;
}

#define PARSE_VA_ARGS \
	char buffer[1024]; \
	va_list ap; \
	va_start(ap, format); \
	vsnprintf(buffer, sizeof(buffer), format, ap); \
	va_end(ap);

#define DO_LOG(level) \
	if (owplLogFunction) { \
		owplLogFunction(level, buffer); \
	} else { \
		fprintf(stderr, "owpl[%d]: %s\n", level, buffer); \
	}

void
owplLogMessageAP(OWPL_LOG_LEVEL level, const char* format, va_list ap)
{
	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), format, ap);
	DO_LOG(level);
}

void
owplLogMessage(OWPL_LOG_LEVEL level, const char* format, ...)
{
	PARSE_VA_ARGS;
	DO_LOG(level);
}

void
owplLogDebug(const char* format, ...)
{
	PARSE_VA_ARGS;
	DO_LOG(OWPL_LOG_LEVEL_DEBUG);
}

void
owplLogInfo(const char* format, ...)
{
	PARSE_VA_ARGS;
	DO_LOG(OWPL_LOG_LEVEL_INFO);
}

void
owplLogWarn(const char* format, ...)
{
	PARSE_VA_ARGS;
	DO_LOG(OWPL_LOG_LEVEL_WARN);
}

void
owplLogError(const char* format, ...)
{
	PARSE_VA_ARGS;
	DO_LOG(OWPL_LOG_LEVEL_ERROR);
}
