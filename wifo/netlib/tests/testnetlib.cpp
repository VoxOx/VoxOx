/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2007  Wengo
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
#include <stdio.h>
#include <netlib.h>
#include <owsl.h>

const char* SIP_PROXY_HOST = "213.91.9.206";
const int SIP_PROXY_PORT = 5060;
const int LOCAL_PORT = 5060;
const int PING_TIMEOUT = 3;

const char* stringFromRet(HttpRet ret) {
	static const char* strings[] = { "HTTP_OK", "HTTP_NOK", "HTTP_AUTH" };
	return strings[ret];
}

int main() {
	owsl_initialize();

	HttpRet ret;
	ret = is_http_conn_allowed("www.google.com:80", "", 0, "", "", 
		NETLIB_FALSE /* ssl */,
		10 /* timeout */);
	printf("is_http_conn_allowed NOSSL: %s\n", stringFromRet(ret));

	ret = is_http_conn_allowed("www.google.com:443", "", 0, "", "",
		NETLIB_TRUE /* ssl */,
		10 /* timeout */);
	printf("is_http_conn_allowed SSL: %s\n", stringFromRet(ret));

	NETLIB_BOOLEAN ok;
	ok = udp_sip_ping(SIP_PROXY_HOST, SIP_PROXY_PORT, LOCAL_PORT, PING_TIMEOUT, "nobody", SIP_PROXY_HOST);
	printf("udp_sip_ping: %s\n", ok == NETLIB_TRUE ? "ok" : "fail");
	return 0;
}
