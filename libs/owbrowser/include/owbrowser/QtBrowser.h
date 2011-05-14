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

#ifndef OWQTBROWSER_H
#define OWQTBROWSER_H

#include <owbrowser/OWBrowser.h>

#include <util/Event.h>
#include <cutil/global.h>

#include <QtCore/QObject>

class QWebView;
class QWidget;
class QTextBrowser;
class QVBoxLayout;
class QUrl;

/**
 * Qt implementation of OWBrowser.
 *
 * Implemented with:
 *  - QTextBrowser: local url
 *  - Microsoft Web Browser: local & remote url
 *
 * @author Mathieu Stute
 */
class QtBrowser : public QObject, public OWBrowser {
	Q_OBJECT
public:

	/**
	 * Default constructor.
	 *
	 * @param parent the parent widget
	 * @param mode browser mode to use (IE or Qt)
	 */
	QtBrowser(QWidget * parent);

	~QtBrowser();

	void setUrl(const std::string & url, const std::string & data = "");

	void show();

	void * getWidget() const;

	QWebView * getQtWebView() const;

	std::string getUrl() const;

	void backward();

	void forward();

private Q_SLOTS:

	/**
	 * Slot called before loading a page in a Microsoft Web Browser.
	 */
	

	/**
	 * Slot called before loading a page in a QTextBrowser.
	 */
	void beforeNavigate(const QUrl & link);


	void documentComplete(bool ok);
Q_SIGNALS:
	void beforeNavigateSignal(QString);
	void documentCompleteSignal(bool);
private:

	/**
	 * Current url.
	 */
	QString _url;

	/**
	 * Inits the browser (IE or Qt).
	 */
	void initBrowser();


	

	/**
	 * Main widget.
	 */
	QWidget * _browserWidget;


	/**
	 * Qt html viewer.
	 */
	QWebView * _webKitBrowser;

	/**
	 * Widget layout.
	 */
	QVBoxLayout * _layout;
};

#endif	//OWQTBROWSER_H
