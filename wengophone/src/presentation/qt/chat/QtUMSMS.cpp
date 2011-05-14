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
* Class email to support chat to email 
* @author Chris Jimenez C 
* @date 2009.05.11
*/


#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtUMSMS.h"
#include "QtChatUtils.h"
#include "ui_UMSMS.h"
#include "chatroom/QtChatRoomInviteDlg.h"
#include "emoticons/QtEmoticonsWidget.h"
#include "translation/QtTranslationWidget.h"
#include "translation/QtTranslationLanguage.h"

#include <model/config/Config.h>
#include <model/config/ConfigManager.h>
#include <model/profile/Profile.h>
#include <model/profile/UserProfile.h>
#include <model/history/History.h>
#include <control/contactlist/CContactList.h>
#include <control/profile/CUserProfile.h>
#include <control/history/CHistory.h>
#include <model/history/HistoryMemento.h>
#include <presentation/qt/QtToolBar.h> //VOXOX CHANGE by ASV 07-02-2009

//#include <presentation/qt/QtEnumIMProtocol.h>	//VOXOX - JRT - 2009.04.21 

//VOXOX CHANGE by Alexander 02-16-09: Includes the class that will hanfle the spell checker for Mac.
#if (defined OS_MACOSX)
#include <qtutil/macspellchecker.h>
#endif

#include <imwrapper/Account.h>
#include <imwrapper/EnumIMProtocol.h>
#include <imwrapper/IMContactSet.h>
#include <imwrapper/QtEnumIMProtocol.h>			//VOXOX - JRT - 2009.05.29 

#include <util/File.h>
#include <util/Logger.h>
#include <util/SafeDelete.h>
#include <cutil/global.h>
#include <qtutil/StringListConvert.h>
#include <qtutil/SafeConnect.h>
//VOXOX CHANGE by Alexander 02-16-09: This will help load the spell checker objects into the ui.
#include <qtutil/Widget.h>

#include <string>

#include <QtCore/QTime>
#include <QtCore/QTimer>

#include <QtGui/QColorDialog>
#include <QtGui/QFontDialog>
#include <QtGui/QKeyEvent>
#include <presentation/qt/messagebox/QtVoxMessageBox.h>
#include <QtGui/QPainter>

static const int MAX_CHARACTERS = 160;


QtUMSMS::QtUMSMS(CChatHandler & cChatHandler,
	CContactList & cContactList, IMChatSession * imChatSession,
	QWidget * parent,QString key,QtEnumUMMode::Mode mode) :
	QtUMBase(parent,key,mode),
	_cChatHandler(cChatHandler),
	_cContactList(cContactList){

	//Default nickname for testing purpose
	_nickName = "VoxOx";
	_sessionId = imChatSession->getId();
	
	_widget =  new QWidget(parent);

	_translation =  new QtUMTranslation(cChatHandler);
	_isContactConnected = true;

	_untabbed = false;//VOXOX CHANGE by Rolando - 2009.06.08 
//
//
	_ui = new Ui::UMSMS();
	_ui->setupUi(_widget);
	
	buildUI();
//	
//	//VOXOX CHANGE CJC THIS IS NOT NEEDED ANY MORE
	////
	//install event filters
	_ui->chatHistory->installEventFilter(this);
	_chatEdit->installEventFilter(this);
	////

	updateMaxCharacterText();

	setIMChatSession(imChatSession);
	SAFE_CONNECT(_translation,	SIGNAL(translateMessageSignal(const QString &, const QString &, const QString &,const QString &, const QString &, bool, bool)), SLOT(translateMessageSlot(const QString &, const QString &, const QString &,const QString &,const QString &, bool, bool)));//VOXOX - CJC - 2010.01.18 
	SAFE_CONNECT(_translation,	SIGNAL(translateErrorSignal()), SLOT(translateErrorSlot()));//VOXOX - CJC - 2010.01.18 
		
}
void QtUMSMS::setIMChatSession(IMChatSession * imChatSession) {
	
	_imChatSession = imChatSession;

}

