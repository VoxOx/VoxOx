//Telcentris copyright

#include "stdafx.h"	//VOXOX - JRT - 2009.04.01
#include "EnumVisibility.h"
#include "assert.h"		//TODO remove one logger is enabled.

//#include <util/Logger.h>

using namespace std;

string EnumVisibility::toString(EnumVisibility::Visibility vis) {
	string result;

	//JRT - 2009.03.25 - TODO: Localize
	switch (vis) {
	case VisibilityUnknown:
		result = "unknown";
		break;
	
	case VisibilityPrivate:
		result = "private";
		break;

	case VisibilityProtected:
		result = "protected";
		break;

	case VisibilityPublic:
		result = "public";
		break;
	default:
//		LOG_FATAL("unknown case");	//TODO
		assert(false);
	}

	return result;
}

//-----------------------------------------------------------------------------

EnumVisibility::Visibility EnumVisibility::toVisibility(const string & vis) 
{
	if		(vis == "private") 
	{
		return VisibilityPrivate;
	} 
	else if (vis == "protected") 
	{
		return VisibilityProtected;
	} 
	else if ( vis == "public" ) 
	{
		return VisibilityPublic;
	} 
	else 
	{
		return VisibilityUnknown;
	}
}

//-----------------------------------------------------------------------------
