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

#ifndef OWIMACCOUNT_H
#define OWIMACCOUNT_H

#include <imwrapper/Account.h>
#include <imwrapper/EnumIMProtocol.h>
#include <imwrapper/EnumPresenceState.h>
#include <imwrapper/IMAccountParameters.h>
#include <imwrapper/QtEnumIMProtocol.h>

#include <util/Event.h>
#include <util/Trackable.h>
#include <util/NonCopyable.h>


#include <string>

/**
 * Instant Messaging account.
 *
 * Associates a login/password to an IM protocol.
 * Example: login: "bob@hotmail.com" password: "axBi9opZ" protocol: IMProtocolMSN
 *
 * @author Tanguy Krotoff
 * @author Philippe Bernery
 */
class IMAccount : public Account, public Trackable 
{
//	friend class IMAccountXMLSerializer;
//	friend class IMAccountXMLSerializer1;
public:

	/**
	 * Emitted when the IMAccount has been changed.
	 *
	 * @param sender this class
	 */
	Event< void (IMAccount & sender) > imAccountChangedEvent;

	IMAccount();
	IMAccount(EnumIMProtocol::IMProtocol protocol);
	IMAccount(const std::string & login, const std::string & password, EnumIMProtocol::IMProtocol protocol, const std::string & statusMessage = "");//VOXOX CHANGE by Rolando - 2009.07.01 
	IMAccount(const std::string & login, const std::string & password, EnumIMProtocol::IMProtocol protocol,const bool & voxoxAccount, const std::string & statusMessage = "");//VOXOX CHANGE by Rolando - 2009.07.01 
	IMAccount(const IMAccount & imAccount);

	virtual ~IMAccount();

	IMAccount & operator = (const IMAccount & imAccount);

	virtual IMAccount * clone() const;


	const std::string&					getLogin()				const			{ return _login;			}
	const std::string&					getPassword()			const			{ return _password;			}
	const std::string&					getStatusMessage()		const			{ return _statusMessage;	}	//VOXOX CHANGE by Rolando - 2009.07.01 
	EnumPresenceState::PresenceState	getPresenceState()		const			{ return _presenceState;	}
	EnumPresenceState::PresenceState	getInitialPresenceState()	const		{ return _initialPresenceState;	}
	bool								isConnected()			const			{ return _connected;		}
	bool								shouldAutoReconnect()	const			{ return _autoReconnect;	}
	IMAccountParameters&				getIMAccountParameters()				{ return _imAccountParameters;	}
	int									getUserNetworkId()		const			{ return _userNetworkId;	}
	std::string							getServer()				const			{ return _server;			}
	QtEnumIMProtocol::ServerProtocolId	getServerProtocolId()	const			{ return _serverProtocolId;	}

	void setLogin			( const std::string& login			);
	void setPassword		( const std::string& password		);
	void setStatusMessage	( const std::string& statusMessage	);		//VOXOX CHANGE by Rolando - 2009.07.01 
	void setIsVoxoxAccount	( const bool& voxoxAccount			);
	void setPresenceState	( EnumPresenceState::PresenceState presenceState)			{ _presenceState		= presenceState;	}
	void setInitialPresenceState( EnumPresenceState::PresenceState presenceState)		{ _initialPresenceState	= presenceState;	}
	void setConnected		( bool connected					)						{ _connected			 = connected;		}
	void setAutoReconnect	( bool val )												{ _autoReconnect		 = val;				}
	void setIMAccountParameters( IMAccountParameters& val )								{ _imAccountParameters	 = val;				}
	void setUserNetworkId	( int val )													{ _userNetworkId		 = val;				}
	void setServer			( const std::string& val )									{ _server				 = val;				}
	void setServerProtocolId( QtEnumIMProtocol::ServerProtocolId val )					{ _serverProtocolId		 = val;				}

	void updateInitialPresenceState( EnumPresenceState::PresenceState presenceState);

