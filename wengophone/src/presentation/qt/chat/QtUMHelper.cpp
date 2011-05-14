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



#include "stdafx.h"		

#include "QtUMHelper.h"
#include "QtUMChat.h"
#include "QtUMEmail.h"
#include "QtUMSMS.h"
#include "QtUMSendFile.h"
#include "QtUMFax.h"
#include "QtUMCall.h"
#include "QtChatToEmailSMSUtils.h"

#include <control/profile/CUserProfile.h>
#include <control/profile/CUserProfileHandler.h>

#include <model/profile/UserProfile.h>
#include <model/config/Config.h>
#include <model/config/ConfigManager.h>

#include <imwrapper/IMChatSession.h>
#include <imwrapper/IMContact.h>
#include <imwrapper/IMAccount.h>

#include <presentation/qt/contactlist/QtContactList.h>
#include <presentation/qt/phonecall/QtContactCallListWidget.h>
#include <QtCore/QString>

#include "chatroom/QtChatRoomInviteDlg.h"

#include <util/SafeDelete.h>
#include <qtutil/SafeConnect.h>
#include <util/Logger.h>
QtUMHelper::QtUMHelper(CChatHandler & cChatHandler, CContactList & cContactList) : QObject(),
	_cChatHandler(cChatHandler),
	_cContactList(cContactList) {


	SAFE_CONNECT_TYPE(this, SIGNAL(typingStateChangedSignal(const IMChatSession *, const IMContact *, const  IMChat::TypingState *)),
		SLOT(typingStateChangedThreadSafe(const IMChatSession *, const IMContact *, const IMChat::TypingState *)), Qt::QueuedConnection);

	SAFE_CONNECT_TYPE(this, SIGNAL(messageReceivedSignal(IMChatSession *)),
		SLOT(messageReceivedSlot(IMChatSession *)), Qt::QueuedConnection);

	SAFE_CONNECT_TYPE(dynamic_cast<QtContactList *>(_cContactList.getPresentation()),
		SIGNAL(contactChangedEventSignal(QString )), SLOT(statusChangedSlot(QString)),Qt::QueuedConnection);//VOXOX - CJC - 2009.08.31 Ugly, but only way to do it I think, It will be good to trigger this from cContactlist

	SAFE_CONNECT_TYPE(this, SIGNAL(contactAddedEventSignal(IMChatSession *,const IMContact &)),
		SLOT(contactAddedEventSlot(IMChatSession *, const IMContact &)), Qt::QueuedConnection);
	SAFE_CONNECT_TYPE(this, SIGNAL(contactRemovedEventSignal(IMChatSession *, const IMContact &)),
		SLOT(contactRemovedEventSlot(IMChatSession *, const IMContact &)), Qt::QueuedConnection);
	
}

QtUMHelper::~QtUMHelper() {

}

void QtUMHelper::addUMItem(QtUMBase * item, bool userCreated){

	addUMItemToMap(item);
	addUMItemSignal(item,userCreated);

}

//VOXOX - CJC - 2009.08.26 General Functions

void QtUMHelper::handleIMChatSessionCreated(IMChatSession * imChatSession){

	//VOXOX - CJC - 2009.05.08 Check if this chat session correspond to an actual chat and not to a chat to email;
	//We check if it was user created, this means that we intentionally created a chat session to a user. Chat To Email, Chat to SMS had their own triggers for this event
	if(imChatSession->isUserCreated()){
		openUMChatSession(imChatSession);
	}else{
		Config & config = ConfigManager::getInstance().getCurrentConfig();
		IMContact from = *imChatSession->getIMContactSet().begin();
		//Check if session is from ask.im.voxox.com
		if(from.getContactId() == config.getAskIMVoxOxComAddress()){
			
			if(_lastReceivedMessageIndex.find(imChatSession->getId()) ==_lastReceivedMessageIndex.end()){
				_lastReceivedMessageIndex[imChatSession->getId()] = -1;
			}
			QString chatMessage = "";
			IMChatSession::IMChatMessageList imChatMessageList = 
			imChatSession->getReceivedMessage(_lastReceivedMessageIndex[imChatSession->getId()] + 1);
			//Check if session contains messages
			if (imChatMessageList.size() > 0){
				IMChatSession::IMChatMessageList::iterator imChatMessageListIterator = imChatMessageList.begin();
				while (imChatMessageListIterator != imChatMessageList.end()){
					IMChatSession::IMChatMessage * imChatMessage = * imChatMessageListIterator;
					chatMessage = QString::fromStdString(imChatMessage->getMessage());
					imChatMessageListIterator++;
				}
				if(chatMessage!=""){
					QtChatToEmailSMSUtils::Type type = QtChatToEmailSMSUtils::checkMesageType(chatMessage);
					//Check Message content to determine what type of message we have.
					if(type == QtChatToEmailSMSUtils::ChatToEmail){
						QtChatToEmailMessageData data = QtChatToEmailSMSUtils::getChatToMailMessageData(chatMessage);
						//VOXOX - CJC - 2009.06.13 
						QString contactId = "";
						Contact * from = _cContactList.getContactList().getContactByEmail(data.getFrom().toStdString());
						if(from){
							contactId = QString::fromStdString(from->getKey());
						}
						if(data.getFrom()!=""){
							openUMEmailSession(imChatSession,contactId,data.getFrom(),false);
						}

					}else if(type == QtChatToEmailSMSUtils::ChatToSMS){

						QtChatToSMSMessageData data = QtChatToEmailSMSUtils::getChatToSMSMessageData(chatMessage);
						//VOXOX - CJC - 2009.06.13 
						QString contactId = "";
						Contact * from = _cContactList.getContactList().getContactByNumber(data.getFrom().toStdString());
						if(from){
							contactId = QString::fromStdString(from->getKey());
						}
						if(data.getFrom()!=""){
							openUMSMSSession(imChatSession,contactId,data.getFrom(),false);
						}

					}else{
						imChatSession->close();
					}
				}else{
					imChatSession->close();
				}
			}else{
				imChatSession->close();
			}

		}else{
			openUMChatSession(imChatSession);
		}
	
	}
}

void QtUMHelper::statusChangedSlot(QString contactId) 
{
				
		QtUMChat * chat = getUMChatByContactId(contactId);//VOXOX - CJC - 2009.08.31 DO WE WANT THIS ONLY FOR CHAT?
		if (chat) 
		{
				//VOXOX - JRT - 2009.07.26 

				bool bConnected = _cContactList.isContactOnline( contactId.toStdString() );	//VOXOX - JRT - 2009.07.26 

				//This is what we want to do.
				chat->setContactConnected(bConnected);

				statusChangedSignal(chat);
		
		}		

}


QString QtUMHelper::generateKey(const QString & id, QtEnumUMMode::Mode mode){
	QString key;
	key = id+"@"+QtEnumUMMode::toString(mode);
	return key;
}

//VOXOX - CJC - 2009.08.26 UMChatMethods

void QtUMHelper::openUMChatSession(IMChatSession * imChatSession)
{

//	// If this chat session already exists, display the tab
//		int tabs = _universalMessageTabWidget->count();
//		for (int i = 0; i < tabs; i++) {
//			//VOXOX CHANGE CJC VALIDATE THAT IT ONLY DO THIS FOR CHAT WIDGET MODE
//			if(getWidgetMode(i) == QtEnumUMMode::UniversalMessageChat){
//				QtChatWidget * widget = dynamic_cast<QtChatWidget *>(_universalMessageTabWidget->widget(i));
//				if (widget->getSessionId() == imChatSession->getId()) {
//					_universalMessageTabWidget->setCurrentIndex(i);
//					show();
//					return;
//				}
//			}
//		}
//
		


		if (imChatSession->getIMContactSet().size() != 0 ) 
		{
				if(!existUMChat(imChatSession)){
					createUMChat(imChatSession);
				}else{
					//TODO: ADD SHOW IF Exist instead Notify UI
				}
		} 
		else 
		{
			LOG_FATAL("New chat session is empty!");
		}
		
}



