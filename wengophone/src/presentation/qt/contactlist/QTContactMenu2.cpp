/*
  VOXOX !!!!

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

#include "stdafx.h"	
#include "QtContactMenu2.h"
#include "contactList/QtEnumSortOption.h"		//VOXOX - JRT - 2009.05.03

#include <qtutil/SafeConnect.h>

#include <model/config/ConfigManager.h>
#include <model/config/Config.h>

//-----------------------------------------------------------------------------

QtContactMenu2::QtContactMenu2( QObject* parent, Ui::WengoPhoneWindow* ui )
{
	_ui = ui;
	setObjectName(QString::fromUtf8("menuContacts2"));
	setTitle( tr("&View") );
	create( parent );
}

//----------------------------------------------------------------------------

void QtContactMenu2::create( QObject* parent )
{
	//VOXOX - SEMR - 2009.05.19 NOT USE ANYMORE
	/*createAction( &_actionAddContact, this, tr("Add Contact..."), false, QtEnumSortOption::AddContact, ":/pics/iconbar/add_contact.png", QKeySequence(tr("Ctrl+D")) );
	addSeparator();*/

	//	createViewMenu( parent );
	// VOXOX CHANGE by ASV 05-12-2009: Ugly hack for showing something at this action. For some reason on the Mac the showEvent is not
	// triggered; this appears to be problem with the version of QT (4.4) I am using.
	// TODO: check if showEvent is triggered with QT 4.5 when we are able to switch to it on the Mac platform. 
#if defined(OS_MACOSX)
	createAction( &_actionViewUnavailable, this, tr("Show Offline Contacts"), true, QtEnumSortOption::ViewUnavailable, "", QKeySequence(tr("CTRL+Shift+O")) ); // VOXOX CHANGE by ASV 06-29-2009
#endif
#if defined OS_WINDOWS
	createAction( &_actionViewUnavailable, this, tr(""), false, QtEnumSortOption::ViewUnavailable, "", QKeySequence(tr("CTRL+Shift+O")) ); // VOXOX CHANGE by ASV 06-29-2009
#endif	
	// end VOXOX CHANGE by ASV

	syncShowOfflineContacts();//VOXOX CHANGE by ASV 06-30-2009: need this to sync _actionViewUnavailable
	
	addSeparator();

	createSortMenu( parent );

#if defined OS_WINDOWS	
	addSeparator(); // VOXOX CHANGE by ASV 06-29-2009
#endif

	addAction( _ui->actionAppearance );
	addSeparator();
	addAction( _ui->actionProfileBar );
	addAction( _ui->actionCallBar	 );
}

//-----------------------------------------------------------------------------

//void QtContactMenu2::createViewMenu( QObject* parent )
//{
//	_menuView = new QMenu( tr("View") );
//
//	createAction( &_actionViewUnavailable, _menuView, tr(""), false, QtEnumSortOption::ViewUnavailable, "" );
////	createAction( &_actionViewGroups,	   _menuView, tr(""), false, QtEnumSortOption::ViewGroups,		"" );
//
//	addMenu( _menuView );
//}

//-----------------------------------------------------------------------------

