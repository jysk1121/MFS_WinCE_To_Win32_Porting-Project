#include "StdAfx.h"
#include ".\Common\NHCtrlThread.h"
#include ".\Common\CmnLib.h"
//#include ".\CmnLib\NHDbgApi.h"

// ----------------------------------------------------------------------------
//	For debugging
// ----------------------------------------------------------------------------
#define DBG_ERROR	1

// ----------------------------------------------------------------------------
//	static variable initialize.
// ----------------------------------------------------------------------------


/*-------------------------------------------------------------------
 CLASS    NAME: CNHCtrlThread
 FUNCTION NAME: CNHCtrlThread()
 WRITER		  : AIREAT
 RETURN TYPE  : 
 PARAMETER    : CNHDevThreadHandler
 DESCRIPTION  : 생성자에서 스레드가 생성 된다.
				생성된 스레드는 소멸자에서 삭제 된다.
-------------------------------------------------------------------*/
CNHCtrlThread::CNHCtrlThread( ) : m_eMethodUse(FALSE, TRUE)
{
	m_hThread = NULL;
	m_nThreadID = 0;

	// Create Thread.
	m_hThread = CreateThread(NULL,		/* Must be NULL */ 
		0,								/* Stack size */ 
		CallThreadHandlerProc,			/* Thread Start Routine */ 
		(void*)this,					/* Thread Parameter */ 
		CREATE_SUSPENDED,				/* Creation Flag */ 
		&m_nThreadID);					/* Thread ID */ 
}

/*-------------------------------------------------------------------
 CLASS    NAME: CNHCtrlThread
 FUNCTION NAME: ~CNHCtrlThread()
 WRITER		  : AIREAT
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 생성자에서 스레드가 생성 된다.
				생성된 스레드는 소멸자에서 삭제 된다.
-------------------------------------------------------------------*/
CNHCtrlThread::~CNHCtrlThread()
{
	// 스레드를 종료한다.
	if (m_hThread != NULL)
	{
		::TerminateThread(m_hThread, 0);
	}
}

/*-------------------------------------------------------------------
 CLASS    NAME: CNHCtrlThread
 FUNCTION NAME: ThreadHandlerProc()
 WRITER		  : AIREAT
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
unsigned CNHCtrlThread::ThreadHandlerProc(void)
{
	return -1; // This indicates an incomplete implementation of the proc
}

/*-------------------------------------------------------------------
 CLASS    NAME: CNHCtrlThread
 FUNCTION NAME: CallThreadHandlerProc()
 WRITER		  : AIREAT
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
DWORD _stdcall CNHCtrlThread::CallThreadHandlerProc( void *pThreadHandler )
{
	CNHCtrlThread *pcHandler = static_cast<CNHCtrlThread*> (pThreadHandler);

	DWORD ret = 0;
	while ((ret = pcHandler->ThreadHandlerProc()) == -1)
	{
		RETAILMSG(1, (L"ThreadProc exited with %d (err: %d)\r\n", ret, GetLastError()));
	}
	
	return ret;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CNHCtrlThread
 FUNCTION NAME: Run()
 WRITER		  : AIREAT
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 스레드가 생성되었는지 판단 한다.
-------------------------------------------------------------------*/
BOOL CNHCtrlThread::IsCreation()
{
	if (m_hThread == NULL)
		return FALSE;
	
	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CNHCtrlThread
 FUNCTION NAME: Suspend()
 WRITER		  : AIREAT
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 스레드를 대기 상태로 만든다.
-------------------------------------------------------------------*/
DWORD CNHCtrlThread::Suspend()
{
	return ::SuspendThread(m_hThread);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CNHCtrlThread
 FUNCTION NAME: Resume()
 WRITER		  : AIREAT
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 대기상태의 스레드를 다시 수행시킨다.
-------------------------------------------------------------------*/
DWORD CNHCtrlThread::Resume()
{
	return ::ResumeThread(m_hThread);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CNHCtrlThread
 FUNCTION NAME: GetPriority()
 WRITER		  : AIREAT
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 스레드의 우선순위를 읽는다.
-------------------------------------------------------------------*/
int CNHCtrlThread::GetPriority()
{
	return ::GetThreadPriority(m_hThread);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CNHCtrlThread
 FUNCTION NAME: SetPriority()
 WRITER		  : AIREAT
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 스레드의 우선순위를 지정한다.
-------------------------------------------------------------------*/
BOOL CNHCtrlThread::SetPriority( int nPriority )
{
	return ::SetThreadPriority(m_hThread, nPriority);
}

/*-------------------------------------------------------------------
 CLASS    NAME: CNHCtrlThread
 FUNCTION NAME: GetThreadID()
 WRITER		  : AIREAT
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 스레드의 ID를 읽는다.
-------------------------------------------------------------------*/
DWORD CNHCtrlThread::GetThreadID()
{
	return m_nThreadID;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CNHCtrlThread
 FUNCTION NAME: GetRandomID()
 WRITER		  : AIREAT
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Random한 CMD ID를 생성한다.
-------------------------------------------------------------------*/
int CNHCtrlThread::GetRequestID()
{
	srand(GetTickCount());

	int		nNumber = 0;
	while(nNumber > 10)
	{
		nNumber = (rand() % 10000);
	}
	return nNumber;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CNHCtrlThread
 FUNCTION NAME: WaitForExitThread()
 WRITER		  : AIREAT
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 스레드의 종료를 기다린다.
-------------------------------------------------------------------*/
void CNHCtrlThread::WaitForExitThread()
{
	DWORD dwExitCode = STILL_ACTIVE;

	if (m_hThread == NULL)
		return;
	
	while (dwExitCode == STILL_ACTIVE)
	{
		Delay_Msg(100);
		GetExitCodeThread(m_hThread, &dwExitCode);
	}
	
	CloseHandle(m_hThread);
}