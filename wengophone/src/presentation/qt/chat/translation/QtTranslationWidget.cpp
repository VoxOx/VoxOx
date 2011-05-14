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
* CLASS DESCRIPTION 
* @author Chris Jimenez C 
* @date 2010.01.16
*/


#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtTranslationWidget.h"

#include "QtTranslationLanguage.h"
#include "QtTranslationManager.h"

#include "ui_TranslationWidget.h"

#include <util/Logger.h>
#include <util/SafeDelete.h>
#include <cutil/global.h>

#include <qtutil/SafeConnect.h>
#include <presentation/qt/messagebox/QtVoxMessageBox.h>
#include <QtGui/QtGui>
#include <QtXml/QtXml>

QtTranslationWidget::QtTranslationWidget(CChatHandler & cChatHandler,QWidget * parent)
	: QDialog(parent),
	  _cChatHandler(cChatHandler){
		

		_ui = new Ui::TranslationWidget();
		_ui->setupUi(this);
		  
		initComboBoxes();

		SAFE_CONNECT(_ui->chkTranslateMessages, SIGNAL(stateChanged(int)), SLOT(updateComboState())); // VOXOX -ASV- 2010.02.01
				
		SAFE_CONNECT(_ui->btnOk,	SIGNAL(clicked()), SLOT(okClicked())	);
}

//void QtTranslationWidget::buttonClicked(const QtEmoticon & emoticon) {
//	if (_state == Popup) {
//		close();
//	}
//	emoticonClicked(emoticon);
//}

//void QtTranslationWidget::changeState() {
//	if (_state == Popup) {
//		close();
//		setWindowFlags(Qt::Window);
//		_state = Window;
//		show();
//	} else {
//		close();
//		setWindowFlags(Qt::Popup);
//		_state = Popup;
//	}
//}


void QtTranslationWidget::showDialog(){

	this->show();
	
}


void QtTranslationWidget::okClicked(){

	QtTranslationLanguage source = getLanguage(_ui->cmbSrcLanguage->currentText());
	QtTranslationLanguage dest = getLanguage(_ui->cmbDestLanguage->currentText());
	QtEnumTranslationMode::Mode mode;
	if(_ui->chkTranslateMessages->isChecked() && _ui->cmbTranslateMessages->currentText() == QString("All Messages")){
		
		mode = QtEnumTranslationMode::Both;

	}else if(_ui->chkTranslateMessages->isChecked() && _ui->cmbTranslateMessages->currentText() == QString("Messages Received")){

		mode = QtEnumTranslationMode::Incoming;
		
	}else if(_ui->chkTranslateMessages->isChecked() && _ui->cmbTranslateMessages->currentText() == QString("Messages Sent")){

		mode = QtEnumTranslationMode::Outgoing;
	}else{

		mode = QtEnumTranslationMode::None;
	}
	
	//VOXOX - CJC - 2010.02.04 Validate Form
	if(mode != QtEnumTranslationMode::None){

		if(source.getTranslationName() == "" && dest.getTranslationName() == ""){
			QtVoxMessageBox box(0);
			box.setWindowTitle(tr("VoxOx - Language problem"));
			box.setText(tr("You need to select the translation languages, please select your languange and your contact's language."));
			box.setStandardButtons(QMessageBox::Ok);
			box.exec();
			return;
		}else if(source.getTranslationName() == ""){
			QtVoxMessageBox box(0);
			box.setWindowTitle(tr("VoxOx - Language problem"));
			box.setText(tr("Please select your language."));
			box.setStandardButtons(QMessageBox::Ok);
			box.exec();
			return;
		}
		else if(dest.getTranslationName() == ""){
			QtVoxMessageBox box(0);
			box.setWindowTitle(tr("VoxOx - Language problem"));
			box.setText(tr("Please select your contact's language."));
			box.setStandardButtons(QMessageBox::Ok);
			box.exec();
			return;
		}
	}

	translationSettingsSignal(mode,source,dest);

	this->accept();
	
}

