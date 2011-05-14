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

#ifndef GAIMIMPRESENCE_H
#define GAIMIMPRESENCE_H

#include <imwrapper/IMPresence.h>
#include <thread/Mutex.h>

#include "GaimEnumIMProtocol.h"

typedef struct AuthRequest_s AuthRequest;

/**
 * Stub for Instant Messaging presence.
 *
 * @ingroup model
 * @author Julien Bossart
 */
class GaimIMPresence : public IMPresence 
{
	friend class GaimIMFactory;

public:

	GaimIMPresence(IMAccount account);

	virtual ~GaimIMPresence();

	virtual void changeMyPresence(EnumPresenceState::PresenceState state, const std::string & note);
	virtual void changeMyAlias(const std::string & nickname);
	virtual void changeMyIcon(const OWPicture & picture);
	virtual void subscribeToPresenceOf(const std::string & contactId);
	virtual void unsubscribeToPresenceOf(const std::string & contactId);
	virtual void blockContact(const std::string & contactId);
	virtual void unblockContact(const std::string & contactId);
	virtual void authorizeContact(const std::string & contactId, bool authorized, const std::string message);

	static const char *getGaimPresenceId(void *gaimPresence);
	void addAuthRequestInList(const std::string & contactId, void *acceptCbk, void *denyCbk, void *userData);

	const IMAccount & getIMAccount() const { return _imAccount; }

	static bool changeMyPresenceCbk(void * data);
	static bool changeMyAliasCbk(void * data);
	static bool changeMyIconCbk(void * data);
	static bool subscribeToPresenceOfCbk(void * data);
	static bool unsubscribeToPresenceOfCbk(void * data);
	static bool blockContactCbk(void * data);
	static bool unblockContactCbk(void * data);
	static bool authorizeContactCbk(void * data);

	std::map<const std::string, AuthRequest *> authRequestMap;

	static Mutex _mutex;
};

#endif	//GAIMIMPRESENCE_H
