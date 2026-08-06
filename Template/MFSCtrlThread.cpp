#include "StdAfx.h"
#include "MFSCtrlThread.h"

// ----------------------------------------------------------------------------
//	static variable initialize.
// ----------------------------------------------------------------------------


CMFSCtrlThread::CMFSCtrlThread( ) : m_eMethodUse(FALSE, TRUE)
{
	m_hThread = NULL;
	m_nThreadID = 0;

	// Create Thread.
	m_hThread = CreateThread(NULL,							/* Must be NULL */ 
							 0,								/* Stack size */ 
							 CallThreadHandlerProc,			/* Thread Start Routine */ 
							 (void*)this,					/* Thread Parameter */ 
							 0,								/* Creation Flag */ 
							 &m_nThreadID);					/* Thread ID */ 
}

CMFSCtrlThread::~CMFSCtrlThread()
{
	// 스레드를 종료한다.
	if (m_hThread != NULL)
	{
		::TerminateThread(m_hThread, 0);
	}
}

DWORD _stdcall CMFSCtrlThread::CallThreadHandlerProc( void *pThreadHandler )
{
	CMFSCtrlThread *pcHandler = static_cast<CMFSCtrlThread*> (pThreadHandler);

	return pcHandler->ThreadHandlerProc();
}

BOOL CMFSCtrlThread::IsCreation()
{
	if (m_hThread == NULL)
		return FALSE;
	
	return TRUE;
}

DWORD CMFSCtrlThread::Suspend()
{
	return ::SuspendThread(m_hThread);
}

DWORD CMFSCtrlThread::Resume()
{
	return ::ResumeThread(m_hThread);
}

int CMFSCtrlThread::GetPriority()
{
	return ::GetThreadPriority(m_hThread);
}

BOOL CMFSCtrlThread::SetPriority( int nPriority )
{
	return ::SetThreadPriority(m_hThread, nPriority);
}

DWORD CMFSCtrlThread::GetThreadID()
{
	return m_nThreadID;
}

int CMFSCtrlThread::GetRequestID()
{
	srand(GetTickCount());

	int		nNumber = 0;
	while(nNumber > 10)
	{
		nNumber = (rand() % 10000);
	}
	return nNumber;
}

void CMFSCtrlThread::WaitForExitThread()
{
	DWORD dwExitCode = STILL_ACTIVE;

	if (m_hThread == NULL)
		return;
	
	while (dwExitCode == STILL_ACTIVE)
	{
		Sleep(100);
		GetExitCodeThread(m_hThread, &dwExitCode);
	}
	
	CloseHandle(m_hThread);
}