QString QtUMHelper::genereteUMChatKeyBySession(const IMChatSession * session){

	IMContact from = *session->getIMContactSet().begin();
	QString contactId = QString::fromStdString(_cContactList.findContactThatOwns(from));
	return generateKey(contactId,QtEnumUMMode::UniversalMessageChat);
}

QString QtUMHelper::genereteUMGroupChatKeyByGroupName(const QString & groupName){

	QString id = groupName + "GroupChat";
	return generateKey(id,QtEnumUMMode::UniversalMessageChat);
}

QString QtUMHelper::genereteUMChatKeyByContactId(const QString & contactId){

	return generateKey(contactId,QtEnumUMMode::UniversalMessageChat);
}

void QtUMHelper::createUMChat(IMChatSession * imChatSession){


		imChatSession->messageReceivedEvent			+= boost::bind(&QtUMHelper::messageReceivedEventHandler,			this, _1);
		imChatSession->imChatSessionWillDieEvent	+= boost::bind(&QtUMHelper::imChatSessionWillDieEventHandler,		this, _1);
		imChatSession->typingStateChangedEvent		+= boost::bind(&QtUMHelper::typingStateChangedEventHandler,		this, _1, _2, _3);
		imChatSession->contactAddedEvent   += boost::bind(&QtUMHelper::contactAddedEventHandler,   this, _1, _2);//VOXOX - CJC - 2009.09.24 
		imChatSession->contactRemovedEvent += boost::bind(&QtUMHelper::contactRemovedEventHandler, this, _1, _2);//VOXOX - CJC - 2009.09.24 
		IMContact from = *imChatSession->getIMContactSet().begin();

		QString contactId = QString::fromStdString(_cContactList.findContactThatOwns(from));

		QString tmpNickName = "";
		QString nickName = "";
		QString senderName = "";
		QString key = "";

		IMAccount * imAccount = _cChatHandler.getCUserProfile().getUserProfile().getIMAccountManager().getIMAccount(imChatSession->getIMChat().getIMAccountId());
		if (imAccount) 
		{
			tmpNickName = QString::fromUtf8(imAccount->getDisplayAccountId().c_str());	
			OWSAFE_DELETE(imAccount);
		} 
		else 
		{
			LOG_ERROR("cannot get associated IMAccount");
			return;
		}
		
		nickName = tmpNickName;

		if( imChatSession->getIMChatType() == IMChat::ChatGroup  )
		{
		
			QString groupName = QString::fromStdString(imChatSession->getGroupChatInfo().getChatRoomDisplayName().c_str());
			if ( groupName.isEmpty() )
			{
				//senderName = getShortDisplayName(contactId, QString::fromStdString(from.getDisplayContactId()));
				return;
			}
			else
			{
				key = genereteUMGroupChatKeyByGroupName(groupName);
				//trim off domain. TODO: we need a 'jabber id' class that handles all this user@domain related work.  Enhance IMAccount?
				std::string temp = groupName.toStdString();

				size_t posFirstOf = temp.find("@");
				if (posFirstOf != temp.npos) 
				{
					temp = temp.substr(0, posFirstOf);
					senderName = QString::fromStdString(temp.c_str());
				}
			}
			
		}else{

			senderName = getShortDisplayName(contactId, QString::fromStdString(from.getDisplayContactId()));
			key = genereteUMChatKeyBySession(imChatSession);

		}
		
		

		if(!existInMap(key)){

		

			QtUMChat * umChat = new QtUMChat(_cChatHandler, _cContactList, imChatSession, 0,key);
			umChat->init(nickName,senderName,contactId);
			

			// Adding probably missed message
			_lastReceivedMessageIndex[imChatSession->getId()] = -1;
			IMChatSession::IMChatMessageList imChatMessageList = 
				imChatSession->getReceivedMessage(_lastReceivedMessageIndex[imChatSession->getId()] + 1);
			if (imChatMessageList.size() > 0) {
				_lastReceivedMessageIndex[imChatSession->getId()] += imChatMessageList.size();
				IMChatSession::IMChatMessageList::iterator imChatMessageListIterator = imChatMessageList.begin();
				while (imChatMessageListIterator != imChatMessageList.end()){
					IMChatSession::IMChatMessage * imChatMessage = * imChatMessageListIterator;

					//history chat message are reconstructed with no protocol
					QString displayName;
					QTime time;
					QString protocol;
					if(imChatMessage->getIMContact().getProtocol() != EnumIMProtocol::IMProtocolUnknown) {
						displayName = getShortDisplayName(contactId, QString::fromStdString(from.getDisplayContactId()));
						QtEnumIMProtocolMap & rMap = QtEnumIMProtocolMap::getInstance();
						protocol = rMap.toString(imChatMessage->getIMContact().getQtProtocol()).c_str();	//VOXOX - JRT - 2009.07.02 
					} else {
						displayName = QString::fromStdString( const_cast<IMContact&>(imChatMessage->getIMContact()).getCleanContactId());
						Time messageTime = imChatMessage->getTime();
						time = QTime(messageTime.getHour(), messageTime.getMinute());
					}
					/*umChat->addToHistory(contactId, displayName,protocol,
							QString::fromUtf8(imChatMessage->getMessage().c_str()),
							time);*/

					umChat->processRecievedMessage(contactId, displayName,protocol,
							QString::fromUtf8(imChatMessage->getMessage().c_str()));

					imChatMessageListIterator++;
				}
			}
			addUMItem(umChat,imChatSession->isUserCreated());

			if(imChatSession->getIMChatType() == IMChat::ChatGroup){//VOXOX - CJC - 2009.10.02 
				if(imChatSession->getIMContactSet().size() > 1){

					IMContactSet::const_iterator 
					it = imChatSession->getIMContactSet().begin(),
					end = imChatSession->getIMContactSet().end();

					for (; it!=end; ++it) {
						contactAddedEventHandler(*imChatSession,*it);
					}
				}
			}

		}else{
	
			
		
		}
		
}

void QtUMHelper::contactAddedEventHandler(IMChatSession & sender, const IMContact & imContact) {
	contactAddedEventSignal(&sender, imContact);
}

void QtUMHelper::contactRemovedEventHandler(IMChatSession & sender, const IMContact & imContact) {
	contactRemovedEventSignal(&sender, imContact);
}

void QtUMHelper::contactAddedEventSlot(IMChatSession * sender,const IMContact & imContact) {

	QString key;

	//VOXOX - CJC - 2009.10.02 Validate that the contact added is not the same as the account logged
	QString voxoxLogin = QString::fromStdString(_cChatHandler.getCUserProfile().getUserProfile().getVoxOxAccount()->getCleanLogin());
	

	if(sender->getIMChatType() == IMChat::ChatGroup){
		QString groupName = QString::fromStdString(sender->getGroupChatInfo().getChatRoomDisplayName().c_str());
		key = genereteUMGroupChatKeyByGroupName(groupName);
	}
	else{
		key = genereteUMChatKeyBySession(sender);
	}
	QtUMChat * chat = getUMChatByKey(key);
	if(chat){
		QString contactId = QString::fromStdString(imContact.getContactId());
		QString contactKey;
		QString avatarData;
		if(!contactId.contains("@")){//VOXOX - CJC - 2009.09.25 For some reason the contactId is corrupted, lets fix it(TODO: Remove this, just need it to keep working)
			IMAccount * imAccount = _cChatHandler.getCUserProfile().getUserProfile().getIMAccountManager().getIMAccount(sender->getIMChat().getIMAccountId());
			contactId = contactId+"@"+QString::fromStdString(imAccount->getDomain());
			IMContact realIMContact(*imAccount, contactId.toStdString());
			std::string data	   = _cContactList.getContactIconData( realIMContact.getKey() );
			if (data.size() > 0) 
			{
				avatarData = QString::fromStdString(data);
			}
			contactKey = QString::fromStdString(realIMContact.getKey());
		}else{
			std::string data  = _cContactList.getContactIconData( imContact.getKey() );
			if (data.size() > 0) 
			{
				avatarData = QString::fromStdString(data);
			}
			contactKey = QString::fromStdString(imContact.getKey());
		}

		if(voxoxLogin != contactId){
			chat->contactAddedEventSlot(contactId);
			chat->addToAvatarFrame(QString::fromStdString(imContact.getContactId()),avatarData);
			QString contactIdKey = contactKey+"@"+key;
			contactAddedSignal(chat,contactKey,contactIdKey);
		}

		
	}
	
}

