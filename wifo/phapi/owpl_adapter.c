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

#include "owpl_adapter_internal.h"

#include <owlist.h>
#include <string.h>

static OWList * owpl_adapter_list ;

static OWPL_ADAPTER *
owplAdapterNew
(
	const char * adapterName,
	const char * sipDomain,
	const char * sipProxy,
	OWPL_TRANSPORT_PROTOCOL sipTransport,
	OWPL_LINE_CONFIGURATION_HOOK lineConfigurationHook,
	OWSIPAccountMessageFilter sipMessageFilter
)
{
	OWPL_ADAPTER * adapter = malloc (sizeof (OWPL_ADAPTER)) ;
	if (adapter != NULL)
	{
		memset(adapter, 0, sizeof(OWPL_ADAPTER));
		if (adapterName && *adapterName)
		{
			adapter->adapterName = strdup (adapterName) ;
			if (adapter->adapterName == NULL)
			{
				free (adapter) ;
				return NULL ;
			}
		}

		if (sipDomain && *sipDomain)
		{
			adapter->sipDomain = strdup (sipDomain) ;
			if (adapter->sipDomain == NULL)
			{
				free (adapter->adapterName) ;
				free (adapter) ;
				return NULL ;
			}
		}

		if (sipProxy && *sipProxy)
		{
			adapter->sipProxy = strdup (sipProxy) ;
			if (adapter->sipProxy == NULL)
			{
				free (adapter->sipDomain) ;
				free (adapter->adapterName) ;
				free (adapter) ;
				return NULL ;
			}
		}
		adapter->sipTransport = sipTransport ;
		adapter->lineConfigurationHook = lineConfigurationHook ;
		adapter->sipMessageFilter = sipMessageFilter ;
	}
	return adapter ;
}

static void
owplAdapterFree
(
	void * _adapter
)
{
	OWPL_ADAPTER * adapter = _adapter ;
	free (adapter->sipProxy) ;
	free (adapter->sipDomain) ;
	free (adapter->adapterName) ;
	free (adapter) ;
	return ;
}

static int
owplAdapterCompare
(
	const void * void1,
	const void * void2
)
{
	const OWPL_ADAPTER * adapter1 = void1 ;
	const OWPL_ADAPTER * adapter2 = void2 ;
	return strcmp (adapter1->adapterName, adapter2->adapterName) ;
}

int
owplAdapterInitialize
(void)
{
	owpl_adapter_list = owlist_new () ;
	if (owpl_adapter_list == NULL)
	{
		return -1 ;
	}
	return 0 ;
}

int
owplAdapterTerminate
(void)
{
	int return_code ;

	return_code |= owlist_free_all (owpl_adapter_list, owplAdapterFree) ;
	owpl_adapter_list = NULL ;

	return return_code ;
}

OWPL_RESULT
owplAdapterRegister
(
	const char * adapterName,
	const char * sipDomain,
	const char * sipProxy,
	OWPL_TRANSPORT_PROTOCOL sipTransport,
	OWPL_LINE_CONFIGURATION_HOOK lineConfigurationHook,
	OWSIPAccountMessageFilter sipMessageFilter
)
{
	int return_code ;
	OWPL_ADAPTER * adapter ;

	adapter = owplAdapterNew
	(
		adapterName,
		sipDomain,
		sipProxy,
		sipTransport,
		lineConfigurationHook,
		sipMessageFilter
	) ;
	if (adapter == NULL)
	{
		return OWPL_RESULT_OUT_OF_MEMORY ;
	}

	return_code = owlist_add (owpl_adapter_list, adapter, owplAdapterCompare) ;
	if (return_code != 0)
	{
		return OWPL_RESULT_FAILURE ;
	}

	return OWPL_RESULT_SUCCESS ;
}

OWPL_ADAPTER *
owplAdapterGet
(
	const char * adapterName
)
{
	int return_code ;
	OWPL_ADAPTER * adapter = NULL ;
	OWListIterator * iterator ;
	int found = 0 ;

	if (adapterName == NULL)
	{
		return NULL ;
	}

	iterator = owlist_iterator_new (owpl_adapter_list, OWLIST_READ) ;
	if (iterator == NULL)
	{
		return NULL ;
	}
	while (owlist_iterator_next (iterator) == 0)
	{
		adapter = owlist_iterator_get (iterator) ;
		return_code = strcmp (adapter->adapterName, adapterName) ;
		if (return_code == 0)
		{
			found = 1 ;
			break ;
		}
	}
	return_code = owlist_iterator_free (iterator) ;
	if (return_code != 0)
	{
		return NULL ;
	}

	if (! found)
	{
		return NULL ;
	}
	return adapter ;
}
