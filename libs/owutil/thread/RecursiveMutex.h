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

#ifndef OWRECURSIVEMUTEX_H
#define OWRECURSIVEMUTEX_H

#include <boost/thread/recursive_mutex.hpp>
#include <boost/version.hpp>

/**
 * Recursive Mutex helper for Boost.
 *
 * @see Mutex
 * @author Philippe Bernery
 */
class RecursiveMutex : public boost::recursive_mutex {
public:

	typedef scoped_lock ScopedLock;

#if BOOST_VERSION < 103500
	void lock() {
		boost::detail::thread::lock_ops<boost::recursive_mutex>::lock(*this);
	}

	void unlock() {
		boost::detail::thread::lock_ops<boost::recursive_mutex>::unlock(*this);
	}
#endif

};

#endif	//OWRECURSIVEMUTEX_H
