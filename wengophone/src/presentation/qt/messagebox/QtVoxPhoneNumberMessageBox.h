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
* Box for entering phone numbers
* @author Chris Jimenez C 
* @date 2009.06.10
*/


#ifndef OWQTVOXPHONENUMBERMESSAGEBOX_H
#define OWQTVOXPHONENUMBERMESSAGEBOX_H

#include <QtCore/QObject>


#include <presentation/qt/callbar/customCallBar/QtVoxOxCallBarFrame.h>

#include <presentation/qt/QtEnumPhoneType.h>
class QWidget;
class QDialog;
namespace Ui { class VoxPhoneNumberMessageBox; }

class QtVoxPhoneNumberMessageBox : public QDialog {
	Q_OBJECT
public:

	QtVoxPhoneNumberMessageBox(QWidget * parent);

	~QtVoxPhoneNumberMessageBox();
	
	void setDialogText(QString newtext);
	
	void setTitle(QString newtitle);

	void setNumberTypeVisible(bool visible);

	void showOnlyMobileType();
	
	QString getNumber();

	QtEnumPhoneType::Type getPhoneType(); 

private Q_SLOTS:

	void accepted();
	void rejected();


private:

	Ui::VoxPhoneNumberMessageBox * _ui;

	QtVoxOxCallBarFrame * _qtVoxOxCallBarFrame;

};

#endif	//OWQtVoxPhoneNumberMessageBox_H
