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
#include "stdafx.h"	//VOXOX - JRT - 2009.04.01
#include "EnumToolBarMode.h"

// util
#include <util/TwoWayMap.h>

static TwoWayMap<EnumToolBarMode::ToolBarMode, std::string> _twoWayMap;

static void init() {
	if (!_twoWayMap.empty()) {
		return;
	}

	_twoWayMap.addEntry(EnumToolBarMode::ToolBarModeHidden, "Hidden");
	_twoWayMap.addEntry(EnumToolBarMode::ToolBarModeIconsOnly, "IconsOnly");
	_twoWayMap.addEntry(EnumToolBarMode::ToolBarModeTextUnderIcons, "TextUnderIcons");
}

std::string EnumToolBarMode::toString(EnumToolBarMode::ToolBarMode mode) {
	init();
	return _twoWayMap.toSecond(mode);
}

EnumToolBarMode::ToolBarMode EnumToolBarMode::toToolBarMode(const std::string & txt) {
	init();
	return _twoWayMap.toFirst(txt);
}
