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

#include "stdafx.h"		//VOXOX - JRT - 2009.04.05
#include "QtVoxOxCallBarWidget.h"
#include "customCallBar/QtVoxOxCallBarFrame.h"
#include <QtGui/QtGui>
#include <util/SafeDelete.h>
//VOXOX CHANGE Alexander 04-02-09
#include <qtutil/Widget.h>

QtVoxOxCallBarWidget::QtVoxOxCallBarWidget(QWidget * parent): QWidget(parent) {
	_ui = new Ui::VoxOxCallBarWidget();
	_ui->setupUi(this);

	_isCallButtonActive = false;
	setCallPushButtonImage();


//VOXOX CHANGE Alexander 04-02-09: using _qtVoxOxCallBarFrame instead of callBarFrame since it didn't work on Mac
	_qtVoxOxCallBarFrame = new QtVoxOxCallBarFrame();

	Widget::createLayout(_ui->callBarFrame)->addWidget(_qtVoxOxCallBarFrame);

	//VOXOX CHANGE by Rolando - 2009.10.15 - code added to complete task "UI mod for history of numbers dialed"
	_qtVoxOxCallBarFrame->getComboBox()->setStyleSheet(
		_qtVoxOxCallBarFrame->getComboBox()->styleSheet() + " QComboBox#callBarComboBox::down-arrow { image: url(:/pics/callbar/arrow_down_white.png); }");


#ifdef OS_WINDOWS//VOXOX CHANGE by Rolando - 2009.07.13 
	_ui->callBarWidgetLayout->setHorizontalSpacing (9);//VOXOX CHANGE by Rolando - 2009.07.13 
	_ui->callBarWidgetLayout->setVerticalSpacing (0);//VOXOX CHANGE by Rolando - 2009.07.13 
	_ui->callBarWidgetLayout->setContentsMargins( 9, 5, 9, 5 );//VOXOX CHANGE by Rolando - 2009.07.13 
	_ui->callBarFrame->setMinimumSize(1,25);//VOXOX CHANGE by Rolando - 2009.07.13 
	_ui->callBarFrame->setMaximumSize(16777215,25);//VOXOX CHANGE by Rolando - 2009.07.13 
#endif
}

QtVoxOxCallBarWidget::~QtVoxOxCallBarWidget() {
	OWSAFE_DELETE(_ui);
}

void QtVoxOxCallBarWidget::setCUserProfile(CUserProfile* cUserProfile) {
//VOXOX CHANGE Alexander 04-02-09: using _qtVoxOxCallBarFrame instead of callBarFrame since it didn't work on Mac
	_qtVoxOxCallBarFrame->setCUserProfile(cUserProfile);	
}

QPushButton * QtVoxOxCallBarWidget::getCallPushButton(){
	return _ui->callPushButton;
}

void QtVoxOxCallBarWidget::setEnabledCallButton(bool enable){
	_ui->callPushButton->setEnabled(enable);
}

QtVoxOxCallBarFrame * QtVoxOxCallBarWidget::getCallBarFrame(){
//VOXOX CHANGE Alexander 04-02-09: using _qtVoxOxCallBarFrame instead of callBarFrame since it didn't work on Mac
	return _qtVoxOxCallBarFrame;
}

void QtVoxOxCallBarWidget::setCallPushButtonImage(){
	_isCallButtonActive = true;
	_ui->callPushButton->setIcon(QPixmap(QString(":/pics/callbar/btn_initiatecall.png")));
	//VOXOX - CJC - 2009.05.18 Add tooltip
	_ui->callPushButton->setToolTip("Click to Call");
}

void QtVoxOxCallBarWidget::setHangUpPushButtonImage(){
	_isCallButtonActive = false;
	_ui->callPushButton->setIcon(QPixmap(QString(":/pics/callbar/btn_endcall.png")));
	_ui->callPushButton->setToolTip("Click to Hang-Up");
}

bool QtVoxOxCallBarWidget::getIsCallButtonActive(){
	return _isCallButtonActive;
}

QString QtVoxOxCallBarWidget::getCurrentComboBoxText(){
//VOXOX CHANGE Alexander 04-02-09: using _qtVoxOxCallBarFrame instead of callBarFrame since it didn't work on Mac
	return _qtVoxOxCallBarFrame->getCurrentComboBoxText();
}

QString QtVoxOxCallBarWidget::getFullPhoneNumberText(){
//VOXOX CHANGE Alexander 04-02-09: using _qtVoxOxCallBarFrame instead of callBarFrame since it didn't work on Mac
	return _qtVoxOxCallBarFrame->getFullPhoneNumberText();
}

bool QtVoxOxCallBarWidget::textIsPhoneNumber(){
//VOXOX CHANGE Alexander 04-02-09: using _qtVoxOxCallBarFrame instead of callBarFrame since it didn't work on Mac
	return _qtVoxOxCallBarFrame->textIsPhoneNumber();
}


void QtVoxOxCallBarWidget::addComboBoxItem(QString phoneNumber){
//VOXOX CHANGE Alexander 04-02-09: using _qtVoxOxCallBarFrame instead of callBarFrame since it didn't work on Mac
	_qtVoxOxCallBarFrame->addComboBoxItem(phoneNumber);
}