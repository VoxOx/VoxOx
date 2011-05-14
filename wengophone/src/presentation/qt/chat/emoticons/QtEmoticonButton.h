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

#ifndef OWQTEMOTICONBUTTON_H
#define OWQTEMOTICONBUTTON_H

#include "QtEmoticon.h"

#include <QtGui/QToolButton>

class QWidget;
class QString;
class QIcon;

/**
 *
 * @ingroup presentation
 * @author Mr K.
 */
class QtEmoticonButton : public QToolButton {
	Q_OBJECT
public:

	QtEmoticonButton(QWidget * parent);

	void setEmoticon(QtEmoticon emoticon);

Q_SIGNALS:

	void buttonClicked(QtEmoticon emoticon);

private Q_SLOTS:

	void buttonClickedSlot();

private:

	QtEmoticon _emoticon;
};

#endif	//OWQTEMOTICONBUTTON_H
