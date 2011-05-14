//  VOXOX!!!

#ifndef _VOX_API_TIMER_H
#define _VOX_API_TIMER_H

#include <util/owutildll.h>
#include <util/VoxElapsedTimer.h>
#include <string>

//-----------------------------------------------------------------------------

class OWUTIL_API VoxApiTimer
{
	//NOTES: _logStart == true is useful for detecting calls that do NOT return.  Otherwise it is just extra diatrus in the logs.
	//		So in general, turn it off.  If you suspect a hang or non-returning call, then enable it.
public:
	VoxApiTimer( const std::string& strCallingMethod, const std::string& strApiCall, bool bLogStart );

	void Start( const std::string& strSpec = "" );
	void Stop ( const std::string& strSpec = "" );
	
	double GetCurrentSeconds()							{ return _et.GetCurrSeconds();	}


protected:
	void LogIt( const std::string& msg );

private:
	std::string			_strApiCall;
	std::string			_strMethod;
	VoxElapsedTimer		_et;
	double				_nApiTime;
	bool				_logStart;
};

#endif //_VOX_API_TIMER_H
