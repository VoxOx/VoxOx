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

#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtWebBrowser.h"
#include "ui_WebBrowser.h"

#include "QtVoxBrowser.h"

#include <qtutil/SafeConnect.h>
#include <util/Logger.h>

#include <util/SafeDelete.h>





//VOXOX CHANGE by Rolando 2009.05.05

/*
This is a generic window that shows a web browser with a url defined using setUrl and loadUrl methods
*/
QtWebBrowser::QtWebBrowser(QWidget * parent): QDialog(parent){

	setWindowTitle(tr("@product@"));

	setContentsMargins(0,0,0,0);
	resize(QSize(279,380));

	_qtVoxBrowser = new QtVoxBrowser(parent);

	_ui = new Ui::WebBrowser();
	_ui->setupUi(this);
	
    QVBoxLayout * layout = new QVBoxLayout(this);
	layout->setContentsMargins(0,0,0,0);
	layout->setSpacing(0);
    layout->addWidget(getWidget());

	setWindowFlags(Qt::WindowMinMaxButtonsHint);
	
	/*SAFE_CONNECT(_qtVoxBrowser, SIGNAL(beforeNavigateSignal(QString)), SLOT(beforeNavigateSlot(QString)));

	SAFE_CONNECT(_qtVoxBrowser, SIGNAL(documentCompleteSignal(bool)), SLOT(documentCompleteSlot(bool)));*/

	_pageLoaded = false;
	_url = "";

}

QtWebBrowser::~QtWebBrowser() {
	OWSAFE_DELETE(_qtVoxBrowser);
	OWSAFE_DELETE(_ui);
}

//VOXOX CHANGE by Rolando 2009.05.05
void QtWebBrowser::closeEvent(QCloseEvent *event) {
	deleteLater();
}

QWidget * QtWebBrowser::getWidget() const {
	if (!_qtVoxBrowser) {
		return NULL;
	}

	return /*(QWidget*)*/ _qtVoxBrowser;
}

//VOXOX CHANGE by Rolando 2009.05.05
void QtWebBrowser::documentCompleteSlot(bool ok) {
	if(ok){
		_pageLoaded = true;
	}
}

//VOXOX CHANGE by Rolando 2009.05.05
void QtWebBrowser::beforeNavigateSlot(QString link){
	
}

//VOXOX CHANGE by Rolando 2009.05.05
void QtWebBrowser::setUrl(QString url){
	_url = url.toStdString();
}

//VOXOX CHANGE by Rolando 2009.05.05
void QtWebBrowser::loadUrl(){
	if(_url != ""){
		_qtVoxBrowser->setUrl(QString::fromStdString(_url));
	}
	else{
		LOG_DEBUG("url not defined");
	}
}