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
#ifndef QTCONTACTLISTSTYLE_H
#define QTCONTACTLISTSTYLE_H

#include <QtCore/QString>
#include <QtGui/QColor>
#include <QtGui/QPixmap>
#include <util/NonCopyable.h>
#include <QtCore/QStringList>
class QtContactListStyle : public QObject,NonCopyable{
	Q_OBJECT
public:

	static QtContactListStyle * getInstance();

	QtContactListStyle();

	void loadStyleConfig(QString currentStylePath);

    QStringList getStyleList();

	QString getContactStyleFolder();


	QString & getCurrentStylePath(){return _currentStylePath;}

	


	QColor & getGroupFontColor(){return _groupFontColor;}

	QColor & getGroupBackgroundColorTop(){return _groupBackgroundColorTop;}

	QColor & getGroupBackgroundColorBottom(){return _groupBackgroundColorBottom;}

	QColor & getGroupBorderColor(){return _groupBorderColor;}

	QPixmap & getGroupPixmapClose(){return _groupPixmapClose;}

	QPixmap & getGroupPixmapOpen(){return _groupPixmapOpen;}

	QColor & getContactNonSelectedBackgroundColor(){return _contactNonSelectedBackgroundColor;}

	QColor & getContactSelectedBackgroundColorTop(){return _contactSelectedBackgroundColorTop;}

	QColor & getContactSelectedBackgroundColorBottom(){return _contactSelectedBackgroundColorBottom;}

	QColor & getContactUserNameNonSelectedFontColor(){return _contactUserNameNonSelectedFontColor;}

	QColor & getContactSocialStatusNonSelectedFontColor(){return _contactSocialStatusNonSelectedFontColor;}

	QColor & getContactUserNameSelectedFontColor(){return _contactUserNameSelectedFontColor;}

	QColor & getContactSocialStatusSelectedFontColor(){return _contactSocialStatusSelectedFontColor;}



private:

	QString _currentStylePath;
	QColor  _groupFontColor;
	QColor  _groupBackgroundColorTop;
	QColor  _groupBackgroundColorBottom;
	QColor  _groupBorderColor;
	QPixmap  _groupPixmapClose;
	QPixmap  _groupPixmapOpen;
	QColor  _contactNonSelectedBackgroundColor;
	QColor  _contactSelectedBackgroundColorTop;
	QColor  _contactSelectedBackgroundColorBottom;

	QColor  _contactUserNameNonSelectedFontColor;
	QColor  _contactSocialStatusNonSelectedFontColor;
	QColor  _contactUserNameSelectedFontColor;
	QColor  _contactSocialStatusSelectedFontColor;

	static QtContactListStyle * instance;	

};

#endif // QTCHATUTILS_H
