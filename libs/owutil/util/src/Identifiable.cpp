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

//#include "stdafx.h"		//VOXOX - JRT - 2009.04.03 - For precompiled headers.
#include <util/Identifiable.h>

#include <util/Uuid.h>

Identifiable::Identifiable() {
	_uuid = Uuid::generateString();
}

Identifiable::Identifiable(const Identifiable & identifieable) {
	copy(identifieable);
}

Identifiable * Identifiable::clone() const {
	return new Identifiable(*this);
}

bool Identifiable::operator == (const Identifiable & identifieable) const {
	return _uuid == identifieable._uuid;
}

std::string Identifiable::getUUID() const {
	return _uuid;
}

void Identifiable::setUUID(const std::string & uuid) {
	_uuid = uuid;
}

void Identifiable::copy(const Identifiable & identifieable) {
	_uuid = identifieable._uuid;
}
