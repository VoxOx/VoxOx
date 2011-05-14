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

#ifndef SOUNDWIDGET_H
#define SOUNDWIDGET_H

#include <QtGui/QtGui>

#include <memory>

class Sound;

namespace Ui { class SoundWidget; }

class SoundWidget : public QWidget {
	Q_OBJECT
public:

	SoundWidget();

	~SoundWidget();

private Q_SLOTS:

	void playSlot();

	void stopSlot();

	void staticPlay();

	void readSettings();

	void setMute(bool);

	void setVolume(int);

	void selectFile();

private:

	std::auto_ptr<Sound> _sound;
	Ui::SoundWidget * _ui;
	QString _fileName;
};

#endif	//SOUNDWIDGET_H
