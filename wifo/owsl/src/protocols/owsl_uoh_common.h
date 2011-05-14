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


#define ERR_SOCK(err)	(err == ENETDOWN ||\
						 err == EHOSTUNREACH ||\
						 err == ENETRESET ||\
						 err == ENOTCONN ||\
						 err == ESHUTDOWN ||\
						 err == ECONNABORTED ||\
						 err == ECONNRESET ||\
						 err == ETIMEDOUT)

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#define OWSL_UOH_BUFFER_SIZE	5000
#define OWSL_UOH_PACKET_SIZE	2000
#define OWSL_UOH_INT_SIZE		4

#define HTTP_REQUEST_STR \
	"GET / HTTP/1.1\r\n" \
	"UdpHost: %s:%d\r\n" \
	"Connection: Keep-Alive\r\n" \
	"Pragma: no-cache\r\n" \
	"Cache-Control: no-cache\r\n" \
	"Content-lenght: 0\r\n\r\n"

typedef enum
{
	UOH_ST_NOT_CONNECTED,
	UOH_ST_CONNECTING,
	UOH_ST_CONNECTED
}	OWSLConnectionState_UoH;


/* ******************************************************** */
/*                    Parameters structure                  */
/* ******************************************************** */

typedef enum
{
	USHORT_PARAM,
	INT_PARAM,
	STR_PARAM
}	OWSLParamType_UoH;

typedef struct OWSLParamKey_UoH
{
	const char *name;
	OWSLParamType_UoH type;
}	OWSLParamKey_UoH;

typedef union
{
	int		i;
	char	*str;
} OWSLDataType_UoH;

typedef struct OWSLParamNode_UoH
{
	const OWSLParamKey_UoH *key;
	OWSLDataType_UoH value;
}	OWSLParamNode_UoH;


/* ******************************************************** */
/*               Parameters management functions            */
/* ******************************************************** */
struct OWList;

const OWSLParamKey_UoH *_owsl_uoh_parameter_key_get(const OWSLParamKey_UoH *OWSLParamKeyList_UoH,
                                                    const char * name);
int _owsl_uoh_parameter_key_compare(OWSLParamNode_UoH *elm1, OWSLParamNode_UoH *elm2);
OWSLDataType_UoH *owsl_uoh_parameter_value_get(struct OWList * list, const char * name);
int owsl_uoh_common_parameter_set(const OWSLParamKey_UoH *OWSLParamKeyList_UoH,
                                  const char * name, const void * value, struct OWList * list);