void QtUMSMS::buildUI(){


	_translationWidget = new QtTranslationWidget(_cChatHandler,_widget);//VOXOX - CJC - 2010.01.18 
	SAFE_CONNECT(_translationWidget,  SIGNAL(translationSettingsSignal(QtEnumTranslationMode::Mode,const QtTranslationLanguage &,const QtTranslationLanguage &)), SLOT(translationSettingsSlot(QtEnumTranslationMode::Mode,const QtTranslationLanguage &,const QtTranslationLanguage &))	);//VOXOX - CJC - 2010.01.18 
	
	_ui->sendButton->setImages	(":/pics/chat/btnSend.png",		":/pics/chat/btnSendPress.png",		"",	":/pics/chat/btnSendDisable.png"	);
	_ui->btnFont->setImages		(":/pics/chat/btnFont.png",		":/pics/chat/btnFontPress.png",		"",	":/pics/chat/btnFontDisable.png"	);
	_ui->btnEmoticon->setImages	(":/pics/chat/btnEmoticon.png",	":/pics/chat/btnEmoticonPress.png",	"",	":/pics/chat/btnEmoticonDisable.png");
	_ui->btnTranslation->setImages(":/pics/chat/globe-disabled.png",":/pics/chat/globe-disabled.png", "", ":/pics/chat/globe-disabled.png");

	_ui->editFrame->setStyleSheet("background-color: #e1e1e1; border-top:3px solid #898b97;");
	_ui->chatEditFrame->setStyleSheet("border:0;");
	_ui->textCountFrame->setStyleSheet("background-color: #e1e1e1; border:0; border-right:1px solid #898b97;");
	_ui->lblTextCount->setStyleSheet("background-color: #e1e1e1; color: #5a5a5a; border:0;");
	_ui->chatFunctionFrame->setStyleSheet("border:0;");		//VOXOX CHANGE by ASV 05-28-2009: Fix the appearance of the chat buttons on the Mac platform

	//VOXOX CHANGE by Alexander 02-16-09: separated the Text Edit for Mac and Windows to use separate spell checkers.
	// Also changed all _ui->chatEdit to reference this new object _chatEdit
	#if (defined OS_WINDOWS)
		Config & config = ConfigManager::getInstance().getCurrentConfig();
		QString dic = QString::fromStdString(config.getSelectedDictionary());
		_chatEdit = new SpellTextEdit(_widget,dic);
	#endif

	//VOXOX CHAGE by ASV 09-04-2009: se use the normal qt objecto to write on the chat window under Linux
	//TODO: add hunspell to Linux
	#if (defined OS_LINUX)
		_chatEdit = new QTextEdit();
	#endif

	#if (defined OS_MACOSX)
		_chatEdit = new QtMacSpellCheckingTextEdit();
		//VOXOX CHANGE CJC MOVE LAYOUT DOWN A BIT
		//this->layout()->setContentsMargins(0, 11, 0, 0); 
	#endif

	_chatEdit->setStyleSheet("background-color: #e1e1e1; color: #5a5a5a; border-right:1px solid #898b97;");
	Widget::createLayout(_ui->chatEditFrame)->addWidget(_chatEdit);

	//signal connection
	SAFE_CONNECT(_ui->sendButton,	SIGNAL(clicked()), SLOT(sendPressedSlot())	);
	SAFE_CONNECT(_ui->btnFont,		SIGNAL(clicked()), SLOT(changeFont())		);
	SAFE_CONNECT(_ui->btnEmoticon,	SIGNAL(clicked()), SLOT(chooseEmoticon())	);
	SAFE_CONNECT(_ui->btnTranslation,  SIGNAL(clicked()), SLOT(translationClickedSlot())	);
	SAFE_CONNECT(_chatEdit, SIGNAL(textChanged()), SLOT(chatEditTextChanged()));
}

