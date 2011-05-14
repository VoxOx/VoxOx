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
#include "QtChatToaster.h"

#include "ui_QtChatToaster.h"

#include "QtToaster.h"
#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <util/SafeDelete.h>
#include <imwrapper/QtEnumIMProtocol.h>
#include <qtutil/SafeConnect.h>
#include <qtutil/MouseEventFilter.h>
#include <QtGui/QtGui>

//VOXOX CHANGE by Alexander 02-20-09: added growl to chat toaster.
#if defined(OS_MACOSX)
	#include "qtutil/GrowlNotifier.h"
#endif

QtChatToaster::QtChatToaster()
	: QObject(NULL) {

	_chatToasterWidget = new QWidget(NULL);

	_ui = new Ui::ChatToaster();
	_ui->setupUi(_chatToasterWidget);

	_ui->closeButton->setImages(":/pics/toaster/close.png");
	_ui->lblIncomingCallLabel->setStyleSheet("color: #ffffff;  font: bold normal 12px \"Tacoma\";");
	_ui->messageLabel->setStyleSheet("color: #c1bc27;  font: normal 12px \"Tacoma\";");
	_ui->pixmapLabel->setStyleSheet("border:1px solid #898b97;");
	//VOXOX - CJC - 2009.05.23 This will enalbe clicking on all the toaster (Easy for the user)
	MousePressEventFilter * windowsPressEventFilter = new MousePressEventFilter(
		this, SLOT(chatButtonSlot()),Qt::LeftButton);
	_chatToasterWidget->installEventFilter(windowsPressEventFilter);

//	SAFE_CONNECT(_ui->chatButton, SIGNAL(clicked()), SLOT(chatButtonSlot()));

	SAFE_CONNECT(_ui->closeButton, SIGNAL(clicked()), SLOT(close()));

	_toaster = new QtToaster(_chatToasterWidget, _ui->windowFrame);
	_toaster->setTimeOnTop(5000);
}

QtChatToaster::~QtChatToaster() {
	OWSAFE_DELETE(_ui);
}

void QtChatToaster::setMessage(const QString & message) {
	_ui->messageLabel->setText(message);
	//VOXOX CHANGE by Alexander 02-23-09: sets the growl message growl will display when an user sends a chat message.
	_message = message;
}

void QtChatToaster::setPixmap(const QPixmap & pixmap) {
	QPixmap scaledPixmap = pixmap.scaled(_ui->pixmapLabel->width()-1,_ui->pixmapLabel->height()-1,Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation);
	_ui->pixmapLabel->setPixmap(scaledPixmap);
}

void QtChatToaster::setProtocolImage(const QPixmap & pixmap) {

	_ui->lblProtocolIcon->setPixmap(pixmap);
}




void QtChatToaster::show() {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	if(config.getNotificationShowToasterOnIncomingChat()){
		//VOXOX CHANGE by Alexander 02-20-09: added growl to chat toaster.
		#ifdef OS_MACOSX
			// Show Growl notification
			//VOXOX CHANGE by Alexander 06-02-09: implemented Singleton to the Growl notifier
			GrowlNotifier::Instance()->notify("Chat Message", "Chat Message", _message, *(_ui->pixmapLabel->pixmap()), false, this, SLOT(chatButtonSlot()));
		#else
			_toaster->show();
		#endif
	}else{
		close();
	}
}

void QtChatToaster::close() {
	_toaster->close();
}

void QtChatToaster::chatButtonSlot() {
	chatButtonClicked();
	close();
}