void QtUMHelper::contactRemovedEventSlot(IMChatSession * sender,const IMContact & imContact) {

	QString key;

	if(sender->getIMChatType() == IMChat::ChatGroup){
		QString groupName = QString::fromStdString(sender->getGroupChatInfo().getChatRoomDisplayName().c_str());
		key = genereteUMGroupChatKeyByGroupName(groupName);
	}
	else{
		key = genereteUMChatKeyBySession(sender);
	}
	QtUMChat * chat = getUMChatByKey(key);
	if(chat){

		QString contactId = QString::fromStdString(imContact.getContactId());
		QString contactKey;
		if(!contactId.contains("@")){//VOXOX - CJC - 2009.09.25 For some reason the contactId is corrupted, lets fix it(TODO: Remove this, just need it to keep working)
			IMAccount * imAccount = _cChatHandler.getCUserProfile().getUserProfile().getIMAccountManager().getIMAccount(sender->getIMChat().getIMAccountId());
			contactId = contactId+"@"+QString::fromStdString(imAccount->getDomain());
			IMContact realIMContact(*imAccount, contactId.toStdString());
			contactKey = QString::fromStdString(realIMContact.getKey());
		}else{
			contactKey = QString::fromStdString(imContact.getKey());
		}
		QString contactIdKey = contactKey+"@"+key;	
		chat->contactRemovedEventSlot(contactId);
		contactRemovedSignal(chat,contactKey,contactIdKey);
	}

}



//VOXOX - CJC - 2009.08.27 TODO END UMChat
//VOXOX - CJC - 2009.08.27 UMEmail Methods


QString QtUMHelper::genereteUMEmailKey(const QString & email){

	return generateKey(email,QtEnumUMMode::UniversalMessageChatToEmail);
}

//VOXOX - CJC - 2009.05.07 Add support for Chat to Email
void QtUMHelper::handleUMEmailSessionCreated(IMChatSession * imChatSession,const QString & email) {

	//Method triggered by user specific opening a chat to email from the contacts, is created by the user, so we have the contact it, but not the email
	QString contactId = "";
	Contact * from = _cContactList.getContactList().getContactByEmail(email.toStdString());
	if(from){
		contactId = QString::fromStdString(from->getKey());
	}
	//Method triggered by user specific opening a chat to email from the contacts, is created by the user
	openUMEmailSession(imChatSession,contactId,email,true);
}

void QtUMHelper::openUMEmailSession(IMChatSession * imChatSession,const QString & relatedContactId,const QString & emailAddress, bool userCreated){
	
	//if(emailAddress!=""){
	//	int tabs = _universalMessageTabWidget->count();
	//	for (int i = 0; i < tabs; i++) {
	//		//VOXOX CHANGE CJC VALIDATE THAT IT ONLY DO THIS FOR CHAT WIDGET MODE
	//		if(getWidgetMode(i) == QtEnumUMMode::UniversalMessageChatToEmail){
	//			QtChatToEmailWidget * widget = dynamic_cast<QtChatToEmailWidget *>(_universalMessageTabWidget->widget(i));
	//			
	//				if (widget->getSessionId() == imChatSession->getId() && widget->getEmail() == emailAddress) {
	//					_universalMessageTabWidget->setCurrentIndex(i);
	//					show();
	//					return;
	//			}
	//		}
	//	}
	//	QtPopUpChatManager * qtPopUpChatManager = QtPopUpChatManager::getInstance();//VOXOX CHANGE by Rolando - 2009.06.11 
	//	if(qtPopUpChatManager->existPopUpChatToEmail(imChatSession->getId(),emailAddress)){//VOXOX CHANGE by Rolando - 2009.06.11 
	//			return;//VOXOX CHANGE by Rolando - 2009.06.11 
	//	}
	//}

	IMContact * from;
	if(relatedContactId!=""){
		from = _cContactList.getContactList().getContactByKey(relatedContactId.toStdString())->getPreferredIMContact();
	}else{
		from = new IMContact();
		from->setContactId("");
	}
	

	if (imChatSession->getIMContactSet().size() != 0 ) {
	
		createUMEmail(imChatSession,from,emailAddress,true,userCreated);
		
	} else {
		LOG_FATAL("New chat session is empty!");
	}

}



void QtUMHelper::messageRecievedUMEmail(QString message, IMChatSession * imChatSession) {
	//if (!isActiveWindow()) {
	//	if (!isVisible()) {
	//		// Make sure the window appears on the taskbar,
	//		// otherwise it won't flash...
	//		showMinimized();
	//	}
	//	//WidgetUtils::flashWindow(this);//VOXOX CHANGE by Rolando - 2009.06.11 
	//	//VOXOX - CJC - 2009.05.26 
	//	//playIncomingChatSound();
	//}

		QString contactId;
		QString senderDisplayName;
		QString email;
		QString protocol = "Email";//VOXOX CHANGE by Rolando - 2009.06.11 
		QtChatToEmailMessageData data = QtChatToEmailSMSUtils::getChatToMailMessageData(message);
		//TODO SEARCH FOR CONTACT ID BASED ON EMAIL
		//contactId = IMContact.getKey();
		contactId = data.getFrom();
		senderDisplayName = data.getFrom();
		email = data.getFrom();
		
		//TODO FIND CONTACT USING EMAIL
		IMContact * contact;
		Contact * from = _cContactList.getContactList().getContactByEmail(email.toStdString());
		if(from){
			contact = from->getPreferredIMContact();
			contactId = QString::fromStdString(from->getContactId());
			senderDisplayName = QString::fromStdString(from->getDisplayName());
		}else{
			contact = new IMContact();
			contact->setContactId("");
		}

		QtUMEmail * umEmail= getUMEmailByEmail(email);
		//VOXOX - CJC - 2009.08.27 IT was not found lets create it
		if(umEmail==NULL){
			createUMEmail(imChatSession,contact,email,false,false);
			umEmail =  getUMEmailByEmail(email);
		}
		
		if(umEmail){

			//umEmail->addToHistory(contactId, senderDisplayName,"Email", data.getBody());//VOXOX - CJC - 2009.08.26 Add message to widget//VOXOX - CJC - 2010.01.19 
			umEmail->processRecievedMessage(contactId, senderDisplayName,"Email", data.getBody());
			messageReceivedSignal(umEmail);
			//TODO NOTIFY UI TO CHANGE CURRENT WIDGET TO THIS ONE AND TO FLASH
		}
			
		//int tabs = _universalMessageTabWidget->count();
		//for (int i = 0; i < tabs; i++) {
		//	//VOXOX CHANGE CJC VALIDATE THAT ONLY DO THIS FOR CHAT WIDGET
		//	if(getWidgetMode(i) == QtEnumUMMode::UniversalMessageChatToEmail){
		//		QtChatToEmailWidget * widget = dynamic_cast<QtChatToEmailWidget *>(_universalMessageTabWidget->widget(i));
		//		if (widget->getSessionId() == imChatSession->getId() && widget->getEmail() == email) {
		//			widget->addToHistory(contactId, senderDisplayName,protocol, data.getBody());
		//			if (_universalMessageTabWidget->currentWidget() != widget) {
		//				if (isMinimized()) {
		//					_universalMessageTabWidget->setCurrentIndex(i);
		//				} else {
		//					_universalMessageTabWidget->setBlinkingTab(i);
		//				}
		//			}

		//			WidgetUtils::flashWindow(this);//VOXOX CHANGE by ASV 07-31-2009: fix icon bounce when any other window is active				
		//			return;
		//		}
		//	}
		//}

	//	//VOXOX - CJC - 2009.06.13 TODO CHANGES THIS TO CHECK PROPER EMAIL
	//	//VOXOX CHANGE by Rolando - 2009.06.11 
	//	bool contactWasFound = false;
	//	QtPopUpChatManager * qtPopUpChatManager = QtPopUpChatManager::getInstance();
	//	qtPopUpChatManager->verifyChatToEmailMessageReceived(imChatSession->getId(),contactId, email, senderDisplayName, protocol, data.getBody(), contactWasFound);//VOXOX CHANGE by Rolando - 2009.06.11 
	//	if(contactWasFound){//VOXOX CHANGE by Rolando - 2009.06.11 
	//		return;//VOXOX CHANGE by Rolando - 2009.06.11 
	//	}

	//	

	//

		//We dont find an open chat to email widget, lets open one
		//We wont get the message, the iterator is allready moven on the getMessageHandler, so we just send this message on this tab
	
		//Get the widget we just added

		/*QtUMEmail * umEmail= getUMEmailByEmail(email);

		if(umEmail){
		
			return;
		}*/
		/*QtChatToEmailWidget * widget = dynamic_cast<QtChatToEmailWidget *>(_universalMessageTabWidget->widget(_universalMessageTabWidget->count()-1));
		widget->addToHistory(contactId, senderDisplayName,"Email", data.getBody());
		if (_universalMessageTabWidget->currentWidget() != widget) {
			if (isMinimized()) {
				_universalMessageTabWidget->setCurrentIndex(_universalMessageTabWidget->count());
			} else {
				_universalMessageTabWidget->setBlinkingTab(_universalMessageTabWidget->count());
			}
		}*/
		
}


