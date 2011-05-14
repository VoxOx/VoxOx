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
#include "stdafx.h"	
#include "QtContactProfileWidget.h"

#include "ui_ContactProfileWidget.h"

#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <model/profile/UserProfile.h>	//Cheating

#include <control/CWengoPhone.h>
#include <control/profile/CUserProfileHandler.h>
#include <control/profile/CUserProfile.h>

#include "WengoPhoneBuildId.h"	//VOXOX - JRT - 2009.09.24 

#include <QtGui/QShortcut>

#include <util/Logger.h>
#include <util/SafeDelete.h>
#include <qtutil/SafeConnect.h>
#include <qtutil/CloseEventFilter.h>//VOXOX - CJC - 2009.07.09 
#include <string>

#include <QtWebKit/QWebFrame>	//VOXOX - JRT - 2009.06.26 

//VOXOX - JRT - 2009.08.03 - Temp code to address server-side issue with Content-Type
//#include <QtNetwork/QNetworkAccessManager>
//#include <QtNetwork/QNetworkRequest>
//#include <QtNetwork/QNetworkReply>
//
//#ifdef _WINDOWS
//#include <windows.h>		//JRT-XXX - For OutputDebugString()
//#endif
//JRT

//VOXOX - JRT - 2009.07.30 - TODO: As Chris pointed out, this should be moved from the Chat folder. ;-)


std::string QtContactProfileWidget::_newContactIdentifier = "-1";

//-----------------------------------------------------------------------------

//QtContactProfileWidget::QtContactProfileWidget( CWengoPhone* cWengoPhone, QWidget * parent,QtEnumUniversalMessage::Mode mode) 
//	: QDialog(parent, Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint),
//	_cWengoPhone(cWengoPhone) 
//VOXOX CHANGE by ASV 07-03-2009: Added window modality to prevent this window from being on top of the main window
QtContactProfileWidget::QtContactProfileWidget( CWengoPhone* cWengoPhone, const QString contactId, ActionId actionId, QWidget * parent ) :
QtVoxWindowInfo(parent,QtEnumWindowType::ContactManagerWindow, Qt::Window ),
	_cWengoPhone(cWengoPhone)
{
	_contactId        = contactId;
	_actionId	= actionId;

	_ui = new Ui::ContactProfileWidget();

	_ui->setupUi(this);

	setWindowTitle( tr( "Contact Manager" ) );

	_ui->webView->page()->mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
	_ui->webView->page()->mainFrame()->setScrollBarPolicy(Qt::Vertical,   Qt::ScrollBarAlwaysOff);
	
	resize( 800, 500 );
	
	//populateCountryList();
	CloseEventFilter * closeEventFilter = new CloseEventFilter(this, SLOT(closeEvent()));
	this->installEventFilter(closeEventFilter);

	//VOXOX -ASV- 08-25-2009: we close the window when CMD + Shift + W is pressed on Mac
	#if defined(OS_MACOSX)
		_closeWindowShortCut = new QShortcut(QKeySequence("Ctrl+Shift+W"), this);
		SAFE_CONNECT(_closeWindowShortCut, SIGNAL(activated()), SLOT(closeEvent()));
	#endif
	
	//VOXOX - JRT - 2009.06.26
	connect     ( _ui->webView->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(initJavascript() ));
	SAFE_CONNECT(_ui->webView,                       SIGNAL(loadFinished(bool)),                    SLOT(documentCompleteSlot(bool)));
	loadHtml();
}

//-----------------------------------------------------------------------------

QtContactProfileWidget::~QtContactProfileWidget() 
{
	#if defined(OS_MACOSX)
		OWSAFE_DELETE(_closeWindowShortCut);//VOXOX -ASV- 08-25-2009: we close the window when CMD + Shift + W is pressed on Mac
	#endif
	
	OWSAFE_DELETE(_ui);
}

//-----------------------------------------------------------------------------

void QtContactProfileWidget::setAction( ActionId actionId, const QString& contactId )
{
	switch( actionId )
	{
	case Action_None:
		break;

	case Action_Add:
		addNewContact();
		break;

	case Action_Edit:
		updateContactId( contactId );
		break;

	case Action_Delete:
		deleteContactId( contactId );
		break;

//	case Action_SetId:
//		setJsonId( contactId );
//		assert(false);		//VOXOX - JRT - 2009.09.19 - Don't use this path.
//		break;

	default:
		assert(false);		//New ActionId?
	}
}

