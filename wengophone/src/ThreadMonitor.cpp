//
// Copyright (c) 2005-2006 SIPphone, Inc. All Rights Reserved.
// 9333 Genesse Ave, Suite 250
// San Diego, CA  92121
// http://www.sipphone.com
//
// This source code and specific concepts contained herein are confidential
// information and property of SIPphone, Inc. Distribution is prohibited
// without written permission.
//
// $Id: ThreadMonitor.cpp,v 1.4 2008/04/10 08:24:58 jtheinert Exp $ 
//
//#include "stdafx.h"
#include <windows.h>
#include "ThreadMonitor.h"
#include <tlhelp32.h>	//THREADENTRY32

#define _T(x) x
#define TRACE(x) OutputDebugString( x )


//-----------------------------------------------------------------------------

DWORD WINAPI StartThreadMonitorProc( LPVOID pParam )
{
	CThreadMonitor* pThread = reinterpret_cast<CThreadMonitor*>(pParam);

	pThread->Run();

	CloseHandle( GetCurrentThread() );

	return 0;
}

//=============================================================================


//=============================================================================

CThreadInfo::CThreadInfo()
{
	Init();
}

//-----------------------------------------------------------------------------

void CThreadInfo::Init()
{
//	FILETIME stNow;
//	GetLocalTime(&stNow);

	SetIsValid    ( false );
	SetThreadID    ( 0 );
//	SetCreationTime( stNow );
//	SetExitTime    ( stNow );
//	SetKernelTime  ( stNow );
//	SetUserTime	   ( stNow );

	SetKernelDiff ( 0 );
	SetUserDiff   ( 0 );
}

//-----------------------------------------------------------------------------

CThreadInfo& CThreadInfo::operator=( CThreadInfo& src )
{
	if ( this == &src )
		return *this;

	SetIsValid     ( src.IsValid()			);
	SetThreadID    ( src.GetThreadID()		);
	SetCreationTime( src.GetCreationTime()	);
	SetExitTime    ( src.GetExitTime()		);
	SetKernelTime  ( src.GetKernelTime()	);
	SetUserTime	   ( src.GetUserTime()		);

	SetKernelDiff  ( src.GetKernelDiff()	);
	SetUserDiff    ( src.GetUserDiff()		);

	return *this;
}

//----------------------------------------------------------------------------

bool CThreadInfo::CalcDiff( CThreadInfo& rStart, CThreadInfo& rEnd )
{
	bool bValid = false;

	if ( rStart.IsValid() && rEnd.IsValid() )
	{
		ULARGE_INTEGER	ulStart;
		ULARGE_INTEGER	ulEnd;
		ULONGLONG		nDiff = 0;

		//Kernel Time
		memcpy( &ulStart, &(rStart.GetKernelTime()), sizeof(FILETIME) );
		memcpy( &ulEnd,   &(rEnd.GetKernelTime()),   sizeof(FILETIME) );

		nDiff = ulEnd.QuadPart - ulStart.QuadPart;
		SetKernelDiff  ( nDiff );

		//User Time
		memcpy( &ulStart, &(rStart.GetUserTime()), sizeof(FILETIME) );
		memcpy( &ulEnd,   &(rEnd.GetUserTime()),   sizeof(FILETIME) );

		nDiff = ulEnd.QuadPart - ulStart.QuadPart;
		SetUserDiff  ( nDiff );

		bValid = true;
	}

	return bValid;
}

//----------------------------------------------------------------------------

ULONGLONG CThreadInfo::GetTotalTime()
{ 
	ULONGLONG nTime = 0;

	if ( IsValid() )
	{
		ULARGE_INTEGER	ulKernel;
		ULARGE_INTEGER	ulUser;

		memcpy( &ulKernel, &(GetKernelTime()), sizeof(FILETIME) );
		memcpy( &ulUser,   &(GetUserTime()  ), sizeof(FILETIME) );

		nTime = ulKernel.QuadPart + ulUser.QuadPart;
	}

	return nTime;
}