void QtUMHelper::createUMEmail(IMChatSession * imChatSession, const IMContact * from, const QString & emailAddress, bool getMessages, bool userCreated) {
	
		// bind to IMChatSession events
		imChatSession->messageReceivedEvent		 += boost::bind(&QtUMHelper::messageReceivedEventHandler, this, _1);
		imChatSession->imChatSessionWillDieEvent += boost::bind(&QtUMHelper::imChatSessionWillDieEventHandler, this, _1);
		QString key;
		QString email = "";
		QString senderName = "";
		QString contactId = "";
		QString nickName = "";

		if(emailAddress!=""){
			email = emailAddress;
		}
				
		if(from->getContactId()!=""){
			QString fromContactId = QString::fromStdString(_cContactList.findContactThatOwns(*from));
			if(email==""){
				Contact * contactFrom = _cContactList.getContactList().getContactByKey(fromContactId.toStdString());
				if(contactFrom->getEmailAddresses().hasValid()){
					email = QString::fromStdString(contactFrom->getPreferredEmail());
				}
			}
			senderName = getShortDisplayName(fromContactId, QString::fromStdString(from->getDisplayContactId()));
			contactId = QString::fromStdString(from->getKey());
		}else{
			senderName = emailAddress;
			contactId = emailAddress;
		}
		//This should not happen
		if(email=="" && from->getContactId()==""){
			return;
		}

		nickName =  QString::fromStdString(_cChatHandler.getCUserProfile().getUserProfile().getVoxOxAccount()->getDisplayAccountId() );	//VOXOX - JRT - 2009.07.07 
		if(email!=senderName){
			senderName = QString("%1(%2)").arg(senderName).arg(email);
		}else{
			senderName = email;
		}

		key = genereteUMEmailKey(email);

		if(!existInMap(key)){

			QtUMEmail * umEmail = new QtUMEmail(_cChatHandler, _cContactList, imChatSession, 0,key);
			umEmail->init(nickName,senderName,contactId,email,QString::fromStdString(_cChatHandler.getCUserProfile().getUserProfile().getVoxOxAccount()->getDisplayAccountId()));

			if(getMessages){
				/*Check if we allready have this value initialized. We have multiple chat opens with the same ask.im.voxox.com session.
				So we need to not inicialize this variable everytime, cuz the Id is the same for all the session*/
				if(_lastReceivedMessageIndex.find(imChatSession->getId()) ==_lastReceivedMessageIndex.end()){
						_lastReceivedMessageIndex[imChatSession->getId()] = -1;
				}
				IMChatSession::IMChatMessageList imChatMessageList = 
					imChatSession->getReceivedMessage(_lastReceivedMessageIndex[imChatSession->getId()] + 1);

				if (imChatMessageList.size() > 0) {
					_lastReceivedMessageIndex[imChatSession->getId()] += imChatMessageList.size();
					IMChatSession::IMChatMessageList::iterator imChatMessageListIterator = imChatMessageList.begin();
					while (imChatMessageListIterator != imChatMessageList.end()){
						IMChatSession::IMChatMessage * imChatMessage = * imChatMessageListIterator;

						//	history chat message are reconstructed with no protocol
						QString displayName;
						QtChatToEmailMessageData data = QtChatToEmailSMSUtils::getChatToMailMessageData(QString::fromStdString(imChatMessage->getMessage()));
						if(from->getContactId()!="") {
							displayName = getShortDisplayName(QString::fromStdString(_cContactList.findContactThatOwns(*from)), QString::fromStdString(from->getDisplayContactId()));
						} else {
							displayName = data.getFrom();
						}
						umEmail->setSubject(data.getSubject());
						//umEmail->addToHistory(contactId, displayName,"Email",data.getBody());//VOXOX - CJC - 2010.01.19 
						umEmail->processRecievedMessage(contactId, displayName,"Email",data.getBody());//VOXOX - CJC - 2010.01.19 
						
						imChatMessageListIterator++;
					}
				}
			}
			
			addUMItem(umEmail,userCreated);
		

		}else{

		}
}

//VOXOX - CJC - 2009.08.27 END UMEMAIL METHODS

//VOXOX - CJC - 2009.08.27 UMSMS METHODS

QString QtUMHelper::genereteUMSMSKey(const QString & number){

	return generateKey(number,QtEnumUMMode::UniversalMessageChatToSMS);
}


