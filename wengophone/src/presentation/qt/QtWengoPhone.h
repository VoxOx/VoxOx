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

#ifndef OWQTWENGOPHONE_H
#define OWQTWENGOPHONE_H

#include <presentation/PWengoPhone.h>

#include <model/phoneline/EnumMakeCallError.h>
#include <model/network/NetworkProxyDiscovery.h>

#include <presentation/qt/login/QtVoxOxLoginMessage.h>
#include <presentation/qt/phonecall/QtContactCallListWidget.h>
#include <presentation/qt/chat/QtEnumUMMode.h>
#include <presentation/qt/contactlist/QtEnumSortOption.h>	//VOXOX - JRT - 2009.10.08 

#include <imwrapper/EnumPresenceState.h>

#include <qtutil/QObjectThreadSafe.h>

#include <pixertool/pixertool.h>

#include <cutil/global.h>
#include <util/Trackable.h>

#include <QtGui/QMainWindow>
#include <QtCore/QVariant>
#include <QtGui/QAction>

#include <string>
#include <memory>

class CWengoPhone;
class CContactList;		//VOXOX - JRT - 2009.10.08 
class Config;
class ContactInfo;
class IMContact;
class PPhoneLine;
class PhoneCall;
class PresenceHandler;
class QtCallBar;
class QtContactCallListWidget;
class QtContactList;
class QtFileTransfer;
class QtHistoryWidget;
class QtIdle;
class QtLanguage;
class QtPhoneCall;
class QtProfileBar;
class QtSms;
class QtStatusBar;
class QtToolBar;
class QtVoxWindowManager;//VOXOX - CJC - 2009.07.09 
class QtSystray;
class QtToaster;
class QtBrowserWidget;
class QtDialpad;
class QtConferenceCallWidget;
class UserProfile;
class QtPhoneComboBox;
class QtIMAccountMonitor;
class QtVoxOxLogin;
class QtVoxOxLoginMessage;
class QtVoxOxLoadingLogin;
class QtUMWindow;

#if (defined OS_WINDOWS) && (QT_EDITION == QT_EDITION_DESKTOP)
class QtWebDirectory;
#endif

class QWidget;
class QMenu;
namespace Ui { class WengoPhoneWindow; }

/**
 * Qt Presentation component for WengoPhone.
 *
 * @author Tanguy Krotoff
 */
class QtWengoPhone : public QObjectThreadSafe, public PWengoPhone, public Trackable {
	Q_OBJECT
public:

	QtWengoPhone(CWengoPhone & cWengoPhone);

	~QtWengoPhone();

	void addPhoneCall(QtPhoneCall * qtPhoneCall);

	void addToConference(QtPhoneCall * qtPhoneCall);

	void updatePresentation();

	void dialpad(const std::string & tone);

	//VOXOX CHANGE Rolando 04-02-09
	void dialpadBackButtonPressed();

	void connectionStatusEventHandler(int totalSteps, int currentStep, const std::string & infoMsg);

	QWidget * getWidget() const;

	void setQtDialpad(QtDialpad * qtDialpad);

	QtDialpad * getQtDialpad();

	void setQtContactList(QtContactList * qtContactList);
	QtContactList * getQtContactList() const;

	void setQtHistoryWidget(QtHistoryWidget * qtHistoryWidget);

	void setQtSms(QtSms * qtSms);
	QtSms * getQtSms() const;

	QtStatusBar & getQtStatusBar() const;

	QtFileTransfer * getFileTransfer() const;

	QtToolBar * getQtToolBar() const;

	QtVoxWindowManager * getQtVoxWindowManager() const;//VOXOX - CJC - 2009.07.09 

	QtCallBar & getQtCallBar() const;

	QtSystray & getQtSystray() const;

	QtLanguage & getQtLanguage() const;

	QtProfileBar * getQtProfileBar() const;

	CWengoPhone & getCWengoPhone() const;

	QToolBar *getToolBar() const; // VOXOX -ASV- 10.09.2009
	
	QtHistoryWidget * getHistoryWidget(){
		return _qtHistoryWidget;
	};

#if (defined OS_WINDOWS) && (QT_EDITION == QT_EDITION_DESKTOP)
	QtWebDirectory * getQtWebDirectory() const;
#endif

	void setChatWindow(QtUMWindow * chatWindow);
	QtUMWindow* getChatWindow() const;
        void closeChatWindow ();

