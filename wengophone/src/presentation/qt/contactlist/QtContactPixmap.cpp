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

#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtContactPixmap.h"
#include <util/Logger.h>

QtContactPixmap * QtContactPixmap::_instance = NULL;

QtContactPixmap::QtContactPixmap() {
}

QtContactPixmap * QtContactPixmap::getInstance() 
{
	if (_instance == 0) 
	{
		_instance = new QtContactPixmap;
	}

	return _instance;
}

//VOXOX CHANGE by Rolando - 2009.10.23 
void QtContactPixmap::setPixmap(ContactPixmap status, const QPixmap & px, QString pixmapPath) 
{
	_contactPixmap[status].pixmap = px;//VOXOX CHANGE by Rolando - 2009.10.23 
	_contactPixmap[status].pixmapPath = pixmapPath;//VOXOX CHANGE by Rolando - 2009.10.23 
}

const QPixmap & QtContactPixmap::getPixmap(ContactPixmap status)
{
	//VOXOX CHANGE by Rolando - 2009.10.23 
	if(_contactPixmap.contains(status)){

		return _contactPixmap[status].pixmap;//VOXOX CHANGE by Rolando - 2009.10.23 

	}
	else{

		return _contactPixmap[ContactUnknown].pixmap;//VOXOX CHANGE by Rolando - 2009.10.26 

	}
}

//VOXOX CHANGE by Rolando - 2009.10.23 
QString QtContactPixmap::getPixmapPath(ContactPixmap status) 
{
	//VOXOX CHANGE by Rolando - 2009.10.23 
	if(_contactPixmap.contains(status)){
		return _contactPixmap[status].pixmapPath;//VOXOX CHANGE by Rolando - 2009.10.23 
	}
	else{
		return "";//VOXOX CHANGE by Rolando - 2009.10.23 
	}
}

//VOXOX CHANGE by Rolando - 2009.10.22 - added parameter isHover
//VOXOX - JRT - 2009.04.14 
//static
QtContactPixmap::ContactPixmap QtContactPixmap::determinePixmap( QtEnumIMProtocol::IMProtocol eProtocol, EnumPresenceState::PresenceState ePresenceState,
																	 bool hasAvailableSIPNumber, bool isBlocked, bool isHover)
{
	QtContactPixmap::ContactPixmap status = QtContactPixmap::ContactUnknown;

	//VOXOX - CJC - 2009.05.05 Add blocked image
	if(isBlocked){
		return status = getBlockedPix(eProtocol, isHover);//VOXOX CHANGE by Rolando - 2009.10.22 
	}

	switch ( ePresenceState ) 
	{
	case EnumPresenceState::PresenceStateUnavailable:
		LOG_INFO("Received PresenceStateUnavailable state");
		//Fall through intended.

	case EnumPresenceState::PresenceStateUnknown:
		status = getUnknownPix( eProtocol, hasAvailableSIPNumber );
		break;

	case EnumPresenceState::PresenceStateOnline: 
		status = getOnlinePix( eProtocol, isHover);
		break;

	case EnumPresenceState::PresenceStateDoNotDisturb:
		status = getDndPix( eProtocol, isHover);
		break;

	case EnumPresenceState::PresenceStateAway:
		status = getAwayPix( eProtocol, isHover);
		break;

	case EnumPresenceState::PresenceStateInvisible: 
		status = getInvisiblePix( eProtocol, isHover);
		break;

	case EnumPresenceState::PresenceStateOffline:
		/*if(hasAvailableSIPNumber){
			status = getMobilePix( eProtocol );
		}else{*/
			status = getOfflinePix( eProtocol, isHover);
		//}
		break;

	default:
		status = QtContactPixmap::ContactUnknown;
		LOG_FATAL("unknown state=" + String::fromNumber(ePresenceState) );
		break;
	}

	return status;
}

