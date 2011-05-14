/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2005  Wengo
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

#ifndef _NETLIB_BUGFIX_H
#define _NETLIB_BUGFIX_H

typedef struct {
	BOOL(* IsResolvable) (LPSTR lpszHost);
	DWORD(* GetIPAddress) (LPSTR lpszIPAddress, LPDWORD lpdwIPAddressSize);
	BOOL(* ResolveHostName) (LPSTR lpszHostName, LPSTR lpszIPAddress, LPDWORD lpdwIPAddressSize);
	void(* IsInNet) (LPSTR lpszIPAddress, LPSTR lpszDest, LPSTR lpszMask);
} AutoProxyHelperVtbl;

typedef struct {
	AutoProxyHelperVtbl * lpVtbl;
} AutoProxyHelperFunctions;

typedef struct {
	DWORD dwStructSize;
	LPSTR lpszScriptBuffer;
	DWORD dwScriptBufferSize;
} AUTO_PROXY_SCRIPT_BUFFER;

typedef BOOL(CALLBACK * pfnInternetInitializeAutoProxyDll) (
	DWORD dwVersion,
	LPSTR lpszDownloadedTempFile,
	LPSTR lpszMime,
	AutoProxyHelperFunctions * lpAutoProxyCallbacks,
	AUTO_PROXY_SCRIPT_BUFFER * lpAutoProxyScriptBuffer);

typedef BOOL(CALLBACK * pfnInternetDeInitializeAutoProxyDll) (
	LPSTR lpszMime,
	DWORD dwReserved);

typedef BOOL(CALLBACK * pfnInternetGetProxyInfo) (
	LPCSTR lpszUrl,
	DWORD dwUrlLength,
	LPSTR lpszUrlHostName,
	DWORD dwUrlHostNameLength,
	LPSTR * lplpszProxyHostName,
	LPDWORD lpdwProxyHostNameLength);

#define PROXY_AUTO_DETECT_TYPE_DNS_A 2
#define PROXY_AUTO_DETECT_TYPE_DHCP 1

#endif