//VOXOX - CJC - 2009.05.19 Chat to sms
void QtUMHelper::messageRecievedUMSMS(QString message, IMChatSession * imChatSession) {


	//if (!isActiveWindow()) {
	//	if (!isVisible()) {
	//		// Make sure the window appears on the taskbar,
	//		// otherwise it won't flash...
	//		showMinimized();
	//	}
	//	//WidgetUtils::flashWindow(this);//VOXOX CHANGE by Rolando - 2009.06.11 
	//	//VOXOX - CJC - 2009.05.26 
	//	//playIncomingChatSound();
	//}

	
		QString contactId;
		QString senderDisplayName;
		QString number;
		QString key;
		QString protocol = "SMS";//VOXOX CHANGE by Rolando - 2009.06.11 
		QtChatToSMSMessageData data = QtChatToEmailSMSUtils::getChatToSMSMessageData(message);


		contactId = data.getFrom();
		senderDisplayName = data.getFrom();
		number = data.getFrom();
		
		//TODO FIND CONTACT USING EMAIL
		IMContact * contact;
		Contact * from = _cContactList.getContactList().getContactByNumber(number.toStdString());
		if(from){
			contact = from->getPreferredIMContact();
			contactId = QString::fromStdString(from->getContactId());
			senderDisplayName = QString::fromStdString(from->getDisplayName());
		}else{
			contact = new IMContact();
			contact->setContactId("");
		}


		QtUMSMS * umSMS= getUMSMSByNumber(number);
		//VOXOX - CJC - 2009.08.27 IT was not found lets create it
		if(umSMS==NULL){
			createUMSMS(imChatSession,contact,number,false,false);
			umSMS =  getUMSMSByNumber(number);
		}
		
		if(umSMS){

			//umSMS->addToHistory(contactId, senderDisplayName,"SMS", data.getBody());//VOXOX - CJC - 2009.08.26 Add message to widget
			umSMS->processRecievedMessage(contactId, senderDisplayName,"SMS", data.getBody());//VOXOX - CJC - 2010.01.19 
			messageReceivedSignal(umSMS);
			//TODO NOTIFY UI TO CHANGE CURRENT WIDGET TO THIS ONE AND TO FLASH
		}
			


	//	//TODO SEARCH FOR CONTACT ID BASED ON EMAIL
	//	//contactId = IMContact.getKey();
	
	//	
	//	//TODO FIND CONTACT USING EMAIL

	//		
	//	int tabs = _universalMessageTabWidget->count();
	//	for (int i = 0; i < tabs; i++) {
	//		//VOXOX CHANGE CJC VALIDATE THAT ONLY DO THIS FOR CHAT WIDGET
	//		if(getWidgetMode(i) == QtEnumUMMode::UniversalMessageChatToSMS){
	//			QtChatToSMSWidget * widget = dynamic_cast<QtChatToSMSWidget *>(_universalMessageTabWidget->widget(i));
	//			if(from){
	//				if (widget->getSessionId() == imChatSession->getId() && widget->getContactId() == key) {
	//					widget->addToHistory(contactId, senderDisplayName,protocol, data.getBody());

	//					if (_universalMessageTabWidget->currentWidget() != widget) {
	//						if (isMinimized()) {
	//							_universalMessageTabWidget->setCurrentIndex(i);
	//						} else {
	//							_universalMessageTabWidget->setBlinkingTab(i);
	//						}
	//					}
	//					WidgetUtils::flashWindow(this);//VOXOX CHANGE by ASV 07-31-2009: fix icon bounce when any other window is active				
	//					return;

	//				}
	//			}
	//			else if ((widget->getSessionId() == imChatSession->getId() && widget->getNumber() == number) || (widget->getSessionId() == imChatSession->getId() && checkSameNumber(widget->getNumber(),number) == true)){
	//				widget->addToHistory(contactId, senderDisplayName,protocol, data.getBody());

	//				if (_universalMessageTabWidget->currentWidget() != widget) {
	//							if (isMinimized()) {
	//								_universalMessageTabWidget->setCurrentIndex(i);
	//							} else {
	//								_universalMessageTabWidget->setBlinkingTab(i);
	//							}
	//						}
	//				WidgetUtils::flashWindow(this);//VOXOX CHANGE by ASV 07-31-2009: fix icon bounce when any other window is active					

	//				return;
	//			}
	//			
	//		}
	//	}
	//	//VOXOX - CJC - 2009.06.13 TODO CHANGES THIS TO CHECK PROPER EMAIL
	//	//VOXOX CHANGE by Rolando - 2009.06.11 
	//	bool contactWasFound = false;
	//	QtPopUpChatManager * qtPopUpChatManager = QtPopUpChatManager::getInstance();
	//	qtPopUpChatManager->verifyChatToSMSMessageReceived(imChatSession->getId(),contactId, number, senderDisplayName, protocol, data.getBody(),key, contactWasFound);//VOXOX CHANGE by Rolando - 2009.06.11 
	//	if(contactWasFound){//VOXOX CHANGE by Rolando - 2009.06.11 
	//		return;//VOXOX CHANGE by Rolando - 2009.06.11 
	//	}

	//	

	//

	//	//We dont find an open chat to email widget, lets open one
	//	//We wont get the message, the iterator is allready moven on the getMessageHandler, so we just send this message on this tab
	//	addChatToSMSWidgetTab(imChatSession,*contact,number,false,false);
	//	//Get the widget we just added
	//	QtChatToSMSWidget * widget = dynamic_cast<QtChatToSMSWidget *>(_universalMessageTabWidget->widget(_universalMessageTabWidget->count()-1));
	//	widget->addToHistory(contactId, senderDisplayName,"SMS", data.getBody());
	//	if (_universalMessageTabWidget->currentWidget() != widget) {
	//		if (isMinimized()) {
	//			_universalMessageTabWidget->setCurrentIndex(_universalMessageTabWidget->count());
	//		} else {
	//			_universalMessageTabWidget->setBlinkingTab(_universalMessageTabWidget->count());
	//		}
	//	}
	//	
}

void QtUMHelper::handleUMSMSSessionCreated(IMChatSession * imChatSession,const QString & number) {

	//Method triggered by user specific opening a chat to email from the contacts, is created by the user, so we have the contact it, but not the email
	QString contactId = "";
	Contact * from = _cContactList.getContactList().getContactByNumber(number.toStdString());
	if(from){
		contactId = QString::fromStdString(from->getKey());
	}
	//Method triggered by user specific opening a chat to email from the contacts, is created by the user
	openUMSMSSession(imChatSession,contactId,number,true);
}

void QtUMHelper::openUMSMSSession(IMChatSession * imChatSession,const QString & relatedContactId,const QString & number, bool userCreated){
	
	//if(emailAddress!=""){
	//	int tabs = _universalMessageTabWidget->count();
	//	for (int i = 0; i < tabs; i++) {
	//		//VOXOX CHANGE CJC VALIDATE THAT IT ONLY DO THIS FOR CHAT WIDGET MODE
	//		if(getWidgetMode(i) == QtEnumUMMode::UniversalMessageChatToEmail){
	//			QtChatToEmailWidget * widget = dynamic_cast<QtChatToEmailWidget *>(_universalMessageTabWidget->widget(i));
	//			
	//				if (widget->getSessionId() == imChatSession->getId() && widget->getEmail() == emailAddress) {
	//					_universalMessageTabWidget->setCurrentIndex(i);
	//					show();
	//					return;
	//			}
	//		}
	//	}
	//	QtPopUpChatManager * qtPopUpChatManager = QtPopUpChatManager::getInstance();//VOXOX CHANGE by Rolando - 2009.06.11 
	//	if(qtPopUpChatManager->existPopUpChatToEmail(imChatSession->getId(),emailAddress)){//VOXOX CHANGE by Rolando - 2009.06.11 
	//			return;//VOXOX CHANGE by Rolando - 2009.06.11 
	//	}
	//}

	IMContact * from;
	if(relatedContactId!=""){
		from = _cContactList.getContactList().getContactByKey(relatedContactId.toStdString())->getPreferredIMContact();
	}else{
		from = new IMContact();
		from->setContactId("");
	}

	if (imChatSession->getIMContactSet().size() != 0 ) {
	
		createUMSMS(imChatSession,from,number,true,userCreated);
		
	} else {
		LOG_FATAL("New chat session is empty!");
	}

}

void QtUMHelper::createUMSMS(IMChatSession * imChatSession, const IMContact * from, const QString & number, bool getMessages, bool userCreated) {
	
		// bind to IMChatSession events
		imChatSession->messageReceivedEvent		 += boost::bind(&QtUMHelper::messageReceivedEventHandler, this, _1);
		imChatSession->imChatSessionWillDieEvent += boost::bind(&QtUMHelper::imChatSessionWillDieEventHandler, this, _1);
		QString key;
		QString phoneNumber = "";
		QString senderName = "";
		QString contactId = "";
		QString nickName = "";

		if(number!=""){
			phoneNumber = number;
		}
				
		if(from->getContactId()!=""){
			QString fromContactId = QString::fromStdString(_cContactList.findContactThatOwns(*from));
			if(phoneNumber==""){
				Contact * contactFrom = _cContactList.getContactList().getContactByKey(fromContactId.toStdString());
				if(contactFrom->getMobilePhone()!=""){
					phoneNumber = QString::fromStdString(contactFrom->getMobilePhone());
				}
			}
			senderName = getShortDisplayName(fromContactId, QString::fromStdString(from->getDisplayContactId()));
			contactId = QString::fromStdString(from->getKey());
		}else{
			senderName = phoneNumber;
			contactId = phoneNumber;
		}

		//This should not happen
		if(number=="" && from->getContactId()==""){
			return;
		}


		nickName =  QString::fromStdString(_cChatHandler.getCUserProfile().getUserProfile().getVoxOxAccount()->getDisplayAccountId() );	//VOXOX - JRT - 2009.07.07 
		if(phoneNumber!=senderName){
			senderName = QString("%1(%2)").arg(senderName).arg(phoneNumber);
		}else{
			senderName = phoneNumber;
		}

		key = genereteUMSMSKey(phoneNumber);

		if(!existInMap(key)){

			QtUMSMS * umSMS = new QtUMSMS(_cChatHandler, _cContactList, imChatSession, 0,key);
			umSMS->init(nickName,senderName,contactId,phoneNumber,QString::fromStdString(_cChatHandler.getCUserProfile().getUserProfile().getVoxOxAccount()->getDisplayAccountId()));

			if(getMessages){
				/*Check if we allready have this value initialized. We have multiple chat opens with the same ask.im.voxox.com session.
				So we need to not inicialize this variable everytime, cuz the Id is the same for all the session*/
				if(_lastReceivedMessageIndex.find(imChatSession->getId()) ==_lastReceivedMessageIndex.end()){
						_lastReceivedMessageIndex[imChatSession->getId()] = -1;
				}
				IMChatSession::IMChatMessageList imChatMessageList = 
					imChatSession->getReceivedMessage(_lastReceivedMessageIndex[imChatSession->getId()] + 1);

				if (imChatMessageList.size() > 0) {
					_lastReceivedMessageIndex[imChatSession->getId()] += imChatMessageList.size();
					IMChatSession::IMChatMessageList::iterator imChatMessageListIterator = imChatMessageList.begin();
					while (imChatMessageListIterator != imChatMessageList.end()){
						IMChatSession::IMChatMessage * imChatMessage = * imChatMessageListIterator;

						//	history chat message are reconstructed with no protocol
						QString displayName;
						QtChatToSMSMessageData data = QtChatToEmailSMSUtils::getChatToSMSMessageData(QString::fromStdString(imChatMessage->getMessage()));
						if(from->getContactId()!="") {
							displayName = getShortDisplayName(QString::fromStdString(_cContactList.findContactThatOwns(*from)), QString::fromStdString(from->getDisplayContactId()));
						} else {
							displayName = data.getFrom();
						}
						//VOXOX - CJC - 2010.01.19 
						//umSMS->addToHistory(contactId, displayName,"SMS",data.getBody());
						umSMS->processRecievedMessage(contactId, displayName,"SMS",data.getBody());//VOXOX - CJC - 2010.01.19 
						imChatMessageListIterator++;
					}
				}
			}
			
			addUMItem(umSMS,userCreated);

		}else{

		}


}

