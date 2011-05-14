//  VOXOX!!!

#include <util/VoxApiTimer.h>
#include <util/Logger.h>

//-----------------------------------------------------------------------------

VoxApiTimer::VoxApiTimer( const std::string& strCallingMethod, const std::string& strApiCall, bool bLogStart )
{
	_strMethod  = strCallingMethod;
	_strApiCall = strApiCall;
	_logStart   = bLogStart;	//False will suppress logging start entry
}

//-----------------------------------------------------------------------------

void VoxApiTimer::Start( const std::string& strSpec )
{
	std::string strLog;

	strLog = _strMethod + " - Entering API " + _strApiCall;

	if ( !strSpec.empty()  )
	{
		strLog += " ";
		strLog += strSpec;
	}

	if ( _logStart )
	{
		LogIt( strLog );
	}

	_et.Start();
}

//-----------------------------------------------------------------------------

void VoxApiTimer::Stop( const std::string& strSpec )
{
	_nApiTime = _et.Stop();

	std::string strLog;
		
	strLog = _strMethod + " - Returned from API " + _strApiCall + ".  Call took " + String::fromDouble(_nApiTime ) + " seconds.";

	if ( !strSpec.empty() )
	{
		strLog += " ";
		strLog += strSpec;
	}

	LogIt( strLog );
}

//-----------------------------------------------------------------------------

void VoxApiTimer::LogIt( const std::string& msg )
{
	LOG_INFO( msg );

#ifdef _WIN32
	OutputDebugString( msg.c_str() );
	OutputDebugString( "\n" );
#endif
	
}

//-----------------------------------------------------------------------------