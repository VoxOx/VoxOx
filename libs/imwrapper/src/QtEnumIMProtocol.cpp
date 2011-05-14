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

//#include "stdafx.h"
#include <imwrapper/QtEnumIMProtocol.h>

#include <util/String.h>
#include <util/Logger.h>

QtEnumIMProtocolMap* QtEnumIMProtocolMap::_instance = NULL;

//=============================================================================

QtEnumIMProtocol::QtEnumIMProtocol()
{
	initVars();
}

//-----------------------------------------------------------------------------

void QtEnumIMProtocol::initVars()
{
	setQtProtocol   ( QtEnumIMProtocol::IMProtocolUnknown );
	setModelProtocol( EnumIMProtocol::IMProtocolUnknown   );
	setName 		( "unknown"	);

	setOrder			 ( 0 );
	setUseInIMAccountMenu( false );
	setChatProtocol      ( "" );
	setIsIMProtocol		 ( false );
	setSupportsPresence  ( false );
	setSupportsPresenceNote( false );

	setCanDoMultiChat    ( false );
	setDefaultImageFile  ( "" );

	setDefaultProtocolImage  ( "" );//VOXOX - CJC - 2009.07.29 

	setServerProtocolId( QtEnumIMProtocol::ServerProtocolIdUnknown );
}

//-----------------------------------------------------------------------------

String QtEnumIMProtocol::getIconPath()
{
	String protocol = getDefaultProtocolImage();//VOXOX - CJC - 2009.07.29 
	protocol = protocol.toLowerCase();

	String result = ":pics/protocols/" + protocol + ".png";

	return result;
}

//-----------------------------------------------------------------------------

String QtEnumIMProtocol::getBigIconPath()
{
	String protocol = getDefaultProtocolImage();//VOXOX - CJC - 2009.07.29 
	protocol = protocol.toLowerCase();

	String result = ":pics/protocols/big/" + protocol + ".png";

	return result;
}

//-----------------------------------------------------------------------------
	
String QtEnumIMProtocol::getNetworkIconPath( EnumPresenceState::PresenceState presenceState )
{
	bool	bValid		 = true;
	String path		 = "";
	String presenceName = EnumPresenceState::toString( presenceState ).c_str();
	
	presenceName = presenceName.toLowerCase();

	//if ( isIMProtocol() )//VOXOX CHANGE by Rolando - 2009.07.27 
	if ( isIconPresenceBased() )
	{
		String protocolName = getDefaultProtocolImage();
		protocolName = protocolName.toLowerCase();

		switch( presenceState )
		{
		case EnumPresenceState::PresenceStateOnline:
			path = ":/pics/status/networks/" + protocolName + "_" + presenceName + ".png";
			break;

		case EnumPresenceState::PresenceStateUnknown:
			//Leave blank.  TODO: do we want a default icon path?
			break;

		default:
			path = ":/pics/status/networks/" + protocolName + "_" + presenceName + ".png";
		}
	}
	else
	{
		if(presenceName != "unknown" )
		{
			path = ":/pics/status/" + presenceName + ".png";
		}
	}

	return path;
}

//-----------------------------------------------------------------------------

String QtEnumIMProtocol::getImageName()
{
	String result = getDefaultProtocolImage();
	result  = result.toLowerCase();
	result += ".png";

	return result;
}

//-----------------------------------------------------------------------------

bool QtEnumIMProtocol::isIconPresenceBased()
{
	return isIMProtocol() || _qtProtocol == QtEnumIMProtocol::IMProtocolTwitter;
}

//-----------------------------------------------------------------------------
	
void QtEnumIMProtocol::addPresenceStateMapEntry( EnumPresenceState::PresenceState psKey, EnumPresenceState::PresenceState psValue )
{
	_psMap[psKey] = psValue;
}

//-----------------------------------------------------------------------------

EnumPresenceState::PresenceState QtEnumIMProtocol::getMappedPresenceState( EnumPresenceState::PresenceState ps )
{
	if ( _psMap.size() == 0 )
	{
		return ps;
	}
	else
	{
		return _psMap[ps];
	}
}

//=============================================================================


//=============================================================================

QtEnumIMProtocolMap::QtEnumIMProtocolMap()
{
	create();
}

//-----------------------------------------------------------------------------