//----------------------------------------------------------------------------
//static 
CThreadInfo* CThreadInfo::Make( DWORD dwThreadID, FILETIME ftCreation, FILETIME ftExit,
												  FILETIME ftKernel,   FILETIME ftUser )
{
	CThreadInfo* pNew = new CThreadInfo;

	pNew->SetThreadID( dwThreadID );
	pNew->SetCreationTime( ftCreation );
	pNew->SetExitTime    ( ftExit     );
	pNew->SetKernelTime  ( ftKernel   );
	pNew->SetUserTime    ( ftUser     );

	pNew->SetIsValid( true );

	return pNew;
}

//============================================================================



//=============================================================================
	
CThreadInfoDiff::CThreadInfoDiff()
{
	Init();
}

//-----------------------------------------------------------------------------

void CThreadInfoDiff::Init()
{
	m_CurrInfo.Init();
	m_PrevInfo.Init();
}

//-----------------------------------------------------------------------------

CThreadInfoDiff& CThreadInfoDiff::operator=( CThreadInfoDiff& src )
{
	if ( this == &src )
		return *this;

	SetCurrInfo( src.GetCurrInfo() );
	SetPrevInfo( src.GetPrevInfo() );

	return *this;
}

//-----------------------------------------------------------------------------

void CThreadInfoDiff::Update( CThreadInfo& rInfo )
{
	if ( GetCurrInfo().IsValid() )
	{
		SetPrevInfo( GetCurrInfo() );
		SetCurrInfo( rInfo         );

		GetCurrInfo().CalcDiff( GetPrevInfo(), GetCurrInfo() );
	}
}

//=============================================================================



//=============================================================================

CThreadInfoDiffList::CThreadInfoDiffList()
{
//	SetDesc( _T("CThreadInfoDiffList") );
}

//-----------------------------------------------------------------------------

CThreadInfoDiff* CThreadInfoDiffList::Update( CThreadInfo& rInfo )
{
//	ASSERT( rInfo.IsValid() );

	CThreadInfoDiff* pDiff = FindByThreadID( rInfo.GetThreadID() );

	if ( pDiff )
	{
//		ASSERT( rInfo.GetThreadID() == pDiff->GetCurrInfo().GetThreadID() );
		pDiff->Update( rInfo );
	}
	else
	{
		pDiff = new CThreadInfoDiff;
		pDiff->SetCurrInfo( rInfo );
		Add( pDiff );
	}

	pDiff->SetIsValid( true );

	return pDiff;
}

//-----------------------------------------------------------------------------

CThreadInfoDiff* CThreadInfoDiffList::FindByThreadID( DWORD nID )
{
	CThreadInfoDiff* pRet  = NULL;

	for ( iterator it = begin(); it != end(); it++ )
	{
		if ( (*it).GetCurrInfo().GetThreadID() == nID )
		{
			pRet = &(*it);
			break;
		}
	}

	return pRet;
}

//-----------------------------------------------------------------------------

CThreadInfoDiff* CThreadInfoDiffList::FindHighestUsage()
{
	ULONGLONG		 nHigh = 0;
	CThreadInfoDiff* pRet  = NULL;

	for ( iterator it = begin(); it != end(); it++ )
	{
		if ( (*it).GetCurrInfo().GetTotalTime() > nHigh )
		{
			pRet  = &(*it);
			nHigh = (*it).GetCurrInfo().GetTotalTime();
		}
	}
//	while ( pTemp )
//	{
//		if ( pTemp->GetCurrInfo().GetTotalTime() > nHigh )
//		{
//			pRet = pTemp;
//			nHigh = pTemp->GetCurrInfo().GetTotalTime();
//		}
//
//		pTemp = GetNext();
//	}

	return pRet;
}

//-----------------------------------------------------------------------------

