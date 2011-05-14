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
#include "QtVoxOxCallBarFrame.h"

#include <QtGui/QtGui>
#include <QtGui/QFrame>
#include <util/SafeDelete.h>
#include <qtutil/SafeConnect.h>
#include <qtutil/VoxOxToolTipLineEdit.h>
#include <qtutil/VoxOxFrame.h>

#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <model/profile/UserProfile.h>
#include <model/phonecall/SipAddress.h>
#include <model/contactlist/ContactList.h>
#include <model/contactlist/Contact.h>

#include <control/dtmf/CDtmfThemeManager.h>
#include <control/history/CHistory.h>
#include <control/profile/CUserProfile.h>
#include <control/profile/CUserProfileHandler.h>
#include <control/CWengoPhone.h>

#include <presentation/qt/QtWengoPhone.h>

const QString FLAGS_NAME_LANGUAGE		 = QString("english");
const QString CLEAR_RECENT_CALLS_MESSAGE = QString("Clear Recent");
const QString DEFAULT_CALLBAR_MESSAGE	 = QString("Type Name or Number");
const char*	  DEFAULT_THEME_KEYS		 = "default";
const QString COUNTRY_NAME_USA			 = QString("United States");
const QString CODE_AREA_USA              = QString("1");
const QString FACEBOOK_SUFFIX			 = "@facebook.voxox.com";


QtVoxOxCallBarFrame::QtVoxOxCallBarFrame(QWidget * parent): VoxOxFrame(parent) {
	_ui = new Ui::VoxOxCallBarFrame();
	_ui->setupUi(this);

	//VOXOX CHANGE by Rolando - 2009.06.16 
	setStyleSheet(  QString("QtVoxOxCallBarFrame{ background: #000000; border: 1px solid #ababab; border-radius: 5px; } ")
		          + QString( "QComboBox#callBarComboBox{ border: 1px solid black; border-radius: 5px; background: black; } ")
				  + QString( "QComboBox::drop-down:editable { background: black; border: black; } ")
				  + QString( "QFrame#flagFrame{ background-color: #808080; border: 1px solid #666666; border-radius: 2px; }"));

	Config & config = ConfigManager::getInstance().getCurrentConfig();

	_voxOxToolTipLineEdit = new VoxOxToolTipLineEdit(this);
	SAFE_CONNECT(_voxOxToolTipLineEdit, SIGNAL(keyPressedSignal(int)), SLOT(keyPressedSlot(int)));	
	
	//_voxOxToolTipLineEdit->setText(DEFAULT_CALLBAR_MESSAGE);
	_voxOxToolTipLineEdit->setToolTipDefaultText(DEFAULT_CALLBAR_MESSAGE);
	_ui->callBarComboBox->setLineEdit(_voxOxToolTipLineEdit);
	_voxOxToolTipLineEdit->displayToolTipMessage();
	repaint();

	_qtFlagsManager = new QtFlagsManager();
	_qtFlagsListWidget =  new QtFlagsListWidget();
	initFlagListWidget();

	SAFE_CONNECT(_qtFlagsListWidget, SIGNAL(currentFlagChanged(QString)), SLOT(currentFlagChangedSlot(QString)));

	SAFE_CONNECT(_ui->callBarComboBox, SIGNAL(editTextChanged(QString)), SLOT(comboBoxTextChangedSlot(QString)));
	SAFE_CONNECT(_ui->callBarComboBox, SIGNAL(activated(int)),			 SLOT(itemActivatedComboBoxSlot(int)));

	SAFE_CONNECT(_ui->flagLabel,     SIGNAL(clicked()), SLOT(flagClickedSlot()));//VOXOX CHANGE Rolando 03-24-09
	SAFE_CONNECT(_ui->flagDropLabel, SIGNAL(clicked()), SLOT(flagClickedSlot()));//VOXOX CHANGE Rolando 03-24-09
	SAFE_CONNECT(_ui->areaCodeLabel, SIGNAL(clicked()), SLOT(flagClickedSlot()));//VOXOX CHANGE Rolando 03-24-09

#ifdef OS_WINDOWS//VOXOX CHANGE by Rolando - 2009.07.13 
	_ui->callbarFrameLayout->setHorizontalSpacing (5);//VOXOX CHANGE by Rolando - 2009.07.13 
	_ui->callbarFrameLayout->setVerticalSpacing (0);//VOXOX CHANGE by Rolando - 2009.07.13 
	_ui->callbarFrameLayout->setContentsMargins( 1, 1, 2, 1 );//VOXOX CHANGE by Rolando - 2009.07.13 
	setMinimumHeight(23);//VOXOX CHANGE by Rolando - 2009.07.13 
	_ui->flagFrame->setMaximumSize(16777215,19);//VOXOX CHANGE by Rolando - 2009.07.13
	_ui->areaCodeLabel->setMinimumHeight(19);//VOXOX CHANGE by Rolando - 2009.07.13 
	_ui->areaCodeLabel->setMaximumHeight(19);//VOXOX CHANGE by Rolando - 2009.07.13 
#endif
}

