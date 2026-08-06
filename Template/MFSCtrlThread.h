#ifndef __MFS_CTRL_THREAD_H__
#define __MFS_CTRL_THREAD_H__

#include "SyncObject.h"
#include "Collection.h"

//////////////////////////////////////////////////////////////////////////
// 주고받을 Request/Response 정보 Class
class  CCtrlCmdInfo
{
public:
	CCtrlCmdInfo()
	{
		m_nRequestID = -1;
		m_nRequestCmd = -1;
		m_nResponseCmd = -1;
	};

	CCtrlCmdInfo(int nID, int nCmd)
	{
		m_nRequestID = nID;
		m_nRequestCmd = nCmd;
		m_nResponseCmd = -1;
	}

	CCtrlCmdInfo(const CCtrlCmdInfo &Info)
	{
		m_nRequestID = Info.m_nRequestID;
		m_nRequestCmd = Info.m_nRequestCmd;
		m_nResponseCmd = Info.m_nResponseCmd;
	}

	CCtrlCmdInfo& operator= (const CCtrlCmdInfo &Info)
	{
		this->m_nRequestID = Info.m_nRequestID;
		this->m_nRequestCmd = Info.m_nRequestCmd;
		this->m_nResponseCmd = Info.m_nResponseCmd;

		return *this;
	}

public:
	int		m_nRequestID;			// 요청한 명령을 구분한다.
	int		m_nRequestCmd;			// 요청할 Cmd
	int		m_nResponseCmd;			// Cmd에 대한 응답.
};

//////////////////////////////////////////////////////////////////////////
//	Ctrl 간의 멀티 스레드 구현시 사용한다.

#ifdef _WIN32_WCE
class AFX_EXT_CLASS CMFSCtrlThread
#else
// Win32: this class is compiled directly into each project (not into an MFC
// extension DLL), so it must not carry dllimport/dllexport linkage.
class CMFSCtrlThread
#endif
{
public:
	CMFSCtrlThread();
	~CMFSCtrlThread();

private:
	HANDLE		m_hThread;
	DWORD		m_nThreadID;

protected:
	// 외부 Method 사용 Event
	CMFSEvent	m_eMethodUse;
	// Request Queue
	CMFSQueue < CCtrlCmdInfo >		m_RequestQueue;
	// Response Queue
	CMFSQueue < CCtrlCmdInfo >		m_ResponseQueue;

protected:
	// Random한 CMD ID를 생성한다.
	int		GetRequestID();
	// 스레드 생성 상태를 판단.
	BOOL	IsCreation();
	// 스레드를 대기 상태로 만든다.
	DWORD	Suspend( void );
	// 대기상태의 스레드를 다시 수행시킨다.
	DWORD	Resume( void );
	// 스레드의 우선순위를 지정한다.
	BOOL	SetPriority( int nPriority );
	// 스레드의 우선순위를 읽는다.
	int		GetPriority( void );
	// 스레드의 ID를 읽는다.
	DWORD	GetThreadID( void );
	// 스레드의 종료를 기다린다.
	void	WaitForExitThread();
	// 스레드의 Handler.
	virtual unsigned ThreadHandlerProc(void) = 0;

private:
	static DWORD _stdcall CallThreadHandlerProc( void *pThreadHandler );
protected:
	
};

#endif // __MFS_CTRL_THREAD_H__