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

#ifndef OWQTUMWINDOW_H
#define OWQTUMWINDOW_H

#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>




#include <control/chat/CChatHandler.h>

#include <imwrapper/IMChat.h>

#include <util/Trackable.h>
#include <model/phoneline/EnumMakeCallError.h>
#include <thread/ThreadEvent.h>
#include <presentation/PFactory.h>
#include <presentation/qt/contactlist/QtContactPixmap.h>
#include <presentation/qt/chat/umtreewidget/QtUMItemList.h>//VOXOX CHANGE by Rolando - 2009.08.17 

#include <string>
#include <map>

//#include "QtChatWidget.h"//VOXOX CHANGE by Rolando - 2009.10.28 
#include "QtUMBase.h"
#include "QtUMHelper.h"

#include "QtEnumUMMode.h"
#include "QtPopUpChatDialog.h"//VOXOX CHANGE by Rolando - 2009.06.10
#include "QtPopUpChatManager.h"//VOXOX CHANGE by Rolando - 2009.06.10 

class QtUniversalMessageTabWidget;
class QtWengoPhone;

class QtStatusBar;

class QtContactCallListWidget;

class CUserProfile;
class IMContact;
class IMChatSession;
class ContactProfile;
class Contact;		

class QVBoxLayout;
class QAction;

namespace Ui { class UMWindow; }

/**
 *
 * @ingroup presentation
 * @author Mathieu Stute.
 */
class QtUMWindow : public QMainWindow, public Trackable {
	Q_OBJECT
public:
	QtUMWindow(QWidget * parent, CChatHandler & cChatHandler, QtWengoPhone & qtWengoPhone);
	virtual ~QtUMWindow();

	void createToolButtons();//VOXOX - CJC - 2009.08.17 

	void imChatSessionCreatedHandler(IMChatSession * imChatSession);

	//VOXOX - CJC - 2009.05.07 Handler for created chat to email session
	void imChatToEmailSessionCreatedHandler(IMChatSession * imChatSession,std::string & email);

	void sendMessageCreatedHandler(IMChatSession * imChatSession,std::string & message);//VOXOX - CJC - 2009.10.14 

	//VOXOX - CJC - 2009.05.19 Chat to sms
	void imChatToSMSSessionCreatedHandler(IMChatSession * imChatSession,std::string & number);


	void addSendFileWidgetTab(const IMContact & from);	//VOXOX CHANGE CJC - add different tabs

	void addSendFaxWidgetTab(std::string faxNumber);	//VOXOX CHANGE CJC - add different tabs

	void addCall(const QString & phoneNumber,QtContactCallListWidget * qtContactCallListWidget);	//VOXOX - CJC - 2009.06.03 

	void removeCallTabWidget(QString key);//VOXOX CHANGE by Rolando - 2009.05.28

	void hangUpCurrentCall();
	
	
Q_SIGNALS:
	void messageReceivedSignal        (IMChatSession * sender);
	void typingStateChangedSignal     ( const IMChatSession* sender, const IMContact * imContact, const IMChat::TypingState * state);
	void incomingGroupChatInviteSignal( const IMChatSession* sender, const GroupChatInfo* gcInfo );
	void tabNumberChangedSignal(int tabCount); // VOXOX CHANGE by ASV 06-29-2009: added to implement the close tab functionality

	
public Q_SLOTS:

	void show();

	void showMinimized();

	void incomingGroupChatInvite( const GroupChatInfo& gcInfo);	//VOXOX - JRT - 2009.06.16 

	void closeWindow();

	void startChat();	//VOXOX CHANGE CJC START CHAT
	void startCall();
	void startSMS();

	void startEmail();

	void startSendFile();
	void startSendFax();//VOXOX - CJC - 2009.06.23 
	void startProfile();
	void startPhoneSettings();//VOXOX CHANGE by Rolando - 2009.07.09 

	void handleAddToChat();		//VOXOX - JRT - 2009.06.12 
	void closeCurrentUMItemSlot(QString itemKey);//VOXOX CHANGE by Rolando - 2009.08.27 
	void groupClosedSlot(QString itemKey);//VOXOX - CJC - 2009.10.02 
	void itemClickedSlot(QString itemKey);//VOXOX CHANGE by Rolando - 2009.08.28 

	void groupClickedSlot(QString key);//VOXOX - CJC - 2009.09.08 This slot triggers when a group or contact inside group is clicked, TODO: use it when doing group chat functionality

protected:


