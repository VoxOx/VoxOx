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

#include "dlltest.h"

#include <Logger.h>
#include <StringList.h>

void function_noparam() {
	LOG_DEBUG("function_noparam");
}

void function_param_int(int i) {
	LOG_DEBUG("function_param_int=" + String::fromNumber(i));
}

int function_return_int() {
	int i = 10;
	LOG_DEBUG("function_return_int=" + String::fromNumber(i));
	return i;
}

int * function_return_pointer() {
	static int i = 10;
	LOG_DEBUG("function_return_int=" + String::fromNumber(i));
	return &i;
}