	void showHistory();

	void showHomeTab();//VOXOX - CJC - 2009.06.29 

	void ensureVisible();

	void showAddContact(ContactInfo contactInfo);

	virtual void bringMainWindowToFront();

	void installQtBrowserWidget();

	void uninstallQtBrowserWidget();

	void updateStyle();

	QtBrowserWidget * getQtBrowserWidget() const;

	void setCallPushButtonImage();

	void callButtonDialpadClicked(std::string phoneNumber);

	void makeCall(std::string phoneNumber);

	void showConferenceWidget();

	void setActiveTabBeforeCallCurrent();

	void showLoginWindow(std::string profileName = "", bool isARetry = false);//VOXOX CHANGE by Rolando 2009.05.05

	void removeLoginWindow();//VOXOX CHANGE by Rolando 2009.05.05

	void showLoginMessageWindow();//VOXOX CHANGE by Rolando 2009.05.05

	QtVoxOxLoginMessage * getLoginMessageWindow();//VOXOX CHANGE by Rolando 2009.05.05

	QtVoxOxLogin * getLoginWindow();//VOXOX CHANGE by Rolando 2009.05.05

	void setVisibleStatusBar(bool visible);//VOXOX CHANGE by Rolando 2009.05.05

	QWidget * getCurrentTabWidget();


	//QtContactCallListWidget * getQtContactCallListWidget(QWidget *);//VOXOX CHANGE by Rolando - 2009.05.15

	//VOXOX CHANGE by Rolando - 2009.06.02 - method used to show a message indicating that a call failed
	void showCallErrorMessage(EnumMakeCallError::MakeCallError error, std::string phoneNumber);

	bool textIsPhoneNumber(QString text);//VOXOX CHANGE by Rolando - 2009.06.23 

	void readMainWindowSettings();//VOXOX - CJC - 2009.09.23 

	void saveMainWindowSettings();//VOXOX - CJC - 2009.09.23 
	
	void notifyContactManagerContactAdded( const std::string& contactId, int qtId );	//VOXOX - JRT - 2009.09.23 

	void UpdateViewGroupBy( QtEnumSortOption::SortOption groupOption );		//VOXOX - JRT - 2009.10.08 
	void UpdateViewSortBy ( QtEnumSortOption::SortOption sortOption  );		//VOXOX - JRT - 2009.10.08 

public Q_SLOTS:

	void currentUserProfileWillDieEventHandlerSlot();

	void userProfileInitializedEventHandlerSlot();

	void hangUpButtonClicked();

	void logOff();			//VOXOX - JRT - 2009.07.12 
	void exitApplication();	//VOXOX - JRT - 2009.07.13 

	void addToConference(QString phoneNumber, PhoneCall * targetCall);

	void setCurrentUserProfileEventHandlerSlot();

	//VOXOX CHANGE Rolando 03-20-09
	void showOrHideProfileBarSlot(bool);

	//VOXOX CHANGE Rolando 03-25-09
	void showOrHideCallBarSlot(bool showCallBar);

	void slotQtVoxOxLoginDestroyed(QObject* object);//VOXOX CHANGE by Rolando 2009.05.05

	void slotQtVoxOxLoginMessageDestroyed(QObject* object);//VOXOX CHANGE by Rolando 2009.05.05

	void slotQtVoxOxLoadingLoginDestroyed(QObject*);//VOXOX CHANGE by Rolando 2009.05.05

	void showLoadingLoginMessage(QString profileName);//VOXOX CHANGE by Rolando 2009.05.05

	void acceptLoginButtonClicked(QString profileName);//VOXOX CHANGE by Rolando 2009.05.05

	void tryAgainSlot(std::string profileName);//VOXOX CHANGE by Rolando 2009.05.05

	void slotQtContactCallListWidgetIsClosing(QString key);//VOXOX CHANGE by Rolando - 2009.05.15

	void cancelConferenceCall(std::string phoneNumber);//VOXOX CHANGE by Rolando - 2009.05.29 



private Q_SLOTS:

	void changeGearMenu();

	void callButtonClicked();

	void enableCallButton();

	void phoneComboBoxClicked();

	void closeWindow();

	void languageChanged();
	//VOXOX - CJC - 2009.05.01 Remember users last tab
	void tabIndexChange(int);

