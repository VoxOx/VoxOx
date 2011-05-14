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

#include <util/Startup.h>

#include <Cocoa/Cocoa.h>

Startup::Startup(const std::string & applicationName, const std::string & executablePath)
	: _applicationName(applicationName),
	_executablePath(executablePath) {
}

Startup::~Startup() {
}

bool Startup::setStartup(bool startup) {
	NSAutoreleasePool *pool = [NSAutoreleasePool new];

	NSUserDefaults * loginWindowDefaults = [NSUserDefaults new];
	[loginWindowDefaults addSuiteNamed:@"loginwindow"];

	NSMutableDictionary *loginWindowPrefs = 
		[[[loginWindowDefaults persistentDomainForName:@"loginwindow"] mutableCopy] autorelease];
	NSArray *loginItems = [loginWindowPrefs objectForKey:@"AutoLaunchedApplicationDictionary"];
	NSMutableArray *mutableLoginItems = [[loginItems mutableCopy] autorelease];
	NSEnumerator *e = [loginItems objectEnumerator];
	NSDictionary *item = nil;

	//Removing trailing '/' if it exists
	NSString *tmpPath = [[NSString alloc] initWithCString:_executablePath.c_str()];
	NSString *appPath = [tmpPath stringByStandardizingPath];
	[tmpPath release];
	////

	// The startup item must be removed every time method is called,
	// otherwhise there will be duplicate entries in the Startup items.
	while ((item = [e nextObject])) {
		if ([[[item objectForKey:@"Path"] stringByExpandingTildeInPath] isEqualToString:appPath]) {
			[mutableLoginItems removeObject:item];
		}
	}

	if (startup) {
		NSMutableDictionary *launchDict = [NSMutableDictionary dictionary];
		[launchDict setObject:[NSNumber numberWithBool:NO] forKey:@"Hide"];
		[launchDict setObject:appPath forKey:@"Path"];
		[mutableLoginItems addObject:launchDict];
	}

	[loginWindowPrefs setObject:[NSArray arrayWithArray:mutableLoginItems]
		forKey:@"AutoLaunchedApplicationDictionary"];
	[loginWindowDefaults setPersistentDomain:[NSDictionary dictionaryWithDictionary:loginWindowPrefs]
		forName:@"loginwindow"];
	[loginWindowDefaults synchronize];

	[loginWindowDefaults release];
	[pool release];

	return true;
}

bool Startup::isStartup() {
	NSAutoreleasePool *pool = [NSAutoreleasePool new];

	NSUserDefaults * loginWindowDefaults = [NSUserDefaults new];
	[loginWindowDefaults addSuiteNamed:@"loginwindow"];

	NSMutableDictionary *loginWindowPrefs = [[[loginWindowDefaults persistentDomainForName:@"loginwindow"] mutableCopy] autorelease];
	NSArray *loginItems = [loginWindowPrefs objectForKey:@"AutoLaunchedApplicationDictionary"];
	NSEnumerator *e = [loginItems objectEnumerator];
	NSDictionary *item = nil;
	bool result = false;

	//Removing trailing '/' if it exists
	NSString *tmpPath = [[NSString alloc] initWithCString:_executablePath.c_str()];
	NSString *appPath = [tmpPath stringByStandardizingPath];
	[tmpPath release];
	////

	while ((item = [e nextObject])) {
		if ([[[item objectForKey:@"Path"] stringByExpandingTildeInPath] isEqualToString:appPath]) {
			result = true;
			break;
		}
	}

	[loginWindowDefaults release];
	[pool release];

	return result;
}
