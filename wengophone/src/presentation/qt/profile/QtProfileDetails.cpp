/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2007  Wengo
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
#include "QtProfileDetails.h"

#include "ui_ProfileDetails.h"

#include <imwrapper/IMContactSet.h>

#include <presentation/qt/imcontact/QtAdvancedIMContactManager.h>
#include <presentation/qt/imcontact/QtSimpleIMContactManager.h>
#include <presentation/qt/imaccount/QtIMAccountManager.h>

#include <presentation/qt/config/QtWengoConfigDialog.h>//VOXOX - CJC - 2009.07.03 
#include <control/contactlist/CContactList.h>
#include <control/profile/CUserProfile.h>

#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <model/contactlist/ContactProfile.h>
#include <model/profile/UserProfile.h>

#include <cutil/global.h>

#include <qtutil/DesktopService.h>
#include <qtutil/ImageSelector.h>
#include <qtutil/PixmapMerging.h>
#include <qtutil/SafeConnect.h>
#include <qtutil/StringListConvert.h>

#include <util/CountryList.h>
#include <util/Logger.h>
#include <util/WebBrowser.h>
#include <util/SafeDelete.h>
#include <qtutil/Widget.h>
//VOXOX - CJC - 2009.06.10 
#include <presentation/qt/messagebox/QtVoxMessageBox.h>
#include <presentation/qt/callbar/customCallBar/QtVoxOxCallBarFrame.h>
#include <qtutil/VoxOxToolTipLineEdit.h>
#include <QtGui/QtGui>
#include <qtutil/CloseEventFilter.h>//VOXOX - CJC - 2009.07.09 
#include <presentation/qt/QtWengoPhone.h>//VOXOX - CJC - 2009.07.09 
#include <presentation/qt/QtVoxWindowManager.h>//VOXOX - CJC - 2009.07.09 
#include <control/CWengoPhone.h>//VOXOX - CJC - 2009.07.11 
#ifdef OS_WINDOWS
	#include <windows.h>
#endif

static const char * PNG_FORMAT = "PNG";
//QtProfileDetails::QtProfileDetails(CUserProfile & cUserProfile,
//	ContactProfile & contactProfile, QWidget * parent, const QString & windowTitle)
//	: QtVoxWindowInfo(parent,QtEnumWindowType::ProfileWindow,Qt::Window | Qt::WindowTitleHint), //VOXOX CHANGE by ASV 07-03-2009: This window should allow other windows to be in front
//	_cUserProfile(cUserProfile),
//	_profile(contactProfile) {
//	_ui = new Ui::ProfileDetails();
//	_ui->setupUi(this);
//	
//
//	init(parent);
//	setWindowTitle(tr("VoxOx") + " - " + windowTitle);
//
//	//FIXME we should keep in memory the UUID of the group
////	std::vector< std::pair<std::string, std::string> > tmp = _cUserProfile.getCContactList().getContactGroups();
////	for (std::vector< std::pair<std::string, std::string> >::const_iterator it = tmp.begin(); it != tmp.end(); ++it) 
////	ContactGroupSet& rGroups = _cUserProfile.getCContactList().getContactGroupSet();
////	for ( ContactGroupSet::const_iterator it = rGroups.begin(); it != rGroups.end(); it++ )
////	{
////		if ( (*it).second.isUser() )
////		{
//////			_ui->groupComboBox->addItem(QString::fromUtf8((*it).second.c_str()), QString::fromStdString((*it).first.c_str()));
////			_ui->groupComboBox->addItem(QString::fromUtf8((*it).second.getName().c_str()), QString::fromStdString((*it).first.c_str()));
////			std::string str1 = (*it).first;
////	//		std::string str2 = contactProfile.getGroupId();
////			std::string str2 = contactProfile.getFirstUserGroupId();	//VOXOX - JRT - 2009.05.06 JRT-GRPS - TODO: How would multiple user groups affect this?
////			if (str1 == str2) 
////			{
//////				_ui->groupComboBox->setCurrentIndex(_ui->groupComboBox->findText(QString::fromUtf8((*it).second.c_str())));
////				_ui->groupComboBox->setCurrentIndex(_ui->groupComboBox->findText(QString::fromUtf8((*it).second.getName().c_str())));	//VOXOX - JRT - 2009.05.09 
////			}
////		}
////	}
//
//	////QtSimpleIMContactManager
//	//_qtIMContactManager = new QtSimpleIMContactManager(contactProfile, _cUserProfile, _profileDetailsWindow);
//	//int index = _ui->imStackedWidget->addWidget(_qtIMContactManager->getWidget());
//	//_ui->imStackedWidget->setCurrentIndex(index);
//	//
//	//SAFE_CONNECT(_qtIMContactManager, SIGNAL(contentHasChanged()), SLOT(updateSaveButtonState()));
//	//SAFE_CONNECT(_ui->mobilePhoneLineEdit, SIGNAL(textChanged(const QString &)), SLOT(updateSaveButtonState()));
//	//SAFE_CONNECT(_ui->homePhoneLineEdit, SIGNAL(textChanged(const QString &)), SLOT(updateSaveButtonState()));
//
//	////saveButton
//	//updateSaveButtonState();
//	//SAFE_CONNECT(_ui->saveButton, SIGNAL(clicked()), SLOT(saveContact()));
//
//	////avatarPixmapButton
//	//_ui->avatarPixmapButton->setToolTip("");
//	//_ui->avatarPixmapButton->setEnabled(false);
////	_ui->myProfileLabel->hide();
//}

