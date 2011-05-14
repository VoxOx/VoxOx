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
* @date 2009.07.09
*/


#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtVoxWindowManager.h"
#include "QtWengoPhone.h"

#include <presentation/qt/config/QtWengoConfigDialog.h>
#include <presentation/qt/wizard/QtWizardDialog.h>			//VOXOX - CJC - 2009.06.01 
#include <presentation/qt/chat/QtContactProfileWidget.h>	//VOXOX - CJC - 2009.07.09 Shouldn't this class be somewhere else????
#include <presentation/qt/profile/QtProfileDetails.h>

#include <control/CWengoPhone.h>
#include <control/profile/CUserProfile.h>
#include <control/profile/CUserProfileHandler.h>

#include <qtutil/SafeConnect.h>
#include <util/SafeDelete.h>

QtVoxWindowManager::QtVoxWindowManager(QtWengoPhone & qtWengoPhone, QWidget * parent)
	: QObject(parent),
	_qtWengoPhone(qtWengoPhone)
{
}

QtVoxWindowManager::~QtVoxWindowManager() {
	closeAllWindows();
}


void QtVoxWindowManager::retranslateUi() {
	
}

void QtVoxWindowManager::closeAllWindows(){

	ActiveWindowList::iterator iter    = _activeWindowList.begin();
	ActiveWindowList::iterator iterEnd = _activeWindowList.end();

	while (iter != iterEnd)
	{

		OWSAFE_DELETE(iter->second);
		_activeWindowList.erase(iter);
		iter  = _activeWindowList.begin();
	}	

}

void QtVoxWindowManager::showAllWindows(){

	ActiveWindowList::const_iterator iter    = _activeWindowList.begin();
	ActiveWindowList::const_iterator iterEnd = _activeWindowList.end();

	while (iter != iterEnd)
	{
		iter->second->showWindow();
		
		++iter;
	}	

}



void QtVoxWindowManager::closeWindow(QString id){
		ActiveWindowList::iterator imIter = _activeWindowList.find(id);
		if (imIter != _activeWindowList.end()) {
			OWSAFE_DELETE(imIter->second);
			_activeWindowList.erase(imIter);
		}
}

//VOXOX -ASV- 07-13-2009
void QtVoxWindowManager::closeActiveWindow() {
	ActiveWindowList::iterator iter    = _activeWindowList.begin();
	ActiveWindowList::iterator iterEnd = _activeWindowList.end();
	
	while (iter != iterEnd)
	{	
		if (iter->second->isActiveWindow()) {
				iter->second->close();
			break;
		}
		iter++;
	}
}

void QtVoxWindowManager::showConfigWindow(QString pageName){
	if(!checkIfExist(QtEnumWindowType::ConfigWindow)){
		QtWengoConfigDialog * settings = new QtWengoConfigDialog(getDefaultParent(), _qtWengoPhone.getCWengoPhone());
		SAFE_CONNECT(settings, SIGNAL(windowClose(QString )), SLOT(closeWindow(QString )));
		if(pageName!=""){
			settings->setCurrentTab(pageName);
		}
		settings->showWindow();	
		_activeWindowList[settings->getKey()] = settings;
	}else{
		QString id = QtEnumWindowType::toString(QtEnumWindowType::ConfigWindow);
		QtVoxWindowInfo * info = getWindow(id);
		if(info!=NULL){
			info->setCurrentTab(pageName);
			info->showWindow();
		}

	}
}

void QtVoxWindowManager::showWizardWindow(){
	if(!checkIfExist(QtEnumWindowType::WizardWindow)){
		QtWizardDialog * wizard = new QtWizardDialog(getDefaultParent(), _qtWengoPhone.getCWengoPhone());
		SAFE_CONNECT(wizard, SIGNAL(windowClose(QString )), SLOT(closeWindow(QString )));
		wizard->showWindow();	
		_activeWindowList[wizard->getKey()] = wizard;
	}else{
		QString id = QtEnumWindowType::toString(QtEnumWindowType::WizardWindow);
		QtVoxWindowInfo * info = getWindow(id);
		if(info!=NULL){
			info->showWindow();
		}
	}
}

