/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2005  Wengo
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

#ifndef OBJECT_H
#define OBJECT_H

#include <qtutil/owqtutildll.h>

#include <util/NonCopyable.h>

#include <QtCore/QString>
#include <QtCore/QObject>

/**
 * Replacement for QObject::findChild().
 *
 * Checks if QObject::findChild() returns a NULL pointer or not.
 * In case of a NULL pointer, Object shows an error message + an assert message.
 *
 * This class uses QMessageBox thus the graphical part of Qt.
 *
 * @author Tanguy Krotoff
 */
class OWQTUTIL_API Object : NonCopyable {
public:

	/**
	 * Replacement for QObject::findChild().
	 *
	 * @see QObject::findChild()
	 */
	template<typename T>
	static T findChild(QObject * object, const QString & objectName = QString()) {
		T tmp = object->findChild<T>(objectName);
		if (!tmp) {
			showErrorMessage(objectName);
		}

		return tmp;
	}

	template<typename T>
	static T findChild(const QObject & object, const QString & objectName = QString()) {
		return findChild<T>(&object, objectName);
	}

	/**
	 *
	 *
	 * @see QObject::dumpObjectTree()
	 */
	static void dumpTree(QObject * object);

	static void print(QObject * object, const char * leadingString = "");

private:

	/**
	 * Shows an error message.
	 *
	 * @param objectName object' name that causes the error
	 */
	static void showErrorMessage(const QString & objectName);

	static void dumpRecursive(int level, QObject * object);
};

#endif	//OBJECT_H