QtProfileDetails::QtProfileDetails(CUserProfile & cUserProfile,
	UserProfile & userProfile, QWidget * parent, const QString & windowTitle)
	: QtVoxWindowInfo(parent,QtEnumWindowType::ProfileWindow,Qt::Window | Qt::WindowTitleHint),
	_cUserProfile(cUserProfile),
	_profile(userProfile) {
	_ui = new Ui::ProfileDetails();
	_ui->setupUi(this);

	init(parent);
	setWindowTitle(tr("VoxOx") + " - " + windowTitle);

	//Not needed for UserProfile
	/*_ui->groupLabel->hide();
	_ui->groupComboBox->hide();*/

	////QtIMAccountManager
	//QtIMAccountManager * qtIMAccountManager =
	//	new QtIMAccountManager((UserProfile &) _profile, false, _profileDetailsWindow);
	//int index = _ui->imStackedWidget->addWidget(qtIMAccountManager->getWidget());
	//_ui->imStackedWidget->setCurrentIndex(index);
//	_ui->advancedButton->hide();
	/// no more simple mode

	//VOXOX -ASV- 08-25-2009: we close the window when CMD + Shift + W is pressed on Mac
	#if defined(OS_MACOSX)
		_closeWindowShortCut = new QShortcut(QKeySequence("Ctrl+Shift+W"), this);
		SAFE_CONNECT(_closeWindowShortCut, SIGNAL(activated()), SLOT(closeEvent()));
	#endif
}

QtProfileDetails::~QtProfileDetails() {
	delete _callBarFrameMobile;
	delete _callBarFrameHome;
	delete _callBarFrameWork;
	delete _callBarFrameFax;
	delete _callBarFrameOther;
	delete _ui;
}

