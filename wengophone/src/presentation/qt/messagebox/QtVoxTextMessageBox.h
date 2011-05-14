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


#ifndef OWQTVOXTEXTMESSAGEBOX_H
#define OWQTVOXTEXTMESSAGEBOX_H

#include <QtCore/QObject>
#include <QtGui/QDialog>

class QWidget;

namespace Ui { class VoxTextMessageBox; }

class QtVoxTextMessageBox : public QDialog {
	Q_OBJECT
public:

	QtVoxTextMessageBox(QWidget * parent);

	~QtVoxTextMessageBox();
	
	void setDialogText(QString newtext);
	
	void setTitle(QString newtitle);

	QString exec();

private Q_SLOTS:

	void accepted();
	void rejected();

private:

	Ui::VoxTextMessageBox * _ui;

};

#endif	//OWQtVoxTextMessageBox_H
