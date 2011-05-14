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

#include "QtCrashReport.h"

#include <iostream>
#include <iterator>
#include <vector>
using namespace std;

#include <boost/program_options.hpp>
using namespace boost::program_options;

#include <QtGui/QApplication>

int main(int argc, char** argv) {

	QApplication app(argc, argv);

	string dumpfile;
	string applicationName;
	string lang;
	string style;
	string info;

	try {

		options_description desc("Allowed options");
		desc.add_options()
			// First parameter describes option name/short name
			// The second is parameter to option
			// The third is description
			("help,h", "print usage message")
			("dumpfile,d", value(&dumpfile), "path to the memory dump file")
			("name,n", value(&applicationName), "the application name")
			("lang,l", value(&lang), "the application language")
			("info,i", value(&info), "additionnal info")
			;

		variables_map vm;
		store(parse_command_line(argc, argv, desc), vm);

		if (vm.count("help")) {
			cout << desc << "\n";
			return 0;
		}

		if (vm.count("dumpfile")) {
			cout << "dumpfile = " << vm["dumpfile"].as<string>() << "\n";
			dumpfile = vm["dumpfile"].as<string>();
		} else {
			cout << "No dump file specified" << endl;
			return 1;
		}

		if (vm.count("name")) {
			cout << "name = " << vm["name"].as<string>() << "\n";
			applicationName = vm["name"].as<string>();
		} else {
			cout << "No name specified" << endl;
			return 1;
		}

		if (vm.count("lang")) {
			cout << "lang = " << vm["lang"].as<string>() << "\n";
			lang = vm["lang"].as<string>();
		} else {
			cout << "No lang specified" << endl;
		}

		if (vm.count("info")) {
			cout << "info = " << vm["info"].as<string>() << "\n";
			info = vm["info"].as<string>();
		} else {
			cout << "No info specified" << endl;
		}

	}
	catch(exception& e) {
		cerr << e.what() << "\n";
	}

	QtCrashReport * crashReport = new QtCrashReport(dumpfile, applicationName, lang, info);
	crashReport->show();

	return app.exec();
}