void QtEnumIMProtocolMap::create()
{
	//Legend
	//	#1 - Presentation IMProtocol (QtEnumIMProtocol)
	//	#2 - Model IMProtocol		 (EnumIMProtocol)
	//	#3 - Name
	//	#4 - Display/Sort order
	//	#5 - Use in IMAccount Menu
	//	#6 - Chat Protocol
	//	#7 - Is IM Protocol
	//	#8 - Supports FULL Presence state (more than just off/on line)
	//  #9 - Supports PresenceStatus Note (statusMsg) - Typically IM
	//	#10 - Server numeric network ID.
	//	#11 - Can Do MultiChat
	//  #12 - default image file (no path)
	//  #13 - default image protocol name //VOXOX - CJC - 2009.07.29 

	//VOXOX - JRT - 2009.04.30 - 
	//TODO: review chatProtocol (#6) values of 'default1' and 'default2'. 
	//TODO: we need to enumerate IsImProtocol with EnumProtocolType (IM, SIP, None, microblog?, etc).

	//===============================================================================
	//VOXOX - JRT - 2009.10.12 
	//NOTE: DUE TO ISSUES WITH NON-JABBER GROUP CHAT, I HAVE DISABLED SUPPORT FOR:
	//	MSN, AIM/ICQ, YAHOO AND GTALK.
	//
	//	ONCE THE ISSUES ARE FIXED, BUT ENABLE THOSE VALUES AGAIN (#11 MULTI-CHAT).
	//	MOST LIKELY YOU HAVE ALREADY DONE SO JUST TO DEV AND TEST IT.
	//===============================================================================

	//LEGEND:  #1								  #2								 #3			     #4  #5    #6         #7	  #8	     #9										 #10	 #11						#12
	//IM protocols
	add( QtEnumIMProtocol::IMProtocolMSN,		 EnumIMProtocol::IMProtocolMSN,		 "Windows Live", 30, true, "msn",      true,  true,  true,  QtEnumIMProtocol::ServerProtocolIdMSN,		  false, "default_msn.png",			"msn"		);
	add( QtEnumIMProtocol::IMProtocolMYSPACE,	 EnumIMProtocol::IMProtocolMYSPACE,  "MySpaceIM",	 40, true, "default1", true,  false, false, QtEnumIMProtocol::ServerProtocolIdMySpace,	  false, "default_myspace.png",		"myspaceim"	);
	add( QtEnumIMProtocol::IMProtocolFacebook,	 EnumIMProtocol::IMProtocolFacebook, "Facebook",     70, true, "default1", true,  false, false, QtEnumIMProtocol::ServerProtocolIdFacebook,  false, "default_facebook.png",	"facebook"  );
	add( QtEnumIMProtocol::IMProtocolTwitter,	 EnumIMProtocol::IMProtocolTwitter,	 "Twitter",      80, true, "default1", false, false, false, QtEnumIMProtocol::ServerProtocolIdTwitter,	  false, "default_twitter.png",		"twitter"	);
	add( QtEnumIMProtocol::IMProtocolSkype,		 EnumIMProtocol::IMProtocolSkype,	 "Skype",	     90, true, "default1", true,  true,  true,  QtEnumIMProtocol::ServerProtocolIdSkype,	  false, "default_skype.png",		"skype"		);
	add( QtEnumIMProtocol::IMProtocolYahoo,		 EnumIMProtocol::IMProtocolYahoo,	 "Yahoo",	     20, true, "yahoo",	   true,  true,  true,  QtEnumIMProtocol::ServerProtocolIdYahoo,	  false, "default_yahoo.png",		"yahoo"		);
	add( QtEnumIMProtocol::IMProtocolAIM,		 EnumIMProtocol::IMProtocolAIM,		 "AIM",		     50, true, "default",  true,  true,  true,  QtEnumIMProtocol::ServerProtocolIdAIM,		  false, "default_aim.png",         "aim"		);
	add( QtEnumIMProtocol::IMProtocolICQ,		 EnumIMProtocol::IMProtocolICQ,		 "ICQ",		     60, true, "default",  true,  true,  true,  QtEnumIMProtocol::ServerProtocolIdICQ,		  false, "default_icq.png",         "icq"		);
	add( QtEnumIMProtocol::IMProtocolJabber,	 EnumIMProtocol::IMProtocolJabber,	 "Jabber",	     10, true, "default",  true,  true,  true,  QtEnumIMProtocol::ServerProtocolIdJabber,	  true,  "default_googletalk.png",  "jabber"	);
	add( QtEnumIMProtocol::IMProtocolGoogleTalk, EnumIMProtocol::IMProtocolJabber,	 "GoogleTalk",   11, true, "default1", true,  true,  true,  QtEnumIMProtocol::ServerProtocolIdGoogleTalk, true,  "default_googletalk.png",  "googletalk");
	add( QtEnumIMProtocol::IMProtocolVoxOx,		 EnumIMProtocol::IMProtocolJabber,	 "VoxOx",	      0, false,"default1", true,  true,  true,  QtEnumIMProtocol::ServerProtocolIdVoxOx,	  true,  "contact_avatar_null.png", "voxox"		);
																																		 
	//SIP protocols																														 
	add( QtEnumIMProtocol::IMProtocolWengo,		 EnumIMProtocol::IMProtocolWengo,	 "VoxOx",	      0, false, "wengo",   false, false, false, QtEnumIMProtocol::ServerProtocolIdNone,	  false, "", "" );	//TODO: Do we want to use Wengo?
	add( QtEnumIMProtocol::IMProtocolSIP,		 EnumIMProtocol::IMProtocolSIP,		 "SIP",		    100, false,"default2", false, false, false, QtEnumIMProtocol::ServerProtocolIdNone,	  false, "", "" );
	add( QtEnumIMProtocol::IMProtocolSIPSIMPLE,  EnumIMProtocol::IMProtocolSIPSIMPLE,"SIP-SIMPLE",  100, false,"default2", false, false, false, QtEnumIMProtocol::ServerProtocolIdNone,	  false, "", "" );	//TODO: Is this right? No slash because it may be used for file name.
																																		 
	//Special purpose																													 
	add( QtEnumIMProtocol::IMProtocolUnknown,	 EnumIMProtocol::IMProtocolUnknown,	 "Unknown",	     -1, false, "default", false, false, false, QtEnumIMProtocol::ServerProtocolIdNone,	  false, "", "" );
	add( QtEnumIMProtocol::IMProtocolAll,		 EnumIMProtocol::IMProtocolAll,		 "All",			 -1, false, "all",     false, false, false, QtEnumIMProtocol::ServerProtocolIdNone,	  false, "", "" );

	addPsMaps();
}

