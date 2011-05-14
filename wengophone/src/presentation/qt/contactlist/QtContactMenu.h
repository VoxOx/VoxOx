/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2007  Wengo
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

#ifndef OWQTCONTACTMENU_H
#define OWQTCONTACTMENU_H

#include <imwrapper/EnumPresenceState.h>

class CWengoPhone;
class QMenu;
class QAction;
class QIcon;
class StringList;

/**
 * Contact list QMenu builder.
 *
 * @author Mathieu Stute
 */
class QtContactMenu {

public:

	static void populateMobilePhoneMenu(QMenu * menu, CWengoPhone & cWengoPhone);

	static void populateWengoCallMenu(QMenu * menu, CWengoPhone & cWengoPhone);
	
	static void populateSipCallMenu(QMenu * menu, CWengoPhone & cWengoPhone);

	static void populateHomePhoneMenu(QMenu * menu, CWengoPhone & cWengoPhone);

	static void populateChatMenu(QMenu * menu, CWengoPhone & cWengoPhone);

//	static void populateWengoUsersContactId(QMenu * menun, CWengoPhone & cWengoPhone);	//VOXOX - JRT - 2009.07.27 - Not called.

private:

	static void setPresenceIcon(QAction * action, QIcon icon);

	static void setPresenceIcon(QAction * action, EnumPresenceState::PresenceState presenceState);
};

#endif	//OWQTCONTACTMENU_H