void QtUMSMS::init(const QString & nickname,const QString & senderName,const QString & contactId,const QString & number,const QString & from){

	setNickName(nickname);
	setSenderName(senderName);
	setContactId(contactId);
	setNumber(number);
	setFrom(from);
	updateAvatarFrame(contactId);
	updateUserAvatar();

	_ui->chatHistory->init(contactId,senderName,nickname,QTime::currentTime(),QDate::currentDate());
		_translation->initTranslationSettings(contactId);
		updateTranslationUI();
}

void QtUMSMS::chatEditTextChanged() {

	QString text = _chatEdit->toPlainText();
	if(text.length()==MAX_CHARACTERS+1){
		_chatEdit->setPlainText(text.left(MAX_CHARACTERS));
		_chatEdit->moveCursor(QTextCursor::End,QTextCursor::MoveAnchor); 
	}
	updateMaxCharacterText();
}

void QtUMSMS::updateTranslationUI(){


	if(!_translation->getTranslationDestLocale().getSupportSMS()){

		QtVoxMessageBox box(0);
		box.setWindowTitle(tr("VoxOx - Translate Message"));
		box.setText(QString("Please note, the mobile phone you are texting must support the %1 character set for your messages to display properly.").arg(_translation->getTranslationDestLocale().getTranslationName()));
		box.setStandardButtons(QMessageBox::Ok);
		box.exec();
	}

	if(_translation->isTranslationOn()){
			_ui->btnTranslation->setNormalPixmap(":/pics/chat/globe-enabled.png");
	}else{
			_ui->btnTranslation->setNormalPixmap(":/pics/chat/globe-disabled.png");
	}



}
QtUMSMS::~QtUMSMS() {
	OWSAFE_DELETE(_translation);//VOXOX - CJC - 2010.01.18 
	OWSAFE_DELETE(_translationWidget);//VOXOX - CJC - 2010.01.17 
	OWSAFE_DELETE(_chatEdit);
	OWSAFE_DELETE(_widget);
	OWSAFE_DELETE(_ui);
}

void QtUMSMS::closeSession() {
	_imChatSession->close();
	
}

void QtUMSMS::changeFont() {
	bool ok;
	QFont font = QFontDialog::getFont(&ok, _chatEdit->currentFont(), _widget);
	if (ok) {
		_currentFont = font;
		_chatEdit->setCurrentFont(font);
	}
	_chatEdit->setFocus();
}

void QtUMSMS::updateMaxCharacterText() {

	QString text = QString("%1/%2").arg(_chatEdit->toPlainText().length()).arg(MAX_CHARACTERS);
	_ui->lblTextCount->setText(text);
}