//-----------------------------------------------------------------------------

void QtEnumIMProtocolMap::add( QtEnumIMProtocol::IMProtocol qtProtocol, EnumIMProtocol::IMProtocol modelProtocol, const char* name,
							  int nOrder, bool useInIMAccountMenu, const char* chatProtocol, bool bIMProtocol, bool bSupportsPresence,
							  bool bSupportsPresenceNote,
							  QtEnumIMProtocol::ServerProtocolId protocolId, bool bCanDoMultiChat, const char* defaultImageFile,const char* defaultProtocolImage  )//VOXOX - CJC - 2009.07.29 
{
	QtEnumIMProtocol item;

	item.setQtProtocol		  ( qtProtocol			);
	item.setModelProtocol	  ( modelProtocol		);
	item.setServerProtocolId  ( protocolId			);
	item.setName			  ( name				);

	item.setChatProtocol	  ( chatProtocol		);
	item.setOrder			  ( nOrder				);
	item.setUseInIMAccountMenu( useInIMAccountMenu	);
	item.setIsIMProtocol      ( bIMProtocol			);
	item.setSupportsPresence  ( bSupportsPresence   );
	item.setSupportsPresenceNote( bSupportsPresenceNote   );	//VOXOX - JRT - 2009.09.10 

	item.setCanDoMultiChat	  ( bCanDoMultiChat		);
	item.setDefaultImageFile  ( defaultImageFile	);
	item.setDefaultProtocolImage( defaultProtocolImage	);//VOXOX - CJC - 2009.07.29 

	insert( std::pair<QtEnumIMProtocol::IMProtocol, QtEnumIMProtocol>( item.getKey(), item ) );
}

//-----------------------------------------------------------------------------