//VOXOX CHANGE by Rolando - 2009.10.22 
QtContactPixmap::ContactPixmap QtContactPixmap::determineHoverPixmap( QtEnumIMProtocol::IMProtocol eProtocol, EnumPresenceState::PresenceState ePresenceState,
																	 bool hasAvailableSIPNumber, bool isBlocked)
{
	QtContactPixmap::ContactPixmap status = QtContactPixmap::ContactUnknown;
	bool isHover = true;//VOXOX CHANGE by Rolando - 2009.10.22 
	//VOXOX - CJC - 2009.05.05 Add blocked image
	if(isBlocked){
		return status = getBlockedPix(eProtocol, isHover);//VOXOX CHANGE by Rolando - 2009.10.22 
	}

	switch ( ePresenceState ) 
	{
	case EnumPresenceState::PresenceStateUnavailable:
		LOG_INFO("Received PresenceStateUnavailable state");
		//Fall through intended.

	case EnumPresenceState::PresenceStateUnknown:
		status = getUnknownPix( eProtocol, hasAvailableSIPNumber );
		break;

	case EnumPresenceState::PresenceStateOnline: 
		status = getOnlinePix( eProtocol, isHover);
		break;

	case EnumPresenceState::PresenceStateDoNotDisturb:
		status = getDndPix( eProtocol, isHover);
		break;

	case EnumPresenceState::PresenceStateAway:
		status = getAwayPix( eProtocol, isHover);
		break;

	case EnumPresenceState::PresenceStateInvisible: 
		status = getInvisiblePix( eProtocol, isHover);
		break;

	case EnumPresenceState::PresenceStateOffline:
		/*if(hasAvailableSIPNumber){
			status = getMobilePix( eProtocol );
		}else{*/
			status = getOfflinePix( eProtocol, isHover);
		//}
		break;

	default:
		status = QtContactPixmap::ContactUnknown;
		LOG_FATAL("unknown state=" + String::fromNumber(ePresenceState) );
		break;
	}

	return status;
}

//-----------------------------------------------------------------------------

QtContactPixmap::ContactPixmap QtContactPixmap::getUnknownPix( QtEnumIMProtocol::IMProtocol eProtocol,
															   bool hasAvailableSIPNumber)
{
	QtContactPixmap::ContactPixmap status = QtContactPixmap::ContactUnknown;
	//VOXOX CHANGE CJC THIS NORMALLY WILL HAPPEN FOR ONLY VOXOX CONTATS< AND WE NEED TO SHOW THEM AS OFFLINE, SO THIS IS NOT NEEDED
	//if ( hasAvailableSIPNumber ) 
	//{
	//	// SIP
	//	status = QtContactPixmap::ContactNoStatusSIP;
	//} 
	//else 
	if ( hasAvailableSIPNumber ) 
	{
		//VOXOX - CJC - 2009.06.18 
		//status =getMobilePix(eProtocol);
			//QtContactPixmap::ContactNoStatus;
	} 
	else 
	{
		//VOXOX CHANGE CJC SHOULD RETURN OFFLINE PIXMAP FOR ALL PROTOCOLS
		status = getOfflinePix(eProtocol, false);
		/*if ( eProtocol == QtEnumIMProtocol::IMProtocolFacebook)
		{
			status = QtContactPixmap::ContactFacebookOffline;
		}
		else
		{
			status = QtContactPixmap::ContactUnknown;
		}*/
	}

	return status;
}

