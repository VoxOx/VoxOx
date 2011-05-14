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

#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "QtWengoStyleBar.h"

#include <qtutil/WengoStyleLabel.h>

#include <QtGui/QHBoxLayout>

QtWengoStyleBar::QtWengoStyleBar(QWidget * parent) :
	QWidget(parent) {

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	
	// create layout
	_layout = new QHBoxLayout(this);
	_layout->setMargin(0);
	_layout->setSpacing(0);
	////

	// add emoticon label
	addLabel(
		QPixmap(":/pics/chat/chat_emoticon_button.png"),
		QPixmap(":/pics/chat/chat_emoticon_button_on.png"),
		SIGNAL(emoticonsLabelClicked()),
		WengoStyleLabel::Normal
	);
	////

	// add bold label
	addLabel(
		QPixmap(":/pics/chat/font_bold.png"),
		QPixmap(":/pics/chat/font_bold_on.png"),
		SIGNAL(boldLabelClicked())
	);
	////

	// add italic label
	addLabel(
		QPixmap(":/pics/chat/font_italic.png"),
		QPixmap(":/pics/chat/font_italic_on.png"),
		SIGNAL(italicLabelClicked())
	);
	////

	// add underline label
	addLabel(
		QPixmap(":/pics/chat/font_underline.png"),
		QPixmap(":/pics/chat/font_underline_on.png"),
		SIGNAL(underlineLabelClicked())
	);
	////

	// configure end label
	QPixmap fillPixmap = QPixmap(":/pics/chat/bar_fill.png");
	WengoStyleLabel* endLabel = new WengoStyleLabel(this);
	endLabel->setPixmaps(
		QPixmap(),
		QPixmap(":/pics/chat/bar_end.png"),
		fillPixmap,
		QPixmap(),
		QPixmap(":/pics/chat/bar_end.png"),
		fillPixmap	
	);
	int height = fillPixmap.height();
	endLabel->setMinimumSize(QSize(16, height));
	_layout->addWidget(endLabel);
	////

	setFixedHeight(height);

}

void QtWengoStyleBar::addLabel(
	const QPixmap & normalPixmap, const QPixmap & pressedPixmap,
	const char* signal,
	WengoStyleLabel::Mode mode) {

	WengoStyleLabel * label = new WengoStyleLabel(this, mode);

	label->setPixmaps(
		normalPixmap,
		QPixmap(),
		QPixmap(),
		pressedPixmap,
		QPixmap(),
		QPixmap()
	);
	label->setFixedSize(normalPixmap.size());

	_layout->addWidget(label);

	connect(label, SIGNAL(clicked()), signal);
}