//-----------------------------------------------------------------------------

void QtContactProfileWidget::addNewContact()
{
	_actionId  = Action_Add;
	invokeAddContact();
}

//-----------------------------------------------------------------------------

void QtContactProfileWidget::updateContactId( const QString& contactId )	//VOXOX - JRT - 2009.09.18 
{
	_actionId  = Action_Edit;
	_contactId = contactId;

	invokeEditContact();
}

//-----------------------------------------------------------------------------

void QtContactProfileWidget::deleteContactId( const QString& contactId )	//VOXOX - JRT - 2009.09.18 
{
	_actionId  = Action_Delete;
	_contactId        = contactId;

	invokeDeleteContact();
}

//-----------------------------------------------------------------------------

QString	QtContactProfileWidget::getContactId()
{ 
	QString result = _contactId;

	if ( isAddingNewContact() )
	{
		result = _newContactIdentifier.c_str();
	}

	return _contactId;		
}

//-----------------------------------------------------------------------------

void QtContactProfileWidget::setJsonId( int jsId, int qtId )
{
	invokeNewContactId( jsId, qtId );
}

//-----------------------------------------------------------------------------

void QtContactProfileWidget::loadHtml()
{
	//VOXOX - JRT - 2009.07.30 - This code verifies that no page/view caching is done by default.  Remove after CM issues are resolved.
	//QWebSettings::globalSettings()->setObjectCacheCapacities(0, 0, 0);
	//int nPages1 = _ui->webView->page()->settings()->maximumPagesInCache();
	//_ui->webView->page()->settings()->setMaximumPagesInCache( 0 );
	//int nPages2 = _ui->webView->page()->settings()->maximumPagesInCache();

	//int nPages3 = _ui->webView->settings()->maximumPagesInCache();
	//_ui->webView->settings()->setMaximumPagesInCache( 0 );
	//int nPages4 = _ui->webView->settings()->maximumPagesInCache();

	//VOXOX - JRT - 2009.09.17 - This old R&D code was preventing Flash from working.
//	JSONTestFactory* pFactory = new JSONTestFactory( this );	//TODO: who deletes this?
//	pFactory->setWengoPhone( _cWengoPhone );
//	_ui->webView->page()->setPluginFactory( pFactory );


	Config & config = ConfigManager::getInstance().getCurrentConfig();

	QString version = QString("?v=") + QString(WengoPhoneBuildId::getFullVersion());
	QString url  = config.getContactManagerUrl().c_str();

	if ( url.contains( "http:" ) || url.contains( "https:") )
	{
		url += version;
		_ui->webView->setUrl( url );

		//VOXOX - JRT - 2009.08.03 - Temp code to address server-side issue with Content-Type
		//QNetworkAccessManager* netMgr = new QNetworkAccessManager( this );
		//connect( netMgr, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));


		//QNetworkRequest request;
		//request.setRawHeader(Header( 
  //      ContentTypeHeader,
  //      ContentLengthHeader,
  //      LocationHeader,
  //      LastModifiedHeader,
  //      CookieHeader,
  //      SetCookieHeader
		//request.setUrl( url );

		//netMgr->get(request);
	}
	else
	{
		QString resourcePath = QString::fromStdString(config.getResourcesDir());
		resourcePath.replace( "\\", "/" );
		QString path = resourcePath + url + version; 
	
		_ui->webView->setUrl( QUrl::fromLocalFile( path ) );
	}
}

//-----------------------------------------------------------------------------

void QtContactProfileWidget::invokeLogin()
{
	//So user can connect to server.
	UserProfile& userProfile = _cWengoPhone->getCUserProfileHandler().getCUserProfile()->getUserProfile();
	QString		 accountId   = QString::fromStdString( userProfile.getVoxOxAccount()->getDisplayAccountId());
	QString		 md5		 = QString::fromStdString( userProfile.getMd5());

	QString		 javascript1  = QString("login('%1','%2')").arg(accountId).arg(md5);
	_ui->webView->page()->mainFrame()->evaluateJavaScript(javascript1);
}