void CThreadInfoDiffList::InvalidateAll()
{
	for ( iterator it = begin(); it != end(); it++ )
	{
		(*it).SetIsValid( false );
	}
}

//-----------------------------------------------------------------------------

UINT CThreadInfoDiffList::RemoveInvalid()
{
//	CString					strText;
	char					strText[500];
	UINT					nCount = 0;
//	CThreadInfoDiffListIter iter   = Iterator();
//	CThreadInfoDiff*		pTemp  = iter.GetFirst();

	for ( iterator it = begin(); it != end(); it++ )
	{
		if ( ! (*it).IsValid() )
		{
//			strText.Format( _T("   Removing ThreadID: %4d, Time: %10d, Count: %2d xxx"), 
//							pTemp->GetCurrInfo().GetThreadID(), pTemp->GetCurrInfo().GetTotalTime(), GetCount() );
			sprintf( strText, _T("   Removing ThreadID: %4d, Time: %10d, Count: %2d xxx"), 
							(*it).GetCurrInfo().GetThreadID(), (*it).GetCurrInfo().GetTotalTime(), GetCount() );
			TRACE( strText );

			erase( it );

//			strText.Format( _T(", Left: %2d \n"), GetCount() );
			sprintf( strText, _T(", Left: %2d \n"), GetCount() );
			TRACE( strText );

			nCount++;
			it = begin();	//Restart at top.
		}
	}

	return nCount;
}

//=============================================================================



//=============================================================================

CThreadMonitor::CThreadMonitor()
{
	Init();
}

//-----------------------------------------------------------------------------

void CThreadMonitor::Init()
{
	m_dwPID     =    0;
	m_nInterval = 5000;	//1 second
	m_bRunning	= false;
}

//-----------------------------------------------------------------------------

void CThreadMonitor::Start( DWORD dwPid )
{
	if ( dwPid == 0 )
		m_dwPID = ::GetCurrentProcessId();
	else
		m_dwPID	= dwPid;

	m_bRunning = true;

	DWORD  dw = 0;
	HANDLE hThread = CreateThread( NULL, 0, StartThreadMonitorProc, (void*)this, 0, &dw );
	CloseHandle( hThread );
}

//-----------------------------------------------------------------------------

void CThreadMonitor::Stop()
{
	m_bRunning = false;
}

//-----------------------------------------------------------------------------

void CThreadMonitor::Run()
{
	while ( m_bRunning )
	{
		ListProcessThreads( m_dwPID );

		Sleep( m_nInterval );
	}
}

//-----------------------------------------------------------------------------

bool CThreadMonitor::ListProcessThreads( DWORD dwOwnerPID ) 
{ 
	HANDLE hThreadSnap = INVALID_HANDLE_VALUE; 
	THREADENTRY32 te32; 

	// Take a snapshot of all running threads  
	hThreadSnap = CreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, 0 ); 
	
	if( hThreadSnap == INVALID_HANDLE_VALUE ) 
		return( FALSE ); 

	// Fill in the size of the structure before using it. 
	te32.dwSize = sizeof(THREADENTRY32 ); 

	// Retrieve information about the first thread, and exit if unsuccessful
	if( !Thread32First( hThreadSnap, &te32 ) ) 
	{
		HandleError( _T("Thread32First") );  // Show cause of failure
		CloseHandle( hThreadSnap );     // Must clean up the snapshot object!
		return( FALSE );
	}

	//Invalid all existing so we can remove exited threads
	m_InfoList.InvalidateAll();

	// Now walk the thread list of the system, and display information 
	//about each thread associated with the specified process
	CThreadInfo* pInfo = NULL;
	FILETIME ftCreation;
	FILETIME ftExit;
	FILETIME ftKernel;
	FILETIME ftUser;

	HANDLE	hThread  = NULL;
	BOOL	bSuccess = false;
	DWORD	dwError  = 0;

	do 
	{ 
		if( te32.th32OwnerProcessID == dwOwnerPID )
		{
			hThread = OpenThread( THREAD_QUERY_INFORMATION , FALSE, te32.th32ThreadID);

			if ( hThread )
			{
				bSuccess = ::GetThreadTimes( hThread, &ftCreation, &ftExit, &ftKernel, &ftUser );
				CloseHandle( hThread );

				if ( bSuccess )
				{
					pInfo = CThreadInfo::Make( te32.th32ThreadID, ftCreation, ftExit, ftKernel, ftUser );
					m_InfoList.Update( *pInfo );
					delete pInfo;
				}
				else
				{
					dwError = GetLastError();
				}
			}
		}
	} while( Thread32Next(hThreadSnap, &te32 ) ); 

	CloseHandle( hThreadSnap );

	DisplayResults();

	return( TRUE );
}

