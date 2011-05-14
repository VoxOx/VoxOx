/*
* VoxOx, Take Control
* Copyright (C) 2004-2009  VoxOx

* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
/**
* CLASS DESCRIPTION 
* @author Chris Jimenez C 
* @date 2009.08.17
*/




#ifndef OWQTUMHELPER_H
#define OWQTUMHELPER_H

class QWidget;
class IMChatSession;
class IMContact;
class QtUMChat;
class QtUMEmail;
class QtUMSMS;
class QtUMCall;
class QtContactCallListWidget;
#include "QtUMBase.h"
#include <QtCore/QObject>
#include <control/chat/CChatHandler.h>
#include <control/contactlist/CContactList.h>
#include <imwrapper/IMChat.h>

class QtUMHelper : public QObject {
	Q_OBJECT
public:

	QtUMHelper(CChatHandler & cChatHandler, CContactList & cContactList);

	virtual ~QtUMHelper();

	void handleIMChatSessionCreated(IMChatSession * imChatSession);
	bool exist(QtUMBase * item);
	//VOXOX - CJC - 2009.08.26 UMChat methods
	
	void openUMChatSession(IMChatSession * imChatSession);
	bool existUMChat(IMChatSession * imChatSession);
	//VOXOX - CJC - 2009.08.26 End UMChat methods
	//VOXOX - CJC - 2009.08.26 UMEmail methods
	void handleUMEmailSessionCreated(IMChatSession * imChatSession,const QString & email);
	void openUMEmailSession(IMChatSession * imChatSession,const QString & relatedContactId,const QString & emailAddress, bool userCreated);
	//VOXOX - CJC - 2009.08.27 End UMEMail methods
	//VOXOX - CJC - 2009.08.27 UMSMS methods
	void handleUMSMSSessionCreated(IMChatSession * imChatSession,const QString & number);
	void openUMSMSSession(IMChatSession * imChatSession,const QString & relatedContactId,const QString & number, bool userCreated);

	void createUMSendFile(const IMContact & from); 

	void createUMFax(const QString & faxNumber); 

	void createUMCall(const QString & phoneNumber,QtContactCallListWidget * qtContactCallListWidget); 

	void hangUpCall(const QString & key);

	QtUMBase * getUMItemByKey(const QString & key);

	void deleteUMItemByKey(const QString & key);

	QString getContactIdByKey(const QString & key);

	void handleAddToChat(const QString & key); 

	void sendMessageToChat(IMChatSession * sender,const QString & message);


public Q_SLOTS:
	void messageReceivedSlot(IMChatSession * sender);
	void typingStateChangedThreadSafe(const IMChatSession* sender, const IMContact * imContact,const IMChat::TypingState * state);

	void contactAddedEventSlot(IMChatSession * sender, const IMContact & imContact);
	void contactRemovedEventSlot(IMChatSession * sender, const IMContact & imContact);

	void statusChangedSlot(QString contactId);

Q_SIGNALS:

	void messageReceivedSignal        (IMChatSession * sender);
	void typingStateChangedSignal     ( const IMChatSession* sender, const IMContact * imContact, const IMChat::TypingState * state);

	void contactAddedEventSignal(IMChatSession * sender , const IMContact & imContact);
	void contactRemovedEventSignal(IMChatSession * sender , const IMContact & imContact);


	//VOXOX - CJC - 2009.08.27  UI Notification Signals
	void addUMItemSignal			  (QtUMBase * item,bool userCreated);//VOXOX - CJC - 2009.08.26 This notifys the UI to add an item
	void setTyping				      (QtUMBase * item,bool typing);//VOXOX - CJC - 2009.08.26 This notifys that is typing
	void messageReceivedSignal	      (QtUMBase * item);//VOXOX - CJC - 2009.08.26 This notifys that is typing
	void statusChangedSignal		  (QtUMBase * item);
	void contactAddedSignal			  (QtUMBase * item,const QString & contactId,const QString & contactKey);
	void contactRemovedSignal		  (QtUMBase * item,const QString & contactId,const QString & contactKey);

private:
	//VOXOX - CJC - 2009.08.27 IMChat session handlers

	void addUMItem(QtUMBase * item,bool userCreated);

	void messageReceivedEventHandler     (IMChatSession& sender);
	void typingStateChangedEventHandler  (IMChatSession& sender, const IMContact & imContact, IMChat::TypingState state);
	void imChatSessionWillDieEventHandler(IMChatSession& sender);
	void contactAddedEventHandler(IMChatSession & sender, const IMContact & imContact);
	void contactRemovedEventHandler(IMChatSession & sender, const IMContact & imContact);


	CChatHandler & _cChatHandler;

	CContactList & _cContactList;

	void createUMChat(IMChatSession * imChatSession);

	void createUMEmail(IMChatSession * imChatSession, const IMContact * from, const QString & emailAddress, bool getMessages, bool userCreated);

	void createUMSMS(IMChatSession * imChatSession, const IMContact * from, const QString & number, bool getMessages, bool userCreated);
	


	void messageRecievedUMEmail(QString message, IMChatSession * imChatSession);//VOXOX - CJC - 2009.08.27 

	void messageRecievedUMSMS(QString message, IMChatSession * imChatSession);//VOXOX - CJC - 2009.08.27 


	//VOXOX - CJC - 2009.08.27 Convenient Methods

	QString getShortDisplayName(const QString & contactId, const QString & defaultName);

	
	//VOXOX - CJC - 2009.08.27 End Of Convenient Methods

	//VOXOX - CJC - 2009.08.27 GenerateKey Methods, This should be use as the map key, a quick way to get the proper UMItem Objet

	QString genereteUMSMSKey(const QString & number);

	QString genereteUMEmailKey(const QString & email);
	
	QString genereteUMChatKeyBySession(const IMChatSession * session);

	QString genereteUMGroupChatKeyByGroupName(const QString & groupName);//VOXOX - CJC - 2009.09.24 

	QString genereteUMChatKeyByContactId(const QString & contactId);

	QString genereteUMSendFileKey(const QString & contactId);

	QString genereteUMFaxKey(const QString & faxNumber);

	QString generateKey(const QString & id, QtEnumUMMode::Mode mode);

	//VOXOX - CJC - 2009.08.27 End of GenerateKey Methods

	typedef std::map < QString, QtUMBase * > UMBaseMap;

	UMBaseMap _umBaseMap;

	std::map<int, int> _lastReceivedMessageIndex;

	bool existInMap(QString key);

	bool existInMap(QtUMBase * item);

	bool existByChatSession(IMChatSession * session);

	bool addUMItemToMap(QtUMBase * item);

	QtUMChat * getUMChatByKey(const QString & key);

	QtUMChat * getUMChatBySession(const IMChatSession * session);

	QtUMChat * getUMChatByContactId(const QString & id);

	QtUMEmail * getUMEmailByEmail(const QString & email);

	QtUMCall * getUMCallByKey(const QString & key);

	QtUMSMS* getUMSMSByNumber(const QString & number);

};

#endif //OWQtUMHelper_H
