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

#ifndef OWSIPADDRESS_H
#define OWSIPADDRESS_H

#include <util/String.h>

/**
 * SIP address parsing.
 *
 * A SIP URI is like: tanguy <sip:0170187873@voip.wengo.fr>
 * Separates the username/displayname from the rest of the SIP address.
 *
 * @ingroup model
 * @author Tanguy Krotoff
 */
class SipAddress {
public:

	SipAddress() {
	}

	/**
	 * Constructs a SipAddress given a unmodified complete string raw SIP address.
	 *
	 * @param rawSipAddress string raw SIP address
	 */
	SipAddress(const std::string & rawSipAddress);

	~SipAddress();

	/**
	 * Gets the unmodified SIP address.
	 *
	 * The unmodified SIP address from the SIP stack.
	 *
	 * @return SIP address
	 */
	const std::string & getRawSipAddress() const {
		return _rawSipAddress;
	}

	/**
	 * Gets the simplified SIP address.
	 *
	 * Example: sip:0170187873@voip.wengo.fr
	 * No display name + no < >
	 *
	 * @return SIP address
	 */
	const std::string & getSipAddress() const {
		return _sipAddress;
	}

	/**
	 * Gets the username part of the SIP address.
	 *
	 * Example: tanguy <sip:0170187873@voip.wengo.fr>
	 * username is 0170187873
	 *
	 * @return username
	 */
	const std::string & getUserName() const {
		return _userName;
	}

	/**
	 * Gets the diplay name part of the SIP address.
	 *
	 * Example: tanguy <sip:0170187873@voip.wengo.fr>
	 * display name is tanguy
	 *
	 * @return display name
	 */
	const std::string & getDisplayName() const {
		return _displayName;
	}

	/**
	 * Gets a human readable representation of this SIP address.
	 *
	 * @return display name or user name if display name is empty
	 */
	std::string toString() const;

	/**
	 * Creates a SipAddress from a unknown string.
	 *
	 * @param str unknown string that can be a SIP address or a phone number
	 * @param realm SIP server realm
	 * @return the SipAddress created from the given string
	 */
	static SipAddress fromString(const std::string & str, const std::string & realm);

private:

	/**
	 * Parses the from (SIP address) field.
	 *
	 * Example of a from field (phApi):
	 * "jwagner_office" <sip:0170187873@192.168.70.20;user=phone>;tag=00-01430
	 *
	 * @param sipAddress from field
	 */
	void parseSipAddress(const std::string & sipAddress);

	std::string _rawSipAddress;

	String _sipAddress;

	String _displayName;

	String _userName;
};

#endif	//SIPADDRESS_H