//-----------------------------------------------------------------------------

void CThreadMonitor::DisplayResults()
{
//	CString strText;
	char	strText[500];
	UINT    nDeleted = 0;

	//Reported deleted count.
	nDeleted = m_InfoList.RemoveInvalid();

//	strText.Format( _T("  Removed Threads: %4d\n"), nDeleted );
	sprintf( strText, _T("  Removed Threads: %4d\n"), nDeleted );
	TRACE( strText );

	//Dump list.
	for ( CThreadInfoDiffList::iterator it = m_InfoList.begin(); it != m_InfoList.end(); it++ )
	{
//		if ( pDiff->GetPrevInfo().GetThreadID() != 0 )
//			ASSERT( pDiff->GetCurrInfo().GetThreadID() == pDiff->GetPrevInfo().GetThreadID() );

		if ( (*it).GetCurrInfo().GetTotalTime() > 0 )
		{
//			strText.Format( _T("   ThreadID: %4d, Time: %10d \n"),pDiff->GetCurrInfo().GetThreadID(), pDiff->GetCurrInfo().GetTotalTime() );
			sprintf( strText, _T("   ThreadID: %4d, Time: %10d \n"), (*it).GetCurrInfo().GetThreadID(), (*it).GetCurrInfo().GetTotalTime() );
			TRACE( strText );
		}
	}


	//Display highest usage.
	CThreadInfoDiff* pDiff = m_InfoList.FindHighestUsage();

	if ( pDiff )
	{
//		strText.Format( _T("Thread Count: %4d, High Usage Thread - ID: %4d, Time: %10d \n"), 
//						m_InfoList.GetCount(), pDiff->GetCurrInfo().GetThreadID(), pDiff->GetCurrInfo().GetTotalTime() );
		sprintf( strText, _T("Thread Count: %4d, High Usage Thread - ID: %4d, Time: %10d \n"), 
						m_InfoList.GetCount(), pDiff->GetCurrInfo().GetThreadID(), pDiff->GetCurrInfo().GetTotalTime() );
		TRACE( strText );
	}
}

//-----------------------------------------------------------------------------

void CThreadMonitor::HandleError( TCHAR* pMsg )
{
	DWORD eNum;
	TCHAR sysMsg[256];
	TCHAR* p;

	eNum = GetLastError();
	FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL, eNum, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
					sysMsg, 256, NULL );

	// Trim the end of the line and terminate it with a null
	p = sysMsg;

	while( ( *p > 31 ) || ( *p == 9 ) )
		++p;

	do 
	{ *p-- = 0; 
	} while( ( p >= sysMsg ) && ( ( *p == '.' ) || ( *p < 33 ) ) );

	// Display the message
	printf( "\n  WARNING: %s failed with error %d (%s)", pMsg, eNum, sysMsg );
}

//-----------------------------------------------------------------------------