void QtProfileDetails::init(QWidget * parent) {
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	
	
	//VOXOX - CJC - 2009.07.13 Relationship is missing on the sync
	_ui->cmbRelationship->setVisible(false);
	_ui->lblRelationship->setVisible(false);


	_callBarFrameMobile = new QtVoxOxCallBarFrame();
	_callBarFrameMobile->updateStyleSheet("808080","666666","#000000","#ffffff","none","transparent","white", "black", "gray");//VOXOX CHANGE by Rolando - 2009.06.15 
	_callBarFrameMobile->getVoxOxToolTipLineEdit()->setToolTipDefaultText("Enter Number");
	Widget::createLayout(_ui->framePhone1)->addWidget(_callBarFrameMobile);

	_callBarFrameHome = new QtVoxOxCallBarFrame();
	_callBarFrameHome->updateStyleSheet("808080","666666","#000000","#ffffff","none","transparent","white", "black", "gray");//VOXOX CHANGE by Rolando - 2009.06.15 
	_callBarFrameHome->getVoxOxToolTipLineEdit()->setToolTipDefaultText("Enter Number");
	Widget::createLayout(_ui->framePhone2)->addWidget(_callBarFrameHome);

	_callBarFrameWork = new QtVoxOxCallBarFrame();
	_callBarFrameWork->updateStyleSheet("808080","666666","#000000","#ffffff","none","transparent","white", "black", "gray");//VOXOX CHANGE by Rolando - 2009.06.15 
	_callBarFrameWork->getVoxOxToolTipLineEdit()->setToolTipDefaultText("Enter Number");
	Widget::createLayout(_ui->framePhone3)->addWidget(_callBarFrameWork);

	_callBarFrameFax = new QtVoxOxCallBarFrame();
	_callBarFrameFax->updateStyleSheet("808080","666666","#000000","#ffffff","none","transparent","white", "black", "gray");//VOXOX CHANGE by Rolando - 2009.06.15 
	_callBarFrameFax->getVoxOxToolTipLineEdit()->setToolTipDefaultText("Enter Number");
	Widget::createLayout(_ui->framePhone4)->addWidget(_callBarFrameFax);

	_callBarFrameOther = new QtVoxOxCallBarFrame();
	_callBarFrameOther->updateStyleSheet("808080","666666","#000000","#ffffff","none","transparent","white", "black", "gray");//VOXOX CHANGE by Rolando - 2009.06.15 
	_callBarFrameOther->getVoxOxToolTipLineEdit()->setToolTipDefaultText("Enter Number");
	Widget::createLayout(_ui->framePhone5)->addWidget(_callBarFrameOther);

#ifdef OS_WINDOWS//VOXOX CHANGE by Rolando - 2009.07.13 
	_callBarFrameMobile->setMaximumHeight(23);//VOXOX CHANGE by Rolando - 2009.07.13
	_callBarFrameHome->setMaximumHeight(23);//VOXOX CHANGE by Rolando - 2009.07.13
	_callBarFrameWork->setMaximumHeight(23);//VOXOX CHANGE by Rolando - 2009.07.13
	_callBarFrameFax->setMaximumHeight(23);//VOXOX CHANGE by Rolando - 2009.07.13
	_callBarFrameOther->setMaximumHeight(23);//VOXOX CHANGE by Rolando - 2009.07.13
#endif

	QString accountId = QString::fromStdString(_cUserProfile.getUserProfile().getVoxOxAccount()->getDisplayAccountId());
	QString md5 = QString::fromStdString(_cUserProfile.getUserProfile().getMd5());
	QString url = QString::fromStdString(config.getChangePasswordUrl());

	QString realUrl = QString("%1?username=%2&userkey=%3").arg(url).arg(accountId).arg(md5);

	_ui->webView->load(QUrl(realUrl));



	//populateCountryList();
	CloseEventFilter * closeEventFilter = new CloseEventFilter(this, SLOT(closeEvent()));
	this->installEventFilter(closeEventFilter);

	SAFE_CONNECT(_ui->cancelButton, SIGNAL(clicked()), SLOT(cancelButtonClicked()));

	SAFE_CONNECT(_ui->lblMyNetworks, SIGNAL(linkActivated(const QString &)), SLOT(openMyNetworks()));

	//saveButton
	SAFE_CONNECT(_ui->saveButton, SIGNAL(clicked()), SLOT(saveUserProfile()));

	//avatarPixmapButton
	SAFE_CONNECT(_ui->avatarPixmapButton, SIGNAL(clicked()), SLOT(changeUserProfileAvatar()));
	//SAFE_CONNECT(_ui->advancedButton, SIGNAL(clicked()), SLOT(advancedButtonClicked()));
	//SAFE_CONNECT(_ui->websiteButton, SIGNAL(clicked()), SLOT(websiteButtonClicked()));
	//SAFE_CONNECT(_ui->emailButton, SIGNAL(clicked()), SLOT(emailButtonClicked()));

	//_qtIMContactManager = NULL;

	readProfile();
}

void QtProfileDetails::populateCountryList() {
//	_ui->countryComboBox->addItems(StringListConvert::toQStringList(CountryList::getCountryList()));
}

void QtProfileDetails::openMyNetworks() {

	QtWengoPhone *qtWengoPhone = dynamic_cast<QtWengoPhone *>(_cUserProfile.getCWengoPhone().getPresentation());
	qtWengoPhone->getQtVoxWindowManager()->showConfigWindow(QString("Networks"));

}


void QtProfileDetails::closeEvent() {//VOXOX - CJC - 2009.07.09 
	windowClose(getKey());
}