//-----------------------------------------------------------------------------
//VOXOX CHANGE by Rolando - 2009.10.22 - added parameter isHover
QtContactPixmap::ContactPixmap QtContactPixmap::getOfflinePix( QtEnumIMProtocol::IMProtocol eProtocol, bool isHover )
{
	QtContactPixmap::ContactPixmap status = QtContactPixmap::ContactUnknown;

	if(isHover){
		switch (eProtocol)
		{
		case QtEnumIMProtocol::IMProtocolMSN:
			status = QtContactPixmap::ContactMSNOfflineHover;
			break;

		case QtEnumIMProtocol::IMProtocolMYSPACE:
			status = QtContactPixmap::ContactMYSPACEOfflineHover;
			break;

		case QtEnumIMProtocol::IMProtocolFacebook:
			status = QtContactPixmap::ContactFacebookOfflineHover;
			break;

		case QtEnumIMProtocol::IMProtocolTwitter:
			status = QtContactPixmap::ContactTwitterOfflineHover;
			break;

		case QtEnumIMProtocol::IMProtocolSkype:
			status = QtContactPixmap::ContactSkypeOfflineHover;
			break;

		case QtEnumIMProtocol::IMProtocolYahoo:
			status = QtContactPixmap::ContactYahooOfflineHover;
			break;

		case QtEnumIMProtocol::IMProtocolAIM:
			status = QtContactPixmap::ContactAIMOfflineHover;
			break;

		case QtEnumIMProtocol::IMProtocolICQ:
			status = QtContactPixmap::ContactICQOfflineHover;
			break;

		case QtEnumIMProtocol::IMProtocolJabber:
			status = QtContactPixmap::ContactJabberOfflineHover;
			break;

		case QtEnumIMProtocol::IMProtocolVoxOx:
			status = QtContactPixmap::ContactVoxOxOfflineHover;
			break;

		case QtEnumIMProtocol::IMProtocolGoogleTalk:
			status = QtContactPixmap::ContactGTalkOfflineHover;
			break;

		case QtEnumIMProtocol::IMProtocolUnknown:
			status = QtContactPixmap::ContactUnknown;
			break;

		default:
			LOG_FATAL("unknown IM protocol=" + String::fromNumber(eProtocol));
		}
	}
	else{
		switch (eProtocol)
		{
		case QtEnumIMProtocol::IMProtocolMSN:
			status = QtContactPixmap::ContactMSNOffline;
			break;

		case QtEnumIMProtocol::IMProtocolMYSPACE:
			status = QtContactPixmap::ContactMYSPACEOffline;
			break;

		case QtEnumIMProtocol::IMProtocolFacebook:
			status = QtContactPixmap::ContactFacebookOffline;
			break;

		case QtEnumIMProtocol::IMProtocolTwitter:
			status = QtContactPixmap::ContactTwitterOffline;
			break;

		case QtEnumIMProtocol::IMProtocolSkype:
			status = QtContactPixmap::ContactSkypeOffline;
			break;

		case QtEnumIMProtocol::IMProtocolYahoo:
			status = QtContactPixmap::ContactYahooOffline;
			break;

		case QtEnumIMProtocol::IMProtocolAIM:
			status = QtContactPixmap::ContactAIMOffline;
			break;

		case QtEnumIMProtocol::IMProtocolICQ:
			status = QtContactPixmap::ContactICQOffline;
			break;

		case QtEnumIMProtocol::IMProtocolJabber:
			status = QtContactPixmap::ContactJabberOffline;
			break;

		case QtEnumIMProtocol::IMProtocolVoxOx:
			status = QtContactPixmap::ContactVoxOxOffline;
			break;

		case QtEnumIMProtocol::IMProtocolGoogleTalk:
			status = QtContactPixmap::ContactGTalkOffline;
			break;

		case QtEnumIMProtocol::IMProtocolUnknown:
			status = QtContactPixmap::ContactUnknown;
			break;

		default:
			LOG_FATAL("unknown IM protocol=" + String::fromNumber(eProtocol));
		}	
	}
	return status;
}