// VOXOX -ASV- 2010.02.01
void QtTranslationWidget::updateComboState() {

	_ui->cmbSrcLanguage->setEnabled(_ui->chkTranslateMessages->isChecked());
	_ui->cmbDestLanguage->setEnabled(_ui->chkTranslateMessages->isChecked());	
	_ui->lblMyLanguage->setEnabled(_ui->chkTranslateMessages->isChecked());	
	_ui->lblContactLanguage->setEnabled(_ui->chkTranslateMessages->isChecked());	
}

void QtTranslationWidget::initSettings(QtEnumTranslationMode::Mode mode, const QtTranslationLanguage & sourceLanguage, const QtTranslationLanguage & destLanguage){
	
	if(mode == QtEnumTranslationMode::Both){

		_ui->cmbTranslateMessages->setCurrentIndex(_ui->cmbTranslateMessages->findText(QString("All Messages"),Qt::MatchExactly));
		_ui->chkTranslateMessages->setChecked(true);

	}else if(mode == QtEnumTranslationMode::Incoming){

		_ui->cmbTranslateMessages->setCurrentIndex(_ui->cmbTranslateMessages->findText(QString("Messages Received"),Qt::MatchExactly));
		_ui->chkTranslateMessages->setChecked(true);
		
	}else if(mode == QtEnumTranslationMode::Outgoing){

		_ui->cmbTranslateMessages->setCurrentIndex(_ui->cmbTranslateMessages->findText(QString("Messages Sent"),Qt::MatchExactly));
		_ui->chkTranslateMessages->setChecked(true);
	}else{
		_ui->cmbTranslateMessages->setCurrentIndex(_ui->cmbTranslateMessages->findText(QString("All Messages"),Qt::MatchExactly));
		_ui->chkTranslateMessages->setChecked(false);
	}
	
	if(sourceLanguage.getTranslationName()!= ""){
		_ui->cmbSrcLanguage->setCurrentIndex(_ui->cmbSrcLanguage->findText(sourceLanguage.getTranslationName(),Qt::MatchExactly));
	}else{
		_ui->cmbSrcLanguage->setCurrentIndex(_ui->cmbSrcLanguage->findText("English",Qt::MatchExactly));
	}
	if(destLanguage.getTranslationName()!= ""){
		_ui->cmbDestLanguage->setCurrentIndex(_ui->cmbDestLanguage->findText(destLanguage.getTranslationName(),Qt::MatchExactly));
	}else{
		_ui->cmbDestLanguage->setCurrentIndex(0);
	}

	updateComboState();// VOXOX -ASV- 2010.02.01
}


void QtTranslationWidget::initComboBoxes() {

	QtTranslationManager * qtTranslationManager = QtTranslationManager::getInstance(QString::fromStdString(_cChatHandler.getTranslationLanguageXML()));
	QtTranslationManager::QtTranslationLanguageList translationList = qtTranslationManager->getQtTranslationLanguageList();
	QtTranslationManager::QtTranslationLanguageList::iterator it;

	_ui->cmbSrcLanguage->addItem("Select Language");
	_ui->cmbDestLanguage->addItem("Select Language");

	for (it = translationList.begin(); it != translationList.end(); it++) {

		QString strText = (*it).getTranslationName();

		_ui->cmbSrcLanguage->addItem(strText);
		_ui->cmbDestLanguage->addItem(strText);
	}	
}


QtTranslationLanguage QtTranslationWidget::getLanguage(const QString & text){

	QtTranslationManager * qtTranslationManager = QtTranslationManager::getInstance(QString::fromStdString(_cChatHandler.getTranslationLanguageXML()));
	QtTranslationLanguage language = qtTranslationManager->getTranslationLanguage(text);
	return language;

}

QtEnumTranslationMode::Mode QtTranslationWidget::getMode(const QString & text){

	QtEnumTranslationMode::Mode mode = QtEnumTranslationMode::toTranslationMode(text);
	return mode;

}