void QtVoxWindowManager::showProfileWindow(){
	if(!checkIfExist(QtEnumWindowType::ProfileWindow)){
		if (_qtWengoPhone.getCWengoPhone().getCUserProfileHandler().getCUserProfile()){
			QtProfileDetails * qtProfileDetails   = new QtProfileDetails(*_qtWengoPhone.getCWengoPhone().getCUserProfileHandler().getCUserProfile(),
				_qtWengoPhone.getCWengoPhone().getCUserProfileHandler().getCUserProfile()->getUserProfile(),
				getDefaultParent(), tr("Edit My Profile"));

			SAFE_CONNECT(qtProfileDetails, SIGNAL(windowClose(QString )), SLOT(closeWindow(QString )));
			qtProfileDetails->showWindow();	
			_activeWindowList[qtProfileDetails->getKey()] = qtProfileDetails;
		}
	}else{
		QString id = QtEnumWindowType::toString(QtEnumWindowType::ProfileWindow);
		QtVoxWindowInfo * info = getWindow(id);
		if(info!=NULL){
			info->showWindow();
		}
	}
}

void QtVoxWindowManager::showContactManagerWindow(QString  contactId, QtContactProfileWidget::ActionId actionId )
{
	if(!checkIfExist(QtEnumWindowType::ContactManagerWindow))
	{
		//We don't want to open CM if we are deleting.
		if ( actionId != QtContactProfileWidget::Action_Delete )	//VOXOX - JRT - 2009.09.19 
		{
			QtContactProfileWidget *dlg = new QtContactProfileWidget(&_qtWengoPhone.getCWengoPhone(), contactId, actionId, getDefaultParent() );		

			SAFE_CONNECT(dlg, SIGNAL(windowClose(QString )), SLOT(closeWindow(QString )));
			dlg->showWindow();	

			_activeWindowList[dlg->getKey()] = dlg;
		}
	}
	else
	{
		QString id = QtEnumWindowType::toString(QtEnumWindowType::ContactManagerWindow);
		QtVoxWindowInfo * info = getWindow(id);
		if(info!=NULL)
		{
			info->showWindow();

			((QtContactProfileWidget*)info)->setAction( actionId, contactId );		//VOXOX - JRT - 2009.09.19 
		}
	}
}

//VOXOX - JRT - 2009.09.23 - I hate this hack, but since ContactManager has grown to much more than the original window,
//							 I need to do this for now.
void QtVoxWindowManager::notifyContactManagerContactAdded( const std::string& contactId, int qtId )	//VOXOX - JRT - 2009.09.23 
{
	//We only do this if CM is open.
	if ( checkIfExist( QtEnumWindowType::ContactManagerWindow))
	{
		QString id = QtEnumWindowType::toString(QtEnumWindowType::ContactManagerWindow);
		QtVoxWindowInfo * info = getWindow(id);

		if ( info )
		{
			info->showWindow();

			((QtContactProfileWidget*)info)->setJsonId( 0, qtId );
		}
	}
}




// VOXOX CHANGE by ASV 07-08-2009: added to clean up the code a little
QWidget *QtVoxWindowManager::getDefaultParent()
{
	// VOXOX CHANGE by ASV 05-15-2009: we need to have the QMainWindow from QtWengoPhone as the parent for this Window
	// in order to show all the menu items
	QWidget *parent = NULL;
#if defined(OS_MACOSX)
	parent = _qtWengoPhone.getWidget();
#endif
	return parent;
}

QtVoxWindowInfo * QtVoxWindowManager::getWindow(QString id){

	ActiveWindowList::const_iterator imIter = _activeWindowList.find(id);
		if (imIter != _activeWindowList.end()) {
			return imIter->second;
		}
	return NULL;
}

bool QtVoxWindowManager::checkIfExist(QtEnumWindowType::Type type){

	ActiveWindowList::const_iterator iter    = _activeWindowList.begin();
	ActiveWindowList::const_iterator iterEnd = _activeWindowList.end();

	while (iter != iterEnd)
	{
		if(iter->second->getType() == type){
			if(iter->second->getAllowedInstances()==1){
				return true;
			}
		}
		++iter;
	}	
	return false;
}


		
				