//VOXOX - CJC - 2009.05.01 Load last history
void QtUMSMS::loadLastChatHistory(QString contactId){
	//Config & config = ConfigManager::getInstance().getCurrentConfig();
	//QtEnumIMProtocolMap& qtProtocolMap = QtEnumIMProtocolMap::getInstance();
	//HistoryMementoCollection * collection = _cChatHandler.getCUserProfile().getCHistory()->getHistory().getHistoryMementoCollection();
	//QMap<QString, int> map;
	////Last Conversation with contact variables
	//QString lastDate= NULL;
	//int lastId = NULL;
	////Fech all the chatsessions historys
	//for (HistoryMap::iterator it = collection->begin(); it != collection->end(); it++) {
	//	HistoryMemento * memento = (*it).second;
	//	//Only chat sessions
	//	if (memento->getState() == HistoryMemento::ChatSession) {
	//		//Only from the contact of this chat session
	//		if(contactId.toStdString() == memento->getPeer()){
	//			QDate date = QDate::fromString(QString::fromStdString(memento->getDate().toString()), "yyyy-MM-dd");
	//			QTime time = QTime::fromString(QString::fromStdString(memento->getTime().toString()));
	//			//Items are stored in a QMap because of its sortable capabilities/ We want the last conversation only
	//			 map.insert(date.toString("yyyy-MM-dd") + QString(" %1").arg(time.toString(Qt::TextDate)), (*it).first);
	//			}
	//		}
	//	
	//}
	//
	//QMap<QString, int>::const_iterator i = map.constBegin();
	////We go over the map, it is sortable desendant so, the last item will the the last conversation
	//	 while (i != map.constEnd()) {
	//			 lastDate = i.key();
	//			 lastId = i.value();
	//			 ++i;
	//}
	////Check if we have a winner!
	//if(lastId){
	//	//Get all the history items from that last conversation
	//	HistoryMemento * hm = collection->getMemento(lastId);
	//	if (hm) {	

	//		std::string userlogin;
	//		HistoryMementoCollection hmc;
	//		StringList cuuidList;
	//		QStringList qcuuidList;
	//		//Get the chat log information
	//		_cChatHandler.getCUserProfile().getUserProfile().getHistory().loadChatLog(hm->getData(), &hmc, &userlogin, &cuuidList);
	//		qcuuidList = StringListConvert::toQStringList(cuuidList);
	//		//User name
	//		QString selfContactName = QString::fromUtf8(userlogin.c_str());
	//		QDate previousDate;
	//		//Get the max number of messages that we will like to you till the end of the conversation
	//		//VOXOX - CJC - 2009.05.01 TODO: We need to add this to a config key, if this is modified by the user
	//		unsigned int maxMessages = 5;
	//		int count=0;
	//		int start;
	//		//If the conversation is less than the max number of messages
	//		if(hmc.size()<=maxMessages){
	//			start=0;
	//		}else{
	//			//We will start adding messages until the max number of messages is reached
	//			start=hmc.size()-maxMessages;
	//		}
	//		QDate date;
	//		//For Each conversation
	//		for (HistoryMap::iterator it = hmc.begin(); it != hmc.end(); it++) {
	//			//Gets the info
	//			HistoryMemento * memento = it->second;
	//			// Read date and time
	//			Date mementoDate = memento->getDate();
	//			date = QDate(mementoDate.getYear(), mementoDate.getMonth(), mementoDate.getDay());
	//			Time mementoTime = memento->getTime();
	//			const QTime time = QTime(mementoTime.getHour(), mementoTime.getMinute());
	//			//Gets contact from conversation
	//			QString contactName = QString::fromUtf8(memento->getPeer().c_str());
	//			QString contactId;
	//			
	//			IMAccount * imAccount =
	//			_cChatHandler.getCUserProfile().getUserProfile().getIMAccountManager().getIMAccount(_imChatSession->getIMChat().getIMAccountId());
	//			QtEnumIMProtocol* protocol = qtProtocolMap.findByModelProtocol(  imAccount->getProtocol() );
	//			QString protocolName =protocol->getName();
	//			OWSAFE_DELETE(imAccount);
	//			if (contactName == selfContactName) {
	//				contactId = "self";
	//			} else {
	//				contactId = qcuuidList[0];
	//				// FIXME: History format is broken: it does not store the uuid
	//				// associated with a peer. Using the first uid will work for
	//				// all one-to-one chat, but will fail for chat conference.
	//			}
	//			//If we get to the point of start adding messages
	//			if(count>=start){
	//				//Add message
	//				addToHistory(
	//					contactId,
	//					protocolName,
	//					contactName,
	//					QString::fromUtf8(memento->getData().c_str()),
	//					time,
	//					date
	//				);
	//			}
	//			count++;
	//		}
	//		//Add history chat time
	//		QString txt ="Last conversation loaded: "+ QLocale::system().toString(date);
	//		addStatusMessage(txt);
	//	}
	//}
}


void QtUMSMS::changeFontColor() {
	bool ok;
	QFont font = QFontDialog::getFont(&ok, _chatEdit->currentFont(), _widget);
	if (ok) {
		_currentFont = font;
		_chatEdit->setCurrentFont(font);
	}
	_chatEdit->setFocus();
}


