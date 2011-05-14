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

#ifndef OWQtContactProfileWidgetWIDGET_H
#define OWQtContactProfileWidgetWIDGET_H

#include <QtGui/QDialog>
#include <presentation/qt/QtVoxWindowInfo.h>
class CWengoPhone;
class QShortcut;

//class QNetworkReply;	//VOXOX - JRT - 2009.07.31 

namespace Ui { class ContactProfileWidget; }
/**
 *
 * @author J R Theinert
 */
class QtContactProfileWidget :  public QtVoxWindowInfo
{
	Q_OBJECT
public:
	enum ActionId
	{
		Action_None		= 0,
		Action_Show		= 1,

		Action_Add		= 2,
		Action_Edit		= 3,
		Action_Delete	= 4,
//		Action_SetId	= 5,

		//TODO: Group Add, Edit, Delete?
	};

	QtContactProfileWidget( CWengoPhone* cWengoPhone, const QString contactId, ActionId action, QWidget * parent );

	virtual ~QtContactProfileWidget();

	void setAction( ActionId actionId, const QString& contactId );

	//Public so JSON object can access.
	QString	getContactId();			
	void	setJsonId( int jsId, int qtId );

	//Related to Window Manager
	virtual QWidget* getWidget() const;
	virtual QString getKey() const;
	virtual QString getDescription() const;
	virtual int getAllowedInstances() const;
	virtual int getCurrentTab() const ;
	virtual void setCurrentTab(QString  tabName);
	virtual void showWindow();

public Q_SLOTS:
	void closeEvent();//VOXOX - CJC - 2009.07.09 
	void initJavascript();

private Q_SLOTS:
	void documentCompleteSlot(bool);
	//VOXOX - JRT - 2009.08.03 - Temp code to address server-side issue with Content-Type
//	void replyFinished( QNetworkReply* );	//VOXOX - JRT - 2009.07.31 

Q_SIGNALS:

protected:
	void loadHtml();

	void clearAction();

	void addNewContact();
	void updateContactId( const QString& contactId );	//VOXOX - JRT - 2009.09.18
	void deleteContactId( const QString& contactId );	//VOXOX - JRT - 2009.09.18 

	bool isAddingNewContact()					{ return _actionId == Action_Add;		}
	bool isEdittingContact()						{ return _actionId == Action_Edit;		}
	bool isDeletingContact()						{ return _actionId == Action_Delete;	}

	//These invoke JavaScript in CM.
	void invokeLogin();			//VOXOX - JRT - 2009.09.18 
	void invokeAddContact();
	void invokeEditContact();
	void invokeDeleteContact();
	void invokeNewContactId( int jsId, int qtId );


private Q_SLOTS:


private:
	Ui::ContactProfileWidget * _ui;

	#if defined(OS_MACOSX)
		QShortcut *_closeWindowShortCut;//VOXOX -ASV- 08-25-2009: we close the window when CMD + Shift + W is pressed on Mac
	#endif
	
	CWengoPhone*	_cWengoPhone;
	QString			_contactId;
	ActionId		_actionId;

	static std::string _newContactIdentifier;
};

//======================================================================

class UserProfile;

class JSONContact : public QObject
{
	Q_OBJECT
public:
	JSONContact( QObject* parent, CWengoPhone*  cWengoPhone);

public Q_SLOTS:
	QString getInitialContact();
	QString getContactList();
	QString getContactGroups();
	QString getNetworks();	//VOXOX - JRT - 2009.09.08 - This is poorly named.  Use getAccounts() instead.
	QString getAccounts();	//VOXOX - JRT - 2009.09.24 


	QString getDataById( const int qtId );		//VOXOX - JRT - 2009.09.23 - Need for AddContact.
	QString getData( const QString& input );
	void	setData( const QString& input );

Q_SIGNALS:
	void signalGetData();
	void signalSetData();

protected:
	UserProfile& getUserProfile();

private:
	CWengoPhone*	_cWengoPhone;	//We just need UserProfile.
};

//=============================================================================
//VOXOX - JRT - 2009.09.17 - Using this old R&D code was preventing Flash from working.
//#include <QtWebKit/QWebPluginFactory>
//
//class JSONTestFactory : public QWebPluginFactory
//{
//	Q_OBJECT
//public:
//	JSONTestFactory( QObject* parent );
//	virtual ~JSONTestFactory();
//
//	QObject* create( const QString& mimeType, const QUrl& url, const QStringList & argumentNames, const QStringList & argumentValues ) const;
//	QList<QWebPluginFactory::Plugin> plugins () const;
//
//	void setWengoPhone( CWengoPhone*  val )		{ _cWengoPhone = val;	}
//
//protected:
//	QWebPluginFactory::MimeType		 _mimeType;
//	QList<QWebPluginFactory::Plugin> _plugins;
//
//	CWengoPhone*	_cWengoPhone;
//};

//===============================================================================

#endif //OWQtContactProfileWidget_H
