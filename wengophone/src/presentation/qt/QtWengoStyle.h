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

#ifndef OWQTWENGOSTYLE_H
#define OWQTWENGOSTYLE_H

#include <cutil/global.h>

#ifdef OS_MACOSX
	#include <QtGui/QMacStyle>
	typedef QMacStyle BaseStyle;
#else
	#include <QtGui/QCommonStyle>
	typedef QCommonStyle BaseStyle;
#endif

/**
 * General Qt style for correcting some bugs or ugly style.
 *
 * - Removes the ugly toolbar bottom line
 * - Changes QToolButton style under MacOSX
 * - Removes the ugly frame/marging around the status bar icons under Windows
 *
 * @author Tanguy Krotoff
 */
class QtWengoStyle : public BaseStyle {
public:

	QtWengoStyle();

	~QtWengoStyle();

	void drawComplexControl(ComplexControl control, const QStyleOptionComplex * option,
		QPainter * painter, const QWidget * widget = 0) const;

	void drawControl(ControlElement element, const QStyleOption * option,
		QPainter * painter, const QWidget * widget = 0) const;

	void drawPrimitive(PrimitiveElement element, const QStyleOption * option,
		QPainter * painter, const QWidget * widget = 0) const;

	/**
	 * Overloaded to make sure rows of list views are fully selected, instead
	 * of only items.
	 */
	int styleHint(StyleHint hint, const QStyleOption * option = 0, const QWidget * widget = 0, QStyleHintReturn * returnData = 0) const;

#ifndef OS_MACOSX
	void drawItemPixmap(QPainter * painter, const QRect & rectangle, int alignment, const QPixmap & pixmap) const {
		_systemStyle->drawItemPixmap(painter, rectangle, alignment, pixmap);
	}

	void drawItemText(QPainter * painter, const QRect & rectangle, int alignment, const QPalette & palette,
		bool enabled, const QString & text, QPalette::ColorRole textRole = QPalette::NoRole) const {
		_systemStyle->drawItemText(painter, rectangle, alignment, palette, enabled, text, textRole);
	}

	QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap & pixmap, const QStyleOption * option) const {
		return _systemStyle->generatedIconPixmap(iconMode, pixmap, option);
	}

	SubControl hitTestComplexControl(ComplexControl control, const QStyleOptionComplex * option, const QPoint & position, const QWidget * widget = 0) const {
		return _systemStyle->hitTestComplexControl(control, option, position, widget);
	}

	QRect itemPixmapRect(const QRect & rectangle, int alignment, const QPixmap & pixmap) const {
		return _systemStyle->itemPixmapRect(rectangle, alignment, pixmap);
	}

	QRect itemTextRect(const QFontMetrics & metrics, const QRect & rectangle, int alignment, bool enabled, const QString & text) const {
		return _systemStyle->itemTextRect(metrics, rectangle, alignment, enabled, text);
	}

	int pixelMetric(PixelMetric metric, const QStyleOption * option = 0, const QWidget * widget = 0) const {
		return _systemStyle->pixelMetric(metric, option, widget);
	}

	void polish(QWidget * widget) {
		_systemStyle->polish(widget);
	}

	void polish(QApplication * application) {
		_systemStyle->polish(application);
	}

	void polish(QPalette & palette) {
		_systemStyle->polish(palette);
	}

	QSize sizeFromContents(ContentsType type, const QStyleOption * option, const QSize & contentsSize, const QWidget * widget = 0) const {
		return _systemStyle->sizeFromContents(type, option, contentsSize, widget);
	}

	QIcon standardIcon(StandardPixmap standardIcon, const QStyleOption * option = 0, const QWidget * widget = 0) const {
		return _systemStyle->standardIcon(standardIcon, option, widget);
	}

	QPalette standardPalette() const {
		return _systemStyle->standardPalette();
	}

	QPixmap standardPixmap(StandardPixmap standardPixmap, const QStyleOption * option = 0, const QWidget * widget = 0) const {
		return _systemStyle->standardPixmap(standardPixmap, option, widget);
	}

	QRect subControlRect(ComplexControl control, const QStyleOptionComplex * option, SubControl subControl, const QWidget * widget = 0) const {
		return _systemStyle->subControlRect(control, option, subControl, widget);
	}

	QRect subElementRect(SubElement element, const QStyleOption * option, const QWidget * widget = 0) const {
		return _systemStyle->subElementRect(element, option, widget);
	}

	void unpolish(QWidget * widget) {
		_systemStyle->unpolish(widget);
	}

	void unpolish(QApplication * application) {
		_systemStyle->unpolish(application);
	}

#endif

private:

	QStyle * _systemStyle;

	/** Used on MacOS X for QToolButton appearance. */
	QStyle * _plastiqueStyle;

};

#endif	//OWQTWENGOSTYLE_H
