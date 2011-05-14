/*
 * PhApi, a voice over Internet library
 * Copyright (C) 2006  Wengo
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

#ifndef _OWPL_ADAPTER_INTERNAL_HEADER_
#define _OWPL_ADAPTER_INTERNAL_HEADER_

#include <owpl_adapter.h>

typedef struct OWPL_ADAPTER
{
	char * adapterName ;
	char * sipDomain ;
	char * sipProxy ;
	OWPL_TRANSPORT_PROTOCOL sipTransport ;
	OWPL_LINE_CONFIGURATION_HOOK lineConfigurationHook ;
	OWSIPAccountMessageFilter sipMessageFilter ;
} OWPL_ADAPTER ;

int
owplAdapterInitialize
(void) ;

int
owplAdapterTerminate
(void) ;

OWPL_ADAPTER *
owplAdapterGet
(
	const char * adapterName
) ;

#endif  /* _OWPL_ADAPTER_INTERNAL_HEADER_ */
