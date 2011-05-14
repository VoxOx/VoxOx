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

#ifndef OWQTBROWSERWIDGET_H
#define OWQTBROWSERWIDGET_H

#include <QtCore/QObject>

#include <util/NonCopyable.h>

#include <string>

class QtBrowser;
class QWebFrame;
class QtWengoPhone;
class QWidget;

class JSButtonHandler;

/**
 * Embedded HTML browser widget inside the main window.
 *
 * Works only under Windows for the moment using a ActiveX control.
 *
 * @author Tanguy Krotoff
 * @author Mathieu Stute
 */
class QtBrowserWidget : public QObject, NonCopyable {
	Q_OBJECT
public:

	QtBrowserWidget(QtWengoPhone & qtWengoPhone);

	~QtBrowserWidget();

	QWidget * getWidget() const;

	/**
	 * Loads the default URL, loaded at startup.
	 */
	void loadDefaultURL();

	void loadOfflineURL();//VOXOX CHANGE by Rolando - 2009.07.24 

	/**
	 * Loads the URL when the user is connected.
	 */
	void loadAccountURL();
	void loginAccount();
	void openVoiceMail();//VOXOX - CJC - 2009.06.29 

	void handleButtonClick( int buttonId, QString action );		//For JS interaction.
	void handleOpenLink(QString link );//VOXOX - CJC - 2009.09.09  Open Link interaction
//	void handleButtonClick( int buttonId );		//For JS interaction.

private Q_SLOTS:
	void slotUserProfileDeleted();
	void beforeNavigateSlot(QString);
	void documentCompleteSlot(bool);
	void initJavascript();

protected:
	void doAfterNavigationWork( bool doLogin = true );
	QWebFrame* getWebFrame();

private:
	QtWengoPhone&	_qtWengoPhone;
	QtBrowser*		_qtBrowser;

	bool			_pageLoaded;
	bool			_accountPageStarted;

	JSButtonHandler* _jsButtonHandler;
};


//======================================================================


//=============================================================================

class JSButtonHandler : public QObject
{
	Q_OBJECT
public:
	JSButtonHandler( QtBrowserWidget* browser, QObject* parent );
	
	
public Q_SLOTS:
	void buttonClicked( int buttonId, QString action );
	void openLink( QString link );//VOXOX - CJC - 2009.09.09  Handle the openLinkCommand
//	void buttonClicked( int buttonId, int actionId );

private:
	QtBrowserWidget*	_browser;
};

//=============================================================================

#endif	//OWQTBROWSERWIDGET_H
