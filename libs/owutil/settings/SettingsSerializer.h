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

#ifndef OWSETTINGSSERIALIZER_H
#define OWSETTINGSSERIALIZER_H

#include <settings/Settings.h>

#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/archive_exception.hpp>

class SettingsSerializer : NonCopyable {
public:

	static const unsigned int SERIALIZATION_VERSION = 1;

	OWSETTINGS_API SettingsSerializer(Settings & settings)
		: _settings(settings) {
	}

private:

	friend class boost::serialization::access;

	template < class Archive >
	void load(Archive & ar, const unsigned int version) {
		if (version == SERIALIZATION_VERSION) {
		}
	}

	template < class Archive >
	void save(Archive & ar, const unsigned int version) const {
		if (version == SERIALIZATION_VERSION) {
		}
	}

	BOOST_SERIALIZATION_SPLIT_MEMBER()

	Settings & settings;
};

BOOST_CLASS_VERSION(SettingsSerializer, SettingsSerializer::SERIALIZATION_VERSION)

#endif	//OWSETTINGSSERIALIZER_H
