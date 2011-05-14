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

#ifndef OWQTWEBBROWSER_H
#define OWQTWEBBROWSER_H
#include <QtGui/QtGui>
#include <QtCore/QObject>

#include <util/NonCopyable.h>

#include <string>

class QtVoxBrowser;
class QWidget;

namespace Ui { class WebBrowser; }

/**
 * Embedded HTML browser widget inside the main window.
 *
 *
 * @author Tanguy Krotoff
 * @author Mathieu Stute
 */


class QtWebBrowser : public QDialog, NonCopyable {
	Q_OBJECT
public:

	QtWebBrowser(QWidget * parent = 0);

	~QtWebBrowser();

	QWidget * getWidget() const;

	void setUrl(QString url);
	void loadUrl();

	bool isAlreadyLoaded(){
		return _pageLoaded;
	}

	void closeEvent(QCloseEvent *event);
private Q_SLOTS:

	void beforeNavigateSlot(QString);

	void documentCompleteSlot(bool);
private:

	Ui::WebBrowser * _ui;

	QtVoxBrowser * _qtVoxBrowser;

	bool _pageLoaded;

	std::string _url;

};

#endif	//OWQTWEBBROWSER_H
