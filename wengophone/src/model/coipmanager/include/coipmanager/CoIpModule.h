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

#ifndef OWCOIPMODULE_H
#define OWCOIPMODULE_H

#include <util/Event.h>
#include <util/Interface.h>
#include <util/Trackable.h>

/**
 * Base class for all CoIp module.
 *
 * @author Philippe Bernery
 */
class CoIpModule : Interface, public Trackable {
public:

	/**
	 * Sent when the module is finished.
	 *
	 * @param sender this class
	 */
	Event< void (CoIpModule & sender) > moduleFinishedEvent;

	/**
	 * Starts the Module.
	 */
	virtual void start() = 0;

	/**
	 * Pauses the Module.
	 *
	 * No effect if paused or stopped.
	 */
	virtual void pause() = 0;

	/**
	 * Resumes the Module.
	 *
	 * No effect if not paused.
	 */
	virtual void resume() = 0;

	/**
	 * Stops the Module.
	 */
	virtual void stop() = 0;
};

#endif //OWCOIPMODULE_H
