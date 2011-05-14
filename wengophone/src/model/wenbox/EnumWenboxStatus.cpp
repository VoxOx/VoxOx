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

#include "stdafx.h"	//VOXOX - JRT - 2009.04.01
#include "EnumWenboxStatus.h"

#include <util/String.h>
#include <util/Logger.h>

#include <map>

typedef std::map<EnumWenboxStatus::WenboxStatus, std::string> WenboxStatusMap;
static WenboxStatusMap _wenboxStatusMap;

static void init() {
	_wenboxStatusMap[EnumWenboxStatus::WenboxStatusEnable] = "WenboxStatusEnable";
	_wenboxStatusMap[EnumWenboxStatus::WenboxStatusDisable] = "WenboxStatusDisable";
	_wenboxStatusMap[EnumWenboxStatus::WenboxStatusNotConnected] = "WenboxStatusNotConnected";
}

std::string EnumWenboxStatus::toString(WenboxStatus wenboxStatus) {
	init();
	std::string tmp = _wenboxStatusMap[wenboxStatus];
	if (tmp.empty()) {
		LOG_FATAL("unknown WenboxStatus=" + String::fromNumber(wenboxStatus));
	}
	return tmp;
}

EnumWenboxStatus::WenboxStatus EnumWenboxStatus::toWenboxStatus(const std::string & wenboxStatus) {
	init();
	for (WenboxStatusMap::const_iterator it = _wenboxStatusMap.begin();
		it != _wenboxStatusMap.end(); ++it) {

		if ((*it).second == wenboxStatus) {
			return (*it).first;
		}
	}

	//LOG_FATAL("unknown WenboxStatus=" + wenboxStatus);
	return WenboxStatusNotConnected;
}
