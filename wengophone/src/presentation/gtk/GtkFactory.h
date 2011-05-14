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

#ifndef GTKFACTORY_H
#define GTKFACTORY_H

#include "presentation/PFactory.h"
#include "GtkWengoPhone.h"
#include "control/CWengoPhone.h"
#include "phoneline/GtkPhoneLine.h"
#include "control/phoneline/CPhoneLine.h"

#include <gtk/gtk.h>

class GtkFactory : public PFactory {
public:

	GtkFactory(int argc, char * argv[]) {
		gtk_init(& argc, & argv);
	}

	int exec() {
		gtk_main();
		return EXIT_SUCCESS;
	}

	PWengoPhone * createPresentationWengoPhone(CWengoPhone * cWengoPhone) {
		return new GtkWengoPhone(cWengoPhone);
	}

	PPhoneLine * createPresentationPhoneLine(CPhoneLine * cPhoneLine) {
		return new GtkPhoneLine(cPhoneLine);
	}

	PPhoneCall * createPresentationPhoneCall(CPhoneCall * cPhoneCall) {
		//return new GtkPhoneCall(cPhoneCall);
		return NULL;
	}

	PWengoPhoneLogger * createPresentationLogger(CWengoPhoneLogger * cWengoPhoneLogger) {
		return NULL;
	}

	PContactList * createPresentationContactList(CContactList * cContactList) {
		return NULL;
	}

	PContactGroup * createPresentationContactGroup(CContactGroup * cContactGroup) {
		return NULL;
	}

	PContact * createPresentationContact(CContact * cContact) {
		return NULL;
	}
};

#endif	//GTKFACTORY_H
