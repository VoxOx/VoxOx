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

#include <Sound.h>
#include <AudioDevice.h>
#include <SoundMixer.h>

#include <qapplication.h>
#include <qpushbutton.h>

#include <iostream>
using namespace std;

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);

	cout << AudioDevice::getDefaultPlaybackDevice() << endl;
	//AudioDevice::setDefaultPlaybackDevice("USB Audio Device");
	cout << AudioDevice::getDefaultPlaybackDevice() << endl;

	SoundMixer soundMixer(AudioDevice::getDefaultPlaybackDevice(),
			AudioDevice::getDefaultPlaybackDevice());
	cout << soundMixer.getOutputVolume() << endl;

	Sound * sound = new Sound("ringin.wav");
	sound->setWaveOutDevice(AudioDevice::getDefaultPlaybackDevice());
	sound->setLoops(-1);
	sound->play();

	QPushButton hello("Sound example", 0);
	hello.resize(100, 30);

	app.setMainWidget(&hello);
	hello.show();

	return app.exec();
}