//VOXOX CHANGE by Rolando - 2009.10.22 - added parameter isHover
//VOXOX - CJC - 2009.05.05 Get Blocked Pix
QtContactPixmap::ContactPixmap QtContactPixmap::getBlockedPix( QtEnumIMProtocol::IMProtocol eProtocol, bool isHover)
{
	QtContactPixmap::ContactPixmap status = QtContactPixmap::ContactUnknown;

	if(isHover){
		switch (eProtocol)
		{
		case QtEnumIMProtocol::IMProtocolMSN:
			status = QtContactPixmap::ContactMSNBlockHover;
			break;

		case QtEnumIMProtocol::IMProtocolMYSPACE:
			status = QtContactPixmap::ContactMYSPACEBlockHover;
			break;

		case QtEnumIMProtocol::IMProtocolFacebook:
			status = QtContactPixmap::ContactFacebookBlockHover;
			break;

		case QtEnumIMProtocol::IMProtocolTwitter:
			status = QtContactPixmap::ContactTwitterBlockHover;
			break;

		case QtEnumIMProtocol::IMProtocolSkype:
			status = QtContactPixmap::ContactSkypeBlockHover;
			break;

		case QtEnumIMProtocol::IMProtocolYahoo:
			status = QtContactPixmap::ContactYahooBlockHover;
			break;

		case QtEnumIMProtocol::IMProtocolAIM:
			status = QtContactPixmap::ContactAIMBlockHover;
			break;

		case QtEnumIMProtocol::IMProtocolICQ:
			status = QtContactPixmap::ContactICQBlockHover;
			break;

		case QtEnumIMProtocol::IMProtocolJabber:
			status = QtContactPixmap::ContactJabberBlockHover;
			break;

		case QtEnumIMProtocol::IMProtocolVoxOx:
			status = QtContactPixmap::ContactVoxOxBlockHover;
			break;

		case QtEnumIMProtocol::IMProtocolGoogleTalk:
			status = QtContactPixmap::ContactGTalkBlockHover;
			break;

		default:
			LOG_FATAL("unknown IM protocol=" + String::fromNumber(eProtocol));
		}
	}
	else{
		switch (eProtocol)
		{
		case QtEnumIMProtocol::IMProtocolMSN:
			status = QtContactPixmap::ContactMSNBlock;
			break;

		case QtEnumIMProtocol::IMProtocolMYSPACE:
			status = QtContactPixmap::ContactMYSPACEBlock;
			break;

		case QtEnumIMProtocol::IMProtocolFacebook:
			status = QtContactPixmap::ContactFacebookBlock;
			break;

		case QtEnumIMProtocol::IMProtocolTwitter:
			status = QtContactPixmap::ContactTwitterBlock;
			break;

		case QtEnumIMProtocol::IMProtocolSkype:
			status = QtContactPixmap::ContactSkypeBlock;
			break;

		case QtEnumIMProtocol::IMProtocolYahoo:
			status = QtContactPixmap::ContactYahooBlock;
			break;

		case QtEnumIMProtocol::IMProtocolAIM:
			status = QtContactPixmap::ContactAIMBlock;
			break;

		case QtEnumIMProtocol::IMProtocolICQ:
			status = QtContactPixmap::ContactICQBlock;
			break;

		case QtEnumIMProtocol::IMProtocolJabber:
			status = QtContactPixmap::ContactJabberBlock;
			break;

		case QtEnumIMProtocol::IMProtocolVoxOx:
			status = QtContactPixmap::ContactVoxOxBlock;
			break;

		case QtEnumIMProtocol::IMProtocolGoogleTalk:
			status = QtContactPixmap::ContactGTalkBlock;
			break;

		default:
			LOG_FATAL("unknown IM protocol=" + String::fromNumber(eProtocol));
		}
	}

	return status;
}

//-----------------------------------------------------------------------------
//VOXOX CHANGE by Rolando - 2009.10.22 - added parameter isHover
QtContactPixmap::ContactPixmap QtContactPixmap::getOnlinePix( QtEnumIMProtocol::IMProtocol eProtocol, bool isHover)
{
	QtContactPixmap::ContactPixmap status = QtContactPixmap::ContactUnknown;

	if(isHover){
		switch (eProtocol) 
		{
		case QtEnumIMProtocol::IMProtocolMSN:
			status = QtContactPixmap::ContactMSNOnlineHover;
			break;

		case QtEnumIMProtocol::IMProtocolMYSPACE:
			status = QtContactPixmap::ContactMYSPACEOnlineHover;
			break;

		case QtEnumIMProtocol::IMProtocolFacebook:
			status = QtContactPixmap::ContactFacebookOnlineHover;
			break;

		case QtEnumIMProtocol::IMProtocolTwitter:
			//status = QtContactPixmap::ContactTwitterOnlineHover;//VOXOX CHANGE by Rolando - 2009.08.04 
			status = QtContactPixmap::ContactTwitterOfflineHover;//VOXOX CHANGE by Rolando - 2009.08.04 
			break;

		case QtEnumIMProtocol::IMProtocolSkype:
			status = QtContactPixmap::ContactSkypeOnlineHover;
			break;

		case QtEnumIMProtocol::IMProtocolYahoo:
			status = QtContactPixmap::ContactYahooOnlineHover;
			break;

		case QtEnumIMProtocol::IMProtocolAIM:
			status = QtContactPixmap::ContactAIMOnlineHover;
			break;

		case QtEnumIMProtocol::IMProtocolICQ:
			status = QtContactPixmap::ContactICQOnlineHover;
			break;

		case QtEnumIMProtocol::IMProtocolJabber:
			status = QtContactPixmap::ContactJabberOnlineHover;
			break;

		case QtEnumIMProtocol::IMProtocolVoxOx:
			status = QtContactPixmap::ContactVoxOxOnlineHover;
			break;

		case QtEnumIMProtocol::IMProtocolGoogleTalk:
			status = QtContactPixmap::ContactGTalkOnlineHover;
			break;

		default:
			LOG_FATAL("unknown IM protocol=" + String::fromNumber(eProtocol));
		}
	}
	else{
		switch (eProtocol) 
		{
		case QtEnumIMProtocol::IMProtocolMSN:
			status = QtContactPixmap::ContactMSNOnline;
			break;

		case QtEnumIMProtocol::IMProtocolMYSPACE:
			status = QtContactPixmap::ContactMYSPACEOnline;
			break;

		case QtEnumIMProtocol::IMProtocolFacebook:
			status = QtContactPixmap::ContactFacebookOnline;
			break;

		case QtEnumIMProtocol::IMProtocolTwitter:
			//status = QtContactPixmap::ContactTwitterOnline;//VOXOX CHANGE by Rolando - 2009.08.04 
			status = QtContactPixmap::ContactTwitterOffline;//VOXOX CHANGE by Rolando - 2009.08.04 
			break;

		case QtEnumIMProtocol::IMProtocolSkype:
			status = QtContactPixmap::ContactSkypeOnline;
			break;

		case QtEnumIMProtocol::IMProtocolYahoo:
			status = QtContactPixmap::ContactYahooOnline;
			break;

		case QtEnumIMProtocol::IMProtocolAIM:
			status = QtContactPixmap::ContactAIMOnline;
			break;

		case QtEnumIMProtocol::IMProtocolICQ:
			status = QtContactPixmap::ContactICQOnline;
			break;

		case QtEnumIMProtocol::IMProtocolJabber:
			status = QtContactPixmap::ContactJabberOnline;
			break;

		case QtEnumIMProtocol::IMProtocolVoxOx:
			status = QtContactPixmap::ContactVoxOxOnline;
			break;

		case QtEnumIMProtocol::IMProtocolGoogleTalk:
			status = QtContactPixmap::ContactGTalkOnline;
			break;

		default:
			LOG_FATAL("unknown IM protocol=" + String::fromNumber(eProtocol));
		}
	
	}

	return status;
}