void QtProfileDetails::readProfile() {

	QString firstName = QString::fromUtf8(_profile.getFirstName().c_str());

	QString lastName = QString::fromUtf8(_profile.getLastName().c_str());

	StreetAddress address = _profile.getStreetAddress();
	QString location = QString::fromStdString(address.getStreet1());
	//VOXOX - CJC - 2009.06.24 Missing gender from integration
	QString gender = QString::fromStdString(EnumSex::toString(_profile.getSex()));
	
	int year = _profile.getBirthdate().getYear();
	int month = _profile.getBirthdate().getMonth();
	int day = _profile.getBirthdate().getDay();
	QDate birthday(year,month,day);
	//VOXOX - CJC - 2009.06.24 Missing relationship
	QString interest = QString::fromUtf8(_profile.getInterests().c_str());

	bool searchable = _profile.isSearchable();

	_ui->firstNameLineEdit->setText(firstName);
	_ui->lastNameLineEdit->setText(lastName);

	if(gender=="male"){
		_ui->cmbGender->setCurrentIndex(0);
	}else if(gender == "female"){
		_ui->cmbGender->setCurrentIndex(1);
	}

	_ui->txtLocation->setText(location);
	_ui->dtBirthday->setDate(birthday);
	_ui->txtInterest->setText(interest);
	_ui->chkShareProfile->setChecked(searchable);

	QString mobile = QString::fromStdString(_profile.getMobilePhone());
	QString home = QString::fromStdString(_profile.getHomePhone());
	QString work = QString::fromStdString(_profile.getWorkPhone());
	QString fax = QString::fromStdString(_profile.getFax());
	QString other = QString::fromStdString(_profile.getOtherPhone());
	
	if(mobile!=""){
		_callBarFrameMobile->setFullPhoneNumber(mobile);
	}
	if(home!=""){
	_callBarFrameHome->setFullPhoneNumber(home);
	}
	if(work!=""){
	_callBarFrameWork->setFullPhoneNumber(work);
	}
	if(fax!=""){
	_callBarFrameFax->setFullPhoneNumber(fax);
	}
	if(other!=""){
	_callBarFrameOther->setFullPhoneNumber(other);
	}


	//_ui->genderComboBox->setCurrentIndex((int) _profile.getSex());

	//Date date = _profile.getBirthdate();
	//_ui->birthDate->setDate(QDate(date.getYear(), date.getMonth(), date.getDay()));

	//StreetAddress address = _profile.getStreetAddress();
	//_ui->countryComboBox->setCurrentIndex(_ui->countryComboBox->findText(QString::fromUtf8(address.getCountry().c_str())));
	//_ui->cityLineEdit->setText(QString::fromUtf8(address.getCity().c_str()));
	//_ui->stateLineEdit->setText(QString::fromUtf8(address.getStateProvince().c_str()));

	//_ui->mobilePhoneLineEdit->setText(QString::fromStdString(_profile.getMobilePhone()));
	//_ui->homePhoneLineEdit->setText(QString::fromStdString(_profile.getHomePhone()));

	//_ui->emailLineEdit->setText(QString::fromStdString(_profile.getPersonalEmail()));
	//_ui->webLineEdit->setText(QString::fromStdString(_profile.getWebsite()));

	//_ui->notesEdit->setPlainText(QString::fromUtf8(_profile.getNotes().c_str()));

	updateAvatarButton();
}

void QtProfileDetails::updateAvatarButton() {
	//std::string backgroundPixmapFilename = ":/pics/avatar_background.png";//VOXOX CHANGE by Rolando - 2009.06.29 
	//std::string foregroundPixmapData = _profile.getIcon().getData();//VOXOX CHANGE by Rolando - 2009.06.29 

	//_ui->avatarPixmapButton->setIcon(PixmapMerging::merge(foregroundPixmapData, backgroundPixmapFilename,Qt::KeepAspectRatio));//VOXOX CHANGE by Rolando - 2009.06.29 

	std::string foregroundPixmapData = _profile.getIcon().getData();//VOXOX CHANGE by Rolando - 2009.06.29 
	QPixmap foregroundPixmap;//VOXOX CHANGE by Rolando - 2009.06.29 
	foregroundPixmap.loadFromData((uchar*) foregroundPixmapData.c_str(), foregroundPixmapData.size());//VOXOX CHANGE by Rolando - 2009.06.29 
	_ui->avatarPixmapButton->setIcon(foregroundPixmap);//VOXOX CHANGE by Rolando - 2009.06.29 
}