void QtContactMenu2::createSortMenu( QObject* parent ){
//	_menuSort = new QMenu( tr("Sort") );
//	QMenu* pMenu = _menuSort;

	//VOXOX - SEMR - 2009.06.01 Regrouping the View Menu
	_sortContactsBy = new QMenu( tr("Sort Contacts By") );

	createAction( &_actionSortAlpha,    _sortContactsBy, tr("Alphabetical\t Ctrl+Shift+1"),		 true, QtEnumSortOption::SortAlpha,    "", NULL/*QKeySequence(tr("Ctrl+Shift+1"))*/ );
	createAction( &_actionSortPresence, _sortContactsBy, tr("Presence\t Ctrl+Shift+2"),			 true, QtEnumSortOption::SortPresence, "", NULL/*QKeySequence(tr("Ctrl+Shift+2"))*/ );
	createAction( &_actionSortRandom,   _sortContactsBy, tr("Random\t Ctrl+Shift+3"),			 true, QtEnumSortOption::SortRandom,   "", NULL/*QKeySequence(tr("Ctrl+Shift+3"))*/ );

	_groupContactsBy = new QMenu( tr("Group Contacts By") );

	createAction( &_actionGroupUserGroup, _groupContactsBy, tr("Groups\t Ctrl+1"		   ), true, QtEnumSortOption::GroupByUserGroup, "", NULL/*QKeySequence(tr("Ctrl+1"))*/ );
	createAction( &_actionGroupNetwork,   _groupContactsBy, tr("Networks\t Ctrl+2"		   ), true, QtEnumSortOption::GroupByNetwork,   "", NULL/*QKeySequence(tr("Ctrl+2"))*/ );
	createAction( &_actionGroupMostComm,  _groupContactsBy, tr("Most Contacted\t Ctrl+3"   ), true, QtEnumSortOption::GroupByMostComm,  "", NULL/*QKeySequence(tr("Ctrl+3"))*/ );
	createAction( &_actionGroupNone,      _groupContactsBy, tr("None\t Ctrl+0"		       ), true, QtEnumSortOption::GroupByNone,      "", NULL/*QKeySequence(tr("Ctrl+0"))*/ );

	QMenu* pMenu = this;

	pMenu->addMenu(_sortContactsBy);
	pMenu->addMenu(_groupContactsBy);
		
	pMenu->addSeparator();

//	pMenu->addSeparator();

//	createGroupSortMenu( parent );

//	addMenu( pMenu );
}

//-----------------------------------------------------------------------------

//void QtContactMenu2::createGroupSortMenu( QObject* parent )
//{
//	_menuGroupSort = new QMenu( tr("Group Sort") );
//
//	createAction( &_actionGroupSortAlpha,  _menuGroupSort, tr("Alphabetically"), true, QtEnumSortOption::SortGroupAlpha,  "" );
//	createAction( &_actionGroupSortManual, _menuGroupSort, tr("Manual"        ), true, QtEnumSortOption::SortGroupManual, "" );
//
////	_menuSort->addMenu( _menuGroupSort );
//	this->addMenu( _menuGroupSort );
//}

//-----------------------------------------------------------------------------

void QtContactMenu2::updateUi()
{
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	QString text;
	
	//Show/Hide offline contacts.
	text = config.getShowOfflineContacts() ? tr("Show Online Contacts") : tr("Show All Contacts");
	_actionViewUnavailable->setText(text);
	
	//Show/Hide Groups
	//text = config.getShowGroups() ? tr("Hide Groups") : tr("Show Groups");
	//_actionViewGroups->setText(text);

	//Clear all checks
	setCheckAll( this, false, true );

	//Check proper sort option
	int nSort = config.getContactSort();
	setCheckContactSort( nSort );

	//Check proper grouping option
	int nGrouping = config.getContactGrouping();
	this->setCheckGrouping( nGrouping );

	//Disable unimplemented sort options
//	_actionGroupNetwork->setEnabled ( false );
	_actionGroupMostComm->setEnabled( false );

	//Group Sort
//	int nGroupSort = config.getGroupSort();
//	setCheckByData( _menuGroupSort, true, nGroupSort );

	//Disable unimplemented group sort options
//	_actionGroupSortManual->setEnabled( false );

	//VOXOX - JRT - 2009.05.27 
	_ui->actionCallBar->setChecked   ( config.getShowCallBar()	  );	
	_ui->actionProfileBar->setChecked( config.getShowProfileBar() );
}

//-----------------------------------------------------------------------------
	
void QtContactMenu2::setCheckContactSort( int nOption )
{
	switch( nOption )
	{

	case QtEnumSortOption::SortPresence:
		_actionSortPresence->setChecked( true );;
		break;

	case QtEnumSortOption::SortRandom:
		_actionSortRandom->setChecked( true );;
		break;

	case QtEnumSortOption::SortAlpha:
	default:
		_actionSortAlpha->setChecked( true );
		break;
	}

}

//----------------------------------------------------------------------------
	