QtVoxOxCallBarFrame::~QtVoxOxCallBarFrame() {
	OWSAFE_DELETE(_ui);
	OWSAFE_DELETE(_qtFlagsListWidget);
	OWSAFE_DELETE(_qtFlagsManager);	
}

//VOXOX CHANGE by Rolando - 2009.06.16 
void QtVoxOxCallBarFrame::updateStyleSheet(QString backgroundFlagColor, 
										   QString borderFlagColor,
										   QString flagTextColor,
										   QString backgroundComboBoxColor, 
										   QString borderComboBoxColor, 
										   QString backgroundGeneralColor,
										   QString backgroundComboBoxDropDown,
										   QString primaryComboBoxTextColor,
										   QString secondaryComboBoxTextColor,
										   QString comboBoxDropDownPixmapPath,
										   QString flagDropPixmapPath){

   //VOXOX CHANGE by Rolando - 2009.06.16 
	setStyleSheet(  QString("QtVoxOxCallBarFrame{ background: %1; border-radius: 5px; } ").arg(backgroundGeneralColor)
		          + QString( "QComboBox#callBarComboBox{ border: 1px solid %1; background: %2; border-radius: 0px;} ").arg(borderComboBoxColor).arg(backgroundComboBoxColor)
				  + QString( "QFrame#flagFrame{ border: 1px solid %1; background: %2; border-radius: 2px; } ").arg(borderFlagColor).arg(backgroundFlagColor)
				  + QString( "QLabel#areaCodeLabel{ color: %1; } ").arg(flagTextColor)
				  + QString( "QComboBox::drop-down:editable { background: %1; border: none; } ").arg(backgroundComboBoxDropDown)
				  + QString( "QLabel#flagDropLabel{border: 0; image: url(%1); } ").arg(flagDropPixmapPath)
				  + QString( "VoxOxToolTipLineEdit{ margin-left: 2px; border-radius: 0px;} "));

	if(comboBoxDropDownPixmapPath != ""){
		setStyleSheet( styleSheet() + QString( "QComboBox::down-arrow { image: url(%1); } ").arg(comboBoxDropDownPixmapPath));
	}
				 
	_voxOxToolTipLineEdit->setPrimaryColorStyleSheet(QString(" QWidget{ border: none; color: %1; background: %2; border-radius: 0px; }").arg(primaryComboBoxTextColor).arg(backgroundComboBoxColor));
	_voxOxToolTipLineEdit->setSecondaryColorStyleSheet(QString(" QWidget{ border: none; color: %1; background: %2; border-radius: 0px; }").arg(secondaryComboBoxTextColor).arg(backgroundComboBoxColor));
	update();
}

void QtVoxOxCallBarFrame::setCUserProfile(CUserProfile* cUserProfile) {
	_cUserProfile = cUserProfile;
	fillComboBox();
	fillCompletionList();
}


VoxOxToolTipLineEdit * QtVoxOxCallBarFrame::getVoxOxToolTipLineEdit(){
	return _voxOxToolTipLineEdit;
}

