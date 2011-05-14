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
* Box for entering EMAILs
* @author Chris Jimenez C 
* @date 2009.06.10
*/


#ifndef OWQTVOXEMAILMESSAGEBOX_H
#define OWQTVOXEMAILMESSAGEBOX_H

#include <QtCore/QObject>
#include <QtGui/QDialog>

#include <presentation/qt/QtEnumEmailType.h>
class QWidget;

namespace Ui { class VoxEmailMessageBox; }

class QtVoxEmailMessageBox : public QDialog {
	Q_OBJECT
public:

	QtVoxEmailMessageBox(QWidget * parent);

	~QtVoxEmailMessageBox();
	
	void setDialogText(QString newtext);
	
	void setTitle(QString newtitle);

	void setEmailTypeVisible(bool visible);

	
	QString getEmail();

	QtEnumEmailType::Type getEmailType(); 

private Q_SLOTS:

	void accepted();
	void rejected();


private:

	Ui::VoxEmailMessageBox * _ui;

};

#endif	//OWQtVoxEmailMessageBox_H