//VOXOX CHANGE by Rolando - 2009.06.08 
//void QtUMChat::unTabClickedSlot(){
//
///*if(!_untabbed){
//	unTabClicked();
//}
//else{
//	implodeChat();
//}
//_untabbed = !_untabbed;
//
//updateUntabbedTooltipMessage();*/
//
//}

//VOXOX CHANGE by Rolando - 2009.06.09 
//bool QtUMChat::isUntabbed(){
//	return _untabbed;
//}

//VOXOX CHANGE by Rolando - 2009.06.09 
//void QtUMChat::updateUntabbedTooltipMessage(){
//	/*if(isUntabbed()){
//		_ui->btnUntab->setToolTip(tr("Close Window And Insert It Inside The Tabbed Chat Window"));
//	}
//	else{
//		_ui->btnUntab->setToolTip(tr("Open Selected Tab in New Window"));
//	}*/
//}

//VOXOX CHANGE by Rolando - 2009.06.09 
//void QtUMChat::setUntabButtonEnabled(bool enabled){
//	/*_ui->btnUntab->setEnabled(enabled);*/
//}

void QtUMSMS::setVisible(bool visible) {
	if (visible) {
		_chatEdit->setFocus();
	}
}


void QtUMSMS::addToHistory(const QString & contactId, const QString & senderName, const QString & protocolName, const QString & str, const QTime & time, const QDate & date) {
	QTime tmp;
	if (time.isNull()) {
		tmp = QTime::currentTime();
	} else {
		tmp = time;
	}

	QDate dateTmp;
	if (date.isNull()) {
		dateTmp = QDate::currentDate ();
	} else {
		dateTmp = date;
	}

	_ui->chatHistory->insertMessage(contactId, senderName,protocolName, str, tmp, dateTmp);
}

void QtUMSMS::addToHistoryTranslation(const QString & contactId, const QString & senderName, const QString & protocolName, const QString & originalMessage,const QString & translatedMessage, const QTime & time, const QDate & date) {
	QTime tmp;
	if (time.isNull()) {
		tmp = QTime::currentTime();
	} else {
		tmp = time;
	}

	QDate dateTmp;
	if (date.isNull()) {
		dateTmp = QDate::currentDate ();
	} else {
		dateTmp = date;
	}
	
	_ui->chatHistory->insertTranslatedMessage(contactId, senderName,protocolName, originalMessage,translatedMessage , tmp, dateTmp);
}
void QtUMSMS::addStatusMessage(const QString & statusMessage) {
	_ui->chatHistory->insertStatusMessage(statusMessage, QTime::currentTime());
}

void QtUMSMS::sendPressedSlot(){//VOXOX - CJC - 2010.01.18 
	
	if(_translation->isTranslationOn()){

		if(_translation->validateTranslationSettings()){

			if(_translation->isTranslationForSendingMessages()){
				
				sendTranslatedMessage();

			}else{
				sendMessage();
			}
		}

	}else{
		sendMessage();
	}

}
void QtUMSMS::sendMessage() {

	//Drop empty message
	if (_chatEdit->toPlainText().isEmpty()) {
		return;
	}

	QString text = _chatEdit->toPlainText();
	//VOXOX - CJC - 2009.05.08 VoxOx change remove special character
	text.replace("\"", "");
	text.replace("&", tr("and"));
	
	//Change userName


	QString message = QtChatUtils::encodeMessageChatToSMS(getNumber(),getFrom(),text);
	addToHistory("self", _nickName,"SMS", text);
	_imChatSession->sendMessage(message.toUtf8().constData());
	//

	_chatEdit->clear();
	_chatEdit->setFocus();
	_chatEdit->ensureCursorVisible();
	

}