void QtProfileDetails::saveProfile() {

	_profile.setFirstName(_ui->firstNameLineEdit->text().toStdString());
	_profile.setLastName(_ui->lastNameLineEdit->text().toStdString());

	StreetAddress address = _profile.getStreetAddress();
	address.setStreet1(_ui->txtLocation->text().toStdString());
	_profile.setStreetAddress(address);

	Date date(_ui->dtBirthday->date().day(),_ui->dtBirthday->date().month(),_ui->dtBirthday->date().year());

	_profile.setBirthdate(date);
	_profile.setInterests(_ui->txtInterest->toPlainText().toStdString());
	QString sex;
	if(_ui->cmbGender->currentIndex()==0){
		sex = "male";
	}else if(_ui->cmbGender->currentIndex()==1){
		sex = "female";
	}
	_profile.setSex(EnumSex::toSex(sex.toStdString()));
	_profile.setIsSearchable(_ui->chkShareProfile->isChecked());

	QString mobile = _callBarFrameMobile->getFullPhoneNumberText();
	QString home = _callBarFrameHome->getFullPhoneNumberText();
	QString work = _callBarFrameWork->getFullPhoneNumberText();
	QString fax = _callBarFrameFax->getFullPhoneNumberText();
	QString other = _callBarFrameOther->getFullPhoneNumberText();

	_profile.setMobilePhone(mobile.toStdString());
	_profile.setHomePhone(home.toStdString());
	_profile.setWorkPhone(work.toStdString());
	_profile.setFax(fax.toStdString());
	_profile.setOtherPhone(other.toStdString());





	/*QDate date = _ui->birthDate->date();
	_profile.setBirthdate(Date(date.day(), date.month(), date.year()));

	_profile.setSex((EnumSex::Sex) _ui->genderComboBox->currentIndex());

	StreetAddress address;
	address.setType( "main" );
	address.setCountry(_ui->countryComboBox->currentText().toUtf8().data());
	address.setStateProvince(_ui->stateLineEdit->text().toUtf8().data());
	address.setCity(_ui->cityLineEdit->text().toUtf8().data());

	_profile.setStreetAddress(address);

	_profile.setMobilePhone(_ui->mobilePhoneLineEdit->text().toStdString());
	_profile.setHomePhone(_ui->homePhoneLineEdit->text().toStdString());

	_profile.setPersonalEmail(_ui->emailLineEdit->text().toStdString());
	_profile.setWebsite(_ui->webLineEdit->text().toStdString());

	_profile.setNotes(_ui->notesEdit->toPlainText().toUtf8().constData());*/
}

//ContactProfile & QtProfileDetails::fillAndGetContactProfile() {
//	
//	//if (_qtIMContactManager) {
//	//	_qtIMContactManager->saveIMContacts();
//	//}
//
//	//saveProfile();
//
//	//return (ContactProfile &) _profile;	//VOXOX - JRT - 2009.05.09 TODO: VERY bad to upcast Profile to ContactProfile!
//}

void QtProfileDetails::saveContact() {

//	if (_ui->groupComboBox->currentText().isEmpty()) {
//
//		//VOXOX - CJC - 2009.06.10 
//		QtVoxMessageBox box(_profileDetailsWindow);
//		box.setWindowTitle("VoxOx  - No Group Selected");
//		box.setText(tr("A contact must have a group, please set a group."));
//		box.setStandardButtons(QMessageBox::Ok);
//		box.exec();
//
//	/*	QMessageBox::warning(_profileDetailsWindow,
//			tr("@product@ - No Group Selected"),
//			tr("A contact must have a group, please set a group."),
//			QMessageBox::NoButton,
//			QMessageBox::NoButton,
//			QMessageBox::Ok);*/
//
//		return;
//	}
//	
//	ContactProfile & contactProfile = fillAndGetContactProfile();
//
//	int index = _ui->groupComboBox->findText(_ui->groupComboBox->currentText());
//	QVariant groupIdTemp;
//	groupIdTemp = _ui->groupComboBox->itemData(index);
//
//	bool bIsGroupName	  = false;
//	std::string groupId   = "";
//	std::string groupName = std::string(_ui->groupComboBox->currentText().toUtf8().data());
//	//If the group does not exist
//	//VOXOX - JRT - 2009.05.06 - TODO: too much business logic.  Controller should just take new group name and handle the business logic.
//	if (groupIdTemp.isNull()) 
//	{
//		bIsGroupName = true;
//		groupId      = std::string(_ui->groupComboBox->currentText().toUtf8().data());
////		std::string groupName = std::string(_ui->groupComboBox->currentText().toUtf8().data());
////		ContactGroup contactGroup( groupName, EnumGroupType::GroupType_User);	//VOXOX - JRT - 2009.05.07 JRT-GRPS
////		_cUserProfile.getCContactList().addContactGroup( contactGroup );		//VOXOX - JRT - 2009.05.07 JRT-GRPS
////		groupId = QString::fromStdString(_cUserProfile.getCContactList().getContactGroupIdFromName(groupName));
//	}
//	else
//	{
//		bIsGroupName = false;
//		groupId      = groupIdTemp.toString().toStdString() ;
//	}
//
////	contactProfile.setGroupId(groupId.toString().toStdString());
////	contactProfile.changeUserGroup( groupId.toString().toStdString(), EnumGroupType::GroupType_User );	//VOXOX - JRT - 2009.05.06 JRT-GRPS
//	contactProfile.changeUserGroup( ContactGroup(groupName, EnumGroupType::GroupType_User) );	//VOXOX - JRT - 2009.05.06 JRT-GRPS
////	_cUserProfile.getCContactList().moveContactToGroup( contactProfile.getKey(), groupId, bIsGroupName ) ;
//
//
//
//	_profileDetailsWindow->accept();
}

