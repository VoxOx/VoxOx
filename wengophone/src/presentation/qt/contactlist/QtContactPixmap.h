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

#ifndef OWQTCONTACTPIXMAP_H
#define OWQTCONTACTPIXMAP_H

#include <imwrapper/EnumIMProtocol.h>		//VOXOX - JRT - 2009.04.21 
//#include <QtEnumIMProtocol.h>				//VOXOX - JRT - 2009.04.21 
#include <imwrapper/QtEnumIMProtocol.h>		//VOXOX - JRT - 2009.05.29 
#include <imwrapper/EnumPresenceState.h>
#include <QtCore/QHash>
#include <QtGui/QPixmap>

//VOXOX - JRT - 2009.04.17 - Let's make maintenance of this easier.
//This map will represent supported Presence States  mapped to QtContactPixMap value.
//We will declare one for each support Protocol.
//
//class PresenceStatePixMap		//TODO
//{
//};

//=============================================================================
/**
 * Store status pixmaps
 *
 * @author Mr K
 */
class QtContactPixmap 
{
public:

	enum ContactPixmap 
	{
		ContactUnknown,
		ContactNoStatus,

		ContactOnline,
		ContactOffline,
		ContactDND,
		ContactInvisible,
		ContactAway,
		
		ContactGroupOpen,
		ContactGroupClose,
		
		ContactWDeal,
		ContactNoStatusSIP,
		
		ContactMSNOnline,
		ContactMSNOffline,
		ContactMSNInvisible,
		ContactMSNDND,
		ContactMSNAway,
		ContactMSNBlock,
		ContactMSNMobile,
		
		ContactMYSPACEOnline,
		ContactMYSPACEOffline,
		ContactMYSPACEInvisible,
		ContactMYSPACEDND,
		ContactMYSPACEAway,
		ContactMYSPACEBlock,
		ContactMYSPACEMobile,

		ContactFacebookOnline,
		ContactFacebookOffline,
		ContactFacebookInvisible,
		ContactFacebookDND,
		ContactFacebookAway,
		ContactFacebookBlock,
		ContactFacebookMobile,

		ContactTwitterOnline,
		ContactTwitterOffline,
		ContactTwitterInvisible,
		ContactTwitterDND,
		ContactTwitterAway,
		ContactTwitterBlock,
		ContactTwitterMobile,

		ContactSkypeOnline,
		ContactSkypeOffline,
		ContactSkypeInvisible,
		ContactSkypeDND,
		ContactSkypeAway,
		ContactSkypeBlock,
		ContactSkypeMobile,

		ContactYahooOnline,
		ContactYahooOffline,
		ContactYahooInvisible,
		ContactYahooDND,
		ContactYahooAway,
		ContactYahooBlock,
		ContactYahooMobile,

		ContactJabberOnline,
		ContactJabberOffline,
		ContactJabberInvisible,
		ContactJabberDND,
		ContactJabberAway,
		ContactJabberBlock,
		ContactJabberMobile,
		
		ContactICQOnline,
		ContactICQOffline,
		ContactICQInvisible,
		ContactICQDND,
		ContactICQAway,
		ContactICQBlock,
		ContactICQMobile,
		
		ContactAIMOnline,
		ContactAIMOffline,
		ContactAIMInvisible,
		ContactTestCall,
		ContactAIMDND,
		ContactAIMAway,
		ContactAIMBlock,
		ContactAIMMobile,
		
		ContactVoxOxOnline,
		ContactVoxOxOffline,
		ContactVoxOxInvisible,
		ContactVoxOxDND,
		ContactVoxOxAway,
		ContactVoxOxBlock,
		ContactVoxOxMobile,
		
		ContactGTalkOnline,
		ContactGTalkOffline,
		ContactGTalkInvisible,
		ContactGTalkDND,
		ContactGTalkAway,
		ContactGTalkBlock,
		ContactGTalkMobile,

		ContactMSNOnlineHover,
		ContactMSNOfflineHover,
		ContactMSNInvisibleHover,
		ContactMSNDNDHover,
		ContactMSNAwayHover,
		ContactMSNBlockHover,
		ContactMSNMobileHover,
		
		ContactMYSPACEOnlineHover,
		ContactMYSPACEOfflineHover,
		ContactMYSPACEInvisibleHover,
		ContactMYSPACEDNDHover,
		ContactMYSPACEAwayHover,
		ContactMYSPACEBlockHover,
		ContactMYSPACEMobileHover,

		ContactFacebookOnlineHover,
		ContactFacebookOfflineHover,
		ContactFacebookInvisibleHover,
		ContactFacebookDNDHover,
		ContactFacebookAwayHover,
		ContactFacebookBlockHover,
		ContactFacebookMobileHover,

		ContactTwitterOnlineHover,
		ContactTwitterOfflineHover,
		ContactTwitterInvisibleHover,
		ContactTwitterDNDHover,
		ContactTwitterAwayHover,
		ContactTwitterBlockHover,
		ContactTwitterMobileHover,

		ContactSkypeOnlineHover,
		ContactSkypeOfflineHover,
		ContactSkypeInvisibleHover,
		ContactSkypeDNDHover,
		ContactSkypeAwayHover,
		ContactSkypeBlockHover,
		ContactSkypeMobileHover,