void QtUMSMS::updateAvatarFrame(const QString & contactId) {
	QMutexLocker locker(&_mutex);

	QString dataString = "";

	std::string data	   = getCContactList().getContactIconData( contactId.toStdString() );

	if (data.size() > 0) 
	{
		dataString = QString::fromStdString(data);
	}

	_ui->chatHistory->setAvatarPixmap ( contactId, dataString );

	
}
void QtUMSMS::sendTranslatedMessage() {//VOXOX - CJC - 2010.01.18 

	//Drop empty message
	if (_chatEdit->toPlainText().isEmpty()) {
		return;
	}

	QString text = _chatEdit->toPlainText();//VOXOX CHANGE by Rolando - 2009.12.10 

	_translation->translateAndSendMessage(QString("self"),_nickName,"SMS",text);
	
	_chatEdit->clear();
	_chatEdit->setFocus();
	_chatEdit->ensureCursorVisible();
	
}
void QtUMSMS::processRecievedMessage(const QString & contactId, const QString & senderName, const QString & protocolName,const QString & originalText){
	
	
	if(_translation->isTranslationOn()){

		if(_translation->validateTranslationSettings()){

			if(_translation->isTranslationForReceivingMessages()){

				_translation->translateRecievedMessage(contactId, senderName,protocolName, originalText);
				return;
			}
		}
	}
	addToHistory(contactId, senderName,protocolName, originalText);
	
}
void QtUMSMS::translateMessageSlot(const QString & contactId, const QString & senderName, const QString & protocolName,const QString & originalText,const QString & translatedText, bool success, bool sendMessageToSession){

	if(success){

		QString	originalMessage   = QtChatUtils::encodeMessage( originalText );
		QString	translatedMessage   = QtChatUtils::encodeMessage( translatedText );

		if(sendMessageToSession){
			addToHistoryTranslation(contactId, senderName, protocolName, originalMessage,QString("[translated] "+translatedMessage));		

			QString text = translatedMessage;//VOXOX CHANGE by Rolando - 2009.12.10 
			//VOXOX - CJC - 2009.05.08 VoxOx change remove special character
			text.replace("\"", "");
			text.replace("&", tr("and"));

			QString message = QtChatUtils::encodeMessageChatToSMS(getNumber(),getFrom(),text);
			_imChatSession->sendMessage(message.toUtf8().constData());

		}else{
			addToHistoryTranslation(contactId, senderName, protocolName,translatedMessage ,QString("[original] "+originalMessage));
		}

	}else{
		
		QtVoxMessageBox box(0);
		box.setWindowTitle(tr("VoxOx - Translate Message"));
		box.setText(tr("There was an issue translating your message. Please try again."));//VOXOX - CJC - 2010.01.18 Change this message
		box.setStandardButtons(QMessageBox::Ok);
		box.exec();
		
		//VOXOX - CJC - 2010.01.18 This is a recieved message, so if for some reason errors out, lets add the original message
		if(sendMessageToSession){
			_imChatSession->sendMessage(originalText.toUtf8().constData());
			addToHistory(contactId, senderName, protocolName, originalText);		
		}else{
			addToHistory(contactId, senderName, protocolName, originalText);	
		}
	}
}
void QtUMSMS::translateErrorSlot(){
		
	QtVoxMessageBox box(0);
	box.setWindowTitle(tr("VoxOx - Translate Message"));
	box.setText(tr("There was an issue translating your message. Please try again."));//VOXOX - CJC - 2010.01.18 Change this message
	box.setStandardButtons(QMessageBox::Ok);
	box.exec();
		
}
void QtUMSMS::updateUserAvatar() {

	QString data;
	UserProfile& userProfile = _cChatHandler.getCUserProfile().getUserProfile();
	std::string myData = userProfile.getIcon().getData();
	data = QString::fromStdString(myData);

	_ui->chatHistory->setAvatarPixmap("self", data);
}


void QtUMSMS::saveHistoryAsHtml() {
	_ui->chatHistory->saveHistoryAsHtml();
}

void QtUMSMS::setContactConnected(bool connected) 
{

	QString contactName = getCContactList().getContactBestShortDisplayName( _contactId.toStdString() ).c_str();

	if (connected && !_isContactConnected) 
	{
		addStatusMessage(QString(tr("%1 is connected.")).arg(contactName));
	} 
	else if (!connected && _isContactConnected) 
	{
		addStatusMessage(QString(tr("%1 appears to be offline. Messages you send will be delivered when they sign in")).arg(contactName));
	}

	_isContactConnected = connected;
}


