
#include "stdafx.h"
#include "QtVoxBrowser.h"



#include <qtutil/SafeConnect.h>
#include <util/Logger.h>
#include <util/SafeDelete.h>
#include <util/WebBrowser.h>
#include <QtGui/QtGui>



QtVoxBrowser::QtVoxBrowser(QWidget * parent): QWebView(parent){

	
	SAFE_CONNECT (this->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), SLOT(initJavascript() ));

	_jsHandler	= NULL;

}

QtVoxBrowser::~QtVoxBrowser() {
	OWSAFE_DELETE( _jsHandler );
}


void QtVoxBrowser::initJavascript()
{
	JSHandler* _jsHandler = new JSHandler( this, NULL );
	this->page()->mainFrame()->addToJavaScriptWindowObject( "qtEventSource", _jsHandler );
}



void QtVoxBrowser::handleOpenLink(QString link ){//VOXOX - CJC - 2009.09.09 Javascript executed code.

	WebBrowser::openUrl(link.toStdString());
}



JSHandler::JSHandler( QtVoxBrowser* browser, QObject* parent ) : QObject(parent)
{
	_browser = browser;
}



void JSHandler::openLink( QString link )
{
	_browser->handleOpenLink( link);
}
