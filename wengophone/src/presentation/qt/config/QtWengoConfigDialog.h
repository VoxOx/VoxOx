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

#ifndef OWQTWENGOCONFIGDIALOG_H
#define OWQTWENGOCONFIGDIALOG_H

#include <util/NonCopyable.h>

#include <QtCore/QList>

#include <QtGui/QDialog>

#include <presentation/qt/QtVoxWindowInfo.h>

class CWengoPhone;
class QtISettings;
class QTimer;
class QShortcut;

namespace Ui { class WengoConfigDialog; }

/**
 * Main configuration window.
 *
 * @author Tanguy Krotoff
 */
class QtWengoConfigDialog : public QtVoxWindowInfo, NonCopyable {
	Q_OBJECT

public:
	//VOXOX - SEMR - 2009.05.14 MEMORY ISSUE: Config/Settings Window 
	// VOXOX CHANGE by ASV 05-15-2009: we need to have the QMainWindow from QtWengoPhone as the parent for this Window
	// in order to show all the menu items
	QtWengoConfigDialog(QWidget * parent, CWengoPhone & cWengoPhone);

	~QtWengoConfigDialog();

	virtual QWidget * getWidget() const;

	virtual QString getKey() const;

	virtual QString getDescription() const;

	virtual int getAllowedInstances() const;

	virtual int getCurrentTab() const ;

	virtual void setCurrentTab(QString tabName);

	virtual void showWindow();



public Q_SLOTS:

	void closeEvent();//VOXOX - CJC - 2009.07.09 

	void save();
	void saveAndClose();//VOXOX - CJC - 2009.07.03 ;

	void showGeneralPage();

	void showLanguagePage();
	
	void showAccountsPage();

	void showPhoneSettingsPage();							//VOXOX CHANGE by Rolando - 2009.06.11 
	void showPhoneSettingsPageEx( const QString& jsAction);	//VOXOX - JRT - 2009.07.02  

	//VOXOX - SEMR - 2009.05.27 Show appearance window
	void showAppearancePage();

	void showPrivacyPage();

	void showAudioPage();

	void showVideoPage();

	void showNotificationsPage();

	void showCallForwardPage();

	void showAdvancedPage();

	void showVoicemailPage();

	void showSecurityPage();

private Q_SLOTS:

	void showSettingPage(int row);

	void closeTimerSlot();

private:

	void showPage(const QString & pageName);
	
	#if defined(OS_MACOSX)
		QShortcut *_closeWindowShortCut;//VOXOX -ASV- 08-25-2009: we close the window when CMD + Shift + W is pressed on Mac
	#endif
	
	Ui::WengoConfigDialog * _ui;

	typedef QList<QtISettings *> SettingsList;

	SettingsList _settingsList;

	int			_phonePageId;

	QTimer * _closeTimer; //VOXOX - CJC - 2009.07.17 Timer that will give time to the webpages to save their data, and then it will notify the window manager to destroy the window

};

#endif	//OWQTWENGOCONFIGDIALOG_H
