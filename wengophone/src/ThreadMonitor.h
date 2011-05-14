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
// $Id: ThreadMonitor.h,v 1.4 2008/04/10 08:24:58 jtheinert Exp $ 
//

#pragma once

#include "model/profile/VoxListTemplate.h"

typedef char TCHAR, *PTCHAR;

typedef int                 INT;
typedef unsigned int        UINT;
typedef unsigned long       DWORD;

typedef __int64 LONGLONG;
typedef unsigned __int64 ULONGLONG;

//typedef struct _FILETIME {
//    DWORD dwLowDateTime;
//    DWORD dwHighDateTime;
//} FILETIME, *PFILETIME, *LPFILETIME;
//#define _FILETIME_

class CThreadInfoDiff;		//Fwd declaration

//typedef CSPIterator<CThreadInfoDiff>  CThreadInfoDiffListIter;

//=============================================================================

class CThreadInfo
{
public:
	CThreadInfo();

	//Gets -----------------------------------------------------------------------
	bool		IsValid()			const			{ return m_bValid;		}
	DWORD		GetThreadID()		const			{ return m_dwThreadID;	}
	FILETIME	GetCreationTime()	const			{ return m_stCreation;	}
	FILETIME	GetExitTime()		const			{ return m_stExit;		}
	FILETIME	GetKernelTime()		const			{ return m_stKernel;	}
	FILETIME	GetUserTime()		const			{ return m_stUser;		}

	ULONGLONG	GetKernelDiff()		const			{ return m_nKernelDiff;	}
	ULONGLONG	GetUserDiff()		const			{ return m_nUserDiff;	}

	ULONGLONG	GetTotalTime();
	ULONGLONG	GetTotalDiff()		const			{ return m_nKernelDiff + m_nUserDiff;	}

	//Sets -----------------------------------------------------------------------
	void SetIsValid     ( bool     bVal  )			{ m_bValid     = bVal;	}
	void SetThreadID    ( DWORD    dwVal )			{ m_dwThreadID = dwVal;	}
	void SetCreationTime( FILETIME stVal )			{ m_stCreation = stVal;	}
	void SetExitTime    ( FILETIME stVal )			{ m_stExit     = stVal;	}
	void SetKernelTime	( FILETIME stVal )			{ m_stKernel   = stVal;	}
	void SetUserTime	( FILETIME stVal )			{ m_stUser     = stVal;	}

	void SetKernelDiff  ( ULONGLONG nVal  )			{ m_nKernelDiff = nVal;	}
	void SetUserDiff    ( ULONGLONG nVal  )			{ m_nUserDiff   = nVal;	}

	//Misc
	void Init();
	static CThreadInfo* Make( DWORD dwThreadID, FILETIME ftCreation, FILETIME ftExit,
												FILETIME ftKernel,   FILETIME ftUser );

	CThreadInfo& operator=( CThreadInfo& src );
	bool	CalcDiff ( CThreadInfo& rInfo1, CThreadInfo& rInfo2 );

private:
	bool		m_bValid;
	DWORD		m_dwThreadID;

	FILETIME	m_stCreation;
	FILETIME	m_stExit;
	FILETIME	m_stKernel;
	FILETIME	m_stUser;

	ULONGLONG	m_nKernelDiff;
	ULONGLONG	m_nUserDiff;
};

//=============================================================================



//=============================================================================

class CThreadInfoDiff
{
public:
	CThreadInfoDiff();

	//Gets -----------------------------------------------------------------------
	int				GetID()						{ return m_nID;			}
	bool			IsValid()					{ return m_bValid;		}

	CThreadInfo&	GetCurrInfo()				{ return m_CurrInfo;	}
	CThreadInfo&	GetPrevInfo()				{ return m_PrevInfo;	}

	//Sets -----------------------------------------------------------------------
	void SetID      ( int  nVal )				{ m_nID		 = nVal;	}
	void SetIsValid ( bool bVal )				{ m_bValid   = bVal;	}

	void SetCurrInfo( CThreadInfo& rInfo )		{ m_CurrInfo = rInfo;	}
	void SetPrevInfo( CThreadInfo& rInfo )		{ m_PrevInfo = rInfo;	}

	//Misc --------------------------------------------------------------------
	void Init();

	CThreadInfoDiff& operator=( CThreadInfoDiff& src );
	void Update( CThreadInfo& rInfo );

private:
	int			m_nID;
	bool		m_bValid;

	CThreadInfo	m_CurrInfo;
	CThreadInfo	m_PrevInfo;
};

//=============================================================================



//=============================================================================

class CThreadInfoDiffList : public VoxListTemplate<CThreadInfoDiff>
{
public:
	CThreadInfoDiffList();

	CThreadInfoDiff* Update( CThreadInfo& rInfo );
	CThreadInfoDiff* FindByThreadID( DWORD nID );
	CThreadInfoDiff* FindHighestUsage();

	void InvalidateAll();
	UINT RemoveInvalid();
};

//=============================================================================



//=============================================================================

class CThreadMonitor
{
public:
	CThreadMonitor();

	void Start( DWORD dwPID = 0 );
	void Stop();
	void Run();
	void SetInterval( UINT nMilliSecs )			{ m_nInterval = nMilliSecs;	}

protected:
	void Init();
	bool ListProcessThreads( DWORD dwOwnerPID );
//	bool GetProcessList();
//	BOOL ListProcessModules( DWORD dwPID );
	void HandleError( TCHAR* pMsg );
	void DisplayResults();

private:
	DWORD m_dwPID;
	UINT  m_nInterval;
	bool  m_bRunning;

	CThreadInfoDiffList		m_InfoList;
};

//-----------------------------------------------------------------------------
