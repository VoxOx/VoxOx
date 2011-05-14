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

#ifndef OWWSSOFTUPDATE_H
#define OWWSSOFTUPDATE_H

#include <model/webservices/WengoWebService.h>

#include <util/String.h>

class UserProfile;

//==============================================================================
//VOXOX - JRT - 2009.10.05 - Let's create small class to pass around the SoftUpdate 
//							 data elements.  This will make it VERY easy to 
//							 modify going forward

class SoftUpdateInfo
{
public:

	enum EnumType
	{
		EnumType_Recommended = 0,
		EnumType_Mandatory	 = 1,
	};

	SoftUpdateInfo()
	{
		initVars();
	}

	//Gets -----------------------------------------------------------
	bool				isUpdateAvailable() const			{ return _updateAvailable;	}
	std::string			getDownloadUrl()	const			{ return _downloadUrl;		}
	std::string			getDescription()	const			{ return _description;		}
	std::string			getVersion()		const			{ return _version;			}
	std::string			getCmdLineParms()	const			{ return _cmdLineParms;		}
	std::string			getOS()				const			{ return _os;				}
	bool				downloadNow()		const			{ return _downloadNow;		}
	bool				isManualCheck()		const			{ return _manualCheck;		}
	unsigned long		getFileSize()		const			{ return _fileSize;			}
	unsigned long long	getBuildId()		const			{ return _buildId;			}

	unsigned long		getFileSizeMB()		const			{ return (_fileSize / 1024 / 1024);	}


	//Sets -----------------------------------------------------------
	void setUpdateAvailable( bool				val )			{ _updateAvailable	= val;	}
	void setDownloadUrl	   ( const std::string& val )			{ _downloadUrl		= val;	}
	void setDescription    ( const std::string& val )			{ _description		= val;	}
	void setVersion	       ( const std::string& val )			{ _version			= val;	}
	void setCmdLineParms   ( const std::string& val )			{ _cmdLineParms		= val;	}
	void setOS			   ( const std::string& val )			{ _os				= val;	}
	void setDownloadNow    ( bool				val )			{ _downloadNow		= val;	}
	void setManualCheck    ( bool				val )			{ _manualCheck		= val;	}
	void setFileSize       ( unsigned long      val )			{ _fileSize			= val;	}
	void setBuildId        ( unsigned long long val )			{ _buildId			= val;	}

	void setType( const std::string& val )
	{
		String temp = val;
		temp = temp.toLowerCase();

		if		( temp == "recommended" )	//New
		{
			_type = EnumType_Recommended;
		}
		else if (temp == "mandatory")		//New
		{
			_type = EnumType_Mandatory;
		}
		else if (temp == "may")				//Old
		{
			_type = EnumType_Recommended;
		}
		else if (temp == "must")			//Old
		{
			_type = EnumType_Mandatory;
		}
		else
		{
			assert(false);			//New type?
			_type = EnumType_Recommended;
		}
	}

	bool	isRecommended()	const				{ return (_type == EnumType_Recommended);	}
	bool	isMandatory()	const				{ return (_type == EnumType_Mandatory);		}

	SoftUpdateInfo& operator=( const SoftUpdateInfo& src )
	{
		if ( this != &src )
		{
			setUpdateAvailable( src.isUpdateAvailable() );
			setType			  ( src.getType()			);
			setDownloadUrl    ( src.getDownloadUrl()	);
			setDescription    ( src.getDescription()	);
			setVersion	      ( src.getVersion()		);
			setCmdLineParms   ( src.getCmdLineParms()	);
			setOS			  ( src.getOS()				);
			setDownloadNow    ( src.downloadNow()		);
			setManualCheck    ( src.isManualCheck()		);
			setFileSize       ( src.getFileSize()		);
			setBuildId        ( src.getBuildId()		);
		}

		return *this;
	}

protected:
	void initVars()
	{
		setUpdateAvailable( false );
		setType			( EnumType_Recommended );
		setDownloadUrl  ( "" );
		setDescription  ( "" );
		setVersion	    ( "" );
		setCmdLineParms ( "" );
		setOS			( "" );
		setDownloadNow  ( false );
		setManualCheck  ( false );
		setFileSize     ( 0 );
		setBuildId      ( 0 );
	}

	EnumType getType() const					{ return _type;	}
	void     setType( EnumType val )			{ _type = val;	}

private:
//	WsSoftUpdate		_sender;
	bool				_updateAvailable;
	EnumType			_type;
	std::string			_downloadUrl;
	std::string			_description;
	std::string			_version;
	std::string			_cmdLineParms;
	std::string			_os;
	bool				_downloadNow;
	bool				_manualCheck;
	unsigned long long	_buildId;
	unsigned long		_fileSize;
};

//=============================================================================

/**
 * WengoPhone update web service.
 *
 * Checks for the availability of WengoPhone updates.
 *
 * @author Tanguy Krotoff
 */
class WsSoftUpdate : public WengoWebService 
{
public:

	/**
	 * Default constructor.
	 *
	 * @param wengoAccount the WengoAccount used for web services
	 */
	WsSoftUpdate(WengoAccount * wengoAccount, UserProfile& userProfile);	//VOXOX - JRT - 2009.10.27 

	virtual ~WsSoftUpdate() {}

	/**
	 * Event WengoPhone should be updated.
	 *
	 * @param sender this class
	 * @param downloadUrl WengoPhone update download URL
	 * @param buildId WengoPhone update build ID
	 * @param version WengoPhone update version number
	 * @param fileSize WengoPhone update file size in kil bytes
	 */
	Event<void (WsSoftUpdate & sender, SoftUpdateInfo suInfo)> updateWengoPhoneEvent;	//VOXOX - JRT - 2009.10.05 
	Event<void (WsSoftUpdate & sender, SoftUpdateInfo suInfo)> noAvailableUpdateEvent;	//VOXOX - JRT - 2009.10.27 

	/**
	 * Checks if a WengoPhone update is available.
	 */
	void checkForUpdate( bool manualCheck );

private:
	/**
	 * @see WengoWebService
	 */
	void answerReceived (const std::string & answer, int id);

	bool shouldUpdate	( SoftUpdateInfo& suInfo );
	void loadDebugValues( SoftUpdateInfo& suInfo );		//VOXOX - JRT - 2009.11.05 
//	std::string getPlatform();									//VOXOX - JRT - 2009.11.23 


	UserProfile& _userProfile;		//VOXOX - JRT - 2009.10.27 
	bool		 _manualCheck;		//VOXOX - JRT - 2009.10.27 
};

#endif //OWWSSOFTUPDATE_H