void QtVoxOxCallBarFrame::flagClickedSlot(){

	if(_qtFlagsListWidget->isVisible()){
		_qtFlagsListWidget->close();
	}
	else{
		QPoint p = _ui->flagFrame->pos();
		p.setY(p.y() + _ui->flagFrame->height());
		_qtFlagsListWidget->move(this->mapToGlobal(p));
		_qtFlagsListWidget->show();
	}
}

void QtVoxOxCallBarFrame::keyPressedSlot(int key){
	std::string keyString = "";

	//VOXOX - JRT - 2009.05.27 - With the last few lines commented, this method does nothing.  TODO: remove 	
	switch (key){
		case Qt::Key_0:
			keyString = "0";		
			break;
		case Qt::Key_1:
			keyString = "1";			
			break;
		case Qt::Key_2:
			keyString = "2";			
			break;
		case Qt::Key_3:
			keyString = "3";	
			break;
		case Qt::Key_4:
			keyString = "4";
			break;
		case Qt::Key_5:
			keyString = "5";
			break;
		case Qt::Key_6:
			keyString = "6";
			break;
		case Qt::Key_7:
			keyString = "7";
			break;
		case Qt::Key_8:
			keyString = "8";
			break;
		case Qt::Key_9:
			keyString = "9";
			break;
		case Qt::Key_Asterisk:
			keyString = "*";
			break;
		case Qt::Key_NumberSign:			
			keyString = "#";
			break;	
		default:;				
	}

/*	if(keyString != ""){
		_qtWengoPhone->getCWengoPhone().getCDtmfThemeManager().playTone(DEFAULT_THEME_KEYS,keyString); 
	}*/
}

void QtVoxOxCallBarFrame::currentFlagChangedSlot(QString currentCountryName){
	setFlagButtonPixmap(currentCountryName);
	setCountryCodeText(currentCountryName);
	//currentFlagChanged(currentCountryName);	
	_qtFlagsListWidget->setDefaultFlagByCountryName(currentCountryName);
}

void QtVoxOxCallBarFrame::initFlagListWidget(){
	
	QString countryName = _qtFlagsListWidget->init(_qtFlagsManager, _ui->callBarComboBox, FLAGS_NAME_LANGUAGE);
	setFlagButtonPixmap(countryName);
	setCountryCodeText(countryName);
	_qtFlagsListWidget->setDefaultFlagByCountryName(countryName);
}

/*
* setFlagButtonPixmap: displays a flag image according countryName parameter
* parameters: QString countryName
* returns void
*/
void QtVoxOxCallBarFrame::setFlagButtonPixmap(QString countryName){	

	QtFlag qtFlag = _qtFlagsManager->getFlagByCountryName(countryName, FLAGS_NAME_LANGUAGE);
	_ui->flagLabel->setPixmap(qtFlag.getPixmap());

	if(!qtFlag.isNullCountryName()){

		if(qtFlag.getDefaultCountryName() != ""){//if countryName is valid		
			_ui->flagLabel->setToolTip(QString("Current country selected: ") + qtFlag.getDefaultCountryName());
		}
		else{		
			_ui->flagLabel->setToolTip(QString("Select the country where you want to call"));
		}
	}
	else{		
		_ui->flagLabel->setToolTip(QString("Select the country where you want to call"));
	}
}

void QtVoxOxCallBarFrame::setCountryCodeText(QString countryName){	

	if(!countryName.isEmpty() && countryName.toLower() != QString("none")){//if countryName is valid		
		//searches the corresponding flag according countryName and flagsNameLanguage
		QtFlag qtFlag = _qtFlagsManager->getFlagByCountryName(countryName, FLAGS_NAME_LANGUAGE);
		_ui->areaCodeLabel->setText(qtFlag.getCountryCode());
		_ui->areaCodeLabel->setToolTip(QString("Current country selected: ") + countryName);
	}
	else{
		_ui->areaCodeLabel->setText(QString(""));
		_ui->areaCodeLabel->setToolTip(QString("Select the country where you want to call"));
	}
	
}

