/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2007  Wengo
 *
 * This program is free software;
 you can redistribute it and/or modify
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
#ifndef QTUMITEMLISTSTYLE_H
#define QTUMITEMLISTSTYLE_H

#include <QtCore/QString>
#include <QtGui/QColor>
#include <QtGui/QPixmap>
#include <util/NonCopyable.h>
#include <QtCore/QStringList>
class QtUMItemListStyle : public QObject,NonCopyable{
	Q_OBJECT
public:

	static QtUMItemListStyle * getInstance();

	QtUMItemListStyle();

	void loadStyleConfig(QString currentStylePath);

    QStringList getStyleList();

	QString getStyleFolder();


	QString & getCurrentStylePath(){return _currentStylePath;}

	QString getCurrentStyle();//VOXOX CHANGE by Rolando - 2009.09.02

	QColor & getGroupBlinkingBackgroundColor(){return _groupBlinkingBackgroundColor;}//VOXOX CHANGE by Rolando - 2009.09.04 
	QColor & getGroupBlinkingFontColor(){return _groupBlinkingFontColor;}//VOXOX CHANGE by Rolando - 2009.09.04

	QColor & getSelectedGroupFontColor(){return _groupSelectedFontColor;}//VOXOX CHANGE by Rolando - 2009.09.04 
	QColor & getNonSelectedGroupFontColor(){return _groupNonSelectedFontColor;}//VOXOX CHANGE by Rolando - 2009.09.04 

	QColor & getSelectedGroupBackgroundColorTop(){return _groupSelectedBackgroundColorTop;}//VOXOX CHANGE by Rolando - 2009.09.03 
	QColor & getSelectedGroupBackgroundColorBottom(){return _groupSelectedBackgroundColorBottom;}//VOXOX CHANGE by Rolando - 2009.09.03 
	QColor & getNonSelectedGroupBackgroundColorTop(){return _groupNonSelectedBackgroundColorTop;}//VOXOX CHANGE by Rolando - 2009.09.03 
	QColor & getNonSelectedGroupBackgroundColorBottom(){return _groupNonSelectedBackgroundColorBottom;}//VOXOX CHANGE by Rolando - 2009.09.03 

	QColor & getSelectedGroupBorderColor(){return _groupSelectedBorderColor;}//VOXOX CHANGE by Rolando - 2009.09.03 
	QColor & getNonSelectedGroupBorderColor(){return _groupNonSelectedBorderColor;}//VOXOX CHANGE by Rolando - 2009.09.03 

	QPixmap & getGroupPixmapClose(){return _groupPixmapClose;}
	QPixmap & getGroupPixmapOpen(){return _groupPixmapOpen;}

	QColor & getItemNonSelectedBackgroundColor(){return _itemNonSelectedBackgroundColor;}

	QColor & getItemSelectedBackgroundColorTop(){return _itemSelectedBackgroundColorTop;}
	QColor & getItemSelectedBackgroundColorBottom(){return _itemSelectedBackgroundColorBottom;}

	QColor & getItemUserNameNonSelectedFontColor(){return _itemUserNameNonSelectedFontColor;}

	QColor & getItemUserNameSelectedFontColor(){return _itemUserNameSelectedFontColor;}

	QColor & getItemBlinkingBackgroundColor(){return _itemBlinkingBackgroundColor;}//VOXOX CHANGE by Rolando - 2009.09.07 
	QColor & getItemBlinkingFontColor(){return _itemBlinkingFontColor;} //VOXOX CHANGE by Rolando - 2009.09.07 


private:

	QString _currentStylePath;

	QColor	_groupBlinkingBackgroundColor;//VOXOX CHANGE by Rolando - 2009.09.07 
	QColor  _groupBlinkingFontColor;//VOXOX CHANGE by Rolando - 2009.09.07 

	QColor  _groupSelectedFontColor;//VOXOX CHANGE by Rolando - 2009.09.04 
	QColor  _groupNonSelectedFontColor;//VOXOX CHANGE by Rolando - 2009.09.04 

	QColor  _groupSelectedBackgroundColorTop;//VOXOX CHANGE by Rolando - 2009.09.03 
	QColor  _groupSelectedBackgroundColorBottom;//VOXOX CHANGE by Rolando - 2009.09.03 
	QColor  _groupNonSelectedBackgroundColorTop;//VOXOX CHANGE by Rolando - 2009.09.03 
	QColor  _groupNonSelectedBackgroundColorBottom;//VOXOX CHANGE by Rolando - 2009.09.03 

	QColor  _groupSelectedBorderColor;//VOXOX CHANGE by Rolando - 2009.09.03 
	QColor  _groupNonSelectedBorderColor;//VOXOX CHANGE by Rolando - 2009.09.03 

	QPixmap  _groupPixmapClose;
	QPixmap  _groupPixmapOpen;

	QColor  _itemNonSelectedBackgroundColor;
	QColor  _itemSelectedBackgroundColorTop;
	QColor  _itemSelectedBackgroundColorBottom;
	QColor  _itemUserNameNonSelectedFontColor;
	QColor  _itemUserNameSelectedFontColor;

	QColor	_itemBlinkingBackgroundColor;//VOXOX CHANGE by Rolando - 2009.09.07 
	QColor  _itemBlinkingFontColor;//VOXOX CHANGE by Rolando - 2009.09.07 

	static QtUMItemListStyle * instance;	

};

#endif // QTUMITEMLISTSTYLE_H