//-----------------------------------------------------------------------------
//VOXOX CHANGE by Rolando - 2009.10.22 - added parameter isHover
QtContactPixmap::ContactPixmap QtContactPixmap::getAwayPix( QtEnumIMProtocol::IMProtocol eProtocol, bool isHover)
{
	QtContactPixmap::ContactPixmap status = QtContactPixmap::ContactUnknown;

	if(isHover){
		switch (eProtocol) 
		{
		case QtEnumIMProtocol::IMProtocolMSN: 
			status = QtContactPixmap::ContactMSNAwayHover;
			break;

		case QtEnumIMProtocol::IMProtocolMYSPACE:
			status = QtContactPixmap::ContactMYSPACEAwayHover;
			break;
		
		case QtEnumIMProtocol::IMProtocolFacebook:
			status = QtContactPixmap::ContactFacebookAwayHover;
			break;

		case QtEnumIMProtocol::IMProtocolTwitter:
			status = QtContactPixmap::ContactTwitterAwayHover;
			break;

		case QtEnumIMProtocol::IMProtocolSkype:
			status = QtContactPixmap::ContactSkypeAwayHover;
			break;

		case QtEnumIMProtocol::IMProtocolYahoo:
			status = QtContactPixmap::ContactYahooAwayHover;
			break;

		case QtEnumIMProtocol::IMProtocolAIM: 
			status = QtContactPixmap::ContactAIMAwayHover;
			break;

		case QtEnumIMProtocol::IMProtocolICQ:
			status = QtContactPixmap::ContactICQAwayHover;
			break;

		case QtEnumIMProtocol::IMProtocolJabber: 
			status = QtContactPixmap::ContactJabberAwayHover;
			break;

		case QtEnumIMProtocol::IMProtocolVoxOx: 
			status = QtContactPixmap::ContactVoxOxAwayHover;
			break;

		case QtEnumIMProtocol::IMProtocolGoogleTalk: 
			status = QtContactPixmap::ContactGTalkAwayHover;
			break;

		default:
			LOG_FATAL("unknown IM protocol=" + String::fromNumber(eProtocol));
		}
	}
	else{
		switch (eProtocol) 
		{
		case QtEnumIMProtocol::IMProtocolMSN: 
			status = QtContactPixmap::ContactMSNAway;
			break;

		case QtEnumIMProtocol::IMProtocolMYSPACE:
			status = QtContactPixmap::ContactMYSPACEAway;
			break;
		
		case QtEnumIMProtocol::IMProtocolFacebook:
			status = QtContactPixmap::ContactFacebookAway;
			break;

		case QtEnumIMProtocol::IMProtocolTwitter:
			status = QtContactPixmap::ContactTwitterAway;
			break;

		case QtEnumIMProtocol::IMProtocolSkype:
			status = QtContactPixmap::ContactSkypeAway;
			break;

		case QtEnumIMProtocol::IMProtocolYahoo:
			status = QtContactPixmap::ContactYahooAway;
			break;

		case QtEnumIMProtocol::IMProtocolAIM: 
			status = QtContactPixmap::ContactAIMAway;
			break;

		case QtEnumIMProtocol::IMProtocolICQ:
			status = QtContactPixmap::ContactICQAway;
			break;

		case QtEnumIMProtocol::IMProtocolJabber: 
			status = QtContactPixmap::ContactJabberAway;
			break;

		case QtEnumIMProtocol::IMProtocolVoxOx: 
			status = QtContactPixmap::ContactVoxOxAway;
			break;

		case QtEnumIMProtocol::IMProtocolGoogleTalk: 
			status = QtContactPixmap::ContactGTalkAway;
			break;

		default:
			LOG_FATAL("unknown IM protocol=" + String::fromNumber(eProtocol));
		}
		
	}

	return status;
}