//VOXOX - CJC - 2009.08.28 UMSendFile Methods


//VOXOX - CJC - 2009.08.29 Fax Methods
QString QtUMHelper::genereteUMFaxKey(const QString & faxNumber){

	return generateKey(faxNumber,QtEnumUMMode::UniversalMessageFax);
}

void QtUMHelper::createUMFax(const QString & faxNumber)
{
		QString key;
		QString userName = "";
		QString phoneNumber = faxNumber;
		Contact * contact =_cContactList.getContactList().getContactByNumber(phoneNumber.toStdString());
		QString contactId = "";
		if(contact){//VOXOX CHANGE by Rolando - 2009.07.03 
			contactId = QString::fromStdString(contact->getKey());//VOXOX CHANGE by Rolando - 2009.07.03 
		}

		QString md5  = QString::fromStdString(_cChatHandler.getCUserProfile().getUserProfile().getMd5());
		QString senderName = phoneNumber;
		 
		IMAccount * imAccount = _cChatHandler.getCUserProfile().getUserProfile().getVoxOxAccount();
		if (imAccount) {
			userName = QString::fromStdString(imAccount->getLogin());
		} else {
			LOG_ERROR("cannot get associated IMAccount");
		}
		if(userName!=""){

			key = genereteUMFaxKey(phoneNumber);

			if(!existInMap(key)){

				if(contactId==""){
						contactId = phoneNumber;
						senderName = phoneNumber;
				}else{
						if(contact){//VOXOX CHANGE by Rolando - 2009.07.03 
							senderName = QString::fromStdString(contact->getDisplayName());
						}
				}


				QtUMFax * umFax = new QtUMFax(_cChatHandler, 0,key);
				umFax->init(phoneNumber,userName,md5,contactId,senderName);


				addUMItem(umFax,true);

		}else{

		}
		}


	//QtContactList * qtContactList = _qtWengoPhone.getQtContactList();
	//// If this filesend tab already exists, display the tab
	//int tabs = _universalMessageTabWidget->count();
	//for (int i = 0; i < tabs; i++) {
	//	//VOXOX CHANGE CJC VALIDATE THAT IT ONLY DO THIS FOR CHAT WIDGET MODE
	//	if(getWidgetMode(i) == QtEnumUMMode::UniversalMessageFileTransfer){
	//		QtUMBase * widget = dynamic_cast<QtUMBase *>(_universalMessageTabWidget->widget(i));
	//		if (widget->getContactId() == QString::fromStdString(qtContactList->getCContactList().findContactThatOwns(from))) {
	//			_universalMessageTabWidget->setCurrentIndex(i);
	//			show();
	//			return;
	//		}
	//	}
	//}
	//
	//if (qtContactList) {




	//	int tabNumber;
	//	QString senderName = getShortDisplayName(contactId, QString::fromStdString(from.getDisplayContactId()));
	//	sendFileWidget->setTabText(senderName);//VOXOX CHANGE by Rolando - 2009.06.10

	//	if (_universalMessageTabWidget->count() > 0) {

	//		tabNumber = _universalMessageTabWidget->insertTab(_universalMessageTabWidget->count(), sendFileWidget, senderName);
	//	} else {
	//		tabNumber = _universalMessageTabWidget->insertTab(0, sendFileWidget, senderName);
	//	}

	//	//VOXOX CHANGE by Rolando - 2009.06.09 
	//	if (_universalMessageTabWidget->count() > 1) {
	//		setAllUntabButtonsEnabled(true);
	//	}
	//	else{
	//		setAllUntabButtonsEnabled(false);
	//	}

	//	_universalMessageTabWidget->setCurrentIndex(tabNumber);
	//	activeTabChanged(_universalMessageTabWidget->currentIndex());

	//	show();
	//	
	//	updateToolBarActions();
	//	}
	//}
	//
	//tabNumberChangedSignal(_universalMessageTabWidget->count()); // VOXOX CHANGE by ASV 06-29-2009: added to implement the close tab functionality
	
}

//VOXOX - CJC - 2009.08.29 End of SendFile Methods

QString QtUMHelper::genereteUMSendFileKey(const QString & contactId){

	return generateKey(contactId,QtEnumUMMode::UniversalMessageFileTransfer);
}

void QtUMHelper::createUMSendFile(const IMContact & from) 
{
		QString key;
		QString userName = "";
		QString contactId = QString::fromStdString(_cContactList.findContactThatOwns(from));
		QString md5  = QString::fromStdString(_cChatHandler.getCUserProfile().getUserProfile().getMd5());

		 
		IMAccount * imAccount = _cChatHandler.getCUserProfile().getUserProfile().getIMAccountManager().getIMAccount(from.getIMAccountId());
		if (imAccount) {
			userName = QString::fromStdString(imAccount->getLogin());
			OWSAFE_DELETE(imAccount);
		} else {
			LOG_ERROR("cannot get associated IMAccount");
		}
		if(userName!=""){

			key = genereteUMSendFileKey(contactId);

			if(!existInMap(key)){

				//	QtSendFileWidget * sendFileWidget = new QtSendFileWidget(&_qtWengoPhone, 0);

				//	SAFE_CONNECT(sendFileWidget, SIGNAL(closeTab()), SLOT(closeActiveTab()));
				//	
				//	sendFileWidget->setContactId();
				//	sendFileWidget->setUsername(userName);
				//	sendFileWidget->setMd5(md5);
				//	sendFileWidget->setContactIdUploadFile(QString::fromStdString(from.getContactId()));
				//	sendFileWidget->init();

				QString senderName = getShortDisplayName(contactId, QString::fromStdString(from.getDisplayContactId()));
				QString contactIdUploadFile = QString::fromStdString(from.getContactId());


				QtUMSendFile * umSendFile = new QtUMSendFile(_cChatHandler, 0,key);
				umSendFile->init(contactId,userName,md5,contactIdUploadFile,senderName);


				addUMItem(umSendFile,true);

		}else{

		}
		}


	//QtContactList * qtContactList = _qtWengoPhone.getQtContactList();
	//// If this filesend tab already exists, display the tab
	//int tabs = _universalMessageTabWidget->count();
	//for (int i = 0; i < tabs; i++) {
	//	//VOXOX CHANGE CJC VALIDATE THAT IT ONLY DO THIS FOR CHAT WIDGET MODE
	//	if(getWidgetMode(i) == QtEnumUMMode::UniversalMessageFileTransfer){
	//		QtUMBase * widget = dynamic_cast<QtUMBase *>(_universalMessageTabWidget->widget(i));
	//		if (widget->getContactId() == QString::fromStdString(qtContactList->getCContactList().findContactThatOwns(from))) {
	//			_universalMessageTabWidget->setCurrentIndex(i);
	//			show();
	//			return;
	//		}
	//	}
	//}
	//
	//if (qtContactList) {




	//	int tabNumber;
	//	QString senderName = getShortDisplayName(contactId, QString::fromStdString(from.getDisplayContactId()));
	//	sendFileWidget->setTabText(senderName);//VOXOX CHANGE by Rolando - 2009.06.10

	//	if (_universalMessageTabWidget->count() > 0) {

	//		tabNumber = _universalMessageTabWidget->insertTab(_universalMessageTabWidget->count(), sendFileWidget, senderName);
	//	} else {
	//		tabNumber = _universalMessageTabWidget->insertTab(0, sendFileWidget, senderName);
	//	}

	//	//VOXOX CHANGE by Rolando - 2009.06.09 
	//	if (_universalMessageTabWidget->count() > 1) {
	//		setAllUntabButtonsEnabled(true);
	//	}
	//	else{
	//		setAllUntabButtonsEnabled(false);
	//	}

	//	_universalMessageTabWidget->setCurrentIndex(tabNumber);
	//	activeTabChanged(_universalMessageTabWidget->currentIndex());

	//	show();
	//	
	//	updateToolBarActions();
	//	}
	//}
	//
	//tabNumberChangedSignal(_universalMessageTabWidget->count()); // VOXOX CHANGE by ASV 06-29-2009: added to implement the close tab functionality
	
}



