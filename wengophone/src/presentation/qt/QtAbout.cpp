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
#include "QtAbout.h"

#include "ui_AboutWindow.h"

#include <WengoPhoneBuildId.h>

#include <util/Logger.h>
#include <util/SafeDelete.h>
#include <cutil/global.h>

#include <QtGui/QtGui>

#include <qtutil/SafeConnect.h>

QtAbout::QtAbout(QWidget * parent):QDialog(0) {

	
	setAttribute(Qt::WA_DeleteOnClose);
	_ui = new Ui::AboutWindow();
	_ui->setupUi(this);

	/* Set a formatted text to the build id label. */
	_ui->wengoPhoneBuildIdStringLabel->setText(QString(WengoPhoneBuildId::getSoftphoneName()) + " " +
					QString(WengoPhoneBuildId::getVersion()) + " rev" +
					QString(WengoPhoneBuildId::getSvnRevision()) + "-" +
					QString::number(WengoPhoneBuildId::getBuildId()));

	//VOXOX -ASV- 08-25-2009: we close the window when CMD + Shift + W is pressed on Mac
	#if defined(OS_MACOSX)
		_closeWindowShortCut = new QShortcut(QKeySequence("Ctrl+Shift+W"), this);
		SAFE_CONNECT(_closeWindowShortCut, SIGNAL(activated()), SLOT(close()));
	#endif
	//VOXOX - JRT - 2009.07.27 - Easy crash testing.
//	char* temp = NULL;
//	strcpy( temp, "xxxxxxxx" );

	/*QFile file(":/data/AUTHORS");
	if (file.open(QFile::ReadOnly)) {
		QString authors = file.readAll();
		file.close();
		_ui->authorsTextEdit->setPlainText(authors);
	} else {
		LOG_ERROR("couldn't locate file=" + file.fileName().toStdString());
	}*/


//	_ui->versionLabel->setVisible(false);
////	/* Set a formatted text to dependencies version label. */
//	_ui->versionLabel->setText("<i>Qt: </i>" + QString(qVersion()) + "<BR>" +
//				"<i>Boost: </i>" + QString(WengoPhoneBuildId::getBoostVersion()) + "<BR>" +
//				"<i>LibPurple: </i>" + QString(WengoPhoneBuildId::getPurpleVersion()) + "<BR>" +
//				"<i>GLib: </i>" + QString(WengoPhoneBuildId::getGLibVersion()) + "<BR>" +
//				"<i>cURL: </i>" + QString(WengoPhoneBuildId::getCurlVersion()) + "<BR>" +
//				"<i>TinyXML: </i>" + QString(WengoPhoneBuildId::getTinyXmlVersion()) +  "<BR>" +
//				"<i>FFmpeg's libavcodec: </i>" + QString(WengoPhoneBuildId::getAvcodecVersion())
//#ifndef OS_MACOSX
//				+ "<BR>" + "<i>PortAudio: </i>" + QString(WengoPhoneBuildId::getPortaudioVersion())
//#endif
//				);
}

QtAbout::~QtAbout() {
	#if defined(OS_MACOSX)
		OWSAFE_DELETE(_closeWindowShortCut);//VOXOX -ASV- 08-25-2009: we close the window when CMD + Shift + W is pressed on Mac
	#endif
	OWSAFE_DELETE(_ui);
}