void QtEnumIMProtocolMap::addPsMaps()
{
	//Just detect map is empty and return key value.
//	addStdPsMap( QtEnumIMProtocol::IMProtocolMSN		);
//	addStdPsMap( QtEnumIMProtocol::IMProtocolSkype		);
//	addStdPsMap( QtEnumIMProtocol::IMProtocolYahoo		);
//	addStdPsMap( QtEnumIMProtocol::IMProtocolAIM		);
//	addStdPsMap( QtEnumIMProtocol::IMProtocolICQ		);

	addJabberPsMap( QtEnumIMProtocol::IMProtocolJabber		);
	addJabberPsMap( QtEnumIMProtocol::IMProtocolGoogleTalk	);
	addJabberPsMap( QtEnumIMProtocol::IMProtocolVoxOx		);

	addMySpacePsMap( QtEnumIMProtocol::IMProtocolMYSPACE	);

	addFacebookPsMap( QtEnumIMProtocol::IMProtocolFacebook	);	 

	addShortPsMap( QtEnumIMProtocol::IMProtocolTwitter	);
												
	//SIP protocols								
	addShortPsMap( QtEnumIMProtocol::IMProtocolWengo	 );
	addShortPsMap( QtEnumIMProtocol::IMProtocolSIP		 );		 
	addShortPsMap( QtEnumIMProtocol::IMProtocolSIPSIMPLE );
												
	//Special purpose							
//	addStdPsMap( QtEnumIMProtocol::IMProtocolUnknown,	 
//	addStdPsMap( QtEnumIMProtocol::IMProtocolAll,		 
}

//-----------------------------------------------------------------------------

void QtEnumIMProtocolMap::addShortPsMap( QtEnumIMProtocol::IMProtocol tgtProtocol )
{
	QtEnumIMProtocolMap::iterator it = ProtocolMap::find( tgtProtocol );

	if( it != ProtocolMap::end() )
	{
		(*it).second.addPresenceStateMapEntry( EnumPresenceState::PresenceStateOffline,		 EnumPresenceState::PresenceStateOffline );
		(*it).second.addPresenceStateMapEntry( EnumPresenceState::PresenceStateOnline,		 EnumPresenceState::PresenceStateOnline  );
		(*it).second.addPresenceStateMapEntry( EnumPresenceState::PresenceStateInvisible,	 EnumPresenceState::PresenceStateOnline  );	//Not Supported
		(*it).second.addPresenceStateMapEntry( EnumPresenceState::PresenceStateAway,		 EnumPresenceState::PresenceStateOnline  );	//Not Supported
		(*it).second.addPresenceStateMapEntry( EnumPresenceState::PresenceStateDoNotDisturb, EnumPresenceState::PresenceStateOnline  );	//Not Supported
	}
}

//-----------------------------------------------------------------------------

void QtEnumIMProtocolMap::addFacebookPsMap( QtEnumIMProtocol::IMProtocol tgtProtocol )
{
	QtEnumIMProtocolMap::iterator it = ProtocolMap::find( tgtProtocol );

	if( it != ProtocolMap::end() )
	{
		(*it).second.addPresenceStateMapEntry( EnumPresenceState::PresenceStateOffline,		 EnumPresenceState::PresenceStateOffline );
		(*it).second.addPresenceStateMapEntry( EnumPresenceState::PresenceStateOnline,		 EnumPresenceState::PresenceStateOnline  );
		(*it).second.addPresenceStateMapEntry( EnumPresenceState::PresenceStateInvisible,	 EnumPresenceState::PresenceStateOnline  );	//Not supported
		(*it).second.addPresenceStateMapEntry( EnumPresenceState::PresenceStateAway,		 EnumPresenceState::PresenceStateAway    );
		(*it).second.addPresenceStateMapEntry( EnumPresenceState::PresenceStateDoNotDisturb, EnumPresenceState::PresenceStateOnline  );	//Not supported
	}
}

//-----------------------------------------------------------------------------

void QtEnumIMProtocolMap::addMySpacePsMap( QtEnumIMProtocol::IMProtocol tgtProtocol )
{
	QtEnumIMProtocolMap::iterator it = ProtocolMap::find( tgtProtocol );

	if( it != ProtocolMap::end() )
	{
		(*it).second.addPresenceStateMapEntry( EnumPresenceState::PresenceStateOffline,		 EnumPresenceState::PresenceStateOffline	);
		(*it).second.addPresenceStateMapEntry( EnumPresenceState::PresenceStateOnline,		 EnumPresenceState::PresenceStateOnline		);
		(*it).second.addPresenceStateMapEntry( EnumPresenceState::PresenceStateInvisible,	 EnumPresenceState::PresenceStateInvisible  );
		(*it).second.addPresenceStateMapEntry( EnumPresenceState::PresenceStateAway,		 EnumPresenceState::PresenceStateAway		);
		(*it).second.addPresenceStateMapEntry( EnumPresenceState::PresenceStateDoNotDisturb, EnumPresenceState::PresenceStateOnline		);	//Not supported.
	}
}