//-----------------------------------------------------------------------------
//VOXOX CHANGE by Rolando - 2009.10.22 - added parameter isHover
QtContactPixmap::ContactPixmap QtContactPixmap::getInvisiblePix( QtEnumIMProtocol::IMProtocol eProtocol, bool isHover)
{
	QtContactPixmap::ContactPixmap status = QtContactPixmap::ContactUnknown;

	if(isHover){
		switch (eProtocol) 
		{
		case QtEnumIMProtocol::IMProtocolMSN:
			status = QtContactPixmap::ContactMSNInvisibleHover;
			break;

		case QtEnumIMProtocol::IMProtocolMYSPACE:
			status = QtContactPixmap::ContactMYSPACEInvisibleHover;
			break;

		case QtEnumIMProtocol::IMProtocolFacebook:
			status = QtContactPixmap::ContactFacebookInvisibleHover;
			break;

		case QtEnumIMProtocol::IMProtocolTwitter:
			status = QtContactPixmap::ContactTwitterInvisibleHover;
			break;

		case QtEnumIMProtocol::IMProtocolSkype:
			status = QtContactPixmap::ContactSkypeInvisibleHover;
			break;

		case QtEnumIMProtocol::IMProtocolYahoo:
			status = QtContactPixmap::ContactYahooInvisibleHover;
			break;

		case QtEnumIMProtocol::IMProtocolAIM:
			status = QtContactPixmap::ContactAIMInvisibleHover;
			break;

		case QtEnumIMProtocol::IMProtocolICQ:
			status = QtContactPixmap::ContactICQInvisibleHover;
			break;

		case QtEnumIMProtocol::IMProtocolJabber:
			status = QtContactPixmap::ContactJabberInvisibleHover;
			break;

		case QtEnumIMProtocol::IMProtocolVoxOx:
			status = QtContactPixmap::ContactVoxOxInvisibleHover;
			break;

		case QtEnumIMProtocol::IMProtocolGoogleTalk:
			status = QtContactPixmap::ContactGTalkInvisibleHover;
			break;

		default:
			LOG_FATAL("unknown IM protocol=" + String::fromNumber(eProtocol));
		}
	}
	else{
		switch (eProtocol) 
		{
		case QtEnumIMProtocol::IMProtocolMSN:
			status = QtContactPixmap::ContactMSNInvisible;
			break;

		case QtEnumIMProtocol::IMProtocolMYSPACE:
			status = QtContactPixmap::ContactMYSPACEInvisible;
			break;

		case QtEnumIMProtocol::IMProtocolFacebook:
			status = QtContactPixmap::ContactFacebookInvisible;
			break;

		case QtEnumIMProtocol::IMProtocolTwitter:
			status = QtContactPixmap::ContactTwitterInvisible;
			break;

		case QtEnumIMProtocol::IMProtocolSkype:
			status = QtContactPixmap::ContactSkypeInvisible;
			break;

		case QtEnumIMProtocol::IMProtocolYahoo:
			status = QtContactPixmap::ContactYahooInvisible;
			break;

		case QtEnumIMProtocol::IMProtocolAIM:
			status = QtContactPixmap::ContactAIMInvisible;
			break;

		case QtEnumIMProtocol::IMProtocolICQ:
			status = QtContactPixmap::ContactICQInvisible;
			break;

		case QtEnumIMProtocol::IMProtocolJabber:
			status = QtContactPixmap::ContactJabberInvisible;
			break;

		case QtEnumIMProtocol::IMProtocolVoxOx:
			status = QtContactPixmap::ContactVoxOxInvisible;
			break;

		case QtEnumIMProtocol::IMProtocolGoogleTalk:
			status = QtContactPixmap::ContactGTalkInvisible;
			break;

		default:
			LOG_FATAL("unknown IM protocol=" + String::fromNumber(eProtocol));
		}
		
	}

	return status;
}

