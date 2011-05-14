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

#include <sipwrapper/EnumNatType.h>

#include <util/String.h>
#include <util/Logger.h>

#include <map>

typedef std::map<EnumNatType::NatType, std::string> NatTypeMap;
static NatTypeMap _natTypeMap;

static void init() {
	_natTypeMap[EnumNatType::NatTypeUnknown] = "NatTypeUnknown";
	_natTypeMap[EnumNatType::NatTypeOpen] = "NatTypeOpen";
	_natTypeMap[EnumNatType::NatTypeFullCone] = "NatTypeFullCone";
	_natTypeMap[EnumNatType::NatTypeRestrictedCone] = "NatTypeRestrictedCone";
	_natTypeMap[EnumNatType::NatTypePortRestrictedCone] = "NatTypePortRestrictedCone";
	_natTypeMap[EnumNatType::NatTypeSymmetric] = "NatTypeSymmetric";
	_natTypeMap[EnumNatType::NatTypeSymmetricFirewall] = "NatTypeSymmetricFirewall";
	_natTypeMap[EnumNatType::NatTypeBlocked] = "NatTypeBlocked";
	_natTypeMap[EnumNatType::NatTypeFailure] = "NatTypeFailure";
}

std::string EnumNatType::toString(NatType natType) {
	init();
	std::string tmp = _natTypeMap[natType];
	if (tmp.empty()) {
		LOG_FATAL("unknown NatType=" + String::fromNumber(natType));
	}
	return tmp;
}

EnumNatType::NatType EnumNatType::toNatType(const std::string & natType) {
	init();
	for (NatTypeMap::const_iterator it = _natTypeMap.begin();
		it != _natTypeMap.end(); ++it) {

		if ((*it).second == natType) {
			return (*it).first;
		}
	}

	LOG_FATAL("unknown NatType=" + natType);
	return NatTypeUnknown;
}
