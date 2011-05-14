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

#include <softupdater/SoftUpdater.h>

#include <QtGui/QtGui>

#include <iostream>
using namespace std;

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);

	/*if (argc < 3) {
		cout << "Usage: " << argv[0]
			<< " URL" << " output_filename" << endl;
		return EXIT_FAILURE;
	}

	if (argv[3] != NULL) {
		//Translation file
		QString lang = argv[3];
		QTranslator translator;
		translator.load(lang);
		app.installTranslator(&translator);
	}

	QString input = argv[1];
	QString output = argv[2];*/

	SoftUpdater * softUpdater = new SoftUpdater();
	softUpdater->download(
			"http://wengofiles.wengo.fr/nightlybuilds/installer/NG/win32/WengoPhoneNG-setup-release-symbols-20060326160934.exe",
			"setup.exe");

	return app.exec();
}
