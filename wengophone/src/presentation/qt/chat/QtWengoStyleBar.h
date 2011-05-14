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

#ifndef OWQTWENGOSTYLEBAR_H
#define OWQTWENGOSTYLEBAR_H

#include <QtGui/QWidget>
#include <QtCore/QMap>
#include <QtCore/QList>
#include <QtCore/QMutex>

#include <qtutil/WengoStyleLabel.h>

class QHBoxLayout;

/**
 *
 * @author Nicolas Couturier
 * @author Mathieu Stute
 */
class QtWengoStyleBar : public QWidget {
	Q_OBJECT
public:

	/**
	 * Constructor.
	 */
	QtWengoStyleBar(QWidget * parent);

Q_SIGNALS:

	/**
	 * Emoticon label has been clicked.
	 */
	void emoticonsLabelClicked();

	/**
	 * Font label has been clicked.
	 */
	void fontLabelClicked();

	/**
	 * Font color label has been clicked.
	 */
	void fontColorLabelClicked();

	/**
	 * Bold label has been clicked.
	 */
	void boldLabelClicked();

	/**
	 * Italic label has been clicked.
	 */
	void italicLabelClicked();

	/**
	 * Underline label has been clicked.
	 */
	void underlineLabelClicked();

private:
	/**
	 * Adds a new button in the action bar
	 *
	 * @param normalPixmap : the image to be used for the button.
	 * @param pressedPixmap : the image to be used when the button is pressed.
	 * @param size : the size of the button.
	 * @param signal : the signal to emit on click.
	 * @param mode : the label mode. Set to WengoStyleLabel::Toggled by
	 * default.
	 */
	void addLabel(const QPixmap & normalPixmap, const QPixmap & pressedPixmap,
		const char* signal, WengoStyleLabel::Mode mode = WengoStyleLabel::Toggled);

	QHBoxLayout* _layout;
};

#endif	//OWQTWENGOSTYLEBAR_H
