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

#include "stdafx.h"	//VOXOX - JRT - 2009.04.01
#include "WsSoftUpdate.h"

#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <model/account/wengo/WengoAccount.h>	//VOXOX - JRT - 2009.08.12 
#include <model/profile/UserProfile.h>			//VOXOX - JRT - 2009.10.26 

#include <WengoPhoneBuildId.h>

#include <cutil/global.h>
#include <util/Logger.h>

#include <tinyxml.h>

#include <sstream>

WsSoftUpdate::WsSoftUpdate(WengoAccount * wengoAccount, UserProfile& userProfile)
	: WengoWebService(wengoAccount),
	  _userProfile( userProfile )
{
	Config & config = ConfigManager::getInstance().getCurrentConfig();

	//Setup the web service
	setHostname(config.getWengoServerHostname());
	setGet(true);
	setServicePath(config.getWengoSoftUpdatePath());
	setWengoAuthentication(false);

	setHttps(true);
	setPort(443);

	_manualCheck = false;
}

void WsSoftUpdate::checkForUpdate( bool manualCheck ) 
{
	_manualCheck = manualCheck;

	std::string	parms = "";
	
	parms += "&userid="		+ getWengoAccount()->getUserProfileName();
	parms += "&b="			+ String::fromLongLong(WengoPhoneBuildId::getBuildId());

	setParameters( parms );

	call(this);	//Calls the web service
}

void WsSoftUpdate::answerReceived(const std::string & answer, int requestId) 
{
	LOG_INFO( answer );

	if (answer.empty()) 
	{
		return;
	}

	SoftUpdateInfo suInfo;

	suInfo.setManualCheck( _manualCheck );

	TiXmlDocument doc;
	doc.Parse(answer.c_str());

	TiXmlHandle docHandle(&doc);

	//Process <softphone> node
	TiXmlNode* rootNode = docHandle.FirstChild("softphone").ToNode();

	if ( rootNode )
	{
		TiXmlElement* element = rootNode->ToElement();

		if ( element )
		{
			const char* val = element->Attribute( "update" );

			if ( val )
			{
				String temp = val;
				temp = temp.toLowerCase();

				suInfo.setUpdateAvailable( (temp == "y") );
			}

			//Just testing old version.
			val = element->Attribute( "vapi" );

			if ( val )
			{
				String temp = val;
				temp = temp.toLowerCase();
			}
		}
	}

	//Process <version> node
	TiXmlHandle root = docHandle.FirstChild("softphone").FirstChild("version");


	TiXmlText* text = NULL;
	TiXmlNode* node = NULL;

	text = root.FirstChild("build").FirstChild().Text();
	if (text) 
	{
		unsigned long long buildId = 0;
		std::stringstream ss(std::string(text->Value()));
		ss >> buildId;

		suInfo.setBuildId( buildId );	//VOXOX - JRT - 2009.10.05 
	}

	text = root.FirstChild("type").FirstChild().Text();
	if (text) 
	{
		suInfo.setType( text->Value() );		//VOXOX - JRT - 2009.10.26 
	}

	text = root.FirstChild("url").FirstChild().Text();
	if (text) 
	{
		suInfo.setDownloadUrl( text->Value() );		//VOXOX - JRT - 2009.10.05 
	}

	//VOXOX - JRT - 2009.12.16 - Treat this node a pure text.  This allows server to pass HTML more easily.
	node = root.FirstChild("desc").FirstChild().Node();
	if (node) 
	{
		std::string temp;
		temp << *node;
		temp = String::decodeUrl( temp );
		suInfo.setDescription( temp );
	}

	text = root.FirstChild("download_now").FirstChild().Text();
	if (text) 
	{
		String temp = text->Value();
		temp = temp.toLowerCase();

		suInfo.setDownloadNow( (temp == "y") );
	}

	text = root.FirstChild("cmdline_parms").FirstChild().Text();
	if (text) 
	{
		suInfo.setCmdLineParms( text->Value() );
	}


	text = root.FirstChild("version_marketing").FirstChild().Text();
	if (text) 
	{
		suInfo.setVersion( text->Value() );
	}

	text = root.FirstChild("filesize").FirstChild().Text();
	if (text) 
	{
		unsigned fileSize = 0;
		std::stringstream ss(std::string(text->Value()));
		ss >> fileSize;

		suInfo.setFileSize( fileSize );
	}

	text = root.FirstChild("os").FirstChild().Text();
	if (text) 
	{
		suInfo.setOS( text->Value() );
	}

	loadDebugValues( suInfo );

	if ( suInfo.isUpdateAvailable() )	//VOXOX - JRT - 2009.12.10 - Moved to UserProfile::shouldUpdate
	{
		//A new version of WengoPhone is available and we don't have a developer version with buildid=0
		LOG_DEBUG("new VoxOx version available=" + suInfo.getVersion() + " buildid=" + String::fromUnsignedLongLong( suInfo.getBuildId() ) );

		if ( shouldUpdate( suInfo ) )
		{
			updateWengoPhoneEvent(*this, suInfo );	//VOXOX - JRT - 2009.10.05 
		}
	}
	else
	{
		if ( suInfo.isManualCheck() )
		{
			noAvailableUpdateEvent(*this, suInfo );	//VOXOX - JRT - 2009.10.05 
		}
		else 
		{
			LOG_DEBUG("VoxOx is up-to-date");
		}
	}
}

//-----------------------------------------------------------------------------

bool WsSoftUpdate::shouldUpdate( SoftUpdateInfo& suInfo )
{
	return _userProfile.shouldSoftUpdate( suInfo );
}

//-----------------------------------------------------------------------------
	
void WsSoftUpdate::loadDebugValues( SoftUpdateInfo& suInfo )
{
	Config& config = ConfigManager::getInstance().getCurrentConfig();

	if ( config.isDebugEnabled() )
	{
		suInfo.setUpdateAvailable	( config.getDebugSoftUpdateUpdate()			);
		suInfo.setType				( config.getDebugSoftUpdateType()			);
		suInfo.setDownloadUrl		( config.getDebugSoftUpdateDownloadUrl()	);
		suInfo.setDescription		( config.getDebugSoftUpdateDescription()	);
		suInfo.setVersion			( config.getDebugSoftUpdateVersion()		);
		suInfo.setCmdLineParms		( config.getDebugSoftUpdateCmdLineParms()	);
		suInfo.setDownloadNow		( config.getDebugSoftUpdateDownloadNow()	);
	}
}

//-----------------------------------------------------------------------------