//BOOL CThreadMonitor::ListProcessModules( DWORD dwPID )
//{
//	HANDLE		  hModuleSnap = INVALID_HANDLE_VALUE;
//	MODULEENTRY32 me32;
//
//	// Take a snapshot of all modules in the specified process.
//	hModuleSnap = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE, dwPID );
//
//	if( hModuleSnap == INVALID_HANDLE_VALUE )
//	{
//		printError( "CreateToolhelp32Snapshot (of modules)" );
//		return( FALSE );
//	}
//
//	// Set the size of the structure before using it.
//	me32.dwSize = sizeof( MODULEENTRY32 );
//
//	// Retrieve information about the first module, and exit if unsuccessful
//	if( !Module32First( hModuleSnap, &me32 ) )
//	{
//		printError( "Module32First" );  // Show cause of failure
//		CloseHandle( hModuleSnap );     // Must clean up the snapshot object!
//		return( FALSE );
//	}
//
//	// Now walk the module list of the process,
//	// and display information about each module
//	do
//	{
//		printf( "\n\n     MODULE NAME:     %s",             me32.szModule );
//		printf( "\n     executable     = %s",             me32.szExePath );
//		printf( "\n     process ID     = 0x%08X",         me32.th32ProcessID );
//		printf( "\n     ref count (g)  =     0x%04X",     me32.GlblcntUsage );
//		printf( "\n     ref count (p)  =     0x%04X",     me32.ProccntUsage );
//		printf( "\n     base address   = 0x%08X", (DWORD) me32.modBaseAddr );
//		printf( "\n     base size      = %d",             me32.modBaseSize );
//	} while( Module32Next( hModuleSnap, &me32 ) );
//
//	CloseHandle( hModuleSnap );
//
//	return( TRUE );
//}

//-----------------------------------------------------------------------------

//bool CThreadMonitor::GetProcessList()
//{
//	HANDLE hProcessSnap;
//	HANDLE hProcess;
//	PROCESSENTRY32 pe32;
//	DWORD dwPriorityClass;
//
//	// Take a snapshot of all processes in the system.
//	hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
//
//	if( hProcessSnap == INVALID_HANDLE_VALUE )
//	{
//		printError( "CreateToolhelp32Snapshot (of processes)" );
//		return( FALSE );
//	}
//
//	// Set the size of the structure before using it.
//	pe32.dwSize = sizeof( PROCESSENTRY32 );
//
//	// Retrieve information about the first process,
//	// and exit if unsuccessful
//	if( !Process32First( hProcessSnap, &pe32 ) )
//	{
//		printError( "Process32First" );  // Show cause of failure
//		CloseHandle( hProcessSnap );     // Must clean up the snapshot object!
//		return( FALSE );
//	}
//
//	// Now walk the snapshot of processes, and
//	// display information about each process in turn
//	do
//	{
//		printf( "\n\n=====================================================" );
//		printf( "\nPROCESS NAME:  %s", pe32.szExeFile );
//		printf( "\n-----------------------------------------------------" );
//
//		// Retrieve the priority class.
//		dwPriorityClass = 0;
//		hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID );
//
//		if( hProcess == NULL )
//			printError( "OpenProcess" );
//		else
//		{
//			dwPriorityClass = GetPriorityClass( hProcess );
//
//			if( !dwPriorityClass )
//				printError( "GetPriorityClass" );
//
//			CloseHandle( hProcess );
//		}
//
//		printf( "\n  process ID        = 0x%08X", pe32.th32ProcessID );
//		printf( "\n  thread count      = %d",   pe32.cntThreads );
//		printf( "\n  parent process ID = 0x%08X", pe32.th32ParentProcessID );
//		printf( "\n  Priority Base     = %d", pe32.pcPriClassBase );
//
//		if( dwPriorityClass )
//			printf( "\n  Priority Class    = %d", dwPriorityClass );
//
//		// List the modules and threads associated with this process
//		//    ListProcessModules( pe32.th32ProcessID );
//		ListProcessThreads( pe32.th32ProcessID );
//	} while( Process32Next( hProcessSnap, &pe32 ) );
//
//	CloseHandle( hProcessSnap );
//
//	return( TRUE );
//}

//-----------------------------------------------------------------------------