void QtProfileDetails::saveUserProfile() {
	saveProfile();
	_cUserProfile.updateUserProfile();

	accept();
	windowClose(getKey());//VOXOX - CJC - 2009.07.09 
}

void QtProfileDetails::cancelButtonClicked() {
	close();
}

void QtProfileDetails::showWindow(){//VOXOX - CJC - 2009.07.09 
	if(isVisible()){
		activateWindow();
		raise();
	}else{
		showNormal();
	}
}

void QtProfileDetails::changeUserProfileAvatar() {

	//VOXOX CHANGE by Rolando - 2009.06.24 - Commented to implement task "For File Selector / Browse Windows, use OS standard"
	//QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	//Config & config = ConfigManager::getInstance().getCurrentConfig();
	//QString avatarsDir = QString::fromStdString(config.getResourcesDir()) + "pics/avatars";
	//QString startDir = QDesktopServices::storageLocation(QDesktopServices::PicturesLocation);
	//ImageSelector avatarSelector(_profileDetailsWindow);
	//avatarSelector.setWindowTitle(tr("Select your avatar"));
	////avatarSelector.setDefaultImagePath(QString::fromStdString(_profile.getIcon().getFilename()));//VOXOX CHANGE by Rolando - 2009.06.01  - needed when using PictureFlow class
	//avatarSelector.setCurrentDir(startDir);
	//
	//// Add Wengo avatar dir
	//int iconSize = ImageSelector::START_DIR_ICON_SIZE;
	//QPixmap wengoPixmap = QPixmap(avatarsDir + "/default-avatar.png");
	//wengoPixmap = wengoPixmap.scaled(iconSize, iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	//avatarSelector.addStartDirItem(avatarsDir, tr("@company@ Avatars"), wengoPixmap);
	//
	//// Add desktop dirs
	//DesktopService* service = DesktopService::getInstance();
	//QStringList startDirList = service->startFolderList();
	//Q_FOREACH(QString startDir, startDirList) {
	//	QString name = service->userFriendlyNameForPath(startDir);
	//	QPixmap pix = service->pixmapForPath(startDir, iconSize);
	//	avatarSelector.addStartDirItem(startDir, name, pix);
	//}
	//////
	//QApplication::restoreOverrideCursor();

	//if (!avatarSelector.exec()) {
	//	return;
	//}

	//setAvatarImage(avatarSelector.path());

	////setAvatarImage(avatarSelector.getCurrentImage());//VOXOX CHANGE by Rolando - 2009.06.01  - needed when using PictureFlow class
	//updateAvatarButton();

	//VOXOX CHANGE by Rolando - 2009.06.24 - End of commented sentences to implement task "For File Selector / Browse Windows, use OS standard"

	QString tmpString;//VOXOX CHANGE by Rolando - 2009.06.24 
	QString filters = "Images ";//VOXOX CHANGE by Rolando - 2009.06.24 

	QString startDir = QDesktopServices::storageLocation(QDesktopServices::PicturesLocation);//VOXOX CHANGE by Rolando - 2009.06.24 
	QList<QByteArray> qByteArrayList = QImageReader::supportedImageFormats();//VOXOX CHANGE by Rolando - 2009.06.24 

	for (int j = 0; j < qByteArrayList.size(); ++j) {//VOXOX CHANGE by Rolando - 2009.06.24 
		tmpString = "*." + qByteArrayList.at(j);//VOXOX CHANGE by Rolando - 2009.06.24 
		filters += tmpString + " ";  //VOXOX CHANGE by Rolando - 2009.06.24 
	}

	filters = filters.trimmed();//VOXOX CHANGE by Rolando - 2009.06.24 

	QString filePath = QFileDialog::getOpenFileName(this, tr("Select your avatar"),startDir, filters);//VOXOX CHANGE by Rolando - 2009.06.24 

	if(filePath == ""){//VOXOX CHANGE by Rolando - 2009.06.24 
		return;	//VOXOX CHANGE by Rolando - 2009.06.24 
	}
	
	setAvatarImage(filePath);//VOXOX CHANGE by Rolando - 2009.06.24 
	updateAvatarButton();//VOXOX CHANGE by Rolando - 2009.06.24 
	

#ifdef OS_WINDOWS
	BringWindowToTop(this->winId());
#endif
}

