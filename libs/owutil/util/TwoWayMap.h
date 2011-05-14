/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2007  Wengo
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
#ifndef TWOWAYMAP_H
#define TWOWAYMAP_H

#include <map>
#include <assert.h>

/**
 * This class makes it easy to implement two way mappings. For example to
 * associate an enum to its string representation and vice-versa.
 */
template <class First, class Second>
class TwoWayMap {
	typedef std::map<First, Second> FirstToSecond;
	typedef std::map<Second, First> SecondToFirst;

public:
	void addEntry(const First& first, const Second& second) {
		_firstToSecond[first] = second;
		_secondToFirst[second] = first;
	}

	Second toSecond(const First& first) const {
		typename FirstToSecond::const_iterator it = _firstToSecond.find(first);
		assert(it != _firstToSecond.end());
		return it->second;
	}

	First toFirst(const Second& second) const {
		typename SecondToFirst::const_iterator it = _secondToFirst.find(second);
		assert(it != _secondToFirst.end());
		return it->second;
	}

	bool empty() const {
		return _firstToSecond.empty();
	}

private:
	FirstToSecond _firstToSecond;
	SecondToFirst _secondToFirst;
};

#endif /* TWOWAYMAP_H */
