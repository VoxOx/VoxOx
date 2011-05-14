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
* @date 2009.06.01
*/


#ifndef OWQTWIZARDDIALOG_H
#define OWQTWIZARDDIALOG_H

#include <util/NonCopyable.h>

#include <QtCore/QList>

#include <QtGui/QDialog>

#include <presentation/qt/QtVoxWindowInfo.h>
#include <imwrapper/QtEnumIMProtocol.h>

class CWengoPhone;
class QtIWizard;

namespace Ui { class WizardDialog; }

/**
 * Main wizard window.
 *
 * @author CJC
 */
class QtWizardDialog : public QtVoxWindowInfo, NonCopyable {
	Q_OBJECT

public:
	
	QtWizardDialog(QWidget * parent, CWengoPhone & cWengoPhone);

	~QtWizardDialog();

	virtual QWidget * getWidget() const;

	virtual QString getKey() const;

	virtual QString getDescription() const;

	virtual int getAllowedInstances() const;

	virtual int getCurrentTab() const ;

	virtual void setCurrentTab(QString tabName);

	virtual void showWindow();


public Q_SLOTS:

	void closeEvent();			//VOXOX - CJC - 2009.07.09 
	void close();				//VOXOX - JRT - 2009.08.20 

	void save();
	void saveShowOnStartUp();	//VOXOX - JRT - 2009.08.20 

	void addNetworksPages();

	void cleanNetworksPages();

	/*void showGeneralPage();

	void showLanguagePage();

	void showAccountsPage();

	void showPrivacyPage();

	void showAudioPage();

	void showVideoPage();

	void showNotificationsPage();

	void showCallForwardPage();

	void showAdvancedPage();

	void showVoicemailPage();

	void showSecurityPage();*/

private Q_SLOTS:

	void showWizardPage(int row);

	void continuePage();
	void previewsPage();

private:

	void showPage(const QString & pageName);
	void addNetworkPageIfNeeded( bool shouldAdd, QtEnumIMProtocol::IMProtocol qtProtocol, int& indexToAdd );

	#if defined(OS_MACOSX)
		QShortcut *_closeWindowShortCut;//VOXOX -ASV- 08-25-2009: we close the window when CMD + Shift + W is pressed on Mac
	#endif
	
	Ui::WizardDialog * _ui;

	typedef QList<QtIWizard *> WizardList;

	WizardList _wizardList;

	int _currentWizardPage;

	CWengoPhone & _cWengoPhone;
	
};

#endif	//OWQtWizardDialog_H