void QtUMHelper::createUMCall(const QString & phoneNumber,QtContactCallListWidget * qtContactCallListWidget)
{
		QString key;
		Contact * contact =_cContactList.getContactList().getContactByNumber(phoneNumber.toStdString());
		QString contactId = phoneNumber;
		QString senderName = phoneNumber;
		 
		if(contact){//VOXOX CHANGE by Rolando - 2009.07.03 
			contactId = QString::fromStdString(contact->getKey());//VOXOX CHANGE by Rolando - 2009.07.03 
			senderName= QString::fromStdString(contact->getDisplayName());
			if(senderName!=""){
				senderName = QString("%1(%2)").arg(senderName).arg(phoneNumber);
			}
		}

		key = qtContactCallListWidget->getKey();

		if(!existInMap(key)){

			QtUMCall * umCall = new QtUMCall(_cChatHandler, 0,key);
			umCall->init(phoneNumber,contactId,senderName,qtContactCallListWidget);
			addUMItem(umCall,true);

		}else{

		}
		


	//QtContactList * qtContactList = _qtWengoPhone.getQtContactList();
	//// If this filesend tab already exists, display the tab
	//int tabs = _universalMessageTabWidget->count();
	//for (int i = 0; i < tabs; i++) {
	//	//VOXOX CHANGE CJC VALIDATE THAT IT ONLY DO THIS FOR CHAT WIDGET MODE
	//	if(getWidgetMode(i) == QtEnumUMMode::UniversalMessageFileTransfer){
	//		QtUMBase * widget = dynamic_cast<QtUMBase *>(_universalMessageTabWidget->widget(i));
	//		if (widget->getContactId() == QString::fromStdString(qtContactList->getCContactList().findContactThatOwns(from))) {
	//			_universalMessageTabWidget->setCurrentIndex(i);
	//			show();
	//			return;
	//		}
	//	}
	//}
	//
	//if (qtContactList) {




	//	int tabNumber;
	//	QString senderName = getShortDisplayName(contactId, QString::fromStdString(from.getDisplayContactId()));
	//	sendFileWidget->setTabText(senderName);//VOXOX CHANGE by Rolando - 2009.06.10

	//	if (_universalMessageTabWidget->count() > 0) {

	//		tabNumber = _universalMessageTabWidget->insertTab(_universalMessageTabWidget->count(), sendFileWidget, senderName);
	//	} else {
	//		tabNumber = _universalMessageTabWidget->insertTab(0, sendFileWidget, senderName);
	//	}

	//	//VOXOX CHANGE by Rolando - 2009.06.09 
	//	if (_universalMessageTabWidget->count() > 1) {
	//		setAllUntabButtonsEnabled(true);
	//	}
	//	else{
	//		setAllUntabButtonsEnabled(false);
	//	}

	//	_universalMessageTabWidget->setCurrentIndex(tabNumber);
	//	activeTabChanged(_universalMessageTabWidget->currentIndex());

	//	show();
	//	
	//	updateToolBarActions();
	//	}
	//}
	//
	//tabNumberChangedSignal(_universalMessageTabWidget->count()); // VOXOX CHANGE by ASV 06-29-2009: added to implement the close tab functionality
	
}





void QtUMHelper::hangUpCall(const QString & key){

	QtUMCall * call = getUMCallByKey(key);

	if(call){
		call->hangUp();
	}
}









bool QtUMHelper::addUMItemToMap(QtUMBase * item){

	_umBaseMap[item->getKey()] = item;
	return true;

}


QString QtUMHelper::getShortDisplayName(const QString & contactId, const QString & defaultName){
	QString displayName;
	std::string	tmpSendername = _cContactList.getContactDisplayName( contactId.toStdString() );

	if (tmpSendername.empty()) 
	{
		tmpSendername = defaultName.toStdString();
	}
	
	displayName = QString::fromUtf8(tmpSendername.c_str());

	return displayName;
	
}


bool QtUMHelper::existUMChat(IMChatSession * session){


	QString key = genereteUMChatKeyBySession(session);
	return existInMap(key);
	
}


bool QtUMHelper::exist(QtUMBase * item){

	return existInMap(item);
}



bool QtUMHelper::existInMap(QtUMBase * item){

	UMBaseMap::iterator iter = _umBaseMap.find(item->getKey());//Check if item exists

	if (iter == _umBaseMap.end()) {
		return false;
		
	}
	return true;
}

bool QtUMHelper::existInMap(QString key){

	UMBaseMap::iterator iter = _umBaseMap.find(key);//Check if item exists

	if (iter == _umBaseMap.end()) {
		return false;
		
	}
	return true;
}


QtUMBase * QtUMHelper::getUMItemByKey(const QString & key){

	UMBaseMap::iterator iter = _umBaseMap.find(key);//Check if item exists

	if (iter != _umBaseMap.end()) {

		 return iter->second;
	}

	return NULL;
}

QString QtUMHelper::getContactIdByKey(const QString & key){

	UMBaseMap::iterator iter = _umBaseMap.find(key);//Check if item exists

	if (iter != _umBaseMap.end()) {

		return iter->second->getContactId();
	}
	return QString("");
}

void QtUMHelper::deleteUMItemByKey(const QString & key){

	UMBaseMap::iterator iter = _umBaseMap.find(key);//Check if item exists

	if (iter != _umBaseMap.end()) {

		OWSAFE_DELETE(iter->second);
		_umBaseMap.erase(iter);
	}
}



QtUMChat * QtUMHelper::getUMChatBySession(const IMChatSession * session){
	
	QString key = genereteUMChatKeyBySession(session);

	UMBaseMap::iterator iter = _umBaseMap.find(key);//Check if item exists

	if (iter != _umBaseMap.end()) {

		 return qobject_cast<QtUMChat *>(iter->second);
	}

	return NULL;
}


QtUMChat * QtUMHelper::getUMChatByKey(const QString & key){

	UMBaseMap::iterator iter = _umBaseMap.find(key);//Check if item exists

	if (iter != _umBaseMap.end()) {

		 return qobject_cast<QtUMChat *>(iter->second);
	}

	return NULL;
}


