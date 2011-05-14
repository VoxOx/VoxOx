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

#ifndef OWQTVOXOXCALLBARWIDGET_H
#define OWQTVOXOXCALLBARWIDGET_H

#include "ui_VoxOxCallBarWidget.h"

class QPushButton;
class QtVoxOxCallBarFrame;
//VOXOX CHANGE Alexander 04-02-09
class  CUserProfile;

namespace Ui { class VoxOxCallBarWidget; }

class QtVoxOxCallBarWidget: public QWidget{
	Q_OBJECT
public:

	QtVoxOxCallBarWidget(QWidget * parent = 0);

	~QtVoxOxCallBarWidget();

	QPushButton * getCallPushButton();

	QtVoxOxCallBarFrame * getCallBarFrame();

	void setCUserProfile(CUserProfile* cUserProfile);

	void setCallPushButtonImage();
	void setHangUpPushButtonImage();
	bool getIsCallButtonActive();
	void setEnabledCallButton(bool enable);
	QString getCurrentComboBoxText();
	QString getFullPhoneNumberText();
	bool textIsPhoneNumber();

	void addComboBoxItem(QString phoneNumber);

Q_SIGNALS:
	

public Q_SLOTS:

private:
	Ui::VoxOxCallBarWidget * _ui;
	
	bool _isCallButtonActive;

//VOXOX CHANGE Alexander 04-02-09: using _qtVoxOxCallBarFrame instead of callBarFrame since it didn't work on Mac
	QtVoxOxCallBarFrame *_qtVoxOxCallBarFrame;
};

#endif	//OWQTVOXOXCALLBARWIDGET_H
