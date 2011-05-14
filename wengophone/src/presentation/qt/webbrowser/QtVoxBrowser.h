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
* Base class for the rest of the browsers
* @author Chris Jimenez C 
* @date 2009.10.29
*/


#ifndef OWQTVOXBROWSER_H
#define OWQTVOXBROWSER_H

#include <QtCore/QObject>

#include <util/NonCopyable.h>

#include <string>

#include <QtWebKit/QtWebKit>

class QWidget;

class JSHandler;

class QtVoxBrowser : public QWebView, NonCopyable {
	Q_OBJECT
public:

	QtVoxBrowser(QWidget * parent = 0);

	~QtVoxBrowser();
	
	void handleOpenLink(QString link );//VOXOX - CJC - 2009.09.09  Open Link interaction


private Q_SLOTS:
	void initJavascript();
	

private:
	JSHandler* _jsHandler;
};


//======================================================================


//=============================================================================

class JSHandler : public QObject
{
	Q_OBJECT
public:
	JSHandler( QtVoxBrowser* browser, QObject* parent );
	
	
public Q_SLOTS:
	void openLink( QString link );//VOXOX - CJC - 2009.09.09  Handle the openLinkCommand
//	void buttonClicked( int buttonId, int actionId );

private:
	QtVoxBrowser*	_browser;
};

//=============================================================================

#endif	//OWQtVoxBrowser_H