QString QtVoxOxCallBarFrame::getCurrentCountryName(){
	return _qtFlagsListWidget->getCurrentCountryName();
}

QString QtVoxOxCallBarFrame::getCurrentAreaCode(){
	return _qtFlagsListWidget->getCurrentCountryCode();
}

QPixmap QtVoxOxCallBarFrame::getCurrentFlagPixmap(){
	return _qtFlagsListWidget->getCurrentFlagPixmap();	
}

QString QtVoxOxCallBarFrame::getCurrentComboBoxText(){
	return getVoxOxToolTipLineEdit()->text();
}

//VOXOX CHANGE by Rolando - 2009.06.28 
QString QtVoxOxCallBarFrame::getPhoneNumberOnlyDigits(QString phoneNumber){
	QString unFormattedPhoneNumber;
	unFormattedPhoneNumber = phoneNumber;
	unFormattedPhoneNumber.remove(QRegExp(QString("[/\\- \\(\\)]+")));

	return unFormattedPhoneNumber;

}

//VOXOX CHANGE by Rolando - 2009.06.28 
void QtVoxOxCallBarFrame::setFullPhoneNumber(QString fullPhoneNumber){

	fullPhoneNumber = getPhoneNumberOnlyDigits(fullPhoneNumber);//VOXOX CHANGE by Rolando - 2009.06.28 

	QString phoneNumber;
	QString codeAreaFound;
	bool error;
	bool removePrefix = false;
	QString prefix = QString("011");

	if(fullPhoneNumber.startsWith(QString("+"))){//VOXOX CHANGE by Rolando - 2009.06.28 
		fullPhoneNumber = fullPhoneNumber.replace(0,1,prefix);
	}

	if(fullPhoneNumber.startsWith(prefix)){//VOXOX CHANGE by Rolando - 2009.06.28 
		removePrefix = true;
	}

	QtFlag flag = getFlagByPhoneNumber(_qtFlagsManager, FLAGS_NAME_LANGUAGE, fullPhoneNumber, prefix, &error, &codeAreaFound);//VOXOX CHANGE by Rolando - 2009.06.28 
			
	if(!error){//VOXOX CHANGE by Rolando - 2009.06.28 
		setFlagButtonPixmap(flag.getDefaultCountryName());
		setCountryCodeText(flag.getDefaultCountryName());
		_qtFlagsListWidget->setDefaultFlagByCountryName(flag.getDefaultCountryName());
		if(removePrefix){
			codeAreaFound = prefix + codeAreaFound;			
		}

		if(codeAreaFound == CODE_AREA_USA){
			phoneNumber = fullPhoneNumber;
		}
		else{
			phoneNumber = fullPhoneNumber.remove(0,codeAreaFound.length());
		}
		
		_voxOxToolTipLineEdit->setText(phoneNumber);//VOXOX CHANGE by Rolando - 2009.08.05 
	}
	else{
		setFlagButtonPixmap(QString(""));//VOXOX CHANGE by Rolando - 2009.06.28 
		setCountryCodeText(QString(""));
		_qtFlagsListWidget->setDefaultFlagByCountryName(QString(""));
		_voxOxToolTipLineEdit->setText(fullPhoneNumber);//VOXOX CHANGE by Rolando - 2009.08.05 
	}


}

QString QtVoxOxCallBarFrame::getFullPhoneNumberText(){
	QString fullPhoneNumber = getCurrentComboBoxText();
	QString areaCode = getCurrentAreaCode();

	if(fullPhoneNumber != ""){//VOXOX CHANGE by Rolando - 2009.05.15 - if phone number is not empty
		if(areaCode != ""){//VOXOX CHANGE by Rolando - 2009.05.15 - if area code is selected
			if(areaCode != "1"){//VOXOX CHANGE by Rolando - 2009.05.15 - if area code is not USA then adds prefix 011 + phone number
				if(!fullPhoneNumber.startsWith("+")){
					fullPhoneNumber = QString("011") + areaCode + fullPhoneNumber;//VOXOX CHANGE by Rolando - 2009.09.09 
				}				
			}
			else{
				if(!fullPhoneNumber.startsWith(QString("1")))//VOXOX CHANGE by Rolando - 2009.07.30 
				{
					if(!fullPhoneNumber.startsWith("+")){//VOXOX CHANGE by Rolando - 2009.09.09 
						fullPhoneNumber = areaCode + fullPhoneNumber;//VOXOX CHANGE by Rolando - 2009.09.09 
					}					
				}				
			}
		}
	}

	return fullPhoneNumber;
}

