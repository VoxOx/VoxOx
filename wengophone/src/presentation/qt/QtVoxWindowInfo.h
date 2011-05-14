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
* Base class for the diferent chat widgets
* @author Chris Jimenez C 
* @date 2009.05.11
*/


#ifndef OWQTVOXWINDOWINFO_H
#define OWQTVOXWINDOWINFO_H


#include <util/Trackable.h>
#include <QtGui/QDialog>
#include "QtEnumWindowType.h"

/**
 * Main window status bar.
 *
 * Contains status icons for Internet connection, sound status, SIP (network) status...
 *
 * @author Tanguy Krotoff
 * @author Mathieu Stute
 */

class QtVoxWindowInfo : public QDialog, public Trackable {
	Q_OBJECT
public:


	QtVoxWindowInfo(QWidget * parent,QtEnumWindowType::Type type, Qt::WindowFlags f = 0);

	~QtVoxWindowInfo();

	void setType(QtEnumWindowType::Type type){_type = type;}

	QtEnumWindowType::Type getType() const { return _type; }

	virtual QWidget * getWidget() const = 0;

	virtual QString getKey() const = 0;

	virtual QString getDescription() const = 0;

	virtual int getAllowedInstances() const = 0;

	virtual int getCurrentTab() const = 0;

	virtual void setCurrentTab(QString tabName)= 0;

	virtual void showWindow() = 0;

	
protected:

	

Q_SIGNALS:

	void windowClose(QString Id);


private Q_SLOTS:

	


private:


	QtEnumWindowType::Type _type;


};

#endif	//OWQtVoxWindowInfo_H
