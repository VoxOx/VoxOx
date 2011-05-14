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

#ifndef OWENUMNATTYPE_H
#define OWENUMNATTYPE_H

#include <util/NonCopyable.h>

#include <string>

/**
 * NAT types from STUN protocol.
 *
 * @see http://en.wikipedia.org/wiki/STUN
 * @see http://en.wikipedia.org/wiki/Network_address_translation
 * @see http://www.ietf.org/rfc/rfc3489.txt
 * @author Tanguy Krotoff
 */
class EnumNatType : NonCopyable {
public:

	enum NatType {

		/** Unknown NAT type. */
		NatTypeUnknown,

		/** No NAT, direct connection. */
		NatTypeOpen,

		/**
		 * Full cone NAT.
		 *
		 * All requests from the same internal IP address
		 * and port are mapped to the same external IP address and port.
		 * Furthermore, any external host can send a packet to the internal host,
		 * by sending a packet to the mapped external address. It is also known as "one-to-one NAT".
		 */
		NatTypeFullCone,

		/**
		 * Restricted cone NAT.
		 *
		 * All requests from the same internal IP address
		 * and port are mapped to the same external IP address and port.
		 * Unlike a full cone NAT, an external host (with IP address X)
		 * can send a packet to the internal host only if the internal
		 * host had previously sent a packet to IP address X.
		 */
		NatTypeRestrictedCone,

		/**
		 * Port restricted cone NAT.
		 *
		 * Like a restricted cone NAT, but the restriction includes
		 * port numbers. Specifically, an external host can send a packet,
		 * with source IP address X and source port P, to the internal host
		 * only if the internal host had previously sent a packet
		 * to IP address X and port P.
		 */
		NatTypePortRestrictedCone,

		/**
		 * Symmetric NAT.
		 *
		 * All requests from the same internal IP address and port,
		 * to a specific destination IP address and port,
		 * are mapped to the same external IP address and
		 * port. If the same host sends a packet with the same source
		 * address and port, but to a different destination, a different
		 * mapping is used. Furthermore, only the external host that
		 * receives a packet can send a UDP packet back to the internal host.
		 */
		NatTypeSymmetric,

		/**
		 * FIXME to remove or not in the future.
		 *
		 * No idea what is it, check netlib
		 */
		NatTypeSymmetricFirewall,

		/** Cannot pass the network. */
		NatTypeBlocked,

		/** Failed to detect the NAT type. */
		NatTypeFailure
	};

	/**
	 * Converts a NatType into a string.
	 *
	 * @return the string
	 */
	static std::string toString(NatType natType);

	/**
	 * Converts a string into a NatType.
	 *
	 * @return the NatType
	 */
	static NatType toNatType(const std::string & natType);
};

#endif	//OWENUMNATTYPE_H
