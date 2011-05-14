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

#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtFactory.h"

#include "QtWengoPhone.h"
#include "QtWengoStyle.h"
#include "chat/QtChatHandler.h"
#include "contactlist/QtContactList.h"
#include "history/QtHistory.h"
#include "phoneline/QtPhoneLine.h"
#include "phonecall/QtPhoneCall.h"
#include "profile/QtUserProfile.h"
#include "profile/QtUserProfileHandler.h"
#include "webservices/sms/QtSms.h"
#include "webservices/softupdate/QtSoftUpdate.h"
#include "wenbox/QtWenboxPlugin.h"
#include "dialpad/QtDialpad.h"//VOXOX CHANGE by Rolando 04-29-09, now QtDialpad is in dialpad folder

#include <util/File.h>
#include <util/Logger.h>
#include <util/Path.h>
#include <util/SafeDelete.h>
#include <util/WebBrowser.h>

#include <cutil/global.h>

#include <qtutil/ThreadEventFilter.h>
#include <qtutil/QtThreadEvent.h>

#include <QtCore/QUrl>
#include <QtGui/QDesktopServices>

#ifndef OS_WINDOWS
	#include <QtGui/QApplication>
#else
	#include "WinApp.h"
#endif

#if defined(OS_LINUX)
#include <stdlib.h>
#endif

#if defined(OS_MACOSX)
	#include "macosx/QtMacApplication.h"
#endif

PFactory * PFactory::_factory = NULL;

QtFactory::QtFactory(int & argc, char * argv[]) {
#if defined(OS_LINUX)
	// FIXME: Disable Qt Glib event loop, because it conflicts with Gaim 
	setenv("QT_NO_GLIB", "1", 1);
#endif

#if defined(OS_MACOSX)
	_app = new QtMacApplication(argc, argv);

	std::string qtPlugins = Path::getApplicationPrivateFrameworksDirPath() +
		File::convertPathSeparators("qt-plugins/");
	QCoreApplication::addLibraryPath(QString::fromStdString(qtPlugins));
#else

#ifndef OS_WINDOWS
	_app = new QApplication(argc, argv);
#else
	_app = new WinApp(argc, argv);
#endif

	QCoreApplication::addLibraryPath(".");
#endif

	Q_INIT_RESOURCE(qtutil);

	//QtWengoStyle
	QtWengoStyle * qtWengoStyle = new QtWengoStyle();
	_app->setStyle(qtWengoStyle);

	//Filter for post event (=thread event)
	ThreadEventFilter * threadEventFilter = new ThreadEventFilter();
	_app->installEventFilter(threadEventFilter);

	reset();
	_qtWengoPhone		  = NULL;
	_qtUserProfileHandler = NULL;
	_cWengoPhone		  = NULL;	//VOXOX - JRT - 2009.04.13 - proper initialization.

	// Integrate QDesktopServices with WebBrowser, this way we can use the
	// QLabel::openExternalLinks property.
	QDesktopServices::setUrlHandler("http", this, "openUrl" );
	QDesktopServices::setUrlHandler("https", this, "openUrl" );
}

//-----------------------------------------------------------------------------
//VOXOX - JRT - 2009.04.13 
void QtFactory::cleanup()
{
//JRT-XXX	OWSAFE_DELETE(_app);				//OK

//	OWSAFE_DELETE(_qtContactList);
//	OWSAFE_DELETE(_qtChatHandler);
//	OWSAFE_DELETE(_qtWenboxPlugin);
//	OWSAFE_DELETE(_qtSoftUpdate);
//	OWSAFE_DELETE(_qtHistory);
//	OWSAFE_DELETE(_qtWengoPhone);	//Deleted in CWengoPhone
//	OWSAFE_DELETE(_qtSms);			//Part of qtWengoPhone
//	OWSAFE_DELETE(_qtDialpad);		//Part of qtWengoPhone

//	OWSAFE_DELETE(_qtUserProfileHandler);	//VOXOX - JRT - 2009.04.13 - deleted elsewhere.
//	OWSAFE_DELETE(_cWengoPhone);			//VOXOX - JRT - 2009.04.13 - deleted elsewhere.
}
//End VoxOx

void QtFactory::processEvents() {
	_app->processEvents();
}

int QtFactory::exec() {
	return _app->exec();
}

void QtFactory::reset() {
	//Objects are deleted by the class who constructs them.
	//So we can set the pointer to NULL safely.
	_qtContactList = NULL;
	_qtChatHandler = NULL;
	_qtWenboxPlugin = NULL;
	_qtSms = NULL;
	_qtSoftUpdate = NULL;
	_qtHistory = NULL;
	_qtDialpad = NULL;
}