//-----------------------------------------------------------------------------
//VOXOX CHANGE by Rolando - 2009.10.22 - added parameter isHover
QtContactPixmap::ContactPixmap QtContactPixmap::getDndPix( QtEnumIMProtocol::IMProtocol eProtocol, bool isHover )
{
	QtContactPixmap::ContactPixmap status = QtContactPixmap::ContactUnknown;

	if(isHover){//VOXOX CHANGE by Rolando - 2009.10.22 
		switch (eProtocol) 
		{
		case QtEnumIMProtocol::IMProtocolMSN:
			status = QtContactPixmap::ContactMSNDNDHover;
			break;

		case QtEnumIMProtocol::IMProtocolMYSPACE:
			status = QtContactPixmap::ContactMYSPACEDNDHover;
			break;

		case QtEnumIMProtocol::IMProtocolFacebook:
			status = QtContactPixmap::ContactFacebookDNDHover;
			break;

		case QtEnumIMProtocol::IMProtocolTwitter:
			status = QtContactPixmap::ContactTwitterDNDHover;
			break;

		case QtEnumIMProtocol::IMProtocolSkype:
			status = QtContactPixmap::ContactSkypeDNDHover;
			break;

		case QtEnumIMProtocol::IMProtocolYahoo:
			status = QtContactPixmap::ContactYahooDNDHover;
			break;

		case QtEnumIMProtocol::IMProtocolAIM:
			status = QtContactPixmap::ContactAIMDNDHover;
			break;

		case QtEnumIMProtocol::IMProtocolICQ:
			status = QtContactPixmap::ContactICQDNDHover;
			break;

		case QtEnumIMProtocol::IMProtocolJabber:
			status = QtContactPixmap::ContactJabberDNDHover;
			break;

		case QtEnumIMProtocol::IMProtocolVoxOx:
			status = QtContactPixmap::ContactVoxOxDNDHover;
			break;

		case QtEnumIMProtocol::IMProtocolGoogleTalk:
			status = QtContactPixmap::ContactGTalkDNDHover;
			break;

		default:
			LOG_FATAL("unknown IM protocol=" + String::fromNumber(eProtocol));
		}
	}
	else{
		switch (eProtocol) 
		{
		case QtEnumIMProtocol::IMProtocolMSN:
			status = QtContactPixmap::ContactMSNDND;
			break;

		case QtEnumIMProtocol::IMProtocolMYSPACE:
			status = QtContactPixmap::ContactMYSPACEDND;
			break;

		case QtEnumIMProtocol::IMProtocolFacebook:
			status = QtContactPixmap::ContactFacebookDND;
			break;

		case QtEnumIMProtocol::IMProtocolTwitter:
			status = QtContactPixmap::ContactTwitterDND;
			break;

		case QtEnumIMProtocol::IMProtocolSkype:
			status = QtContactPixmap::ContactSkypeDND;
			break;

		case QtEnumIMProtocol::IMProtocolYahoo:
			status = QtContactPixmap::ContactYahooDND;
			break;

		case QtEnumIMProtocol::IMProtocolAIM:
			status = QtContactPixmap::ContactAIMDND;
			break;

		case QtEnumIMProtocol::IMProtocolICQ:
			status = QtContactPixmap::ContactICQDND;
			break;

		case QtEnumIMProtocol::IMProtocolJabber:
			status = QtContactPixmap::ContactJabberDND;
			break;

		case QtEnumIMProtocol::IMProtocolVoxOx:
			status = QtContactPixmap::ContactVoxOxDND;
			break;

		case QtEnumIMProtocol::IMProtocolGoogleTalk:
			status = QtContactPixmap::ContactGTalkDND;
			break;

		default:
			LOG_FATAL("unknown IM protocol=" + String::fromNumber(eProtocol));
		}
	}

	return status;
}

