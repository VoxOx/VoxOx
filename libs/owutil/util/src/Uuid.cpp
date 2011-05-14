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
#include <util/Uuid.h>

#include <util/String.h>

#include <ctime>

int Uuid::generateInteger() {
	//Under GNU systems
	//int = 32 bits from -2,147,483,647 to 2,147,483,647

	//Number of seconds since 1970
	int seconds = time(NULL);

	//Until 10 id generated per second
	static int seed = 0;
	String tmp = String::fromNumber(seconds) + String::fromNumber(seed++ % 10);
	String date;

	//Removes the first digit
	for (unsigned i = 1; i < tmp.length(); i++) {
		date += tmp[i];
	}

	int id = date.toInteger();
	return id;
}

