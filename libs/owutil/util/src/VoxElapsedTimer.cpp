//  VOXOX !!!

//#include "stdafx.h"
#include <util/VoxElapsedTimer.h>
#include <assert.h>

//-----------------------------------------------------------------------------

VoxElapsedTimer::VoxElapsedTimer( bool bStart )
{
#ifdef _WIN32
	m_liStart.LowPart  = 0;
	m_liStart.HighPart = 0;

	m_liEnd.LowPart    = 0;
	m_liEnd.HighPart   = 0;

	m_liFreq.LowPart   = 0;
	m_liFreq.HighPart  = 0;
#endif

	if ( bStart )
		Start();
}

//-----------------------------------------------------------------------------

void VoxElapsedTimer::Start()
{
#ifdef _WIN32
	QueryPerformanceCounter( &m_liStart );
#else
	m_stStart = clock();
#endif
}

//-----------------------------------------------------------------------------

double VoxElapsedTimer::Stop()
{
#ifdef _WIN32
	QueryPerformanceCounter( &m_liEnd );
	return CalcSeconds( m_liStart, m_liEnd );
#else
	m_stEnd = clock();
	return CalcSeconds( m_stStart, m_stEnd );
#endif

}

//-----------------------------------------------------------------------------

double VoxElapsedTimer::GetCurrSeconds()
{
#ifdef _WIN32
	LARGE_INTEGER liNow;
	QueryPerformanceCounter( &liNow );
	return CalcSeconds( m_liStart, liNow );
#else
	clock_t stNow;
	stNow = clock();
	return CalcSeconds( m_stStart, stNow );
#endif
}

//-----------------------------------------------------------------------------

void VoxElapsedTimer::Restart()
{
	Stop();
	Start();
}

//-----------------------------------------------------------------------------

double VoxElapsedTimer::CalcSeconds( clock_t stStart, clock_t stEnd )
{
	double	nSecs  = (double)((stEnd - stStart)) / (double)CLOCKS_PER_SEC;	//Defined in <time.h>

	return nSecs;
}

//-----------------------------------------------------------------------------
#ifdef _WIN32
//static
LARGE_INTEGER VoxElapsedTimer::GetFreq()
{
	if ( m_liFreq.QuadPart == 0 )
	{
		QueryPerformanceFrequency(&m_liFreq);
	}

	return m_liFreq;
}

//-----------------------------------------------------------------------------

double VoxElapsedTimer::CalcSeconds( FILETIME& ftStart, FILETIME& ftEnd )
{
	double	nSecs  = 0;

	ULARGE_INTEGER ulStart;
	ULARGE_INTEGER ulEnd;

	memcpy( &ulStart, &ftStart, sizeof(ftStart ) );
	memcpy( &ulEnd,   &ftEnd,   sizeof(ftEnd   ) );

	ULONGLONG nDiff = ulEnd.QuadPart - ulStart.QuadPart;

	nSecs  = ((double)nDiff / 10000000.0);

	return nSecs;
}

//-----------------------------------------------------------------------------

double VoxElapsedTimer::CalcSeconds( LARGE_INTEGER& liStart, LARGE_INTEGER& liEnd )
{
	double	nSecs  = 0;

	LARGE_INTEGER liFreq = GetFreq();

	if ( liFreq.QuadPart > 0 )
	{
		nSecs = (double) (liEnd.QuadPart - liStart.QuadPart) / (double)liFreq.QuadPart;
	}

	return nSecs;
}

//-----------------------------------------------------------------------------

double VoxElapsedTimer::CalcSeconds( SYSTEMTIME& stStart, SYSTEMTIME& stEnd )
{
	assert( stStart.wYear != 0 );
	assert( stEnd.wYear   != 0 );

	bool	bValid = false;
	double	nSecs  = 0;

	FILETIME ftStart;
	FILETIME ftEnd;

	if ( SystemTimeToFileTime( &stStart, &ftStart ) )
	{
		if ( SystemTimeToFileTime( &stEnd, &ftEnd ) )
		{
			nSecs  = CalcSeconds( ftStart, ftEnd );
			bValid = true;
		}
	}

	assert( bValid );

	return nSecs;
}
#endif
//-----------------------------------------------------------------------------
