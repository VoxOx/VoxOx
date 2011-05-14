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

#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtCallToaster.h"

#include "QtToaster.h"

#include "ui_QtCallToaster.h"
#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <util/SafeDelete.h>

#include <qtutil/SafeConnect.h>
#include <qtutil/PixmapMerging.h>
#include <QtGui/QtGui>

//VOXOX CHANGE by Alexander 02-20-09: added growl to call toaster.
#if defined(OS_MACOSX)
	#include "qtutil/GrowlNotifier.h"
#endif

QtCallToaster::QtCallToaster()
	: QObject(NULL) {

	_callToasterWidget = new QWidget(NULL);

	_ui = new Ui::CallToaster();
	_ui->setupUi(_callToasterWidget);
	

	_ui->pickUpButton->setImages(":/pics/toaster/pickup.png");
	_ui->hangUpButton->setImages(":/pics/toaster/hangup.png");
	_ui->closeButton->setImages(":/pics/toaster/close.png");

	_ui->lblIncomingCallLabel->setStyleSheet("color: #ffffff;  font: bold normal 12px \"Tacoma\";");
	_ui->messageLabel->setStyleSheet("color: #c1bc27;  font: normal 12px \"Tacoma\";");
	_ui->pixmapLabel->setStyleSheet("border:1px solid #898b97;");


	/*_ui->hangUpButton->setPixmaps(QPixmap(":/pics/toaster/hangup.png"),
			QPixmap(),
			QPixmap(),
			QPixmap(":/pics/toaster/hangup.png"),
			QPixmap(),
			QPixmap());

	_ui->pickUpButton->setPixmaps(QPixmap(":/pics/toaster/pickup.png"),
			QPixmap(),
			QPixmap(),
			QPixmap(":/pics/toaster/pickup.png"),
			QPixmap(),
			QPixmap());*/

	
	SAFE_CONNECT(_ui->pickUpButton, SIGNAL(clicked()), SLOT(pickUpButtonSlot()));

	SAFE_CONNECT(_ui->hangUpButton, SIGNAL(clicked()), SLOT(hangUpButtonSlot()));

	SAFE_CONNECT(_ui->closeButton, SIGNAL(clicked()), SLOT(close()));

	_toaster = new QtToaster(_callToasterWidget, _ui->windowFrame);
	_toaster->setTimeOnTop(10000);
}

QtCallToaster::~QtCallToaster() {
	OWSAFE_DELETE(_ui);
}

void QtCallToaster::setMessage(const QString & message) {
	_ui->messageLabel->setText(message);
	//VOXOX CHANGE by Alexander 02-23-09: sets the growl message growl will display when an user sends a call request.	
	_message = message;
}

void QtCallToaster::setPixmap(const QPixmap & pixmap) {

	QPixmap scaledPixmap = pixmap.scaled(_ui->pixmapLabel->width()-1,_ui->pixmapLabel->height()-1,Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation);
	_ui->pixmapLabel->setPixmap(scaledPixmap);
}

void QtCallToaster::show() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	if(config.getNotificationShowToasterOnIncomingCall()){
		//VOXOX CHANGE by Alexander 02-20-09: added growl to call toaster.
		#ifdef OS_MACOSX
			// Show Growl notification
			//VOXOX CHANGE by Alexander 06-02-09: implemented Singleton to the Growl notifier
			GrowlNotifier::Instance()->notify("Incoming Call", "Incoming Call", _message, *(_ui->pixmapLabel->pixmap()), false, this, SLOT(pickUpButtonSlot()));
		#else
			_toaster->show();
		#endif
	}else{
		close();
	}
}

void QtCallToaster::close() {
	_toaster->close();
}

void QtCallToaster::hangUpButtonSlot() {
	hangUpButtonClicked();
	close();
}

void QtCallToaster::pickUpButtonSlot() {
	pickUpButtonClicked();
	close();
}
