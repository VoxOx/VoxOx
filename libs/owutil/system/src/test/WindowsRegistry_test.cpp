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

#include <system/WindowsRegistry.h>

#include <stdio.h>

int main(int argc, char **argv) {

	if (WindowsRegistry::keyExists(HKEY_CURRENT_USER, "Software\\Microsoft")) {
		printf("WindowsRegistry::keyExists: ok\n");
	} else {
		printf("WindowsRegistry::keyExists: failed\n");
	}

	if (WindowsRegistry::entryExists(HKEY_CURRENT_USER, "Software\\Microsoft\\Clock", "iFormat")) {
		printf("WindowsRegistry::entryExists: ok\n");
	} else {
		printf("WindowsRegistry::entryExists: failed\n");
	}

	if (WindowsRegistry::createKey(HKEY_CURRENT_USER, "test", "pipo")) {
		printf("WindowsRegistry::createKey: ok\n");
	} else {
		printf("WindowsRegistry::createKey: failed\n");
	}

	if (WindowsRegistry::deleteKey(HKEY_CURRENT_USER, "test", "pipo")) {
		printf("WindowsRegistry::deleteKey: ok\n");
	} else {
		printf("WindowsRegistry::deleteKey: failed\n");
	}

	if (WindowsRegistry::createEntry(HKEY_CURRENT_USER, "test\\zorro", "pipo", "toto")) {
		printf("WindowsRegistry::createEntry: ok\n");
	} else {
		printf("WindowsRegistry::createEntry: failed\n");
	}

	if (WindowsRegistry::removeEntry(HKEY_CURRENT_USER, "test\\zorro", "pipo")) {
		printf("WindowsRegistry::removeEntry: ok\n");
	} else {
		printf("WindowsRegistry::removeEntry: failed\n");
	}

	if (WindowsRegistry::entryExists(HKEY_CLASSES_ROOT, "http", "")) {
		printf("http:// is a registered protocol\n");
		printf("protocol description: %s\n", WindowsRegistry::getValue(HKEY_CLASSES_ROOT, "http", "").c_str() );
	} else {
		printf("http:// is not a registered protocol\n");
	}

	return 0;
}