// VOXOX CHANGE by ASV 06-05-2009: added to implement the functionality of the Edit Menu
QTextEdit * QtUMSMS::getChatEdit() {
	return _chatEdit;
}

/**
 * Helper method to clone a key event, so that it can be send to another widget
 */
static QKeyEvent* cloneKeyEvent(QKeyEvent* event) {
	return new QKeyEvent(event->type(), event->key(), event->modifiers(), event->text());
}

bool QtUMSMS::eventFilter(QObject* object, QEvent* event) {
	/*if (object == _ui->chatHistory && event->type() == QEvent::KeyPress) {
		return historyKeyPressEventFilter(static_cast<QKeyEvent*>(event));
	}*/
	if (object == _chatEdit && event->type() == QEvent::KeyPress) {
		return editKeyPressEventFilter(static_cast<QKeyEvent*>(event));
	}
	return false;
}

bool QtUMSMS::historyKeyPressEventFilter(QKeyEvent* event) {
	// Set focus on edit widget if the user types a "printable" character
	/*if (event->text().size() > 0 && event->text().at(0).isPrint() && _chatEdit->isEnabled()) {
		_chatEdit->setFocus();
		QKeyEvent* newEvent = cloneKeyEvent(event);
		QApplication::postEvent(_chatEdit, newEvent);
		return true;
	}*/
	return false;
}

bool QtUMSMS::editKeyPressEventFilter(QKeyEvent* event) {
	int key = event->key();
	
	/*if (key == Qt::Key_PageUp || key == Qt::Key_PageDown) {
		QKeyEvent* newEvent = cloneKeyEvent(event);
		QApplication::postEvent(_ui->chatHistory, newEvent);
		return true;
	}*/

	// Send message with Enter key, unless a modifier is pressed
	if ((key == Qt::Key_Enter || key == Qt::Key_Return) && 
		(event->modifiers() == Qt::NoModifier || event->modifiers() == Qt::KeypadModifier)) 
	{
		sendPressedSlot();//VOXOX - CJC - 2010.01.19 
		return true;
	}

#ifdef OS_MACOSX
	static const int REAL_CTRL_MODIFIER = Qt::MetaModifier;
#else
	static const int REAL_CTRL_MODIFIER = Qt::ControlModifier;
#endif
	bool realCtrlPressed = event->modifiers() & REAL_CTRL_MODIFIER;

	if (key == Qt::Key_Tab && realCtrlPressed) {
		// We use realCtrlPressed because on MacOS, Qt::ControlModifier
		// corresponds to the Command key and Command + Tab is used to switch
		// between applications, (like Alt + Tab on Windows)
//		ctrlTabPressed();
		return true;
	}

	return false;
}

void QtUMSMS::translationClickedSlot(){

	_translationWidget->initSettings(_translation->getTranslationMode(),_translation->getTranslationSourceLocale(),_translation->getTranslationDestLocale());
	_translationWidget->showDialog();

}


void QtUMSMS::translationSettingsSlot(QtEnumTranslationMode::Mode mode,const QtTranslationLanguage & source,const QtTranslationLanguage & dest){

	_translation->setTranslationSourceLocale(source);
	_translation->setTranslationDestLocale(dest);
	_translation->setTranslationMode(mode);
	_translation->saveTranslationSettings();
	if(_translation->isTranslationOn()){
			addStatusMessage(QString("The Universal Translator has been turned on. Your language is %1 and your contact's language is %2. Click on a message to view the translated text").arg(_translation->getTranslationSourceLocale().getTranslationName()).arg(_translation->getTranslationDestLocale().getTranslationName()));
	}else{
			addStatusMessage("The Universal Translator has been turned off.");
	}

	updateTranslationUI();
}
CContactList& QtUMSMS::getCContactList()
{
	return _cContactList;
}