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

#ifndef QTCONTACTWIDGET_H
#define QTCONTACTWIDGET_H

#include <model/contactlist/ContactProfile.h>

#include <QtGui/QWidget>
#include <QtCore/QString>
#include <qtutil/ImageButton.h>//VOXOX CHANGE by Rolando - 2009.10.22

#include <presentation/qt/contactlist/QtContactNetworkMenu.h>//VOXOX CHANGE by Rolando - 2009.10.27 


class CUserProfile;		//VOXOX - JRT - 2009.04.15 
class CContact;
class Contact;			//VOXOX - JRT - 2009.08.10 
class CWengoPhone;
class QtContactNetworkMenu;//VOXOX CHANGE by Rolando - 2009.10.27 

//class QtContactManager;

class QLabel;
class QPushButton;
class QResizeEvent;
class QRect;
class QFont;
class QMenu;

namespace Ui { class ContactWidget; }

/**
 * Qt user widget.
 * Represents a contact info in the contact list.
 *
 * @author Mr K
 * @author Mathieu Stute
 * @author Philippe Bernery
 */
class QtContactWidget : public QWidget {
	Q_OBJECT
public:

	QtContactWidget(const std::string & contactId, const std::string& qtContactKey, 
//					CWengoPhone & cWengoPhone, QtContactManager * qtContactManager, QWidget * parent);	//VOXOX - JRT - 2009.08.10 
					CWengoPhone & cWengoPhone, QWidget * parent);	//VOXOX - JRT - 2009.10.15 

	~QtContactWidget();

	void    setText(const QString & text)		{ _text = text;	}
	QString text() const						{ return _text; }

	void updateButtons();
	virtual void resizeEvent(QResizeEvent* event);

Q_SIGNALS:

	/**
	 * Emitted when a the contact icon has been clicked and so the user wants to edit the contact.
	 */
	void editContact(QString contact);

private Q_SLOTS:
	void callButtonClicked();					//VOXOX - CJC - 2009.06.11 
	void callButtonClickAndHolded();
	void callNumberFromMenu(QAction * action);	//VOXOX - CJC - 2009.06.11 

	void faxButtonClicked();					//VOXOX - CJC - 2009.06.23 
	void faxButtonClickAndHolded();				//VOXOX - CJC - 2009.06.23 
	void faxNumberFromMenu(QAction * action); //VOXOX - CJC - 2009.06.23 

	void smsButtonClicked();
	void smsButtonClickAndHolded();				//VOXOX - CJC - 2009.06.14 
	void sendSMSFromMenu(QAction * action);		//VOXOX - CJC - 2009.06.14 
	
	void emailButtonClicked();
	void emailButtonClickAndHolded();			//VOXOX - CJC - 2009.06.11 
	void sendEmailFromMenu(QAction * action);	//VOXOX - CJC - 2009.06.11 

	//Add contact profile element
	QString addNumber();						//VOXOX - CJC - 2009.06.11 
	QString addMobileNumber();					//VOXOX - CJC - 2009.06.14 
	QString addFaxNumber();						//VOXOX - CJC - 2009.06.23 
	QString addEmail();							//VOXOX - CJC - 2009.06.11 

	//Clicked handlers
	void avatarButtonClicked();
	void chatButtonClicked();
	void profileButtonClicked();

	void networkIconClicked();					//VOXOX - CJC - 2009.07.08 
	void sendFileButtonClicked();
	void profileContactClicked();				//VOXOX - CJC - 2009.07.08 
	void webProfileContactClicked();			//VOXOX - CJC - 2009.07.08 

private:
	QMenu * _callMenu;
	QMenu * _emailMenu;
	QMenu * _smsMenu;
	QMenu * _faxMenu;							//VOXOX - CJC - 2009.06.23

	//VOXOX CHANGE by Rolando - 2009.10.27 
	//QMenu * _networkIconMenu;					//VOXOX - CJC - 2009.07.08

	QtContactNetworkMenu * _contactNetworkMenu;//VOXOX CHANGE by Rolando - 2009.10.27

	void updateNetworkPixmaps(QString normalPixmapPath, QString hoverPixmapPath);//VOXOX CHANGE by Rolando - 2009.10.22 

	//VOXOX CHANGE by Rolando - 2009.10.27 
	//void buildMenuNetworkIcon();				//VOXOX - CJC - 2009.07.08

	QPoint getPositionToShowMenu(QPoint currentPoint, int widthMenu, int heightMenu);//VOXOX CHANGE by Rolando - 2009.10.20 

	CUserProfile* getCUserProfile();
	Contact*	  getContact();			//VOXOX - JRT - 2009.08.10 

	void updateText();

//	void updateToolTips();	//VOXOX - JRT - 2009.10.15 

	QString verifyText(QRect painterRect, QFont font, QString text);

	void paintEvent(QPaintEvent *);

	void paintContact(QPainter * painter, const QRect & rect);

	std::string _contactId;
	std::string	_qtContactKey;	//VOXOX - JRT - 2009.08.10 

	CWengoPhone & _cWengoPhone;

	QString _text;

	Ui::ContactWidget * _ui;
};

#endif	//QTCONTACTWIDGET_H
