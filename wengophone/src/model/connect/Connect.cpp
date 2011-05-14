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

#include "stdafx.h"	//VOXOX - JRT - 2009.04.01
#include "Connect.h"

#include <imwrapper/IMAccount.h>
#include <imwrapper/IMWrapperFactory.h>

#include <model/network/NetworkObserver.h>
#include <model/profile/IMAccountManager.h>
#include <model/profile/UserProfile.h>

#include <util/Logger.h>
#include <util/SafeDelete.h>

//=============================================================================
//VOXOX - JRT - 2009.07.03 - simple class to handle connect retry loop counts and intervals
VoxConnectRetry::VoxConnectRetry()
{
	initVars();
}

VoxConnectRetry::~VoxConnectRetry()
{
}

void VoxConnectRetry::initVars()
{
	_retryCount = 0;

	_retryLevel1Max =  5;
	_retryLevel2Max = 10;

	_retryLevel1Interval   =   3000;	//ms
	_retryLevel2Interval   =  20000;	//ms
	_retryLevelMaxInterval = 180000;	//ms
}

int VoxConnectRetry::getRetryInterval()
{
	int result = 0;

	_retryCount++;

	if ( _retryCount <= _retryLevel1Max )
	{
		result = _retryLevel1Interval;
	}
	else if ( _retryCount <= _retryLevel2Max )
	{
		result = _retryLevel2Interval;
	}
	else
	{
		result = _retryLevelMaxInterval;
	}

	return result;
}

//=============================================================================


//============================================================================

Connect::Connect(IMAccount account, UserProfile & userProfile)
	: _imAccount(account),
	_userProfile(userProfile) 
{
	_imConnect = IMWrapperFactory::getFactory().createIMConnect(account);

	_imConnect->connectedEvent			+= boost::bind(&Connect::connectedEventHandler,			 this, _1);
	_imConnect->disconnectedEvent		+= boost::bind(&Connect::disconnectedEventHandler,		 this, _1, _2, _3, _4);	//VOXOX - JRT - 2009.07.13 
	_imConnect->connectionProgressEvent += boost::bind(&Connect::connectionProgressEventHandler, this, _1, _2, _3, _4);

	_timerIsRunning = false;
	_timer.timeoutEvent += boost::bind(&Connect::timeoutEventHandler, this, _1);

	_isConnecting    = false;
	_isDisconnecting = false;

	_apiTimer = NULL;
}

Connect::~Connect() 
{
	_timer.stop();
	delete _imConnect;

	OWSAFE_DELETE( _apiTimer );
}

void Connect::connect() 
{
	_isDisconnecting = false;

	autoConnect();
}

void Connect::disconnect(bool force) 
{
	_timer.stop();
	_timerIsRunning = false;
	_isConnecting  = false;

	// Gets up-to-date IMAccount
	//VOXOX -ASV- 07-14-2009: we can't get just a copy since we will need to set the presence state
	IMAccount * imAccount = _userProfile.getIMAccountManager().getIMAccountNoClone( _imAccount );	

	if (imAccount) 
	{
		if (imAccount->isConnected()) 
		{
			_isDisconnecting = true;

			if ( _imAccount.isSip() )
			{
				_userProfile.disconnectSipAccounts(force);
			}

			_imConnect->disconnect(force);
			// VOXOX -ASV- 07-14-2009: We need to set the new presence state of the 
			//imAccount so it doesn't reconnect after putting it offline
//			imAccount->setPresenceState(EnumPresenceState::PresenceStateOffline);
//			imAccount->updateInitialPresenceState(EnumPresenceState::PresenceStateOffline);
		}

		//VOXOX - JRT - 2009.09.14 - Let's just NOT delete it.
//		imAccount = NULL; // VOXOX -ASV- 07-14-2009: point it to null before delete it because this is the real object not a copy 
//		OWSAFE_DELETE(imAccount);
	}
}