void QtContactMenu2::setCheckGrouping( int nOption )
{
	switch( nOption )
	{
	case QtEnumSortOption::GroupByUserGroup:
		_actionGroupUserGroup->setChecked( true );
		break;

	case QtEnumSortOption::GroupByNetwork:
		_actionGroupNetwork->setChecked( true );;
		break;

	case QtEnumSortOption::GroupByMostComm:
		_actionGroupMostComm->setChecked( true );;
		break;

	case QtEnumSortOption::GroupByNone:
		_actionGroupNone->setChecked( true );;
		break;

	default:
		assert(false);		//New type?
		_actionGroupUserGroup->setChecked( true );
		break;
	}

}

//----------------------------------------------------------------------------

void QtContactMenu2::showEvent ( QShowEvent* pEvent )  
{
	updateUi();		
	QWidget::showEvent( pEvent );
}

//-----------------------------------------------------------------------------
//VOXOX - JRT - 2009.05.03  - add to VoxMenuBase class (when we create it).
//VOXOX - SEMR - 2009.05.19 Add shortcut to action
void QtContactMenu2::createAction( QAction** ppAction, QMenu* pMenu, const QString& text, bool bCheckable, int data, const char* iconPath, const QKeySequence & shortcut ){
	*ppAction = new QAction( text, NULL );	//, parent	);

	(*ppAction)->setCheckable( bCheckable );
	(*ppAction)->setData(qVariantFromValue(data) );

	if ( iconPath && (strlen( iconPath ) > 0 ) )
	{
		QIcon icon1;
		icon1.addPixmap(QPixmap( QString::fromUtf8(iconPath)), QIcon::Normal, QIcon::Off);
		(*ppAction)->setIcon(icon1);
	}
	if (shortcut){
		(*ppAction)->setShortcut(shortcut);
	}
	
	pMenu->addAction( *ppAction );

}

//-----------------------------------------------------------------------------
//VOXOX - JRT - 2009.05.03  - add to VoxMenuBase class (when we create it).
void QtContactMenu2::setCheckAll( QMenu* pMenu, bool bCheck, bool recurse )
{
	QList<QAction*> actions = pMenu->actions();

	for ( int x = 0; x < actions.size(); x++ ) 
	{
		actions.at(x)->setChecked( bCheck );

		if ( recurse )
		{
			QMenu* subMenu = actions.at(x)->menu();

			if ( subMenu )
			{
				setCheckAll(subMenu, bCheck, recurse );
			}
		}
	}
}

//-----------------------------------------------------------------------------
//VOXOX - JRT - 2009.05.03  - add to VoxMenuBase class (when we create it).
void QtContactMenu2::setCheckByData( QMenu* pMenu, bool bCheck, int tgtData )
{
	int				data    = 0;
	QAction*		pAction = NULL;
	QList<QAction*> actions = pMenu->actions();

	for ( int x = 0; x < actions.size(); x++ ) 
	{
		pAction = actions.at(x);		//Shorthand

		data = pAction->data().value<int>();

		if ( data == tgtData )
		{
			pAction->setChecked( bCheck );
			break;
		}
	}
}

//-----------------------------------------------------------------------------
//VOXOX - JRT - 2009.05.03  - add to VoxMenuBase class (when we create it).
void QtContactMenu2::addIcon( QMenu* pMenu, const char* iconPath )
{
	QIcon icon1;
	icon1.addPixmap(QPixmap(QString::fromUtf8( iconPath)), QIcon::Normal, QIcon::Off);
	pMenu->setIcon(icon1);
}

//-----------------------------------------------------------------------------
//VOXOX CHANGE by ASV 06-30-2009: need this to sync _actionViewUnavailable
void QtContactMenu2::syncShowOfflineContacts() {
#if defined(OS_MACOSX)
	Config & config = ConfigManager::getInstance().getCurrentConfig();
	if (config.getShowOfflineContacts()) {
		_actionViewUnavailable->setChecked(true);
	}
	else {
		_actionViewUnavailable->setChecked(false);
	}
#endif	
}