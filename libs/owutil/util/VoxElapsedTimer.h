//	VOXOX!!!

#ifndef _VOX_ELAPSED_TIMER_H
#define _VOX_ELAPSED_TIMER_H

#include <util/owutildll.h>
#include <ctime>

#ifdef _WIN32
#include <Windows.h>
#else
#endif

//-----------------------------------------------------------------------------

class OWUTIL_API VoxElapsedTimer
{
public:
	VoxElapsedTimer( bool bStart = true );

	void	Start();
	double	Stop();
	double	GetCurrSeconds();
	void	Restart();

	static double CalcSeconds( clock_t		  stStart, clock_t		  stEnd );

#ifdef _WIN32
	static double CalcSeconds( FILETIME&	  stStart, FILETIME&	  stEnd );
	static double CalcSeconds( SYSTEMTIME&	  stStart, SYSTEMTIME&	  stEnd );

	double			CalcSeconds( LARGE_INTEGER& liStart, LARGE_INTEGER& liEnd );	//We need m_liFreq
	LARGE_INTEGER	GetFreq();
#endif //_WIN32

private:
#ifdef _WIN32
	LARGE_INTEGER m_liStart;
	LARGE_INTEGER m_liEnd;

	LARGE_INTEGER m_liFreq;
#endif

	clock_t		m_stStart;
	clock_t		m_stEnd;
};

#endif //_VOX_ELAPSED_TIMER_H