//-----------------------------------------------------------------------------

void QtContactProfileWidget::invokeAddContact()
{
	if ( isAddingNewContact() )
	{
		QString javascript1  = QString("qtAddContact()");
		_ui->webView->page()->mainFrame()->evaluateJavaScript(javascript1);

		clearAction();
	}
}

//-----------------------------------------------------------------------------

void QtContactProfileWidget::invokeEditContact()
{
	if ( isEdittingContact() && _contactId != "" )
	{
		QString javascript1  = QString("qtEditContact('%1')").arg(_contactId);
		_ui->webView->page()->mainFrame()->evaluateJavaScript(javascript1);

		clearAction();
	}
}

//-----------------------------------------------------------------------------

void QtContactProfileWidget::invokeDeleteContact()
{
	if ( isDeletingContact() && _contactId != "" )
	{
		QString javascript1  = QString("qtDeleteContact('%1')").arg(_contactId);
		_ui->webView->page()->mainFrame()->evaluateJavaScript(javascript1);

		clearAction();
	}
}

//-----------------------------------------------------------------------------

void QtContactProfileWidget::invokeNewContactId( int jsId, int qtId )
{
	QString javascript1  = QString("qtContactID('%1', '%2')").arg(jsId).arg(qtId);
	_ui->webView->page()->mainFrame()->evaluateJavaScript(javascript1);
}

//-----------------------------------------------------------------------------

void QtContactProfileWidget::initJavascript()
{
	JSONContact* pJson = new JSONContact( this, _cWengoPhone );
	_ui->webView->page()->mainFrame()->addToJavaScriptWindowObject( "qtEventSource", pJson );

//	QString		 javaScript  = QString("setInitialContact('%1')").arg(_contactId);
//	_ui->webView->page()->mainFrame()->evaluateJavaScript( javaScript );

//	_ui->webView->page()->mainFrame()->evaluateJavaScript( "setInitialContact( qtEventSource.signalGetData.connect( mouseOver );" );
//	_ui->webView->page()->mainFrame()->evaluateJavaScript( "qtEventSource.signalSetData.connect( mouseOut );" );
}

//-----------------------------------------------------------------------------

void QtContactProfileWidget::clearAction()
{
	_actionId = Action_None;
	_contactId = "";
}

//-----------------------------------------------------------------------------

void QtContactProfileWidget::documentCompleteSlot(bool ok) 
{
	invokeLogin();

	//Now trigger any pending events.
	invokeAddContact();
	invokeEditContact();
	invokeDeleteContact();
}

//-----------------------------------------------------------------------------

//VOXOX - JRT - 2009.08.03 - Temp code to address server-side issue with Content-Type
//void QtContactProfileWidget::replyFinished( QNetworkReply* reply )
//{
//	QVariant hdr1 = reply->header( QNetworkRequest::ContentTypeHeader );
//	QString  val = hdr1.toString();
//
//	char buffer[1000];
//	sprintf( buffer, "CM content type: %s\n", val.toStdString().c_str() );
//	OutputDebugString( buffer );
//
//	QString myHdr1 = "text/html";
//	QByteArray data = reply->readAll();
//	_ui->webView->setContent( data, myHdr1 );
//}


//-----------------------------------------------------------------------------
//Window Manager methods
//-----------------------------------------------------------------------------

void QtContactProfileWidget::closeEvent() {//VOXOX - CJC - 2009.07.09 
	windowClose(getKey());
}

void QtContactProfileWidget::showWindow(){//VOXOX - CJC - 2009.07.09 
	if(isVisible()){
		activateWindow();
		raise();
	}else{
		showNormal();
	}
}

QWidget * QtContactProfileWidget::getWidget() const {
	return (QWidget*)this;
}

QString QtContactProfileWidget::getKey() const {
	return QtEnumWindowType::toString(getType());
}

QString QtContactProfileWidget::getDescription() const {
	return QString("VoxOx Contact Manager");
}

int QtContactProfileWidget::getAllowedInstances() const{
	return 1;
}

int QtContactProfileWidget::getCurrentTab() const{
	return 0;
}

void QtContactProfileWidget::setCurrentTab(QString tabName){
	
}