PWengoPhone * QtFactory::createPresentationWengoPhone(CWengoPhone & cWengoPhone, bool runInBackground) {
	_cWengoPhone = &cWengoPhone;
#ifdef OS_WINDOWS
	_app->setCWengoPhone(_cWengoPhone);
#endif
	if (!_qtWengoPhone) {
		_qtWengoPhone = new QtWengoPhone(cWengoPhone);
	}

	if (!runInBackground) {
		//_qtWengoPhone->getWidget()->show();//VOXOX CHANGE by Rolando 2009.05.05, we now show main window until is authenticated the user profile
	}

	return _qtWengoPhone;
}

PUserProfile * QtFactory::createPresentationUserProfile(CUserProfile & cUserProfile) {
	//FIXME: QtWengoPhone must be instanciated before any QtUserProfile
	return new QtUserProfile(cUserProfile, *_qtWengoPhone);
}

PUserProfileHandler * QtFactory::createPresentationUserProfileHandler(CUserProfileHandler & cUserProfileHandler) {
	if (!_qtUserProfileHandler) {
		//FIXME: QtWengoPhone must be instanciated before _qtUserProfileHandler
		_qtUserProfileHandler = new QtUserProfileHandler(cUserProfileHandler, *_qtWengoPhone);
	}

	return _qtUserProfileHandler;
}

PPhoneLine * QtFactory::createPresentationPhoneLine(CPhoneLine & cPhoneLine) {
	//FIXME: memory leak?
	QtPhoneLine * qtPhoneLine = new QtPhoneLine(cPhoneLine);
	return qtPhoneLine;
}

PPhoneCall * QtFactory::createPresentationPhoneCall(CPhoneCall & cPhoneCall) {
	//FIXME: memory leak?
	QtPhoneCall * qtPhoneCall = new QtPhoneCall(cPhoneCall);
	return qtPhoneCall;
}

PContactList * QtFactory::createPresentationContactList(CContactList & cContactList) {
	if (!_qtContactList) {
		if (!_cWengoPhone) {
			LOG_FATAL("PWengoPhone must be created before PContactList");
		}
		_qtContactList = new QtContactList(cContactList, *_cWengoPhone);
	}
	return _qtContactList;
}

PWenboxPlugin * QtFactory::createPresentationWenboxPlugin(CWenboxPlugin & cWenboxPlugin) {
	if (!_qtWenboxPlugin) {
		_qtWenboxPlugin = new QtWenboxPlugin(cWenboxPlugin);
	}
	return _qtWenboxPlugin;
}

PChatHandler * QtFactory::createPresentationChatHandler(CChatHandler & cChatHandler) {
	if (!_qtChatHandler) {
		if (!_qtWengoPhone) {
			LOG_FATAL("PWengoPhone must be created before PContactList");
		} else {
			_qtChatHandler = new QtChatHandler(cChatHandler, *_qtWengoPhone);
		}
	}
	return _qtChatHandler;
}

PSms * QtFactory::createPresentationSms(CSms & cSms) {
	if (!_qtSms) {
		_qtSms = new QtSms(cSms);
	}
	return _qtSms;
}

PSoftUpdate * QtFactory::createPresentationSoftUpdate(CSoftUpdate & cSoftUpdate) {
	if (!_qtSoftUpdate) {
		_qtSoftUpdate = new QtSoftUpdate(cSoftUpdate);
	}
	return _qtSoftUpdate;
}

PHistory * QtFactory::createPresentationHistory(CHistory & cHistory) {
	if (!_qtHistory) {
		_qtHistory = new QtHistory(cHistory);
	}
	return _qtHistory;
}

PConferenceCall * QtFactory::createPresentationConferenceCall(CConferenceCall & cConferenceCall) {
	return NULL;
}

void QtFactory::postEventImpl(IThreadEvent * event) {
	QCoreApplication::postEvent(this, new QtThreadEvent(event));
}

//DtmfThemeManager
void QtFactory::createPresentationDtmfThemeManager(CDtmfThemeManager & _cDtmfThemeManager) {
	if (!_qtDialpad) {
		_qtDialpad = new QtDialpad(_cDtmfThemeManager, _qtWengoPhone);
		_qtWengoPhone->setQtDialpad(_qtDialpad);
	}
}

void QtFactory::openUrl(const QUrl& url) {
	std::string urlString = url.toString().toStdString();
	WebBrowser::openUrl(urlString);
}