//-----------------------------------------------------------------------------

void QtEnumIMProtocolMap::addJabberPsMap( QtEnumIMProtocol::IMProtocol tgtProtocol )
{
	QtEnumIMProtocolMap::iterator it = ProtocolMap::find( tgtProtocol );

	if( it != ProtocolMap::end() )
	{
		(*it).second.addPresenceStateMapEntry( EnumPresenceState::PresenceStateOffline,		 EnumPresenceState::PresenceStateOffline	  );
		(*it).second.addPresenceStateMapEntry( EnumPresenceState::PresenceStateOnline,		 EnumPresenceState::PresenceStateOnline		  );
		(*it).second.addPresenceStateMapEntry( EnumPresenceState::PresenceStateInvisible,	 EnumPresenceState::PresenceStateOnline       );	//Not supported.
		(*it).second.addPresenceStateMapEntry( EnumPresenceState::PresenceStateAway,		 EnumPresenceState::PresenceStateAway		  );
		(*it).second.addPresenceStateMapEntry( EnumPresenceState::PresenceStateDoNotDisturb, EnumPresenceState::PresenceStateDoNotDisturb );
	}
}

//-----------------------------------------------------------------------------

QtEnumIMProtocol* QtEnumIMProtocolMap::Lookup( const QtEnumIMProtocol::IMProtocol tgtProtocol )
{
	QtEnumIMProtocol* pRet = NULL;

	QtEnumIMProtocolMap::iterator it = ProtocolMap::find( tgtProtocol );

	if( it != ProtocolMap::end() )
	{
		pRet = &((*it).second);
	}

	return pRet;
}

//-----------------------------------------------------------------------------
	
QtEnumIMProtocol* QtEnumIMProtocolMap::findByQtProtocol( QtEnumIMProtocol::IMProtocol tgtQtProtocol )
{
	QtEnumIMProtocol* result = NULL;

	for ( QtEnumIMProtocolMap::iterator it = ProtocolMap::begin(); it != ProtocolMap::end(); ++it ) 
	{
		if ( (*it).second.getQtProtocol() == tgtQtProtocol  ) 
		{
			result = &(*it).second;
			break;
		}
	}

	if ( result == NULL )
	{
		result = Lookup( QtEnumIMProtocol::IMProtocolUnknown );
		LOG_FATAL( "unknown protocol=" + String::fromNumber(tgtQtProtocol) );
	}

	return result;
}

//-----------------------------------------------------------------------------
	
QtEnumIMProtocol* QtEnumIMProtocolMap::findByModelProtocol( EnumIMProtocol::IMProtocol tgtModelProtocol )
{
	QtEnumIMProtocol* result = NULL;

	for ( QtEnumIMProtocolMap::iterator it = ProtocolMap::begin(); it != ProtocolMap::end(); ++it ) 
	{
		if ( (*it).second.getModelProtocol() == tgtModelProtocol  ) 
		{
			result = &(*it).second;
			break;
		}
	}

	if ( result == NULL )
	{
		result = Lookup( QtEnumIMProtocol::IMProtocolUnknown );
		LOG_FATAL( "unknown protocol=" + String::fromNumber(tgtModelProtocol) );
	}

	return result;
}

//-----------------------------------------------------------------------------
	
QtEnumIMProtocol* QtEnumIMProtocolMap::findByServerProtocolId( QtEnumIMProtocol::ServerProtocolId  tgtProtocolId )
{
	QtEnumIMProtocol* result = NULL;

	for ( QtEnumIMProtocolMap::iterator it = ProtocolMap::begin(); it != ProtocolMap::end(); ++it ) 
	{
		if ( (*it).second.getServerProtocolId() == tgtProtocolId  ) 
		{
			result = &(*it).second;
			break;
		}
	}

	return result;
}

//-----------------------------------------------------------------------------

String QtEnumIMProtocolMap::toString( QtEnumIMProtocol::IMProtocol protocol) 
{
	String result = "";
	
	QtEnumIMProtocol* temp = Lookup( protocol );

	if ( temp )
	{
		result = temp->getName();
	}
	else
	{
		LOG_FATAL("unknown IMProtocol=" + String::fromNumber(protocol));
	}

	return result;
}

//-----------------------------------------------------------------------------

