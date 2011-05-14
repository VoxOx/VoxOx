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
* Class widget to support chat to email
* @author Chris Jimenez C 
* @date 2009.05.11
*/



#ifndef OWQTUMEMAIL_H
#define OWQTUMEMAIL_H

#include "emoticons/QtEmoticon.h"
#include "translation/QtTranslationLanguage.h"
#include "QtUMTranslation.h"
#include "QtUMBase.h"

#include "QtEnumUMMode.h"
#include "translation/QtEnumTranslationMode.h"//VOXOX - CJC - 2010.01.18 

#include <control/chat/CChatHandler.h>

#include <imwrapper/IMContact.h>
#include <imwrapper/IMChatSession.h>
#include <imwrapper/IMChat.h>

#include <util/Trackable.h>

#include <QtCore/QTime>

#include <QtCore/QDate>

#include <QtCore/QMutex>


#include <QtGui/QTextEdit>
#if (defined OS_WINDOWS)

	#include <qtutil/SpellTextEdit.h>
#endif

class CContactList;	//VOXOX - JRT - 2009.07.26 
class QtEmoticonsWidget;
class QtTranslationWidget;//VOXOX - CJC - 2010.01.18 

class QFont;

namespace Ui { class UMEmail; }
/**
 *
 * @ingroup presentation
 * @author Mr K.
 * @author Mathieu Stute
 */
class QtUMEmail : public QtUMBase {
	Q_OBJECT
public:


	QtUMEmail(CChatHandler & cChatHandler, CContactList & cContactList,
		IMChatSession * imChatSession, QWidget * parent,QString key,QtEnumUMMode::Mode mode = QtEnumUMMode::UniversalMessageChatToEmail);

	virtual ~QtUMEmail();

	void init(const QString & nickname,const QString & senderName,const QString & contactId,const QString & email,const QString & from);

	void setNickName(const QString & nickname) {_nickName = nickname;}

	void setEmail(const QString & email) {_email = email;}
	
	void setSubject(const QString & subject){_subject =subject;}

	void setFrom(const QString & from) {_from = from;}

	const QString & nickName() const {return _nickName;}

	QString & getEmail(){return _email;}

	QString & getFrom(){return _from;}

	QString & getSubject(){return _subject;}

	bool canDoMultiChat() {return false;}

	int getSessionId() const {return _sessionId;}

	virtual QWidget * getWidget(){return _widget;}
	IMChatSession * getIMChatSession() {return _imChatSession;}

	virtual bool	isGroupChat(){return false;}

	void buildUI();
	// called from the model's thread

	void setIMChatSession(IMChatSession * imChatSession);

	void loadLastChatHistory(QString contactId);

	void closeSession();

	/**
	 * Adds a message to the history and display it.
	 *
     * @param contactId id of the contact who sends the message, or "self" if the sender is the local user
	 * @param senderName name of the contact who sends the message
	 * @param str the message
	 * @param time the time of the message, will use the current time if empty
	 */
	//VOXOX CHANGE CJC SUPPORT ALSO DATE
	void addToHistory(const QString & contactId, const QString & senderName, const QString & protocolName, const QString & str, const QTime & time = QTime(), const QDate & date = QDate());

	void addToHistoryTranslation(const QString & contactId, const QString & senderName, const QString & protocolName, const QString & originalMessage,const QString & translatedMessage, const QTime & time = QTime(), const QDate & date = QDate());
	/**
	 * Displays a satus message.
	 *
	 * Behaves same as addToHistory but set the text color to the status message
	 * color and does not display a contact name.
	 */
	void addStatusMessage(const QString & statusMessage);


	void saveHistoryAsHtml();

	/**
	 * Sets the Contact state.
	 *
	 * If not connected and the last status was connected,
	 * the ChatEditWidget will be disabled and a status message
	 * will be displayed saying the contact went offline.
	 *
	 * If connect and the last status was disconnected,
	 * the ChatEditWidget will be enabled and a status message
	 * will be displayed saying the contact when online.
	 **/
	void setContactConnected(bool connected);


//	bool canDoFileTransfer();	//VOXOX - JRT - 2009.07.27 - Moved to model

	QTextEdit *getChatEdit(); // VOXOX CHANGE by ASV 06-05-2009: added to implement the functionality of the Edit Menu

	//bool isUntabbed();//VOXOX CHANGE by Rolando - 2009.06.08

	//void setUntabButtonEnabled(bool enabled);//VOXOX CHANGE by Rolando - 2009.06.09 

public Q_SLOTS:

	/**
	 * QtChatEditWidget text has changed
	 */

	void processRecievedMessage(const QString & contactId, const QString & senderName, const QString & protocolName,const QString & originalText);//VOXOX - CJC - 2010.01.18 
	void changeFont();

	virtual void setVisible(bool visible);

//	void sendFileToSession(const QString & filename);	//VOXOX - JRT - 2009.07.27 - Not called anywhere.

	void translationClickedSlot();//VOXOX - CJC - 2010.01.17 

Q_SIGNALS:

//	void newMessage(IMChatSession* session,const QString & msg);	//VOXOX - JRT - 2009.07.07 not used

	void contactAddedEventSignal(const IMContact & imContact);

	void contactRemovedEventSignal(const IMContact & imContact);

	//void ctrlTabPressed();

	//void unTabClicked();//VOXOX CHANGE by Rolando - 2009.06.08

//	void implodeChat();//VOXOX CHANGE by Rolando - 2009.06.08 


protected:
	virtual bool eventFilter(QObject* object, QEvent* event);

private Q_SLOTS:


	void sendPressedSlot();//VOXOX - CJC - 2010.01.18 
	void changeFontColor();



	void updateUserAvatar();

	void translateMessageSlot(const QString & contactId, const QString & senderName, const QString & protocolName,const QString & originalText,const QString & translatedText, bool success, bool sendMessageToSession);//VOXOX - CJC - 2010.01.18 

	void translateErrorSlot();//VOXOX - CJC - 2010.01.18 

	void translationSettingsSlot(QtEnumTranslationMode::Mode mode,const QtTranslationLanguage & source,const QtTranslationLanguage & dest);

private:
	void sendMessage();

	void sendTranslatedMessage();
	CContactList& getCContactList();

	//void updateUntabbedTooltipMessage();//VOXOX CHANGE by Rolando - 2009.06.09 
	


	void updateAvatarFrame(const QString & contactId);

	bool historyKeyPressEventFilter(QKeyEvent* event);

	bool editKeyPressEventFilter(QKeyEvent* event);

	void updateTranslationUI();//VOXOX - CJC - 2010.01.22 
	CChatHandler & _cChatHandler;

	CContactList & _cContactList;

	IMChatSession * _imChatSession;


	int _sessionId;

	QString _nickName;

	QString _email;

	QString _subject;

	QString _from;

	mutable QMutex _mutex;

	QtTranslationWidget * _translationWidget;//VOXOX - CJC - 2010.01.17 

	QtUMTranslation * _translation;
	Ui::UMEmail * _ui;

	QWidget * _widget;
	// font style settings
	QFont _currentFont;
	QColor _currentColor;
	////
	
	bool _untabbed;

	#if (defined OS_MACOSX)
	//VOXOX CHANGE by Alexander 02-16-09: Added this variable to use polymorphism for the chat text edit and be able to use separate spell checkers. 
		QTextEdit * _chatEdit;
	#endif
	#if (defined OS_WINDOWS) || (defined OS_LINUX)
		 QTextEdit * _chatEdit;
	#endif
};

#endif //OWQtUMEmail_H