//-----------------------------------------------------------------------------




//=============================================================================

JSONContact::JSONContact( QObject* parent, CWengoPhone* cWengoPhone) : QObject(parent)
{
	_cWengoPhone = cWengoPhone;
}

//-----------------------------------------------------------------------------

UserProfile& JSONContact::getUserProfile()
{
	return _cWengoPhone->getCUserProfileHandler().getCUserProfile()->getUserProfile();
}

//-----------------------------------------------------------------------------

QString JSONContact::getInitialContact()
{
	return dynamic_cast<QtContactProfileWidget*>(parent())->getContactId();
}

//-----------------------------------------------------------------------------

QString JSONContact::getContactList()
{
	QString result = "";
	result = getUserProfile().contactListToJSON().c_str();

	LOG_INFO( " Sending " + String::fromNumber( result.size() ) + " to CM.");
	return result;
}

//-----------------------------------------------------------------------------

QString JSONContact::getContactGroups()
{
	QString result = "";
	result = getUserProfile().contactGroupSetToJSON().c_str();

	LOG_INFO( " Sending " + String::fromNumber( result.size() ) + " to CM.");
	return result;
}

//-----------------------------------------------------------------------------
//VOXOX - JRT - 2009.09.24 - This is poorly named, but CM is using it at the moment.
QString JSONContact::getNetworks()
{
	return getAccounts();
}

//-----------------------------------------------------------------------------

QString JSONContact::getAccounts()
{
	QString result = "";
	result = getUserProfile().accountListToJSON().c_str();

	LOG_INFO( " Sending " + String::fromNumber( result.size() ) + " to CM.");
	return result;
}

//-----------------------------------------------------------------------------
//VOXOX - JRT - 2009.09.23 
QString JSONContact::getDataById( const int qtId )
{
	QString result = "";
	result = getUserProfile().contactProfileToJSON( qtId ).c_str();

	LOG_INFO( " Sending " + String::fromNumber( result.size() ) + " to CM.");

	return result;
}

//-----------------------------------------------------------------------------

QString JSONContact::getData( const QString& contactId )
{
	QString result = "";
	result = getUserProfile().contactProfileToJSON( contactId.toStdString() ).c_str();

	LOG_INFO( " Sending " + String::fromNumber( result.size() ) + " to CM.");

	return result;
}

//-----------------------------------------------------------------------------

void JSONContact::setData( const QString& data )
{
	LOG_INFO( " Received " + String::fromNumber( data.size() ) + " from CM.");

	int jsId = 0;
	int qtId = 0;

	_cWengoPhone->getCUserProfileHandler().getCUserProfile()->updateContactProfileFromJSON( data.toStdString(), jsId, qtId );

	if ( jsId < 0 )
	{
		dynamic_cast<QtContactProfileWidget*>(parent())->setJsonId( jsId, qtId );
	}
}

//=============================================================================
//VOXOX - JRT - 2009.06.30  - This allows object to be created in JS, but none of the events
//							  are called successfully.  Need to work that out.
//JSONTestFactory::JSONTestFactory( QObject* parent )
//{
//	setParent( parent );
//	_cWengoPhone = NULL;
//
//	_mimeType.description	 = "Test MimeType";
//	_mimeType.fileExtensions = QStringList( "" );
//	_mimeType.name			 = "application/x-vox-contactdata";
//	
//	QWebPluginFactory::Plugin p;
//	p.description = "test Plugin";
//	p.mimeTypes.append( _mimeType );
//
//	_plugins.append( p );
//}
//
////-----------------------------------------------------------------------------
//
//JSONTestFactory::~JSONTestFactory()
//{
//}
//
////-----------------------------------------------------------------------------
//
//QObject* JSONTestFactory::create( const QString& mimeType, const QUrl& url, 
//							    const QStringList & argumentNames, const QStringList & argumentValues ) const 
//{
//	//Lets just ignore the parms for now.
//	JSONContact* pNew = new JSONContact( parent(), _cWengoPhone );
//
//	return pNew;
//}
//
////-----------------------------------------------------------------------------
//
//QList<QWebPluginFactory::Plugin> JSONTestFactory::plugins () const
//{
//	return _plugins;
//}

//================================================================
