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

#ifndef OWQTEMOTICONSWIDGET_H
#define OWQTEMOTICONSWIDGET_H

#include <QtGui/QFrame>

class QtEmoticon;
class QString;
class QStringList;
class QGridLayout;

/**
 *
 * @ingroup presentation
 * @author Mr K.
 */
class QtEmoticonsWidget : public QFrame {
	Q_OBJECT
public:

	QtEmoticonsWidget(QWidget * parent);

	void initButtons();

public Q_SLOTS:

	void changeState();

	void buttonClicked(const QtEmoticon & emoticon);

Q_SIGNALS:

	void emoticonClicked(const QtEmoticon & emoticon);

	void closed();

private:

	void closeEvent(QCloseEvent * event);

	void addButton(const QtEmoticon & emoticon);

	enum EmoticonsWidgetState {
		Window,
		Popup
	};

	QWidget * _widget;

	QStringList _iconName;

	EmoticonsWidgetState _state;

	int _buttonX;

	int _buttonY;

	QGridLayout * _layout;
};

#endif	//OWQTEMOTICONSWIDGET_H
