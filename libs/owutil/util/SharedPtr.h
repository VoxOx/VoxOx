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

#ifndef OWSHAREDPTR_H
#define OWSHAREDPTR_H

#include <boost/shared_ptr.hpp>

/**
 * Shared pointer helper for boost.
 *
 * Example:
 * <pre>
 * </pre>
 *
 * @author Tanguy Krotoff
 */
/*template<typename T>
class SharedPtr : public boost::shared_ptr<T> {
};*/
#define SharedPtr boost::shared_ptr

#endif	//OWSHAREDPTR_H
