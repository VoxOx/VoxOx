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
#include "QtCallBar.h"

#include <qtutil/LanguageChangeEventFilter.h>
//#include <qtutil/WengoStyleLabel.h>
//#include <qtutil/MouseEventFilter.h>
#include <qtutil/SafeConnect.h>

#include "customCallBar/QtVoxOxCallBarFrame.h"	//VOXOX CHANGE Alexander 04-02-09
#include <qtutil/VoxOxToolTipLineEdit.h>
#include <QtGui/QtGui>

QtCallBar::QtCallBar(QWidget * parent)
	: QtVoxOxCallBarWidget(parent) {
	
	init();
}

QtCallBar::~QtCallBar() {
}

//VOXOX CHANGE Rolando 03-25-09
void QtCallBar::init() {	
	
	SAFE_CONNECT(getCallPushButton(),		 SIGNAL(clicked()),						SLOT(callButtonClickedSlot()					));//VOXOX CHANGE Rolando 03-25-09
	SAFE_CONNECT(getQComboBox()->lineEdit(), SIGNAL(returnPressed()),				SLOT(phoneComboBoxReturnPressedSlot()			));//VOXOX CHANGE Rolando 03-25-09
	SAFE_CONNECT(getCallBarFrame(),			 SIGNAL(comboBoxTextChanged(QString)),	SLOT(phoneComboBoxEditTextChangedSlot(QString)	));//VOXOX CHANGE Rolando 03-25-09

	LANGUAGE_CHANGE(this);
}


//VOXOX CHANGE by Rolando - 2009.05.22 - added to get current area code in callbar frame 
std::string QtCallBar::getCurrentAreaCode(){
	return getCallBarFrame()->getCurrentAreaCode().toStdString();
}

//VOXOX CHANGE Rolando 03-25-09
std::string QtCallBar::getPhoneComboBoxCurrentText(){
	//return _phoneComboBox->currentText().toStdString();
	return getCurrentComboBoxText().toStdString();
}

//VOXOX CHANGE Rolando 03-25-09
void QtCallBar::setPhoneComboBoxEditText(const std::string & text) {
	/*_phoneComboBox->setEditText(QString::fromStdString(text));*/
	getCallBarFrame()->getVoxOxToolTipLineEdit()->setText(QString::fromStdString(text));
	
}
//VOXOX CHANGE CJC CLEAR TEXTBOX
void QtCallBar::clearPhoneComboBoxEditText(){
	setPhoneComboBoxEditText("");
}

void QtCallBar::addPhoneComboBoxItem(const std::string & text) {
	//_phoneComboBox->insertItem(0, QString::fromStdString(text));
	getCallBarFrame()->getComboBox()->insertItem(0, QString::fromStdString(text));
}

void QtCallBar::callButtonClickedSlot() {
	
	if(getIsCallButtonActive()){//VOXOX CHANGE Rolando 03-25-09
		callButtonClicked();//VOXOX CHANGE Rolando 03-25-09
	}
	else{
		hangUpButtonClicked();//VOXOX CHANGE Rolando 03-25-09
	}
	
}

//void QtCallBar::hangUpButtonClickedSlot() {
//	hangUpButtonClicked();
//}

//VOXOX CHANGE Rolando 03-25-09
void QtCallBar::phoneComboBoxReturnPressedSlot() {
	//phoneComboBoxReturnPressed();
	if(getIsCallButtonActive()){
		phoneComboBoxReturnPressed();
	}
}

//VOXOX CHANGE Rolando 03-25-09
void QtCallBar::phoneComboBoxEditTextChangedSlot(const QString & text) {
//	if(text != ""){		//VOXOX - JRT - 2009.05.30 - Moved this 'if' to phoneComboBoxEditTextChanged handlers because I need it if text is empty.
		phoneComboBoxEditTextChanged(text);
//	}
}

void QtCallBar::phoneComboBoxClickedSlot() {
	phoneComboBoxClicked();
}

void QtCallBar::languageChanged() {
	/*_phoneComboBox->languageChanged();
	_phoneComboBox->setEditText(tr("+(country code) number or nickname"));*/
}

//QtPhoneComboBox * QtCallBar::getQtPhoneComboBox() {
//	return _phoneComboBox;
//}

//VOXOX CHANGE Rolando 03-25-09
QComboBox * QtCallBar::getQComboBox() {
	return getCallBarFrame()->getComboBox();
}