//VOXOX CHANGE by Rolando - 2009.05.11 - returns true is current fullPhoneNumberText is a phone number
bool QtVoxOxCallBarFrame::textIsPhoneNumber(){
//  formats allowed: 
//  800-555-1212
//  (800) 555-1212
//  (80) 555-1212
//  80-555-1212
//  800-555-12123
//  800 555 1212
//  800/555/1212
//  +1 <number>
//  +39 <number>
//  *<number>

	QString fullPhoneNumberText = getFullPhoneNumberText();
	return fullPhoneNumberText.indexOf(QRegExp("^(\\+\\d\\d?)?[\\-\\s\\/\\.]?[\\(]?(\\d){2,}[\\)]?[\\-\\s\\/\\.]?\\d\\d\\d[\\-\\s\\/\\.]?(\\d){3,}\\b|\\*\\d+$")) >= 0;

}

QComboBox * QtVoxOxCallBarFrame::getComboBox(){
	return _ui->callBarComboBox;
}

void QtVoxOxCallBarFrame::clearComboBoxEditText(){
	getVoxOxToolTipLineEdit()->displayToolTipMessage();
}

void QtVoxOxCallBarFrame::clearComboBox(){
	_ui->callBarComboBox->clear();
	getVoxOxToolTipLineEdit()->displayToolTipMessage();
}


void QtVoxOxCallBarFrame::comboBoxTextChangedSlot(QString newText){
	comboBoxTextChanged(newText);	
}

void QtVoxOxCallBarFrame::fillComboBox() {
	QStringList tobeinserted = QStringList();
	clearComboBox();

	//CUserProfile * cUserProfile =
	//	_qtWengoPhone->getCWengoPhone().getCUserProfileHandler().getCUserProfile();
	if (!_cUserProfile) {
		return;
	}

	//completion of history
	//if _qtHistoryWidget is set it means that History has been created
	
	bool isWengoAccountConnected = _cUserProfile->getUserProfile().hasWengoAccount();
	
	CHistory* chistory = _cUserProfile->getCHistory();
	if (chistory) {
		HistoryMementoCollection * mementos = chistory->getMementos(HistoryMemento::OutgoingCall, 10);
		Config & config = ConfigManager::getInstance().getCurrentConfig();
		QString wengoSuffix = "@" + QString::fromStdString( config.getWengoRealm() );
		for (HistoryMap::iterator it = mementos->begin(); it != mementos->end(); it++) {
			HistoryMemento * memento = (*it).second;
			SipAddress sipAddress(memento->getPeer());
			
			QString username(sipAddress.getSipAddress().c_str());
			if (isWengoAccountConnected) {
				username.remove(wengoSuffix);
			}
			username.remove("sip:");	

			if( !username.isEmpty() && (memento->getState()==HistoryMemento::OutgoingCall) && !tobeinserted.contains(username) ) {
				tobeinserted << username;
			}
		}
		OWSAFE_DELETE(mementos);
	}
	////

	tobeinserted.sort();

	QStringList::const_iterator constIterator;
	for (constIterator = tobeinserted.constBegin(); constIterator != tobeinserted.constEnd();++constIterator){		
		
		addComboBoxItem(*constIterator);

	}

	if(tobeinserted.count() > 0){
		if(_ui->callBarComboBox->findText(CLEAR_RECENT_CALLS_MESSAGE) == -1){
			_ui->callBarComboBox->insertItem (_ui->callBarComboBox->count(), CLEAR_RECENT_CALLS_MESSAGE );
		}
	}

	clearComboBoxEditText();
}