	void setAddMenu(QMenu * addMenu);	//VOXOX CHANGE CJC - support different menus on the status bar
	void setSettingsMenu(QMenu * settingsMenu);

	/**
	 * Installed on QTextEdit widgets. Used to update clipboard actions.
	 */
	virtual bool eventFilter(QObject* object, QEvent* event);

private Q_SLOTS:

	void addUMItemSlot(QtUMBase * item, bool userCreated);
	void setTypingSlot(QtUMBase * item,bool typing);
	void messageReceivedSlot(QtUMBase * item);
	void statusChangedSlot(QtUMBase * item);
	void contactAddedSlot(QtUMBase * item,const QString & contactId,const QString & contactKey);
	void contactRemovedSlot(QtUMBase * item,const QString & contactId,const QString & contactKey);

	

	void cut();
	void copy();
	void paste();
	//VOXOX - SEMR - 2009.06.27 Gear Status Bar menu SLOTS
	void changeFontSettings();
	void addContactSlot();
	void spellCheckActive();
	void changeTheme();
	void editContact();
	void deleteContact();
	void openSettings();
	void logOff();
	void showAboutWindow();
	void showSettingsWindow();
	void checkForUpdates();//VOXOX -ASV- 2009.12.03
	void closeActiveTab(); //VOXOX -ASV- 2009.10.09
private:

	void updateClipboardActions();

	bool addUMItem(QtUMBase * item);

	void addUMItemToUMList(QtUMBase * item,bool selectedOnInit);

	bool addUMItemToWindow(QtUMBase * item);

	bool exist(QtUMBase * item);
	bool existInWindow(QtUMBase * item);

	void readDockWidgetSettings();
	void saveDockWidgetSettings();

	void readMainWindowSettings();
	void saveMainWindowSettings();

	bool checkWindowHasToClose();

	void updateToolBarActions(QtEnumUMMode::Mode mode);

	QString getPixmapByMode(QtEnumUMMode::Mode mode);

	QString getTittle(QtUMBase * item);

	void updateWindowTitle(QtUMBase * item,bool isTyping);

	CUserProfile* getCUserProfile();
	CContactList& getCContactList() const;												//VOXOX - JRT - 2009.07.26 

	//QtContactPixmap::ContactPixmap getContactPixmap( const std::string& contactId );	//VOXOX - JRT - 2009.07.26 
	QtContactPixmap::ContactPixmap getNormalContactPixmap( const std::string& contactId );//VOXOX CHANGE by Rolando - 2009.10.26 
	
	QtContactPixmap::ContactPixmap getHoverContactPixmap( const std::string& contactId );//VOXOX CHANGE by Rolando - 2009.10.26 
	
	void openChatSession       (IMChatSession* imChatSession); 	//VOXOX - CJC - 2009.05.08 Encapsulade the opening of the chat session.

	void showToaster(const QString & contactId, const QString & contactDisplayName);


	void buildChatStatusMenus();
	void buildPlusMenu(const QString & key);
	void buildGearMenu();
	void addGroupChatAction( QMenu* menu ,const QString & key);	//VOXOX - JRT - 2009.07.08 
	
	void playIncomingChatSound();					//VOXOX - CJC - 2009.05.26 

	void flashWindow();



#if defined(OS_MACOSX)
	QString setMessage(const QString & message);
#endif

	void fixMacOsChatMenu();

	Ui::UMWindow	*			 _ui;

	QtWengoPhone&				 _qtWengoPhone;
	CChatHandler&				 _cChatHandler;
	QtStatusBar*				 _qtStatusBar;
	QtUMHelper *				_qtUMHelper;

	
	QtUMItemList * _qtUMItemList;//VOXOX CHANGE by Rolando - 2009.08.17 

	bool _windowsHasClose;

	QMenuBar *_chatMenu; //VOXOX CHANGE -ASV- 07-14-2009: we need a menu bar for the chat window
	QMenu *_menuWengo;
	QAction *_actionCloseTab;
	QAction *_actionCloseChatWindow;
	QAction *_actionLogOff;
	QAction *_actionSettings;
	QAction *_actionAbout;
	QAction *_actionUpdate;//VOXOX -ASV- 2009.12.03

	QToolBar *_toolBarToggle;// VOXOX -ASV- 10.09.2009
	
};

#endif	//OWQtUMWindow_H