void Connect::timeoutEventHandler(Timer & sender) 
{
	Mutex::ScopedLock lock(_mutex);
	bool stopToggle = false;

	if (_isConnecting) 
	{
		return;
	}

	IMAccount * imAccount = _userProfile.getIMAccountManager().getIMAccount( _imAccount );

	if (imAccount) 
	{
		// Check if the IMAccount is not already connected. This can happen
		// if a timeout event is emitted during a connection process.
		if (!imAccount->isConnected() )	
		{
			if ( imAccount->shouldAutoReconnect() )
			{
				if ( NetworkObserver::getInstance().isConnected()) //VOXOX - JRT - 2009.07.03 - Using VoxConnectRetry class.
				{
					startApiTimer( "Connect::timeoutEventHandler()" );	//VOXOX - JRT - 2009.09.15 

					_isConnecting = true;

					if ( _imAccount.isSip() )
					{
						if (!_userProfile.connectSipAccounts()) 
						{
							LOG_ERROR("Couldn't connect sip accounts");
							stopToggle = true;
						}
					} 
					else 
					{
						_imConnect->connect();
					}
				} 
				else 
				{
					//VOXOX CHANGE by Rolando - 2009.07.28 - I had to comment this sentence because every time I tested to make a disconnection 
					//of the net cable while VoxOx was running it crashed because of this assert
					//assert(false);	//VOXOX - JRT - 2009.07.03 - should never get here because we now try FOREVER!

					//if (_connectionRetryCount >= RECONNECT_RETRY) {
					//	LOG_ERROR("Too many attempts to connect");
					//} else {
					//	LOG_ERROR("Not connected");
					//}
					//stopToggle = true;
				}
			}
			else
			{
				LOG_ERROR("IMAccount " + imAccount->getLogin() +  " flagged to NOT auto-reconnect.");
				stopToggle = true;
			}


			if (stopToggle) 
			{
				// Either the user has cancelled the connection or the retry count has been exceeded
				_timer.stop();
				_timerIsRunning = false;
			}
		}

		OWSAFE_DELETE(imAccount);
	}
}

void Connect::autoConnect() 
{
	if ( !_timerIsRunning )
	{
		//VOXOX - JRT - 2009.09.15 - Determine if some networks take longer to connect to.
		LOG_INFO( "Connecting to: " + _imAccount.getKey() );

		_timerIsRunning = true;
		_timer.start( 0, _connectRetry.getRetryInterval(), 0 );	//VOXOX - JRT - 2009.07.03 
	}
}

void Connect::connectedEventHandler(IMConnect & sender) 
{
	stopApiTimer( sender.getIMAccountId() );	//VOXOX - JRT - 2009.09.15 

	connectedEvent(sender);
	
	_timer.stop();
	_timerIsRunning = false;
	_isConnecting = false;
}

void Connect::disconnectedEventHandler(IMConnect & sender, bool connectionError, std::string reason, bool fatal ) 	//VOXOX - JRT - 2009.07.13 
{
	disconnectedEvent(sender, connectionError, reason, fatal );	//VOXOX - JRT - 2009.07.13 

	_isConnecting = false;

	if (_isDisconnecting) 
	{
		return;
	}

	// We check if this is a user disconnection or a error disconnection.
	// If so, we try to reconnect
	if (connectionError) 
	{
		autoConnect();
	} 
	else 
	{
		_timer.stop();
		_timerIsRunning = false;
		_connectRetry.reset();		//VOXOX - JRT - 2009.07.03 
	}
}

void Connect::connectionProgressEventHandler(IMConnect & sender, int currentStep, int totalSteps, std::string infoMessage) 
{
	connectionProgressEvent(sender, currentStep, totalSteps, infoMessage);
}

void Connect::unregisterAllEvents() 
{
	_imConnect->connectedEvent			-= boost::bind(&Connect::connectedEventHandler,			 this, _1);
	_imConnect->disconnectedEvent		-= boost::bind(&Connect::disconnectedEventHandler,		 this, _1, _2, _3, _4);	//VOXOX - JRT - 2009.07.13 
	_imConnect->connectionProgressEvent -= boost::bind(&Connect::connectionProgressEventHandler, this, _1, _2, _3, _4);

	_timer.timeoutEvent -= boost::bind(&Connect::timeoutEventHandler, this, _1);
}

void Connect::startApiTimer( const std::string& method )
{
	if ( _apiTimer == NULL )
	{
		_apiTimer = new VoxApiTimer( method, "connect()", false );
	}

	_apiTimer->Start( _imAccount.getKey() );
}

void Connect::stopApiTimer( const std::string& accountId )
{
	if ( _apiTimer )
	{
		_apiTimer->Stop( accountId );
		delete _apiTimer;
		_apiTimer = NULL;
	}
}