		ContactYahooOnlineHover,
		ContactYahooOfflineHover,
		ContactYahooInvisibleHover,
		ContactYahooDNDHover,
		ContactYahooAwayHover,
		ContactYahooBlockHover,
		ContactYahooMobileHover,

		ContactJabberOnlineHover,
		ContactJabberOfflineHover,
		ContactJabberInvisibleHover,
		ContactJabberDNDHover,
		ContactJabberAwayHover,
		ContactJabberBlockHover,
		ContactJabberMobileHover,
		
		ContactICQOnlineHover,
		ContactICQOfflineHover,
		ContactICQInvisibleHover,
		ContactICQDNDHover,
		ContactICQAwayHover,
		ContactICQBlockHover,
		ContactICQMobileHover,
		
		ContactAIMOnlineHover,
		ContactAIMOfflineHover,
		ContactAIMInvisibleHover,
		ContactAIMDNDHover,
		ContactAIMAwayHover,
		ContactAIMBlockHover,
		ContactAIMMobileHover,
		
		ContactVoxOxOnlineHover,
		ContactVoxOxOfflineHover,
		ContactVoxOxInvisibleHover,
		ContactVoxOxDNDHover,
		ContactVoxOxAwayHover,
		ContactVoxOxBlockHover,
		ContactVoxOxMobileHover,
		
		ContactGTalkOnlineHover,
		ContactGTalkOfflineHover,
		ContactGTalkInvisibleHover,
		ContactGTalkDNDHover,
		ContactGTalkAwayHover,
		ContactGTalkBlockHover,
		ContactGTalkMobileHover,
		
		ContactFacebook,
		ContactMySpace,
		ContactTwitter,
		ContactYahooMail
	};

	static QtContactPixmap * getInstance();
	
	//VOXOX CHANGE by Rolando - 2009.10.22 - Added isHover parameter
	//VOXOX - CJC - 2009.05.05 Add blocked to the parameter
	static ContactPixmap     determinePixmap( QtEnumIMProtocol::IMProtocol eProtocol, EnumPresenceState::PresenceState eState,
											  bool hasAvailableSIPNumber , bool isBlocked, bool isHover = false); //VOXOX - JRT - 2009.04.14 

	static ContactPixmap     determineHoverPixmap( QtEnumIMProtocol::IMProtocol eProtocol, EnumPresenceState::PresenceState eState,
											  bool hasAvailableSIPNumber , bool isBlocked);//VOXOX CHANGE by Rolando - 2009.10.22 

	void setPixmap(ContactPixmap status, const QPixmap & px, QString pixmapPath);//VOXOX CHANGE by Rolando - 2009.10.23 

	const QPixmap & getPixmap(ContactPixmap status);

	QString getPixmapPath(ContactPixmap status);//VOXOX CHANGE by Rolando - 2009.10.23 

private:

	QtContactPixmap();

	QtContactPixmap(const QtContactPixmap & other)
		: _contactPixmap(other._contactPixmap) { }

	QtContactPixmap & operator=(const QtContactPixmap & other) {
		_contactPixmap = other._contactPixmap;
		return * this;
	}

	static QtContactPixmap::ContactPixmap getOnlinePix   ( QtEnumIMProtocol::IMProtocol eProtocol, bool isHover);//VOXOX CHANGE by Rolando - 2009.10.22
	static QtContactPixmap::ContactPixmap getAwayPix	 ( QtEnumIMProtocol::IMProtocol eProtocol, bool isHover);//VOXOX CHANGE by Rolando - 2009.10.22
	static QtContactPixmap::ContactPixmap getInvisiblePix( QtEnumIMProtocol::IMProtocol eProtocol, bool isHover);//VOXOX CHANGE by Rolando - 2009.10.22
	static QtContactPixmap::ContactPixmap getOfflinePix  ( QtEnumIMProtocol::IMProtocol eProtocol, bool isHover);//VOXOX CHANGE by Rolando - 2009.10.22
	static QtContactPixmap::ContactPixmap getDndPix      ( QtEnumIMProtocol::IMProtocol eProtocol, bool isHover);//VOXOX CHANGE by Rolando - 2009.10.22
	static QtContactPixmap::ContactPixmap getUnknownPix  ( QtEnumIMProtocol::IMProtocol eProtocol, bool hasAvailableSIPNumber);//VOXOX - CJC - 2009.06.18 
	static QtContactPixmap::ContactPixmap getBlockedPix  ( QtEnumIMProtocol::IMProtocol eProtocol, bool isHover);//VOXOX CHANGE by Rolando - 2009.10.22
	//VOXOX - CJC - 2009.06.18 
	static QtContactPixmap::ContactPixmap getMobilePix   ( QtEnumIMProtocol::IMProtocol eProtocol, bool isHover);//VOXOX CHANGE by Rolando - 2009.10.22


	struct PixmapInfo {//VOXOX CHANGE by Rolando - 2009.10.23 
		QPixmap pixmap;
		QString pixmapPath;
	};

	QHash < ContactPixmap, PixmapInfo > _contactPixmap;//VOXOX CHANGE by Rolando - 2009.10.23 

	static QtContactPixmap * _instance;
};

#endif	//OWQTCONTACTPIXMAP_H
