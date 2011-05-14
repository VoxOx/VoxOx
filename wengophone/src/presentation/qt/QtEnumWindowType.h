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
* Represent Tab widget types
* @author Chris Jimenez C 
* @date 2009.05.11
*/


#ifndef OWQTENUMWINDOWTYPE_H
#define OWQTENUMWINDOWTYPE_H

#include <util/NonCopyable.h>

#include <QtCore/QString>

#include <QtCore/QObject>


class QtEnumWindowType : public QObject, NonCopyable {
Q_OBJECT
public:

	enum Type {
		WizardWindow,
		ConfigWindow,
		ContactManagerWindow,
		ProfileWindow,
		ChatHistoryWindow,
		UnknownWindow
	};

	static QString toString(Type windowType);

	static QtEnumWindowType::Type toWindowType(const QString & windowType);

};

#endif	//OWQtUniversalMessage_H