String QtEnumIMProtocolMap::toString( EnumIMProtocol::IMProtocol protocol) 
{
	String result = "";
	
	QtEnumIMProtocol* temp = findByModelProtocol( protocol );

	if ( temp )
	{
		result = temp->getName();
	}
	else
	{
		LOG_FATAL("unknown IMProtocol=" + String::fromNumber(protocol));
	}

	return result;
}

//-----------------------------------------------------------------------------

QtEnumIMProtocol::IMProtocol QtEnumIMProtocolMap::toQtIMProtocol(EnumIMProtocol::IMProtocol tgtModelProtocol) 
{
	QtEnumIMProtocol::IMProtocol result = QtEnumIMProtocol::IMProtocolUnknown;

	QtEnumIMProtocol* protocol = this->findByModelProtocol( tgtModelProtocol );

	if ( protocol )
	{
		result = protocol->getQtProtocol();
	}
	else
	{
		LOG_FATAL( "unknown protocol=" + String::fromNumber(tgtModelProtocol) );
	}

	return result;
}

//-----------------------------------------------------------------------------

QtEnumIMProtocol::IMProtocol QtEnumIMProtocolMap::toQtIMProtocol(EnumIMProtocol::IMProtocol tgtModelProtocol, bool isVoxOx, bool isGtalk )
{
	QtEnumIMProtocol::IMProtocol result = toQtIMProtocol( tgtModelProtocol );

	if ( tgtModelProtocol == EnumIMProtocol::IMProtocolJabber )
	{
		if ( isVoxOx )
		{
			result = QtEnumIMProtocol::IMProtocolVoxOx;
		}
		else if ( isGtalk )
		{
			result = QtEnumIMProtocol::IMProtocolGoogleTalk;
		}
	}

	return result;
}

//-----------------------------------------------------------------------------

QtEnumIMProtocol::IMProtocol QtEnumIMProtocolMap::toIMProtocol(const String& protocolIn) 
{
	QtEnumIMProtocol::IMProtocol result = QtEnumIMProtocol::IMProtocolUnknown;
	String strWork;
	String strProtocol = protocolIn.toLowerCase();

	for ( QtEnumIMProtocolMap::iterator it = ProtocolMap::begin(); it != ProtocolMap::end(); ++it ) 
	{
		strWork = (*it).second.getName().toLowerCase();

		if ( strWork == strProtocol ) 
		{
			result = (*it).first;
			break;
		}
	}

	if ( result == QtEnumIMProtocol::IMProtocolUnknown )
	{
		LOG_WARN("unknown protocol=" + protocolIn);
	}

	return result;
}

//-----------------------------------------------------------------------------

EnumIMProtocol::IMProtocol QtEnumIMProtocolMap::toModelIMProtocol(QtEnumIMProtocol::IMProtocol tgtModelProtocol) 
{
	EnumIMProtocol::IMProtocol result = EnumIMProtocol::IMProtocolUnknown;
	
	QtEnumIMProtocol* temp = Lookup( tgtModelProtocol );

	if ( temp )
	{
		result = temp->getModelProtocol();
	}
	else
	{
		LOG_FATAL("unknown IMProtocol=" + String::fromNumber(tgtModelProtocol));
	}

	return result;
}

//-----------------------------------------------------------------------------
	
EnumIMProtocol::IMProtocol QtEnumIMProtocolMap::toModelIMProtocol( QtEnumIMProtocol::ServerProtocolId  protocolId )
{
	EnumIMProtocol::IMProtocol modelProtocol = EnumIMProtocol::IMProtocolUnknown;

	QtEnumIMProtocol* pTemp = findByServerProtocolId( protocolId );

	if ( pTemp )
	{
		modelProtocol = pTemp->getModelProtocol();
	}

	return modelProtocol;
}

//-----------------------------------------------------------------------------

QtEnumIMProtocol::ServerProtocolId QtEnumIMProtocolMap::toServerProtocolId( QtEnumIMProtocol::IMProtocol qtProtocol )
{
	QtEnumIMProtocol::ServerProtocolId protocolId = QtEnumIMProtocol::ServerProtocolIdUnknown;

	QtEnumIMProtocol* pTemp = findByQtProtocol( qtProtocol );

	if ( pTemp )
	{
		protocolId = pTemp->getServerProtocolId();
	}

	return protocolId;
}

//-----------------------------------------------------------------------------