QtUMChat * QtUMHelper::getUMChatByContactId(const QString & id){
	
	QString key = genereteUMChatKeyByContactId(id);

	UMBaseMap::iterator iter = _umBaseMap.find(key);//Check if item exists

	if (iter != _umBaseMap.end()) {

		 return qobject_cast<QtUMChat *>(iter->second);
	}

	return NULL;
}

QtUMEmail * QtUMHelper::getUMEmailByEmail(const QString & email){
	
	QString key = genereteUMEmailKey(email);

	UMBaseMap::iterator iter = _umBaseMap.find(key);//Check if item exists

	if (iter != _umBaseMap.end()) {

		 return qobject_cast<QtUMEmail *>(iter->second);
	}

	return NULL;
}

QtUMCall * QtUMHelper::getUMCallByKey(const QString & key){
	

	UMBaseMap::iterator iter = _umBaseMap.find(key);//Check if item exists

	if (iter != _umBaseMap.end()) {

		 return qobject_cast<QtUMCall *>(iter->second);
	}

	return NULL;
}




QtUMSMS * QtUMHelper::getUMSMSByNumber(const QString & number){
	//VOXOX - CJC - 2009.08.27 We cant to this by key because numbers could be diferent when returned by the SMS service so we need to match it diferent.

	UMBaseMap::const_iterator iter    = _umBaseMap.begin();
	UMBaseMap::const_iterator iterEnd = _umBaseMap.end();

	while (iter != iterEnd)
	{
		if(iter->second->getMode() == QtEnumUMMode::UniversalMessageChatToSMS){
			QtUMSMS * sms = qobject_cast<QtUMSMS *>(iter->second);
			//VOXOX - CJC - 2009.08.27 We check the number from left to right;
			QString realNumber1 = number;
			QString realNumber2 = sms->getNumber();

			realNumber1 = realNumber1.replace(0,realNumber1.length()-7,"");
			realNumber2 = realNumber2.replace(0,realNumber2.length()-7,"");
			if(realNumber1== realNumber2){
				return sms;
			}
		}
		iter++;
	}

	return NULL;
}


//VOXOX - CJC - 2009.08.26 Recieve Handlers
void QtUMHelper::messageReceivedEventHandler(IMChatSession & sender) {
	messageReceivedSignal(&sender);
}

//VOXOX - CJC - 2009.08.26  Recieve Handlers
void QtUMHelper::typingStateChangedEventHandler(IMChatSession & sender, const IMContact & imContact, IMChat::TypingState state) {
	IMChat::TypingState * tmpState = new IMChat::TypingState;
	*tmpState = state;
	typingStateChangedSignal(&sender,&imContact,tmpState);
}


void QtUMHelper::handleAddToChat(const QString & key){//VOXOX - CJC - 2009.09.23 
	
	QtUMChat *  chat = getUMChatByKey(key);
	if(chat){

			if (chat->canDoMultiChat()) 
			{
				
				IMChatSession* imChatSession = chat->getIMChatSession();

				if ( imChatSession )
				{
					QtChatRoomInviteDlg dlg( *imChatSession, _cChatHandler.getCUserProfile().getCContactList());

					if ( dlg.exec() == dlg.Accepted )
					{
						IMContactSet imContactSet;

						dlg.getParticipants().toIMContactSet( imContactSet );
						std::string chatRoomName = imChatSession->getGroupChatInfo().getAlias();
						_cChatHandler.getCUserProfile().inviteToGroupChat( chatRoomName, "", imContactSet );
					}
				}
			}

	}

}

//VOXOX - CJC - 2009.08.26  Recieve Handlers
void QtUMHelper::imChatSessionWillDieEventHandler(IMChatSession & sender) {
	_lastReceivedMessageIndex.erase(sender.getId());
}

void QtUMHelper::messageReceivedSlot(IMChatSession * sender) {
	
	// new way to get messages
	IMChatSession::IMChatMessageList imChatMessageList = sender->getReceivedMessage(_lastReceivedMessageIndex[sender->getId()] + 1);
	if (imChatMessageList.size() > 0) {
		_lastReceivedMessageIndex[sender->getId()] += imChatMessageList.size();
		IMChatSession::IMChatMessageList::iterator imChatMessageListIterator = imChatMessageList.begin();
		while (imChatMessageListIterator < imChatMessageList.end()) {
			IMChatSession::IMChatMessage * imChatMessage = * imChatMessageListIterator;
			const IMContact & from = imChatMessage->getIMContact();
			std::string message = imChatMessage->getMessage();
			Config & config = ConfigManager::getInstance().getCurrentConfig();
			//VOXOX - CJC - 2009.05.09 Verify is message is from im.voxox.com
			if(from.getContactId() == config.getAskIMVoxOxComAddress()){
				QtChatToEmailSMSUtils::Type type = QtChatToEmailSMSUtils::checkMesageType(QString::fromStdString(message));
				//Check Message content to determine what type of message we have.
				if(type == QtChatToEmailSMSUtils::ChatToEmail){
					messageRecievedUMEmail(QString::fromStdString(message),sender);
					return;
				}else if(type == QtChatToEmailSMSUtils::ChatToSMS){
					messageRecievedUMSMS(QString::fromStdString(message),sender);
					return;
				}
				LOG_DEBUG(message);
				return;
			}
			QString contactId;
			QString senderName = QString::fromStdString(from.getContactId());
			if(!sender->isGroupChat()){
				contactId = QString::fromStdString(_cContactList.findContactThatOwns(from));
			}else{
				contactId = QString::fromStdString(from.getContactId());
			}
			QString senderDisplayName = getShortDisplayName(contactId, QString::fromStdString(from.getDisplayContactId()));
			QString msg = QString::fromUtf8(message.c_str());
			QtEnumIMProtocolMap & rMap = QtEnumIMProtocolMap::getInstance();
			QString protocol = rMap.toString(from.getQtProtocol()).c_str();	//VOXOX - JRT - 2009.06.28 
			
			QtUMChat * umChat;

			if(sender->getIMChatType() == IMChat::ChatGroup){
				QString groupName = QString::fromStdString(sender->getGroupChatInfo().getChatRoomDisplayName().c_str());
				QString key = genereteUMGroupChatKeyByGroupName(groupName);
				umChat = getUMChatByKey(key);
			}
			else{
				umChat = getUMChatBySession(sender);
			}

			if(umChat){
				umChat->processRecievedMessage(contactId,senderDisplayName,protocol,msg);
				//umChat->addToHistory(contactId, senderDisplayName,protocol, msg);//VOXOX - CJC - 2009.08.26 Add message to widget
				messageReceivedSignal(umChat);
			}
	
			imChatMessageListIterator++;
		}
	}
}

void QtUMHelper::typingStateChangedThreadSafe(const IMChatSession * sender, const IMContact * from,const IMChat::TypingState * state) {
		QtUMChat * umChat = getUMChatBySession(sender);
		if(umChat){
				if (umChat->getSessionId() == sender->getId()) {
					bool typing =  false;
					if(*state == IMChat::TypingStateTyping) {
						typing = true;
					}
					setTyping(umChat,typing);//VOXOX - CJC - 2009.08.26 Notifys UI
						
				}
					
		}
		OWSAFE_DELETE(state);
}

void QtUMHelper::sendMessageToChat(IMChatSession * sender,const QString & message){

		IMAccount * imAccount = _cChatHandler.getCUserProfile().getUserProfile().getIMAccountManager().getIMAccount(sender->getIMChat().getIMAccountId());

		if (imAccount) 
		{
			QString nickname = QString::fromUtf8(imAccount->getDisplayAccountId().c_str());	

			QtEnumIMProtocolMap & rMap = QtEnumIMProtocolMap::getInstance();
			QString protocol = rMap.toString(imAccount->getQtProtocol()).c_str();	//VOXOX - JRT - 2009.07.02
			
			QtUMChat * umChat = getUMChatBySession(sender);

			if(umChat){
				sender->sendMessage(message.toStdString());
				umChat->processRecievedMessage("self", nickname, protocol, message);//VOXOX - CJC - 2010.01.18 
				//umChat->addToHistory("self", nickname.c_str(), protocol.c_str(), message);
			}

			OWSAFE_DELETE(imAccount);
		} 				
}
