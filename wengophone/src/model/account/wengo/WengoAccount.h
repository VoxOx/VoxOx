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

#ifndef OWWENGOACCOUNT_H
#define OWWENGOACCOUNT_H

#include <model/account/SipAccount.h>
#include <model/webservices/WengoWebService.h>

#include <thread/Timer.h>

#include <util/Event.h>

class NetworkObserver;

/**
 * Connects to the single sign-on (SSO) system of the Wengo SIP service.
 *
 * @ingroup model
 * @author Tanguy Krotoff
 * @author Philippe Bernery
 * @author Mathieu Stute
 * @author Xavier Desjardins
 */
class WengoAccount : public SipAccount, public WengoWebService {

	friend class WengoAccountXMLSerializer;

	friend class WengoAccountParser;

public:
	WengoAccount();

	WengoAccount(const std::string & login, const std::string & password, bool rememberPassword);

	virtual ~WengoAccount();

	WengoAccount(const WengoAccount & wengoAccount);

	virtual WengoAccount * clone() const;

	virtual WengoAccount & operator = (const WengoAccount & wengoAccount);
	
	virtual bool operator == (const SipAccount & other) const;

	//virtual void init();

	/** Gets the Wengo login (e-mail address). */
	const std::string & getWengoLogin() const { return _wengoLogin; }

	/** Gets the Wengo Password. */
	const std::string & getWengoPassword() const { return _wengoPassword; }

	/** Sets the Wengo login (e-mail address). */
	void setWengoLogin(const std::string newlogin) { _wengoLogin = newlogin; }

	/** Sets the Wengo Password. */
	void setWengoPassword(const std::string newpass) { _wengoPassword = newpass; }
	
	virtual const std::string getUserProfileName() const {
		return _wengoLogin;
	}
	
	/**
	 * @return password set by user
	 */
	virtual const std::string getUserPassword() const { return _wengoPassword; }

	/**
	 * @return the full identity
	 * e.g: robobob666@voip.wengo.fr
	 */
	virtual const std::string getFullIdentity() const {
		return _wengoLogin;
	}

	virtual const std::string getVisibleName() const {
		return _wengoLogin;
	}

	virtual SipAccountType getType() const {
		return SipAccount::SipAccountTypeWengo;
	}

	/**
	 * @return true if empty
	 */
	virtual bool isEmpty() const;

	bool usingEncryption() const			{ return _useEncryption;	}

private:
	void initVars();
	void copy(const WengoAccount & wengoAccount);

	virtual EnumSipLoginState::SipLoginState discoverNetwork();

	virtual std::string getLoginErrorMessage(){ return _loginErrorMessage; }//VOXOX CHANGE by Rolando - 2009.09.10 

	/**
	 * Discovers network for SSO request.
	 *
	 * Please contact network@openwengo.com before any modifications.
	 */
	bool discoverForSSO();

	/**
	 * Discovers network for SIP.
	 *
	 * Please contact network@openwengo.com before any modifications.
	 */
	bool discoverForSIP();

	void wengoLoginEventHandler();

	void answerReceived(const std::string & answer, int requestId);

	void ssoTimeoutEventHandler();

	void ssoLastTimeoutEventHandler();

	std::string _wengoLogin;

	std::string _wengoPassword;

	Timer _ssoTimer;

	/** True if SSO request can be done with SSL. */
	bool _ssoWithSSL;

	/** True if SSO request is Ok. */
	bool _ssoRequestOk;

	/** True if SSO request is Ok and login/password are valid. */
	bool _wengoLoginOk;

	std::string _statusCode;//VOXOX CHANGE by Rolando - 2009.09.10 

	/** True when _timer is finished. */
	bool _ssoTimerFinished;

	/** Number of testStun retry. */
	static unsigned short _testStunRetry;

	bool		_useEncryption;		//VOXOX - JRT - 2009.09.28 
};

#endif	//OWWENGOACCOUNT_H