	bool shouldConnect() const;		//VOXOX - JRT - 2009.09.08 
	bool isOnline()		 const							{ return (getPresenceState() == EnumPresenceState::PresenceStateOnline );	}	//VOXOX - JRT - 2009.07.04 
	bool isOffline()	 const							{ return (getPresenceState() == EnumPresenceState::PresenceStateOffline);	}	 
	bool isOfflineEx()	 const							{ return EnumPresenceState::isOfflineEx( getPresenceState() );				}	//VOXOX - JRT - 2009.09.10 
	bool isSip()		 const							{ return EnumIMProtocol::isSip( getProtocol() );							}	//VOXOX - JRT - 2009.07.16 



	/** 
	 * @return true if this IMAccount is empty.
	 */
	bool empty() const;

	bool isGtalkAccount() const;
	bool isVoxOxAccount() const;
	
	bool operator == (const IMAccount& imAccount) const;
	
	std::string getDisplayAccountId() const;
	std::string getCleanLogin() const;

	std::string	getKey() const;	
	virtual void setProtocol( EnumIMProtocol::IMProtocol val );


	QtEnumIMProtocol::IMProtocol		getQtProtocol()		 const;
	QtEnumIMProtocol::ServerProtocolId	toServerProtocolId() const;

	bool isOldVoxOxDomain();					//VOXOX - JRT - 2009.07.13 
	std::string getDomain() const;				//VOXOX - JRT - 2009.07.13 

	bool needsSyncing() const								{ return _needsSyncing;	}
	void setNeedsSyncing( bool val )						{ _needsSyncing = val;	}
	bool shouldSync() const;

	void changeLogin(std::string newLogin);		//VOXOX CHANGE by Rolando - 2009.08.28 

	std::string getUseableStatusMsg ( const std::string newStatusMessage );
	bool		isOkToChangePresence( EnumPresenceState::PresenceState newPresenceState );

private:
	void initVars();
	void updateKey();
	
	/** Copy an IMAccount in this object. */
	void copy(const IMAccount & imAccount);

	/**
	 * This method exists because of Jabber that uses contactId with a 'Resource'
	 * e.g.: 'blabla@jabber.org/Resource'.
	 *
	 * It will add '/WengoPhone' to the login id if the login does not contain
	 * a Resource.
	 *
	 * @param login the old login
	 * @return the corrected login
	 */
	static std::string correctedLogin(const std::string & login, EnumIMProtocol::IMProtocol protocol);

	/**
	 * @see Settings::valueChangedEvent
	 */
	void valueChangedEventHandler(const std::string & key);

	std::string _statusMessage;		//VOXOX CHANGE by Rolando - 2009.07.01 
	std::string _login;				//VOXOX - JRT - 2009.04.24 - TODO: should be a class (JabberID, UserID) to handle userid/domain.
	std::string _password;
	bool		_voxoxAccount;
	std::string	_key;
	int									_userNetworkId;
	QtEnumIMProtocol::ServerProtocolId	_serverProtocolId;	//VOXOX - JRT - 2009.06.01 - for syncing.
	std::string	_server;									//VOXOX - JRT - 2009.06.01 - added to make it easier to sync with server.

	bool		_autoReconnect;		//VOXOX - JRT - 2009.07.05 
	bool		_needsSyncing;		//VOXOX - JRT - 2009.07.14 


	/** This Settings is used to contain more parameters (eg.: "use_http => true" for MSN protocol). */
	IMAccountParameters _imAccountParameters;

	/**
	 * Desired PresenceState for this IMAccount.
	 * NOTE:  this is no longer true.   If PresenceState is Offline, the IMAccount will not connect automatically.
	 */
	EnumPresenceState::PresenceState _presenceState;			//VOXOX - JRT - 2009.09.09 - This is current presence. Period.
	EnumPresenceState::PresenceState _initialPresenceState;		//VOXOX - JRT - 2009.09.09 - This is presence to be restored.

	/** Connection state. */
	bool _connected;

	static std::string _oldVoxOxDomain;
};

#endif	//OWIMACCOUNT_H
