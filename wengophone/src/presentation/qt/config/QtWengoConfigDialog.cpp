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
#include "QtWengoConfigDialog.h"

#include "ui_WengoConfigDialog.h"

#include "QtPhoneSettings.h"
#include "QtTextSettings.h"
#include "QtMyAccountSettings.h"
#include "QtEventsSettings.h"
#include "QtNotificationSettings.h"
#include "QtGeneralSettings.h"
#include "QtAccountSettings.h"
#include "QtPrivacySettings.h"
#include "QtAppearanceSettings.h"
#include "QtAudioSettings.h"
#include "QtVideoSettings.h"
#include "QtAdvancedSettings.h"
#include "QtCallForwardSettings.h"
#include "QtLanguagesSettings.h"
#include "QtVoicemailSettings.h"
#include "QtSecuritySettings.h"

#include <control/CWengoPhone.h>
#include <control/profile/CUserProfileHandler.h>

#include <util/Logger.h>
#include <util/SafeDelete.h>
#include <qtutil/SafeConnect.h>
#include <qtutil/WidgetUtils.h>

#include <QtGui/QtGui>

#include <QtCore/QPointer>

#include <presentation/qt/QtEnumWindowType.h>//VOXOX - CJC - 2009.07.09 
// VOXOX CHANGE by ASV 05-15-2009: we need to have the QMainWindow from QtWengoPhone as the parent for this Window
// in order to show all the menu items
//VOXOX CHANGE by ASV 07-03-2009: Added window modality to prevent this window from being on top of the main window
#include <qtutil/CloseEventFilter.h>//VOXOX - CJC - 2009.07.09 
QtWengoConfigDialog::QtWengoConfigDialog(QWidget *parent, CWengoPhone & cWengoPhone)
	: QtVoxWindowInfo(parent,QtEnumWindowType::ConfigWindow, Qt::Window | Qt::WindowTitleHint) {//VOXOX - CJC - 2009.06.28 Remove ? Menu

	_ui = new Ui::WengoConfigDialog();
	_ui->setupUi(this);
	_ui->retranslateUi(this);



	_settingsList += new QtGeneralSettings(cWengoPhone, 0);
	if (cWengoPhone.getCUserProfileHandler().getCUserProfile()) 
	{
		_settingsList += new QtAccountSettings(cWengoPhone, 0);
	}
	_settingsList += new QtAppearanceSettings(cWengoPhone,0);
	_settingsList += new QtPhoneSettings(cWengoPhone, 0);
	_settingsList += new QtTextSettings(cWengoPhone, 0);
	_phonePageId = _settingsList.size() - 1;			//VOXOX - JRT - 2009.07.02 
	_settingsList += new QtAudioSettings(cWengoPhone, 0);
	_settingsList += new QtVideoSettings(cWengoPhone, 0);
	//_settingsList += new QtMyAccountSettings(cWengoPhone, 0);
	_settingsList += new QtEventsSettings(0);
	_settingsList += new QtNotificationSettings(cWengoPhone,0);
//#ifndef DISABLE_CALL_FORWARD
//	_settingsList += new QtCallForwardSettings(cWengoPhone, 0);
//#endif
	
//#ifndef DISABLE_VOICE_MAIL
//	_settingsList += new QtVoicemailSettings(cWengoPhone, 0);
//#endif
//	_settingsList += new QtSecuritySettings(cWengoPhone, 0);
//	
//	_settingsList += new QtAdvancedSettings(0);

	//stackedWidget

	Q_FOREACH(QtISettings* settings, _settingsList) {
		QString iconName = QString(":pics/config/%1.png").arg(settings->getIconName());
		QIcon icon = QPixmap(iconName);
		new QListWidgetItem(icon, settings->getName(), _ui->listWidget);
		QWidget* widget = settings->getWidget();
		widget->layout()->setMargin(0);
		_ui->stackedWidget->addWidget(widget);
	}
	_ui->listWidget->setCurrentRow(0);
	showSettingPage(0);

	//int minWidth = WidgetUtils::computeListViewMinimumWidth(_ui->listWidget);
	_ui->listWidget->setFixedWidth(117);
	//VOXOX - CJC - 2009.06.25 
	_ui->lblDescription->setVisible(false);	

	_closeTimer = new QTimer();

	
	SAFE_CONNECT(_closeTimer, SIGNAL(timeout()), SLOT(closeTimerSlot()));


	CloseEventFilter * closeEventFilter = new CloseEventFilter(this, SLOT(closeEvent()));
	this->installEventFilter(closeEventFilter);


	//listWidget
	SAFE_CONNECT(_ui->listWidget, SIGNAL(currentRowChanged(int)), SLOT(showSettingPage(int)));

	//saveButton
	SAFE_CONNECT(_ui->saveButton, SIGNAL(clicked()), SLOT(save()));
	SAFE_CONNECT(_ui->cancelButton, SIGNAL(clicked()), SLOT(closeEvent()));
	SAFE_CONNECT(_ui->okButton, SIGNAL(clicked()), SLOT(saveAndClose()));
		
		//VOXOX -ASV- 08-25-2009: we close the window when CMD + Shift + W is pressed on Mac
	#if defined(OS_MACOSX)
		_closeWindowShortCut = new QShortcut(QKeySequence("Ctrl+Shift+W"), this);
		SAFE_CONNECT(_closeWindowShortCut, SIGNAL(activated()), SLOT(saveAndClose()));
	#endif	
}

