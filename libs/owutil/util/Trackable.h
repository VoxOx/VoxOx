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

#ifndef OWTRACKABLE_H
#define OWTRACKABLE_H

#include <boost/signals/trackable.hpp>

/**
 * Delegates/Design Pattern Observer.
 *
 * Provides the mechanism for automatic signal/slot disconnection when the object is destructed.
 * You must inherit publicly from this class.
 *
 * The problem solved by Trackable is simple:
 * what if a signal is emitted and the receiver object for this signal is already been destroyed?
 * most likely, a segmentation fault will occur.
 *
 * The emitter of a signal does not inherit from Trackable, the receiver does inherit from Trackable.
 *
 * @author Tanguy Krotoff
 */
class Trackable : public boost::signals::trackable {
};

#endif	//OWTRACKABLE_H
