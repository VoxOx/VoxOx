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

#include "phCallbacks.h"

#include <stdio.h>

phCallbacks_t callbacks = {
	callProgress,
	transferProgress,
	conferenceProgress,
	registerProgress,
	messageProgress,
	onNotify,
	subscriptionProgress
};


void callProgress(int callId, const phCallStateInfo_t * info) 
{
	g_callId = callId;
	if (!info) {
		return;
	}
	switch (info->event) {
		case phINCALL:
			printf(" * New call\n> ");			
			break;

		case phRINGING:
			printf(" * Call Ringing\n> ");
			break;

		case phCALLCLOSED:
			printf(" * No uri for call closed\n> ");
			break;

		case phCALLOK:
			printf(" * Call OK\n> ");
			break;
	}
}

void transferProgress(int callId, const phTransferStateInfo_t * transferInfo)
{
}

void conferenceProgress(int callId, const phConfStateInfo_t * confInfo) 
{
}

void registerProgress(int registerId, int status) 
{	
	if(registerId > 0 && status == 0){
		printf(" * Register OK\n> ");
	}else{
		printf(" * Can't register\n> ");
	}
}

void messageProgress(int messageId, const phMsgStateInfo_t * info) 
{
}

void subscriptionProgress(int subscriptionId, const phSubscriptionStateInfo_t *info) 
{
}

void onNotify(const char * event, const char * from, const char * content)
{
}