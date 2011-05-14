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

#include <qtutil/AnimatedButton.h>

#include <qtutil/SafeConnect.h>

#include <util/SafeDelete.h>

#include <QtGui/QtGui>

static const char * MNG_FORMAT = "MNG";

AnimatedButton::AnimatedButton(QAbstractButton * button, const QString & animatedIconFilename) {
	_button = button;
	_animatedIcon = new QMovie(animatedIconFilename, MNG_FORMAT, _button);
	SAFE_CONNECT(_animatedIcon, SIGNAL(frameChanged(int)), SLOT(updateButtonIcon()));
	_animatedIcon->start();
}

AnimatedButton::~AnimatedButton() {
	_animatedIcon->stop();
	OWSAFE_DELETE(_animatedIcon);
}

void AnimatedButton::updateButtonIcon() {
	QPixmap icon = _animatedIcon->currentPixmap();
	_button->setIcon(QIcon(icon));
	_button->repaint();
}
