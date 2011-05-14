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

#ifndef SKINSTYLE_H
#define SKINSTYLE_H

#include "SkinInfoReader.h"

#include <cutil/global.h>

#ifdef OS_WINDOWS
	#include <QtGui/QWindowsXPStyle>
	class SystemStyle : public QWindowsXPStyle {};
#elif defined(OS_MACOSX)
	#include <QtGui/QMacStyle>
	class SystemStyle : public QMacStyle {};
#else
	#include <QtGui/QWindowsStyle>
	class SystemStyle : public QWindowsStyle {};
#endif

class SkinWidget;
class QWidget;
class QLayout;
class QToolBar;
class QMenuBar;
class QStatusBar;

/**
 * Skin engine implemented as a Qt style that loads pixmaps over widgets.
 *
 * SkinStyle permits to change position and size of widgets.
 * Layouts are broken automatically if needed.
 * These informations are read from a .ini file:
 * <code>
 * [exitButton]
 * xPosition=11
 * yPosition=115
 *
 * [searchButton]
 * hide=true
 *
 * [tabWidget]
 * xPosition=85
 * yPosition=51
 * width=304
 * height=186
 * </code>
 *
 * The name [exitButton] is the name of the QWidget, inside a .ui file:
 * <code>
 * <widget class="QPushButton" name="exitButton" >
 * </code>
 * This is similar to QObject->setObjectName()
 *
 * So the relation between a skin and the widgets inside a Qt application
 * is made only by using the widgets' names.
 *
 * For the .png files it works the same way:
 * - exitButton.png is the normal skin for the widget named exitButton
 * - exitButton-active.png is the skin for exitButton when it is clicked
 * - exitButton-hover.png is the skin for exitButton when the mouse is over the widget
 *
 * For windows there are two states:
 * - mainWindow-background.png
 * - mainWindow-mask.png
 *
 * The extensions -active -hover -mask... are hardcoded (and I don't think this is a bad thing:
 * all the skins use the same conventions for the .png filenames)
 *
 * For now paths are like this (under Windows):
 * app/skins/future
 * app/skins/future/skin.ini
 * app/skins/future/*.png
 * app/app.exe
 * app/styles/skinstyle.dll
 *
 * The path skins/future/ is hardcoded (cf SkinInfoReader.cpp)
 *
 * @author Tanguy Krotoff
 */
class SkinStyle : public SystemStyle {
public:

	SkinStyle();

	~SkinStyle();

	void polish(QWidget * widget);

	void drawControl(ControlElement control, const QStyleOption * option,
				QPainter * painter, const QWidget * widget) const;

	void drawComplexControl(ComplexControl control, const QStyleOptionComplex * option,
				QPainter * painter, const QWidget * widget) const;

	QRect subControlRect(ComplexControl control, const QStyleOptionComplex * option,
				SubControl subControl, const QWidget * widget = 0) const;

	int pixelMetric(PixelMetric metric, const QStyleOption * option = 0,
				const QWidget * widget = 0) const;

private:

	/** QWidget. */
	void skinWidget(QWidget * widget, const QString & objectName);

	/** QAbstractButton (QPushButton, QRadioButton, QCheckBox, QToolButton...). */
	void skinButton(QWidget * widget, const QString & objectName);

	/** QFrame (QAbstractScrollArea, QListView, QListWidget...). */
	void skinFrame(QWidget * widget, const QString & objectName);

	/** QMainWindow and QDialog (QWidget as a separated window). */
	void skinWindow(QWidget * widget, const QString & objectName);

	/** QToolBar. */
	void skinToolBar(QToolBar * toolBar, const QString & objectName);

	/** QMenuBar. */
	void skinMenuBar(QMenuBar * menuBar);

	/** QStatusBar. */
	void skinStatusBar(QStatusBar * statusBar);


	void createButton(const QString & objectName, QWidget * parent, const char * member);

	void createMenuBarContainer(QWidget * widget, const QString & objectName);

	void createWidgetsContainer(QWidget * widget, const QString & objectName);

	void setWidgetPosition(SkinWidget * widget, const QString & objectName);

	void setWidgetSize(SkinWidget * widget, const QString & objectName);

	void setWidgetBackground(SkinWidget * widget, const QString & objectName);

	void setPixmaps(SkinWidget * widget, const QString & objectName);

	void setMask(SkinWidget * widget, const QString & objectName);

	void hide(SkinWidget * widget, const QString & objectName);

	/** Removes the layout associated with a widget. */
	void removeLayout(QWidget * widget);

	void addWidgetToLayout(QWidget * widget, QLayout * layout);

	void drawPixmap(const QString & pixmapFilename, const QRect & rect, QPainter * painter, bool rotate = false) const;

	void drawScaledPixmap(const QString & pixmapFilename, const QRect & rect, QPainter * painter, bool rotate = false) const;


	SkinInfoReader _info;

	QPixmap * _scrollbarHandlePix;
	int _scrollbarHandleHeight;

	QPixmap * _arrowUpPix;
	int _arrowUpWidth;
	int _arrowUpHeight;
};

#endif	//SKINSTYLE_H