QtEnumIMProtocol::IMProtocol QtEnumIMProtocolMap::fromServerProtocolId( QtEnumIMProtocol::ServerProtocolId protocolId  )
{
	QtEnumIMProtocol::IMProtocol qtProtocol = QtEnumIMProtocol::IMProtocolUnknown;

	QtEnumIMProtocol* pTemp = findByServerProtocolId( protocolId );

	if ( pTemp )
	{
		qtProtocol = pTemp->getQtProtocol();
	}

	return qtProtocol;
}

//-----------------------------------------------------------------------------

bool QtEnumIMProtocolMap::canDoMultiChat( EnumIMProtocol::IMProtocol protocol )
{
	bool bCan = false;

	QtEnumIMProtocol* pTemp = findByModelProtocol( protocol );

	if ( pTemp )
	{
		bCan = pTemp->canDoMultiChat();
	}

	return bCan;
}

//-----------------------------------------------------------------------------

String QtEnumIMProtocolMap::getDefaultImageFile( QtEnumIMProtocol::IMProtocol protocol )
{
	String result = "";

	QtEnumIMProtocol* pTemp = findByQtProtocol( protocol );

	if ( pTemp )
	{
		result = pTemp->getDefaultImageFile();
	}

	return result;
}

String QtEnumIMProtocolMap::getDefaultProtocolImage( QtEnumIMProtocol::IMProtocol protocol )//VOXOX - CJC - 2009.07.29 
{
	String result = "";

	QtEnumIMProtocol* pTemp = findByQtProtocol( protocol );

	if ( pTemp )
	{
		result = pTemp->getDefaultProtocolImage();
	}

	return result;
}

//-----------------------------------------------------------------------------
//static 
std::string QtEnumIMProtocolMap::getQtProtocolName( QtEnumIMProtocol::IMProtocol protocol )
{
	std::string result = "";

	QtEnumIMProtocolMap& rMap = QtEnumIMProtocolMap::getInstance();

	QtEnumIMProtocol* p = rMap.findByQtProtocol( protocol );

	if ( p )
	{
		result = p->getName();
	}

	return result;
}

//-----------------------------------------------------------------------------
//static 
bool QtEnumIMProtocolMap::hasIM( QtEnumIMProtocol::IMProtocol protocol )
{
	bool result = false;

	QtEnumIMProtocolMap& rMap = QtEnumIMProtocolMap::getInstance();

	QtEnumIMProtocol* p = rMap.findByQtProtocol( protocol );

	if ( p )
	{
		result = p->isIMProtocol();
	}

	return result;
}

//-----------------------------------------------------------------------------
//static 
bool QtEnumIMProtocolMap::supportsPresence ( QtEnumIMProtocol::IMProtocol protocol )
{
	bool result = false;

	QtEnumIMProtocolMap& rMap = QtEnumIMProtocolMap::getInstance();

	QtEnumIMProtocol* p = rMap.findByQtProtocol( protocol );

	if ( p )
	{
		result = p->supportsPresence();
	}

	return result;
}

//-----------------------------------------------------------------------------
//static 
bool QtEnumIMProtocolMap::supportsPresence ( EnumIMProtocol::IMProtocol protocol )
{
	bool result = false;

	QtEnumIMProtocolMap& rMap = QtEnumIMProtocolMap::getInstance();

	QtEnumIMProtocol* p = rMap.findByModelProtocol( protocol );

	if ( p )
	{
		result = p->supportsPresence();
	}

	return result;
}

//-----------------------------------------------------------------------------
//static 
bool QtEnumIMProtocolMap::supportsPresenceNote( QtEnumIMProtocol::IMProtocol protocol )	//VOXOX - JRT - 2009.09.10 
{
	bool result = false;

	QtEnumIMProtocol* p = QtEnumIMProtocolMap::getInstance().findByQtProtocol( protocol );

	if ( p )
	{
		result = p->supportsPresenceNote();
	}

	return result;
}

//-----------------------------------------------------------------------------
//static
EnumPresenceState::PresenceState QtEnumIMProtocolMap::getMappedPresenceState( QtEnumIMProtocol::IMProtocol protocol, EnumPresenceState::PresenceState ps )
{
	EnumPresenceState::PresenceState result = EnumPresenceState::PresenceStateOnline;

	QtEnumIMProtocol* p = QtEnumIMProtocolMap::getInstance().findByQtProtocol( protocol );

	if ( p )
	{
		result = p->getMappedPresenceState( ps );
	}

	return result;
}


//=============================================================================
