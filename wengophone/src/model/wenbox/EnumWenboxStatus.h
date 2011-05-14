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

#ifndef OWENUMWENBOXSTATUS_H
#define OWENUMWENBOXSTATUS_H

#include <util/NonCopyable.h>

#include <string>

/**
 * The Wenbox has 3 states: enable, disable, no Wenbox connected.
 *
 * @author Tanguy Krotoff
 */
class EnumWenboxStatus : NonCopyable {
public:

	enum WenboxStatus {
		/**
		 * Wenbox connected and enabled.
		 */
		WenboxStatusEnable,

		/**
		 * Wenbox connected but disabled.
		 */
		WenboxStatusDisable,

		/**
		 * Wenbox not connected (not found).
		 */
		WenboxStatusNotConnected
	};

	/**
	 * Converts a WenboxStatus into a string.
	 *
	 * @return the string
	 */
	static std::string toString(WenboxStatus wenboxStatus);

	/**
	 * Converts a string into a WenboxStatus.
	 *
	 * @return the WenboxStatus
	 */
	static WenboxStatus toWenboxStatus(const std::string & wenboxStatus);
};

#endif	//OWENUMWENBOXSTATUS_H