QtWengoConfigDialog::~QtWengoConfigDialog() {
	//VOXOX - SEMR - 2009.05.13 MEMORY ISSUE: Config/Settings Window
	if(_closeTimer){
		if(_closeTimer->isActive()){
			_closeTimer->stop();
		}
		OWSAFE_DELETE(_closeTimer);
	}
	
	#if defined(OS_MACOSX)
		OWSAFE_DELETE(_closeWindowShortCut);//VOXOX -ASV- 08-25-2009: we close the window when CMD + Shift + W is pressed on Mac
	#endif
	
	OWSAFE_DELETE(_ui);
	
	//delete _ui;
}



void QtWengoConfigDialog::showSettingPage(int row) {
	if (row<0 || row >= _settingsList.size()) {
		return;
	}
	QtISettings* settings = _settingsList.at(row);
	_ui->titleLabel->setText(settings->getTitle());
	
	//VOXOX CHANGE by ASV 06-16-2009: The default appearance on Mac for the settings window
	//is to display the name of the selected tab as the window title.
	QString screenTitle = settings->getTitle().remove("Settings");
	setWindowTitle(screenTitle);
	
	_ui->titleIcon->setPixmap(QPixmap(QString(":pics/config/titleIcons/%1.png").arg(settings->getIconName())));
	_ui->lblDescription->setText(settings->getDescription());
	QWidget* widget = settings->getWidget();
	_ui->stackedWidget->setCurrentWidget(widget);
	settings->postInitialize();
}

void QtWengoConfigDialog::save() {
	Q_FOREACH(QtISettings* settings, _settingsList) {
		settings->saveConfig();
	}
}

void QtWengoConfigDialog::closeEvent() {//VOXOX - CJC - 2009.07.09 
	windowClose(getKey());
}

void QtWengoConfigDialog::closeTimerSlot() {//VOXOX - CJC - 2009.07.17 
	if(!isVisible()){
		windowClose(getKey());
	}
}

void QtWengoConfigDialog::showWindow(){//VOXOX - CJC - 2009.07.09 
	if(_closeTimer->isActive()){
		_closeTimer->stop();
	}
	if(isVisible()){
		activateWindow();
		raise();
	}else{
		showNormal();
	}
}

void QtWengoConfigDialog::saveAndClose() {
	save();
	accept();
	_closeTimer->start(10000);
	//windowClose(getKey());
}

void QtWengoConfigDialog::showPage(const QString & pageName) {
	int count = _settingsList.count();
	QString tmpName;
	for (int row = 0; row < count; ++row) {
		if (_settingsList[row]->getName() == pageName) {
			_ui->listWidget->setCurrentRow(row);
			return;
		}
	}
	LOG_WARN("unknown page name=" + pageName.toStdString());
}

void QtWengoConfigDialog::showGeneralPage() {
	showPage(tr("General"));
}

void QtWengoConfigDialog::showLanguagePage() {
	showPage(tr("Language"));
}

void QtWengoConfigDialog::showAccountsPage() {
	showPage(tr("Networks"));
}
//VOXOX - SEMR - 2009.06.01 show appearance page
void QtWengoConfigDialog::showAppearancePage(){
	showPage(tr("Appearance"));
}

void QtWengoConfigDialog::showPrivacyPage() {
	showPage(tr("Privacy"));
}

//VOXOX CHANGE by Rolando - 2009.06.11 
void QtWengoConfigDialog::showPhoneSettingsPage() {
	showPage(tr("Phone"));
	resize(762,680);//VOXOX CHANGE by Rolando - 2009.06.23 
} 

void QtWengoConfigDialog::showPhoneSettingsPageEx( const QString& jsAction)		//VOXOX - JRT - 2009.07.02 
{
	QtPhoneSettings* page = (QtPhoneSettings*)_settingsList[_phonePageId];
	page->setJsAction( jsAction );
	showPhoneSettingsPage();
}

void QtWengoConfigDialog::showAudioPage() {
	showPage(tr("Audio"));
}

void QtWengoConfigDialog::showVideoPage() {
	showPage(tr("Video"));
}

void QtWengoConfigDialog::showNotificationsPage() {
	showPage(tr("Notifications"));
}

void QtWengoConfigDialog::showCallForwardPage() {
	showPage(tr("Call Forward"));
}

void QtWengoConfigDialog::showAdvancedPage() {
	showPage(tr("Advanced"));
}

void QtWengoConfigDialog::showVoicemailPage() {
	showPage(tr("Voicemail"));
}

void QtWengoConfigDialog::showSecurityPage() {
	showPage(tr("security"));
}

QWidget * QtWengoConfigDialog::getWidget() const {
	return (QWidget*)this;
}

QString QtWengoConfigDialog::getKey() const {
	return QtEnumWindowType::toString(getType());
}

QString QtWengoConfigDialog::getDescription() const {
	return QString("VoxOx Config");
}

int QtWengoConfigDialog::getAllowedInstances() const{
	return 1;
}

int QtWengoConfigDialog::getCurrentTab() const{
	return _ui->listWidget->currentIndex().row();
}

void QtWengoConfigDialog::setCurrentTab(QString tabName){
	showPage(tabName);
}