void QtVoxOxCallBarFrame::fillCompletionList(){
	//TODO: this completion list should change when we have SQLite working, so completion list keeps username and we can get their full name like username@im.voxox.com or other network

	QStringList tobeinserted = QStringList();
	QString contactId;
	
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	std::string jabberVoxOxServer = "@" + config.getJabberVoxoxServer();

	if(_cUserProfile) {
		UserProfile & userProfile = _cUserProfile->getUserProfile();
		for (	Contacts::const_iterator it = userProfile.getContactList().getContacts().begin(); 
				it != userProfile.getContactList().getContacts().end(); 
				++it) {

					contactId = QString::fromStdString(it->second.getContactId());//VOXOX CHANGE by Rolando - 2009.05.11 - gets the contact id
					if(!contactId.contains(FACEBOOK_SUFFIX)){//VOXOX CHANGE by Rolando - 2009.05.11 - because id of facebook contacts are "number@"
						// VOXOX CHANGE by ASV 05-12-2009: method "removeDuplicates()" is not part of Q4.4;I had to change it so that it works on the Mac (Qt4.4)
						if (!tobeinserted.contains(contactId)) {						
							tobeinserted << contactId;
						}
						//end VOXOX CHANGE by ASV
					}

					QString temp = QString::fromStdString(it->second.getDisplayName()).toLower();//VOXOX CHANGE by Rolando - 2009.05.11 - gets the display name
					// VOXOX CHANGE by ASV 05-12-2009: method "removeDuplicates()" is not part of Q4.4;I had to change it so that it works on the Mac (Qt4.4)
					if (!tobeinserted.contains(temp)) {
						tobeinserted << temp;
					}
					//end VOXOX CHANGE by ASV

					//tobeinserted << QString::fromStdString(it->second.getVoxOxPhone());//TODO: VOXOX CHANGE by Rolando - 2009.05.08 get VoxOx phone number when SQLite be integrated
		}
	}

	// VOXOX CHANGE by ASV 05-12-2009: method "removeDuplicates()" is not part of Q4.4;I had to change it so that it works on the Mac (Qt4.4)
	//tobeinserted.removeDuplicates();
	//end VOXOX CHANGE by ASV

	if (tobeinserted.size() > 0) {
		tobeinserted.sort();
		QCompleter *completer = new QCompleter(tobeinserted, this);
		completer->setCompletionMode ( QCompleter::PopupCompletion );
		completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
		completer->setCaseSensitivity(Qt::CaseInsensitive);
		_voxOxToolTipLineEdit->setCompleter(completer);
	}	

}

void QtVoxOxCallBarFrame::addComboBoxItem(QString text) {

	QString codeAreaFound;
	bool error;
	QString prefix = QString("011");
	QPixmap flagPixmap;

	QtFlag flag = getFlagByPhoneNumber(_qtFlagsManager, FLAGS_NAME_LANGUAGE, text, prefix, &error, &codeAreaFound);
	
	if(text.startsWith(prefix)){
		text.remove(0, prefix.size());
		text.insert(0, QChar('+'));
	}

	if(!error){
		flagPixmap = flag.getPixmap();
	}
	else{
		flagPixmap = QPixmap();
	}

	if(_ui->callBarComboBox->findText(text) == -1){		
		_ui->callBarComboBox->insertItem ( _ui->callBarComboBox->count() - 1, text );		
	}

	if(_ui->callBarComboBox->findText(CLEAR_RECENT_CALLS_MESSAGE) == -1){
		_ui->callBarComboBox->insertItem (_ui->callBarComboBox->count(), CLEAR_RECENT_CALLS_MESSAGE );
	}
}

