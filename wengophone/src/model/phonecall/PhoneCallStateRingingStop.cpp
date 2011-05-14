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

#include "stdafx.h"	//VOXOX - JRT - 2009.04.01
#include "PhoneCallStateRingingStop.h"

#include "PhoneCall.h"

#include <model/config/ConfigManager.h>
#include <model/config/Config.h>
#include <model/wenbox/WenboxPlugin.h>

#include <sound/Sound.h>

void PhoneCallStateRingingStop::execute(PhoneCall & phoneCall,bool) {

	stopSoundIncomingCall();

	WenboxPlugin & wenboxPlugin = phoneCall.getWenboxPlugin();
	wenboxPlugin.setState(Wenbox::CallRinging);
}
