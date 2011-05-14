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

#include <owbrowser/QtBrowser.h>

#include <util/Logger.h>
#include <util/SafeDelete.h>

#include <qtutil/SafeConnect.h>

#include <QtGui/QtGui>


#include <QtWebKit/QtWebKit>
#include <iostream>
using namespace std;

QtBrowser::QtBrowser(QWidget * parent)
	: QObject(parent) {

	_browserWidget = new QWidget(parent);

	_webKitBrowser = NULL;

	
	_layout = new QVBoxLayout(_browserWidget);
//TODO CJC , THIS SHOULD NOT BE HERE< QT BROWSERS OPEN IN ANY WINDOWS NO JUST TABS, IT SHOULD BE ON THE TAB
//VOXOX CHANGE ASV 04-06-09: we need a margin on top for Mac so the tab bar doesn't get on top of the widget content.
#if defined(OS_MACOSX)
	_layout->setContentsMargins(0, 11, 0, 0);
#else
	_layout->setMargin(0);
#endif

	_layout->setSpacing(0);

	initBrowser();
}

QtBrowser::~QtBrowser() {
	//Clean browser
	if (_webKitBrowser) {
		_layout->removeWidget(_webKitBrowser);
		OWSAFE_DELETE(_webKitBrowser);
	}
	
}

void QtBrowser::setUrl(const std::string & url, const std::string & data) {
		_url = QString::fromStdString(url);
		LOG_WARN(url);

		std::string querystring=url;

		if (!data.empty()) {
			querystring=url+"?"+data;
		}
		_webKitBrowser->setUrl(QUrl(QString::fromStdString(querystring)));
}

void QtBrowser::show() {
	_browserWidget->show();
}

void * QtBrowser::getWidget() const {
	return (void *) _browserWidget;
}

QWebView * QtBrowser::getQtWebView() const{
	return _webKitBrowser;
}

std::string QtBrowser::getUrl() const {
	return _url.toStdString();
}

void QtBrowser::backward() {

	_webKitBrowser->back();

}

void QtBrowser::forward() {
	LOG_WARN("not yet implemented");
}

void QtBrowser::beforeNavigate(const QUrl & link) {
	beforeNavigateSignal(link.toString());
}


void QtBrowser::documentComplete(bool ok) {
	
	documentCompleteSignal(ok);

}	



void QtBrowser::initBrowser() {

		//Init Qt browser

		_webKitBrowser = new QWebView(_browserWidget);
		
		_webKitBrowser->page()->settings()->setAttribute(QWebSettings::PluginsEnabled, true);

		SAFE_CONNECT(_webKitBrowser, SIGNAL(urlChanged(const QUrl &)), SLOT(beforeNavigate(const QUrl &)));

		SAFE_CONNECT(_webKitBrowser, SIGNAL(loadFinished(bool)), SLOT(documentComplete(bool)));

		_layout->addWidget(_webKitBrowser);

}

