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

#include "WidgetFactory.h"

#include <cassert>

WidgetFactory::WidgetFactory(const std::string & gladeFile) {
	_xml = glade_xml_new(gladeFile.c_str(), NULL, NULL);
	assert(_xml && "libglade error: can't load the .glade file");
}

GtkWidget * WidgetFactory::getWidget(const std::string & widgetName) {
	GtkWidget * widget = glade_xml_get_widget(_xml, widgetName.c_str());
	assert(widget && "libglade error: can't get the widget");

	return widget;
}

void WidgetFactory::signalAutoConnect() {
	glade_xml_signal_autoconnect(_xml);
}
