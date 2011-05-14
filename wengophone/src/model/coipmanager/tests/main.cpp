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

#include <coipmanager/CoIpManager.h>
#include <filesessionmanager/ReceiveFileSession.h>
#include <filesessionmanager/SendFileSession.h>
#include <filesessionmanager/FileSessionManager.h>
#include <model/contactlist/Contact.h>
#include <model/contactlist/ContactProfile.h>
#include <model/profile/UserProfile.h>

#include <imwrapper/IMWrapperFactory.h>
#include <sipwrapper/SipWrapperFactory.h>
#include <multiim/MultiIMFactory.h>
#include <GaimIMFactory.h>
#include <PhApiFactory.h>

#include <util/File.h>
#include <util/Logger.h>
#include <util/SafeDelete.h>

static Contact * contact1 = NULL;
static Contact * contact2 = NULL;

void newReceiveFileSessionCreatedEventHandler(FileSessionManager & sender,
	ReceiveFileSession fileSession);

void moduleFinishedEventHandler(CoIpModule & sender);

int main(int argc, char **argv) {
	// Setting factories
	IMWrapperFactory * imFactory = NULL;
	SipWrapperFactory * sipFactory = NULL;

	PhApiFactory * phApiFactory = new PhApiFactory();
	GaimIMFactory * gaimIMFactory = new GaimIMFactory();
	sipFactory = phApiFactory;
	imFactory = new MultiIMFactory(*phApiFactory, *gaimIMFactory);

	SipWrapperFactory::setFactory(sipFactory);
	IMWrapperFactory::setFactory(imFactory);
	////

	UserProfile userProfile;
	CoIpManager coIpManager(userProfile);

	// Populating IMAccount list.
	IMAccount imAccount1("jacob@msn", "cobaj", EnumIMProtocol::IMProtocolMSN);
	userProfile.addIMAccount(imAccount1);
	////

	// Populating ContactList
	userProfile.getContactList().addContactGroup("test");
	std::string groupId = userProfile.getContactList().getContactGroupIdFromName("test");

	contact1 = &(userProfile.getContactList().createContact());
	contact1->setGroupId(groupId);
	IMContact imContact11(imAccount1, "bilibob@msn");
	IMContact imContact12(EnumIMProtocol::IMProtocolYahoo, "bilibob@yahoo");
	contact1->addIMContact(imContact11);
	contact1->addIMContact(imContact12);

	contact2 = &(userProfile.getContactList().createContact());
	contact2->setGroupId(groupId);
	IMContact imContact21(imAccount1, "jacobine@msn");
	IMContact imContact22(EnumIMProtocol::IMProtocolYahoo, "jacobine@yahoo");
	contact2->addIMContact(imContact21);
	contact2->addIMContact(imContact22);
	////

	FileSessionManager & fileSessionManager = coIpManager.getFileSessionManager();
	fileSessionManager.newReceiveFileSessionCreatedEvent +=
		&newReceiveFileSessionCreatedEventHandler;

	// Creating the file session
	SendFileSession * fileSession = fileSessionManager.createSendFileSession();

	File file("/Users/philou/Desktop/La_chanson_de_l_informatique.pps");
	LOG_DEBUG("size: " + String::fromNumber(file.getSize()));
	fileSession->addFile(file);

	fileSession->addContact(contact1->getUUID());
	fileSession->addContact(contact2->getUUID());

	fileSession->moduleFinishedEvent +=
		&moduleFinishedEventHandler;

	LOG_DEBUG("Starting FileSession");
	fileSession->start();
	////

	return 0;
}

void newReceiveFileSessionCreatedEventHandler(FileSessionManager & sender,
	ReceiveFileSession fileSession) {
	LOG_DEBUG("ReceiveFileSession created");
}

void moduleFinishedEventHandler(CoIpModule & sender) {
	LOG_DEBUG("FileSession finished");

	delete &sender;
}
