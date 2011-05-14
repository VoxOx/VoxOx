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

#include <qtutil/Object.h>

#include <QtGui/QMessageBox>
#include <QtCore/QMetaObject>

#include <util/Logger.h>

#include <iostream>
using namespace std;

void Object::showErrorMessage(const QString & objectName) {
	const QString errorMsg = "Cannot find QObject: " + objectName;

	QMessageBox::critical(NULL, "Critical Error", errorMsg);
	LOG_FATAL(errorMsg.toStdString());
}

void Object::print(QObject * object, const char * leadingString) {
	if (!object) {
		return;
	}

	const char * className = object->metaObject()->className();
	QString objectName = object->objectName();
	QString flags = QLatin1String("");

	if (object->isWidgetType()) {
		QWidget * w = qobject_cast<QWidget *>(object);

		QString t("<x:%1 y:%2 w:%3 h:%4>");
		flags += t.arg(w->x()).arg(w->y()).arg(w->width()).arg(w->height());
	}

	qDebug("%s%s::%s %s <%p p:%p>", leadingString,
				className,
				objectName.toLocal8Bit().data(),
				flags.toLatin1().data(),
				object, object->parent());
}

void Object::dumpRecursive(int level, QObject * object) {
	if (!object) {
		return;
	}

	//Creates leading spaces
	QByteArray buf;
	buf.fill('\t', level/2);
	if (level % 2) {
		buf += "    ";
	}

	print(object, (const char *) buf);

	QObjectList children = object->children();
	if (!children.isEmpty()) {
		for (int i = 0; i < children.size(); ++i) {
			dumpRecursive(level + 1, children.at(i));
		}
	}
}

void Object::dumpTree(QObject * object) {
	dumpRecursive(0, object);
}
