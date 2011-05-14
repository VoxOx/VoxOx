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
* @author Chris Jimenez C 
* @date 2009.07.09
*/


#ifndef OWQTVOXWINDOWMANAGER_H
#define OWQTVOXWINDOWMANAGER_H

#include <QtCore/QObject>
#include <QtCore/QMap>

#include <util/NonCopyable.h>
#include "QtVoxWindowInfo.h"
#include "chat/QtContactProfileWidget.h"	//VOXOX - JRT - 2009.09.19 
#include <presentation/qt/QtEnumWindowType.h>//VOXOX - CJC - 2009.07.09 

class QtWengoPhone;
class CWengoPhone;

class QAction;
class QMenu;
class QWidget;


namespace Ui { class WengoPhoneWindow; }

/**
 * Tool bar and menu actions from the main window.
 *
 * @author Tanguy Krotoff
 */
class QtVoxWindowManager : public QObject, NonCopyable {
	Q_OBJECT
public:

	QtVoxWindowManager(QtWengoPhone & qtWengoPhone, QWidget * parent);

	~QtVoxWindowManager();


	/**
	 * Makes sure the toggle actions (like show/hide hidden contacts and
	 * show/hide groups) are correctly translated
	 */
	void retranslateUi();

	void showConfigWindow(QString pageName);

	void showWizardWindow();

	void showProfileWindow();

	void showContactManagerWindow(QString contactId, QtContactProfileWidget::ActionId actionId );

	void closeAllWindows();

	void showAllWindows();

	void notifyContactManagerContactAdded( const std::string& contactId, int qtId );	//VOXOX - JRT - 2009.09.23 
	

	
Q_SIGNALS:
	

public Q_SLOTS:
	void closeActiveWindow();//VOXOX -ASV- 07-13-2009
	

private Q_SLOTS:

	void closeWindow(QString id);
	

private:

	QtWengoPhone & _qtWengoPhone;

	bool checkIfExist(QtEnumWindowType::Type type);

	QtVoxWindowInfo * getWindow(QString id);

	QWidget * getDefaultParent();


	typedef std::map<QString,QtVoxWindowInfo *> ActiveWindowList;//VOXOX CHANGE Rolando 03-20-09
	ActiveWindowList _activeWindowList;//VOXOX CHANGE Rolando 03-20-09
	
};

#endif	