const QtFlag QtVoxOxCallBarFrame::getFlagByPhoneNumber(QtFlagsManager * qtFlagsManager, QString flagsNameLanguage, QString text,QString prefix, bool * error, QString * codeAreaFound ){

	if( text.indexOf(QRegExp("([0-9#\\*])+")) != -1){//if it is a phone number not a contact name
		QtFlagsManager::QtFlagList flagList = qtFlagsManager->getQtFlagList(flagsNameLanguage);
		QtFlagsManager::QtFlagList::iterator it;
		int maxLength = 0;
		int matchedLength = 0;
		QtFlag * flagReturned =  new QtFlag();
		QString areaCode = QString("");
		QRegExp * codeAreaRegExp =  new QRegExp();
		if( text.startsWith ( QChar('+')) ){
			text.replace( 0 ,1, prefix);
		}

		for (it = flagList.begin(); it != flagList.end(); it++) {
			QString countryCode = it->getCountryCode();//gets the country code
			if( countryCode.startsWith ( QChar('+')) ){
				countryCode.replace( 0 ,1, prefix);
				codeAreaRegExp->setPattern(QString("^") + countryCode);
				if(codeAreaRegExp->indexIn(text) != -1){
					matchedLength = codeAreaRegExp->matchedLength();
					if(matchedLength > maxLength){
						areaCode = codeAreaRegExp->cap();						
						*flagReturned = *it;
						maxLength = matchedLength;
					}
				}
			}

			
		}
		
		if(maxLength > 0){
			areaCode.remove(0, prefix.size());
			*codeAreaFound = areaCode;
			*error =  false;
			return *flagReturned;
		}
		else{
			if(text.startsWith(QString("1"))){//VOXOX CHANGE by Rolando - 2009.06.28 
				*codeAreaFound = QString("1");//VOXOX CHANGE by Rolando - 2009.06.28 
				*error =  false;//VOXOX CHANGE by Rolando - 2009.06.28 
				return _qtFlagsManager->getFlagByCountryName(COUNTRY_NAME_USA, FLAGS_NAME_LANGUAGE);	//VOXOX CHANGE by Rolando - 2009.06.28 		
			}
		}
	}
			
	*error = true;//flag was not found so sets error equal true
	return QtFlag();//necessary to return something in case that an areaCode was not found in parameter text
}

void QtVoxOxCallBarFrame::clearOutgoingCalls(){
	/*if (_qtWengoPhone->getCWengoPhone().getCUserProfileHandler().getCUserProfile() && _qtWengoPhone->getCWengoPhone().getCUserProfileHandler().getCUserProfile()->getCHistory()) {
		_qtWengoPhone->getCWengoPhone().getCUserProfileHandler().getCUserProfile()->getCHistory()->clear(HistoryMemento::OutgoingCall);
	}*/
	if (_cUserProfile && _cUserProfile->getCHistory()) {
		_cUserProfile->getCHistory()->clear(HistoryMemento::OutgoingCall);
	}
	clearComboBox();

}

void QtVoxOxCallBarFrame::itemActivatedComboBoxSlot(int index){
	QString newText = _ui->callBarComboBox->itemText(index);

	if(newText == CLEAR_RECENT_CALLS_MESSAGE){
		clearOutgoingCalls();
		clearComboBoxEditText();
	}
	else{
		if(_ui->callBarComboBox->currentText() != newText){
			QString codeAreaFound;
			bool error;
			QString prefix = QString("011");
			QPixmap flagPixmap;

			QtFlag flag = getFlagByPhoneNumber(_qtFlagsManager, FLAGS_NAME_LANGUAGE, newText, prefix, &error, &codeAreaFound);
			
			if(!error){
				setFlagButtonPixmap(flag.getDefaultCountryName());
				setCountryCodeText(flag.getDefaultCountryName());
				_qtFlagsListWidget->setDefaultFlagByCountryName(flag.getDefaultCountryName());
			}
			else{
				setFlagButtonPixmap(QString(""));
				setCountryCodeText(QString(""));
				_qtFlagsListWidget->setDefaultFlagByCountryName(QString(""));
			}
			
			if(_ui->callBarComboBox->findText(newText) != -1){
				
				newText = newText.remove(QString("+") + codeAreaFound);
				_voxOxToolTipLineEdit->setText(newText);//VOXOX CHANGE by Rolando - 2009.08.05 
			}

			comboBoxTextChanged(newText);
		}

	}
	
}
