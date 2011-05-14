/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2006  Wengo
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

#include "PurpleEnumIMProtocol.h"

#include <util/Logger.h>

#include <cstring>

#define PRPL_ALL	"prpl-all"
#define PRPL_MSN	"prpl-msn"
#define PRPL_MYSPACE "prpl-myspace"
#define PRPL_FACEBOOK "prpl-bigbrownchunx-facebookim"
#define PRPL_TWITTER "prpl-twitter"
#define PRPL_SKYPE  "prpl-bigbrownchunx-skype"
#define PRPL_YAHOO	"prpl-yahoo"
#define PRPL_AIM	"prpl-aim"
#define PRPL_ICQ	"prpl-icq"
#define PRPL_JABBER	"prpl-jabber"
#define PRPL_SIMPLE	"prpl-simple"
#define PRPL_UNK	"prpl-unknown"

const char * PurpleIMPrcl::GetPrclId(EnumIMProtocol::IMProtocol protocol) {

	//VOXOX - JRT - 2009.06.28 - TODO Move to QtEnumIMProtocolMap
	switch (protocol) {
	case EnumIMProtocol::IMProtocolAll:
		return PRPL_ALL;

	case EnumIMProtocol::IMProtocolMSN:
		return PRPL_MSN;

	case EnumIMProtocol::IMProtocolMYSPACE:
		return PRPL_MYSPACE;

	case EnumIMProtocol::IMProtocolFacebook:
		return PRPL_FACEBOOK;

	case EnumIMProtocol::IMProtocolTwitter:
		return PRPL_TWITTER;

	case EnumIMProtocol::IMProtocolSkype:
		return PRPL_SKYPE;

	case EnumIMProtocol::IMProtocolYahoo:
		return PRPL_YAHOO;

	case EnumIMProtocol::IMProtocolAIM:
		return PRPL_AIM;

	case EnumIMProtocol::IMProtocolICQ:
		return PRPL_ICQ;

	case EnumIMProtocol::IMProtocolJabber:
		return PRPL_JABBER;

	case EnumIMProtocol::IMProtocolSIPSIMPLE:
		return PRPL_SIMPLE;

	default:
		return PRPL_UNK;
	}
}

EnumIMProtocol::IMProtocol PurpleIMPrcl::GetEnumIMProtocol(const char * PurplePrclId) {
	if (!PurplePrclId || *PurplePrclId == '\0') {
		LOG_DEBUG("unknown purple protocol ID");
		return EnumIMProtocol::IMProtocolUnknown;
	}

	//VOXOX - JRT - 2009.06.28 - TODO: move to QtEnumIMProtocolMap
	if (!strcmp(PurplePrclId, PRPL_ALL)) {
		return EnumIMProtocol::IMProtocolAll;
	}

	else if (!strcmp(PurplePrclId, PRPL_MSN)) {
		return EnumIMProtocol::IMProtocolMSN;
	}
	
	else if (!strcmp(PurplePrclId, PRPL_MYSPACE)) {
		return EnumIMProtocol::IMProtocolMYSPACE;
	}

	else if (!strcmp(PurplePrclId, PRPL_FACEBOOK)) {
		return EnumIMProtocol::IMProtocolFacebook;
	}

	else if (!strcmp(PurplePrclId, PRPL_TWITTER)) {
		return EnumIMProtocol::IMProtocolTwitter;
	}

	else if (!strcmp(PurplePrclId, PRPL_SKYPE)) {
		return EnumIMProtocol::IMProtocolSkype;
	}

	else if (!strcmp(PurplePrclId, PRPL_YAHOO)) {
		return EnumIMProtocol::IMProtocolYahoo;
	}

	else if (!strcmp(PurplePrclId, PRPL_AIM)) {
		return EnumIMProtocol::IMProtocolAIM;
	}

	else if (!strcmp(PurplePrclId, PRPL_ICQ)) {
		return EnumIMProtocol::IMProtocolICQ;
	}

	else if (!strcmp(PurplePrclId, PRPL_JABBER)) {
		return EnumIMProtocol::IMProtocolJabber;
	}

	else if (!strcmp(PurplePrclId, PRPL_SIMPLE)) {
		return EnumIMProtocol::IMProtocolSIPSIMPLE;
	}

	else {
		return EnumIMProtocol::IMProtocolUnknown;
	}
}