void QtProfileDetails::setAvatarImage( const QString& path, QImage& image )
{
	////Size of Wengo avatars
	//QSize size(96, 96);
	////Image is scaled to not save a big picture in userprofile.xml
	//image = image.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

	//QBuffer buffer;
	//buffer.open(QIODevice::ReadWrite);
	//image.save(&buffer, PNG_FORMAT);
	//buffer.close();

	//QByteArray byteArray = buffer.data();
	//std::string data(byteArray.data(), byteArray.size());

	//OWPicture picture = OWPicture::pictureFromData(data);
	//QFileInfo fileInfo(path);
	//picture.setFilename(fileInfo.fileName().toStdString());

	//_profile.setIcon(picture);
}

void QtProfileDetails::setAvatarImage(const QString& path) 
{
	/*QImage image(path);

	setAvatarImage( path, image );*/

	//Size of Wengo avatars
	//Image is scaled to not save a big picture in userprofile.xml
	QSize size(96, 96);//VOXOX - CJC - 2009.06.28 
	QImage image(path);
	image = image.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

	QBuffer buffer;
	buffer.open(QIODevice::ReadWrite);
	image.save(&buffer, PNG_FORMAT);
	buffer.close();

	QByteArray byteArray = buffer.data();
	std::string data(byteArray.data(), byteArray.size());

	OWPicture picture = OWPicture::pictureFromData(data);

//	QFileInfo fileInfo(path);
//	picture.setFilename(fileInfo.fileName().toStdString());
	picture.setFilename( path.toStdString() );	//VOXOX - JRT - 2009.08.14 - FullPath

	_profile.updateIcon(picture);		//VOXOX - JRT - 2009.08.17 
}

//VOXOX CHANGE by Rolando - 2009.06.01  - needed when using PictureFlow class
void QtProfileDetails::setAvatarImage(QImage image) 
{
	QString path = image.text();
	setAvatarImage( path, image );

	////Size of Wengo avatars
	//QSize size(96, 96);
	////Image is scaled to not save a big picture in userprofile.xml
	//image = image.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

	//QBuffer buffer;
	//buffer.open(QIODevice::ReadWrite);
	//image.save(&buffer, PNG_FORMAT);
	//buffer.close();

	//QByteArray byteArray = buffer.data();
	//std::string data(byteArray.data(), byteArray.size());

	//OWPicture picture = OWPicture::pictureFromData(data);
	//QFileInfo fileInfo(path);
	//picture.setFilename(fileInfo.fileName().toStdString());

	//_profile.setIcon(picture);
}

