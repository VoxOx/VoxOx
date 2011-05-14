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

#ifndef OWQTUMCHAT_H
#define OWQTUMCHAT_H

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

class CContactList;	//VOXOX - JRT - 2009.07.27 
class QtEmoticonsWidget;
class QtTranslationWidget;//VOXOX - CJC - 2010.01.18 

class QFont;
class QTimer;

namespace Ui { class UMChat; }
/**
 *
 * @ingroup presentation
 * @author Mr K.
 * @author Mathieu Stute
 */
class QtUMChat : public QtUMBase {
	Q_OBJECT
public:

	QtUMChat(CChatHandler & cChatHandler, CContactList & cContactList,
		IMChatSession * imChatSession, QWidget * parent,QString key,QtEnumUMMode::Mode mode = QtEnumUMMode::UniversalMessageChat);

	virtual ~QtUMChat();
	
	void init(const QString & nickname,const QString & senderName,const QString & contactId);

	void setNickName(const QString & nickname) {_nickName = nickname;}

	const QString & nickName() const {return _nickName;}

	bool	canDoMultiChat();
	virtual bool	isGroupChat();

	int getSessionId() const {return _sessionId;}

	virtual QWidget * getWidget(){return _widget;}

	IMChatSession * getIMChatSession() {return _imChatSession;}

	void buildUI();

	void setIMChatSession(IMChatSession * imChatSession);

	void loadLastChatHistory(QString contactId);

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
	void chatEditTextChanged();

	void changeFont();

	void chooseEmoticon();

	void emoticonSelected(QtEmoticon emoticon);

	void showInviteDialog();

	void contactAddedEventSlot(const QString & contactId);

	void contactRemovedEventSlot(const QString & contactId);

	void addToAvatarFrame(const QString & contactId,const QString & data);

	virtual void setVisible(bool visible);

//	void sendFileToSession(const QString & filename);	//VOXOX - JRT - 2009.07.27 - Not called anywhere.

	void translationClickedSlot();//VOXOX - CJC - 2010.01.17 

Q_SIGNALS:

//	void newMessage(IMChatSession* session,const QString & msg);	//VOXOX - JRT - 2009.07.07 not used

	void ctrlTabPressed();

	//void unTabClicked();//VOXOX CHANGE by Rolando - 2009.06.08

//	void implodeChat();//VOXOX CHANGE by Rolando - 2009.06.08 

protected:
	virtual bool eventFilter(QObject* object, QEvent* event);

private Q_SLOTS:


	void sendPressedSlot();//VOXOX - CJC - 2010.01.18 
	void changeFontColor();

	
	void stoppedTypingSlot();

	void updateUserAvatar();

	void translateMessageSlot(const QString & contactId, const QString & senderName, const QString & protocolName,const QString & originalText,const QString & translatedText, bool success, bool sendMessageToSession);//VOXOX - CJC - 2010.01.18 

	void translateErrorSlot();//VOXOX - CJC - 2010.01.18 

	void translationSettingsSlot(QtEnumTranslationMode::Mode mode,const QtTranslationLanguage & source,const QtTranslationLanguage & dest);

	void disableTranslation();//VOXOX - CJC - 2010.02.12 

private:
	void sendMessage();

	void sendTranslatedMessage();
	CContactList&	getCContactList();

	//void updateUntabbedTooltipMessage();//VOXOX CHANGE by Rolando - 2009.06.09 
	
	void updateAvatarFrame();

	

	bool historyKeyPressEventFilter(QKeyEvent* event);

	bool editKeyPressEventFilter(QKeyEvent* event);

	void updateTranslationUI();//VOXOX - CJC - 2010.01.22 
	CChatHandler & _cChatHandler;

	CContactList & _cContactList;

	IMChatSession * _imChatSession;

	int _sessionId;

	QString _nickName;

	QString _senderName;//VOXOX - CJC - 2009.08.24 

	mutable QMutex _mutex;

	QtEmoticonsWidget * _emoticonsWidget;

	QtTranslationWidget * _translationWidget;//VOXOX - CJC - 2010.01.17 

	QtUMTranslation * _translation;

	Ui::UMChat * _ui;

	QWidget * _widget;

	// font style settings
	QFont _currentFont;
	QColor _currentColor;


	bool _untabbed;//VOXOX CHANGE by Rolando - 2009.06.08 
	
	/** stopped typing timer */
	QTimer * _stoppedTypingTimer;
	
	bool _isTyping;
	
	bool _justOpenedWindow; //VOXOX -ASV- 08-04-2009: fix to eliminate the typing indicator when a new chat session opens
	
	#if (defined OS_MACOSX)
	//VOXOX CHANGE by Alexander 02-16-09: Added this variable to use polymorphism for the chat text edit and be able to use separate spell checkers. 
		QTextEdit * _chatEdit;
	#endif
	#if (defined OS_WINDOWS) || (defined OS_LINUX)
		 QTextEdit * _chatEdit;
	#endif
};

#endif //OWQtUMChat_H
