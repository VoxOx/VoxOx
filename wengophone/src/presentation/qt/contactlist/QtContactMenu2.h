/*
  VOXOX !!!

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

#ifndef VOX_CONTACT_MENU2_H
#define VOX_CONTACT_MENU2_H

#include "ui_WengoPhoneWindow.h"	//VOXOX - JRT - 2009.05.27 

#include <QtCore/QObject>
#include <QtGui/QMenu.h>
//#include <util/NonCopyable.h>
//#include <model/webservices/url/WsUrl.h>

class QAction;
class QMenu;
class QWidget;

class QtContactMenu2 : public QMenu
{
public:
	QtContactMenu2( QObject* parent, Ui::WengoPhoneWindow* ui );

protected:
	void create( QObject* parent );
	void updateUi();

	void createViewMenu( QObject* parent );
	void createSortMenu( QObject* parent );
	void createGroupSortMenu( QObject* parent );

	void showEvent( QShowEvent* pEvent );	//Override QWidget

protected:
	void setCheckContactSort( int nOption );
	void setCheckGrouping   ( int nOption );

	//VOXOX - JRT - 2009.05.05 - TODO: move to VoxMenuBase class
	void createAction  ( QAction** ppAction, QMenu* pMenu, const QString& text, bool bCheckable, int data, const char* iconPath, const QKeySequence & shortcut );
	void setCheckAll   ( QMenu* pMenu, bool bCheck, bool recurse );
	void setCheckByData( QMenu* pMenu, bool bCheck, int tgtData  );
	void addIcon	   ( QMenu* pMenu, const char* iconPath );

private:

	void syncShowOfflineContacts(); //VOXOX CHANGE by ASV 06-30-2009: need this to sync _actionViewUnavailable
	
//	QMenu*	_menuView;
//	QMenu*	_menuSort;
//	QMenu*	_menuGroupSort;

	//VOXOX - SEMR - 2009.06.01 New submenus on View Menu
	QMenu * _groupContactsBy;
	QMenu * _sortContactsBy;

	//Contact view
	QAction* _actionViewUnavailable;
//	QAction* _actionViewGroups;

	//Contact Sort
	QAction* _actionSortAlpha;
	QAction* _actionSortPresence;
	QAction* _actionSortRandom;

	//Groupings
	QAction* _actionGroupUserGroup;
	QAction* _actionGroupNetwork;
	QAction* _actionGroupMostComm;
	QAction* _actionGroupNone;

	//VOXOX - SEMR - 2009.06.01 Action to hide call bar and profile bar
	QAction* _actionShowCallBar;
	QAction* _actionShowProfileBar;


	//Goup Sort
//	QAction* _actionGroupSortAlpha;
//	QAction* _actionGroupSortManual;

	//Separate actions
	QAction* _actionAddContact;

	Ui::WengoPhoneWindow* _ui;
};

#endif	//VOX_CONTACT_MENU2_H
