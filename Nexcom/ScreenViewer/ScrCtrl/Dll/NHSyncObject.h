/*---------------------------------------------------------------------------------
	스레드 or 프로세스 간의 동기화를 제공하는 Object가 정의 되어 있다.

		-. CRITICAL SECTION
		-. EVENT
		-. AUTO LOCK

	작성자 : AIREAT
	작성일 : 2008.01.17
---------------------------------------------------------------------------------*/

#ifndef __NH_SYNC_OBJECT_H__
#define __NH_SYNC_OBJECT_H__

#include <winbase.h>

#ifdef __cplusplus
extern "C" {
#endif
///////////////////////////////////////////
//	
//	SYNC BASE
//

class CNHSyncBase
{
protected:
	HANDLE	m_hHandle;
	DWORD	m_dwStatus;

public:
	// 생성자
	CNHSyncBase()
	{
		m_hHandle = NULL;
		m_dwStatus = ERROR_INVALID_HANDLE;
	};

	// 소멸자
	~CNHSyncBase()
	{
		if (NHIsValidHandle(m_hHandle))
		{
			::CloseHandle(m_hHandle);
			m_hHandle = NULL;
		}
	};

	// 내부 핸들을 가져온다.
	HANDLE GetHandle( void ) const
	{
		return m_hHandle;
	};

	// 내부 커널 객체의 생성된 상태를 얻는다.
	DWORD Status( void ) const
	{
		return m_dwStatus;
	};

	// 현재 커널 객체에 대해서 대기한다.
	DWORD Wait( DWORD dwMilliseconds )
	{
		// [#738] US KMK 2013.08.27 CodeSonar 지적사항 대책
// 		return ::WaitForSingleObject( m_hHandle, dwMilliseconds );
		if (NHIsValidHandle(m_hHandle))
			return ::WaitForSingleObject( m_hHandle, dwMilliseconds );
		else
			return WAIT_FAILED;
		// end of [#738]
	};
	
	BOOL NHIsValidHandle( HANDLE hHandle )
	{
		// 핸들 Valid Check
		return ( ( hHandle != NULL ) && ( hHandle != INVALID_HANDLE_VALUE ) );
	};
	
	BOOL NHWaitSucceeded( DWORD dwWaitResult)
	{
		// 대기 동작이 성공하면 TRUE, 실패 FALSE
		return ( dwWaitResult >= WAIT_OBJECT_0 );
	}
	
	BOOL NHWaitTimeout( DWORD dwWaitResult )
	{
		// 대기 동작이 시간 초과이면 TRUE, 아니면 FALSE
		return ( dwWaitResult == WAIT_TIMEOUT );
	}
	
	BOOL NHWaitFailed( DWORD dwWaitResult )
	{
		// 대기 동작이 실패하면 TRUE, 아니면 FALSE
		// 이함수가 TRUE이면 GetLastError() 함수로 정보 얻음.
		return ( dwWaitResult == WAIT_FAILED );
	}

	
};

///////////////////////////////////////////
//	
//	EVENT
//

class CNHEvent : public CNHSyncBase
{
public:
	// 이벤트 객체를 생성한다.
	CNHEvent( BOOL bManualReset = FALSE, BOOL bInitialState = FALSE, LPCTSTR lpName = NULL, 
			   LPSECURITY_ATTRIBUTES lpEventAttributes = NULL )
	{
		m_hHandle = ::CreateEvent( lpEventAttributes, bManualReset, bInitialState, lpName );
		if (lpName != NULL)
			m_bManualReset = FALSE;
		else
			m_bManualReset = bManualReset;
		
		if (NHIsValidHandle(m_hHandle))
		{
			if (lpName)
				m_dwStatus = GetLastError();
			else
				m_dwStatus = NO_ERROR;
		}
		else
		{
			m_dwStatus = GetLastError();
		}
	};

	// 현재 존재하는 이름을 가진 이벤트 객체를 연다.
	// 객체가 열리지 않은 경우에도 예외를 발생시키기지 않기 때문에
	// 이 생성자를 사용한 후에는 객체의 상태를 반드시 점검해야 한다.
	CNHEvent( LPCTSTR lpName, BOOL bInheritHandle = FALSE, DWORD dwDesiredAccess = EVENT_ALL_ACCESS )
	{
		m_hHandle = ::OpenEvent( dwDesiredAccess, bInheritHandle, lpName );
		m_bManualReset = FALSE;		// 이름으로 동기화 된것은 무조건 AutoReset이다.
		
		if( NHIsValidHandle( m_hHandle ) )
		{
			m_dwStatus = NO_ERROR;
		}
		else
		{
			m_dwStatus = GetLastError();
		}
	};
	
	// 이벤트 객체에 대한 오퍼레이션.
	BOOL Set( void )
	{
		return ::SetEvent( m_hHandle );
	};

	BOOL Reset( void )
	{
		return ::ResetEvent( m_hHandle );
	};

	BOOL Pulse( void )
	{
		return ::PulseEvent( m_hHandle );
	};

	// 이벤트 생성시 Manual Reset을 설정 했는지 조회 한다.
	BOOL IsManualReset()
	{
		return m_bManualReset;
	};

private:
	BOOL	m_bManualReset;
};

///////////////////////////////////////////
//	
//	CRITICAL_SECTION
//

class CNHCritSec
{
private:
	CRITICAL_SECTION	m_CritSec;

public:
	// C++ 객체 안에서 CRITICAL_SECTION을 만드는 생성자
	CNHCritSec( void )
	{
		::InitializeCriticalSection( &m_CritSec );
	};

	// 소멸자
	virtual ~CNHCritSec()
	{
		::DeleteCriticalSection( &m_CritSec );
	};

	// 임계 영역에 들어간다.
	void Enter( void )
	{
		::EnterCriticalSection( &m_CritSec );
	};

	// 임계 영역에서 나온다.
	void Leave( void )
	{
		::LeaveCriticalSection( &m_CritSec );
	};

	// 내부의 임계 영역 포인터를 리턴한다.
	CRITICAL_SECTION *GetCritSec( void )
	{
		return &m_CritSec;
	};
};

///////////////////////////////////////////
//	
//	AUTO LOCK
//

class CNHAutoLock
{
private:
	CRITICAL_SECTION	*m_pCritSec;
	CNHCritSec			*m_pcCritSec;
	CNHEvent			*m_pEvent;

public:
	// 생성자
	CNHAutoLock( CRITICAL_SECTION *pCritSec )
	{
		m_pCritSec = pCritSec;
		m_pcCritSec = NULL;
		m_pEvent = NULL;

		::EnterCriticalSection( m_pCritSec );
	};

	CNHAutoLock( CNHCritSec &rCMclCritSec )
	{
		m_pCritSec = NULL;
		m_pcCritSec = &rCMclCritSec;
		m_pEvent = NULL;
		
		m_pcCritSec->Enter();
	};

	CNHAutoLock( CNHEvent &rEvent )
	{
		m_pcCritSec = NULL;
		m_pCritSec = NULL;
		m_pEvent = &rEvent;

		m_pEvent->Wait(INFINITE);
	};

	// 소멸자
	~CNHAutoLock( void )
	{
		if (m_pCritSec != NULL)
		{
			::LeaveCriticalSection( m_pCritSec );
		}
		else if (m_pcCritSec != NULL)
		{
			m_pcCritSec->Leave();
		}
		else if (m_pEvent != NULL)
		{
			if (m_pEvent->IsManualReset() == FALSE)
				m_pEvent->Set();
		}
	};

};

//[#475] NH AIREAT 2008.12.31 - STRING SORT 함수 추가
// Length까지 비교한다.
// greater :  > 0
// same    : == 0
// less    :  < 0
inline int NHStringCompare(CString str1, CString str2)
{
	if (str1.GetLength() == str2.GetLength())
		return str1.Compare(str2);
	
	if (str1.GetLength() > str2.GetLength())
		return 1;

	return -1;
}
// end of [#475]

#ifdef __cplusplus
}
#endif

#endif //__NH_SYNC_OBJECT_H__