void QtProfileDetails::advancedButtonClicked() {

//	// save modifications before changing of mode
//	fillAndGetContactProfile();
//	////
//
//	if (_qtIMContactManager->getContactManagerMode() == QtIMContactManagerInterface::contactManagerModeSimple) {
//		//Simple mode -> advanced mode
//		_ui->advancedButton->setText(tr("<< Simple"));
//		
//		//QtAdvancedIMContactManager
//		QtAdvancedIMContactManager * qtIMContactManager =
//			new QtAdvancedIMContactManager((ContactProfile &) _profile, _cUserProfile, _profileDetailsWindow);
//
//		_ui->imStackedWidget->removeWidget(_qtIMContactManager->getWidget());
//		OWSAFE_DELETE(_qtIMContactManager);
//		
//		int index = _ui->imStackedWidget->addWidget(qtIMContactManager->getWidget());
//		_ui->imStackedWidget->setCurrentIndex(index);
//		_qtIMContactManager = qtIMContactManager;
//		SAFE_CONNECT(_qtIMContactManager, SIGNAL(contentHasChanged()), SLOT(updateSaveButtonState()));
//		
//	} else /*if (_qtIMContactManager->getContactManagerMode() == QtIMContactManagerInterface::contactManagerModeAdvanced)*/ {
//		//Advanced mode -> simple mode
//		_ui->advancedButton->setText(tr("Advanced >>"));
//		
//		//QtSimpleIMContactManager
//		QtSimpleIMContactManager * qtIMContactManager = 
//			new QtSimpleIMContactManager((ContactProfile &) _profile, _cUserProfile, _profileDetailsWindow);
//
//		_ui->imStackedWidget->removeWidget(_qtIMContactManager->getWidget());
//		OWSAFE_DELETE(_qtIMContactManager);
//		
//		int index = _ui->imStackedWidget->addWidget(qtIMContactManager->getWidget());
//		_ui->imStackedWidget->setCurrentIndex(index);
//		_qtIMContactManager = qtIMContactManager;
//		SAFE_CONNECT(_qtIMContactManager, SIGNAL(contentHasChanged()), SLOT(updateSaveButtonState()));
//	}
//}
//
}
void QtProfileDetails::websiteButtonClicked() {
//	/*String website = _ui->webLineEdit->text().toStdString();
//	if (!website.empty()) {
//		if (!website.contains("http://")) {
//			website = "http://" + website;
//		}
//		WebBrowser::openUrl(website);
//	}*/
}

void QtProfileDetails::emailButtonClicked() {
	/*std::string email = _ui->emailLineEdit->text().toStdString();
	if (!email.empty()) {
		WebBrowser::openUrl("mailto:" + email);
	}*/
}

void QtProfileDetails::setGroup(const QString & group) {
	//_ui->groupComboBox->setEditText(group);
}

void QtProfileDetails::setFirstName(const QString & firstName) {
	//_ui->firstNameLineEdit->setText(firstName);
}

void QtProfileDetails::setLastName(const QString & lastName) {
	//_ui->lastNameLineEdit->setText(lastName);
}

void QtProfileDetails::setCountry(const QString & country) {
//	_ui->countryComboBox->setEditText(country);
}

void QtProfileDetails::setCity(const QString & city) {
//	_ui->cityLineEdit->setText(city);
}

void QtProfileDetails::setState(const QString & state) {
//	_ui->stateLineEdit->setText(state);
}

void QtProfileDetails::setWebsite(const QString & website) {
//	_ui->webLineEdit->setText(website);
}

void QtProfileDetails::setHomePhone(const QString & homePhone) {
	//_ui->homePhoneLineEdit->setText(homePhone);
}

void QtProfileDetails::setWengoName(const QString & wengoName) {
	
	//_qtIMContactManager->setWengoName(wengoName);
}

void QtProfileDetails::setSipAddress(const QString & sipAddress) {
	//_qtIMContactManager->setSipAddress(sipAddress);
}

void QtProfileDetails::updateSaveButtonState() {

	/*if ((!_ui->mobilePhoneLineEdit->text().isEmpty()) ||
		(!_ui->homePhoneLineEdit->text().isEmpty()) ||
		_qtIMContactManager->couldBeSaved()
	   ) {
		_ui->saveButton->setEnabled(true);
	} else {
		_ui->saveButton->setEnabled(false);
	}*/
}

QWidget * QtProfileDetails::getWidget() const {
	return (QWidget*)this;
}

QString QtProfileDetails::getKey() const {
	return QtEnumWindowType::toString(getType());
}

QString QtProfileDetails::getDescription() const {
	return QString("VoxOx Profile Details");
}

int QtProfileDetails::getAllowedInstances() const{
	return 1;
}

int QtProfileDetails::getCurrentTab() const{
	return 0;
}

void QtProfileDetails::setCurrentTab(QString tabName){
	
}

