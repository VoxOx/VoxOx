/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2007  Wengo
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

#include "stdafx.h"		//VOXOX - JRT - 2009.04.01
#include "WinApp.h"

#include <control/CWengoPhone.h>

#include <util/Logger.h>

#include <openssl/opensslv.h>	//VOXOX - JRT - 2009.06.03 
#include <openssl/rand.h>

#include <windows.h>
#include <dbt.h>		//VOXOX - JRT - 2009.04.19 - For DBT_DEVNODES_CHANGED

#define MY_WM_TERM WM_USER + 684
#define MY_WM_EXIT WM_USER + 685

WinApp::WinApp(int & argc, char ** argv) : QApplication(argc, argv) 
{
	initOpenSSL();	//VOXOX - JRT - 2009.06.03 - Improve OpenSSL initialization
}

bool WinApp::winEventFilter(MSG * msg, long * result) 
{
	//VOXOX - JRT - 2009.04.19 - Handle USB audio device changes.  Hold for now.
	//if ( msg->message == WM_DEVICECHANGE )
	//{
	//	return OnDeviceChange( msg->wParam, (long*)msg->lParam );
	//}
	//End VoxOx

	if (msg->message == (MY_WM_TERM)) 
	{
		if (_cWengoPhone) 
		{
			LOG_INFO("Disable serialization");
			_cWengoPhone->enableSerialization(false);
		}
		return true;

	} 
	else if (msg->message == (MY_WM_EXIT)) 
	{
		if (_cWengoPhone) 
		{
			LOG_INFO("Terminating");
			_cWengoPhone->enableSerialization(false);
			// "wait" for pending serialization to be finished
			Sleep(1000);
			exit(1);
		}
	}
	
	randomizeOpenSSL( msg );	//VOXOX - JRT - 2009.06.03 - Improve OpenSSL initialization.

	return false;
}

void WinApp::setCWengoPhone(CWengoPhone *cWengoPhone) 
{
	_cWengoPhone = cWengoPhone;
}

//VOXOX - JRT - 2009.06.03 - Improve OpenSSL initialization	
void WinApp::initOpenSSL()
{
	_randFileName = "";
	char buffer[MAX_PATH+1];

	_randFileName = RAND_file_name(buffer, sizeof(buffer) );

	RAND_load_file(_randFileName.c_str(), -1);
}

void WinApp::randomizeOpenSSL( MSG* msg )
{
	static bool s_seeded = false;
	if ( !s_seeded )
	{
		if ( RAND_event(msg->message, msg->wParam, msg->lParam ) == 1 )
		{
			s_seeded = true;

			if (RAND_write_file(_randFileName.c_str()) <= 0)	//If this fails, then OpenSSL init may take longer.
			{
				assert(false);
			}

			RAND_load_file(_randFileName.c_str(), -1);	//Init now!
		}
	}
}
//End VoxOx


//VOXOX - JRT - 2009.04.19 - Hold this for now.
//	Add device - 56 iterations
//	Remove device - 7 iterations.
//	All parameters are the same!
//bool WinApp::OnDeviceChange( unsigned int nEventType, long* dwData )
//{
//	static int s_nCount = 0;
//
//	s_nCount++;
//
//	char buf[200];
//	sprintf( buf, "%4d - OnDeviceChange() - EventType: %d, dwData; %d\n", s_nCount, nEventType, dwData );
//	OutputDebugString( buf );
//
//	if ( nEventType == DBT_DEVNODES_CHANGED )
//	{
//		//TODO - No need to check before user logs in
//		//	Check for Video
//		//	Check for Audio
//		//	Notify user if no audio device available?
//	}
//
//	return true;
//}
