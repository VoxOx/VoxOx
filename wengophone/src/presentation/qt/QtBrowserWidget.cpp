/*
 * VoxOx, Take Control!
 * Copyright (C) 2005-2009  Telcentris
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

#include "stdafx.h"
#include "QtBrowserWidget.h"
#include "QtWengoPhone.h"
#include "QtToolBar.h"
#include <control/profile/CUserProfile.h>
#include <control/profile/CUserProfileHandler.h>
#include <control/CWengoPhone.h>

#include <model/account/wengo/WengoAccount.h>
#include <model/profile/UserProfile.h>
#include <model/config/Config.h>
#include <model/config/ConfigManager.h>
#include <model/network/NetworkProxyDiscovery.h>

#include <owbrowser/QtBrowser.h>
#include <QtWebKit/QtWebKit>
#include <qtutil/SafeConnect.h>
#include <util/Logger.h>
#include <util/WebBrowser.h>
#include <util/SafeDelete.h>
#include <WengoPhoneBuildId.h>

#include <QtGui/QtGui>

static const std::string LOCAL_WEB_DIR = "webpages";

QtBrowserWidget::QtBrowserWidget(QtWengoPhone & qtWengoPhone)
	: _qtWengoPhone(qtWengoPhone) {

	_qtBrowser = new QtBrowser(_qtWengoPhone.getWidget());
	
	//browser re-initialization
	SAFE_CONNECT(&_qtWengoPhone, SIGNAL(userProfileDeleted()),			SLOT(slotUserProfileDeleted()));
	SAFE_CONNECT(_qtBrowser,     SIGNAL(beforeNavigateSignal(QString)),	SLOT(beforeNavigateSlot(QString)));
	SAFE_CONNECT(_qtBrowser,     SIGNAL(documentCompleteSignal(bool)),	SLOT(documentCompleteSlot(bool)));

	connect     ( getWebFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(initJavascript() ));

	_accountPageStarted = false;
	_jsButtonHandler	= NULL;

	//loadDefaultURL();
}

QtBrowserWidget::~QtBrowserWidget() {
	OWSAFE_DELETE( _qtBrowser		);
	OWSAFE_DELETE( _jsButtonHandler );
}

QWidget * QtBrowserWidget::getWidget() const {
	if (!_qtBrowser) {
		return NULL;
	}

	return (QWidget*) _qtBrowser->getWidget();
}

void QtBrowserWidget::slotUserProfileDeleted() {
	if (_qtBrowser) {
		LOG_DEBUG("delete QtBrowser");
		delete _qtBrowser;
		_qtBrowser = NULL;
	}
}

void QtBrowserWidget::loadDefaultURL() {
	if (!_qtBrowser) {
		return;
	}

	// VOXOX CHANGE by ASV 04-22-2009: getting the resources pages from config instead of QCoreApplication
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	QString url = QString::fromStdString(config.getResourcesDir() + LOCAL_WEB_DIR + "/loading.html");
	url = url.replace("\\","/");
	_qtBrowser->setUrl(url.toStdString());
}

//VOXOX CHANGE by Rolando - 2009.07.24 
void QtBrowserWidget::loadOfflineURL() {
	if (!_qtBrowser) {
		return;
	}

	Config & config = ConfigManager::getInstance().getCurrentConfig();
	QString url = QString::fromStdString(config.getResourcesDir() + LOCAL_WEB_DIR + "/offline.html");
	url = url.replace("\\","/");
	_qtBrowser->setUrl(url.toStdString());
}

void QtBrowserWidget::documentCompleteSlot(bool ok) {
	//VOXOX CHANGE CJC verify that we need have start loading the actual account page and not other
	if (!_accountPageStarted) {
		return;
	}
	if(ok){
		_accountPageStarted = false;
		loginAccount();
	}
}

void QtBrowserWidget::initJavascript()
{
	JSButtonHandler* _jsButtonHandler = new JSButtonHandler( this, NULL );
	getWebFrame()->addToJavaScriptWindowObject( "qtEventSource", _jsButtonHandler );
}

//-----------------------------------------------------------------------------

void QtBrowserWidget::loadAccountURL() {
	if (!_qtBrowser) {
		return;
	}
	
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	////VOXOX - CJC - 2009.06.30 Curtis request
	std::string defaultURL = config.getHttpHomeUrl() + "?v=" + WengoPhoneBuildId::getFullVersion();//VOXOX - CJC - 2009.06.30 
	//std::string defaultURL = config.getHttpHomeUrl() + "?v=1.99";//VOXOX - CJC - 2009.06.30 
	_accountPageStarted = true;
	_qtBrowser->setUrl(defaultURL);
	
}

void QtBrowserWidget::openVoiceMail() {//VOXOX - CJC - 2009.06.29 
	if (!_qtBrowser) {
		return;
	}
	
	QString javascript = QString("showVoicemail()");
//	QWebView * webView = _qtBrowser->getQtWebView();
//	webView->page()->mainFrame()->evaluateJavaScript(javascript);
	getWebFrame()->evaluateJavaScript(javascript);
	
}
//VOXOX CHANGE CJC Login acccount
void QtBrowserWidget::loginAccount()
{
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	CUserProfile * cUserProfile = _qtWengoPhone.getCWengoPhone().getCUserProfileHandler().getCUserProfile();
	if (cUserProfile) {
		UserProfile & userProfile = cUserProfile->getUserProfile();

		if (userProfile.getActivePhoneLine()) {

			if (userProfile.hasWengoAccount()) {

					WengoAccount wengoAccount = *userProfile.getWengoAccount();

					std::string javascriptParams = "voxox.directlogin('" + wengoAccount.getWengoLogin() + "','" + wengoAccount.getWengoPassword() +"')";
					LOG_DEBUG("JS Function:" + javascriptParams);
					QString jsToExecute = QString::fromStdString(javascriptParams);

					//QWebView * webView = _qtBrowser->getQtWebView();
					//webView->page()->mainFrame()->evaluateJavaScript(jsToExecute);
					getWebFrame()->evaluateJavaScript(jsToExecute);
			}
		}
	}
}

//-----------------------------------------------------------------------------
	
QWebFrame* QtBrowserWidget::getWebFrame()
{
	QWebView * webView = _qtBrowser->getQtWebView();
	return webView->page()->mainFrame();
}

//VOXOX - JRT - 2009.06.30 - This eliminates the need to navigate from and back
//		to fake URLs.  JS tells us which button was clicked.
//void QtBrowserWidget::handleButtonClick( int buttonId,int actionId )
void QtBrowserWidget::handleButtonClick( int buttonId, QString action )
{
	switch( buttonId )
	{
	case 1:		//contacts.html
		_qtWengoPhone.getQtToolBar()->showContactsTab();
		break;

	case 2:		//history.html
		_qtWengoPhone.getQtToolBar()->showHistory();
		break;

	case 3:		//keypad.html
		_qtWengoPhone.getQtToolBar()->showKeyTab();
		break;

	case 4:		//settings.html
		_qtWengoPhone.getQtToolBar()->showConfig();
		break;

	case 5:		//wizard.html
		_qtWengoPhone.getQtToolBar()->showWizard();
		break;

	case 6:		//addressbook.html
		_qtWengoPhone.getQtToolBar()->showContactManager( "" );
		break;

	case 8:		//open settings, choose Phone tab
		_qtWengoPhone.getQtToolBar()->showPhoneConfig();
		break;
	}
}


void QtBrowserWidget::handleOpenLink(QString link ){//VOXOX - CJC - 2009.09.09 Javascript executed code.

	WebBrowser::openUrl(link.toStdString());
}

//-----------------------------------------------------------------------------

void QtBrowserWidget::beforeNavigateSlot(QString link)
{
	bool doLogin    = true;
	bool doPostWork = true;

	//This group opens QT Windows.
	if(link.contains(QString("contacts.html")))
	{
		_qtWengoPhone.getQtToolBar()->showContactsTab();
		doLogin = false;
	}
	else if(link.contains(QString("history.html")))
	{
		_qtWengoPhone.getQtToolBar()->showHistory();
		doLogin = false;
	}
	else if(link.contains(QString("keypad.html")))
	{
		_qtWengoPhone.getQtToolBar()->showKeyTab();
	}
	else if(link.contains(QString("settings.html")))	//VOXOX - JRT - 2009.07.01 - should now be handled by buttonClicked()	
	{
		_qtWengoPhone.getQtToolBar()->showConfig();
	}
	else if(link.contains(QString("wizard.html")))		//VOXOX - JRT - 2009.07.01 - should now be handled by buttonClicked()
	{
		_qtWengoPhone.getQtToolBar()->showWizard();
	}
	else if(link.contains(QString("addressbook.html")))	//VOXOX - JRT - 2009.07.01 - should now be handled by buttonClicked()
	{
		_qtWengoPhone.getQtToolBar()->showContactManager( "" );
	}

	//This group opens external URLs
	else if(link.contains(QString("support.voxox.com")))
	{
		WebBrowser::openUrl(link.toStdString());
	}
	else if(link.contains(QString("portal.voxox.com")))
	{
		WebBrowser::openUrl(link.toStdString());
	}
	else if(link.contains(QString("my.voxox.com"))){
		WebBrowser::openUrl(link.toStdString());
	}
	else if(link.contains(QString("forum.voxox.com")))
	{
		WebBrowser::openUrl(link.toStdString());
	}
	else if(link.contains(QString("services/voicemails/")))
	{
		WebBrowser::openUrl(link.toStdString());
	}
	else if(link.contains(QString("openLink$")))
	{
		link.remove(0,(link.indexOf('$')+1));
		WebBrowser::openUrl(link.toStdString());
	}
	else
	{
		doPostWork = false;
	}

	if ( doPostWork )
	{
		doAfterNavigationWork( doLogin );
	}
}

//-----------------------------------------------------------------------------

void QtBrowserWidget::doAfterNavigationWork( bool doLogin )
{
	_accountPageStarted = true;
	_qtBrowser->backward();

	if ( doLogin )
	{
		loginAccount();
	}
}


//=============================================================================


//=============================================================================

JSButtonHandler::JSButtonHandler( QtBrowserWidget* browser, QObject* parent ) : QObject(parent)
{
	_browser = browser;
}

//-----------------------------------------------------------------------------

void JSButtonHandler::buttonClicked( int buttonId, QString action )
{
	_browser->handleButtonClick( buttonId, action );
}

void JSButtonHandler::openLink( QString link )
{
	_browser->handleOpenLink( link);
}

//void JSButtonHandler::buttonClicked( int buttonId )
//{
//	_browser->handleButtonClick( buttonId );
//}

//=============================================================================