//VOXOX CHANGE by Rolando - 2009.10.22 - added parameter isHover
//VOXOX - CJC - 2009.06.18 
QtContactPixmap::ContactPixmap QtContactPixmap::getMobilePix( QtEnumIMProtocol::IMProtocol eProtocol, bool isHover)
{
	QtContactPixmap::ContactPixmap status = QtContactPixmap::ContactUnknown;

	if(isHover){//VOXOX CHANGE by Rolando - 2009.10.22 
		switch (eProtocol) 
		{
		case QtEnumIMProtocol::IMProtocolMSN:
			status = QtContactPixmap::ContactMSNMobileHover;
			break;

		case QtEnumIMProtocol::IMProtocolMYSPACE:
			status = QtContactPixmap::ContactMYSPACEMobileHover;
			break;

		case QtEnumIMProtocol::IMProtocolFacebook:
			status = QtContactPixmap::ContactFacebookMobileHover;
			break;

		case QtEnumIMProtocol::IMProtocolTwitter:
			status = QtContactPixmap::ContactTwitterMobileHover;
			break;

		case QtEnumIMProtocol::IMProtocolSkype:
			status = QtContactPixmap::ContactSkypeMobileHover;
			break;

		case QtEnumIMProtocol::IMProtocolYahoo:
			status = QtContactPixmap::ContactYahooMobileHover;
			break;

		case QtEnumIMProtocol::IMProtocolAIM:
			status = QtContactPixmap::ContactAIMMobileHover;
			break;

		case QtEnumIMProtocol::IMProtocolICQ:
			status = QtContactPixmap::ContactICQMobileHover;
			break;

		case QtEnumIMProtocol::IMProtocolJabber:
			status = QtContactPixmap::ContactJabberMobileHover;
			break;

		case QtEnumIMProtocol::IMProtocolVoxOx:
			status = QtContactPixmap::ContactVoxOxMobileHover;
			break;

		case QtEnumIMProtocol::IMProtocolGoogleTalk:
			status = QtContactPixmap::ContactGTalkMobileHover;
			break;

		default:
			LOG_FATAL("unknown IM protocol=" + String::fromNumber(eProtocol));
		}
	}
	else{//VOXOX CHANGE by Rolando - 2009.10.22 - if it is not hovering
		switch (eProtocol) 
		{
		case QtEnumIMProtocol::IMProtocolMSN:
			status = QtContactPixmap::ContactMSNMobile;
			break;

		case QtEnumIMProtocol::IMProtocolMYSPACE:
			status = QtContactPixmap::ContactMYSPACEMobile;
			break;

		case QtEnumIMProtocol::IMProtocolFacebook:
			status = QtContactPixmap::ContactFacebookMobile;
			break;

		case QtEnumIMProtocol::IMProtocolTwitter:
			status = QtContactPixmap::ContactTwitterMobile;
			break;

		case QtEnumIMProtocol::IMProtocolSkype:
			status = QtContactPixmap::ContactSkypeMobile;
			break;

		case QtEnumIMProtocol::IMProtocolYahoo:
			status = QtContactPixmap::ContactYahooMobile;
			break;

		case QtEnumIMProtocol::IMProtocolAIM:
			status = QtContactPixmap::ContactAIMMobile;
			break;

		case QtEnumIMProtocol::IMProtocolICQ:
			status = QtContactPixmap::ContactICQMobile;
			break;

		case QtEnumIMProtocol::IMProtocolJabber:
			status = QtContactPixmap::ContactJabberMobile;
			break;

		case QtEnumIMProtocol::IMProtocolVoxOx:
			status = QtContactPixmap::ContactVoxOxMobile;
			break;

		case QtEnumIMProtocol::IMProtocolGoogleTalk:
			status = QtContactPixmap::ContactGTalkMobile;
			break;

		default:
			LOG_FATAL("unknown IM protocol=" + String::fromNumber(eProtocol));
		}
	}

	return status;
}


//End VoxOx