	//VOXOX CHANGE Rolando 03-25-09
	void aboutToHangUp();

	void populateFilterGroup();		//VOXOX - JRT - 2009.05.30 

Q_SIGNALS:

	/**
	 * Current user profile has been deleted.
	 *
	 * Graphical components should be re-initialized:
	 * QtContactList, QtHistoryWidget, QtProfileBar are removed.
	 * QtSystray, QtBrowserWidget have to be re-initialized.
	 */
	void userProfileDeleted();

private:
	Config& getConfig();		//VOXOX - JRT - 2009.10.08 

	void doLogOff( bool quitting );			//VOXOX - JRT - 2009.07.13 
	void prepareToExitApplication();		//VOXOX - JRT - 2009.07.13 

	void initUi();

	void initThreadSafe();

	void initQtSettings(); //VOXOX -ASV- 2009.09.17
	
	/**
	 * Set the geometry of the mainwindow (position and size).
	 *
	 * Load last size and position. If the mainwindow is not visible use default values.
	 */
	void mainWindowGeometry(Config & config);

	/**
	 * Initializes pickup and hangup buttons inside the main window.
	 *
	 * This is called by QtPhoneCall to re-initialize the buttons
	 * since QtPhoneCall modifies the behaviour of this buttons.
	 */
	void initCallButtons();

	void updatePresentationThreadSafe();

	void proxyNeedsAuthenticationEventHandler(NetworkProxyDiscovery & sender, NetworkProxy networkProxy);

	void proxyNeedsAuthenticationEventHandlerThreadSafe(NetworkProxy networkProxy);

	void wrongProxyAuthenticationEventHandler(NetworkProxyDiscovery & sender, NetworkProxy networkProxy);

	void makeCallErrorEventHandler(EnumMakeCallError::MakeCallError, std::string);

	void makeCallErrorEventHandlerThreadSafe(EnumMakeCallError::MakeCallError, std::string);

	void exitEvent();

	bool checkLogOffIsOk();

	QString generateCallKey(const QString & number, QtEnumUMMode::Mode mode);//VOXOX - CJC - 2009.09.07 

	QtContactCallListWidget * getCurrentConferenceCall();
	CContactList*			getCContactList();			//VOXOX - JRT - 2009.10.08 


#ifdef OS_MACOSX
	void fixMacOSXMenus();
#endif

	void loadStyleSheets();

	/** Direct link to the control. */
	CWengoPhone & _cWengoPhone;

	Ui::WengoPhoneWindow * _ui;

	QMainWindow * _wengoPhoneWindow;

	QtSystray * _qtSystray;

	QtSms * _qtSms;

	QtContactList * _qtContactList;

	QtHistoryWidget * _qtHistoryWidget;

	typedef std::map < QString, QtContactCallListWidget * > QtContactCallListWidgetMap;

	QtContactCallListWidgetMap _qtContactCallListWidgetMap;

	QtIdle * _qtIdle;

	QtLanguage * _qtLanguage;

	QtToolBar * _qtToolBar;

	QtVoxWindowManager * _qtVoxWindowManager;

	QtStatusBar * _qtStatusBar;

	QtFileTransfer * _qtFileTransfer;

	QtBrowserWidget * _qtBrowserWidget;

	QtDialpad * _qtDialpad;

	QtVoxOxLogin * _qtVoxOxLogin;//VOXOX CHANGE by Rolando 2009.05.05

	QtVoxOxLoginMessage * _qtVoxOxLoginMessage;//VOXOX CHANGE by Rolando 2009.05.05

	QtVoxOxLoadingLogin * _qtVoxOxLoadingLogin;//VOXOX CHANGE by Rolando 2009.05.05

	int _qtBrowserWidgetTabIndex;

	QtUMWindow * _chatWindow;

	QWidget * _activeTabBeforeCall;

	std::auto_ptr<QtIMAccountMonitor> _qtIMAccountMonitor;

	bool		_userIsLogged;	//VOXOX - JRT - 2009.07.12 

#if defined(OS_MACOSX)	
	QToolBar *_toolBarToggle;// VOXOX -ASV- 10.09.2009
#endif
	
#if (defined OS_WINDOWS) && (QT_EDITION == QT_EDITION_DESKTOP)
	QtWebDirectory * _qtWebDirectory;
#endif

};

#endif	//OWQTWENGOPHONE_H
