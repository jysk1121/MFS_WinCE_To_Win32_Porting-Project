// FlowCtrl.cpp: implementation of the CFlowCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Queue.h"

//#define NH_DEBUG

#include ".\Common\NHDbgApi.h"
#include ".\Net\FlowCtrl.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern int		get_queue(char *ctl_code);
extern int		clear_queue();
extern int		put_event(char recv_evt);
extern int		check_queue(int check_kind, char check_data);	// [#595] NH KSK 2009.12.09

//////////////////////////////////////////////////////////////////////
// Global Variable
BOOL	gbTimerDoing = TRUE;		// FlowTimer_WorkerThread 동작 여부. 
DWORD	gdwWaitTime = 0;			// Timer 설정. Time out이 발생하면 0으로 설정된다.

//////////////////////////////////////////////////////////////////////
// Function Define.
DWORD WINAPI  FlowTimer_WorkerThread(LPVOID lpParam);

//////////////////////////////////////////////////////////////////////
// Static Variable 선언.
// [#2012] NH KJW 2011.02.09 4K -> 100K 변경에 따라 new로 생성하도록 수정.
//BYTE	CFlowCtrl::m_SendData[BUFFER_SIZE] = {0,};
//BYTE	CFlowCtrl::m_RecvData[BUFFER_SIZE] = {0,};
PBYTE	CFlowCtrl::m_SendData = NULL;
PBYTE	CFlowCtrl::m_RecvData = NULL;
// end of [#2012]
int		CFlowCtrl::m_SendLen = 0;
int		CFlowCtrl::m_RecvLen = 0;
HANDLE	CFlowCtrl::m_hTimerWorker = NULL;

int		CFlowCtrl::m_nInstanceCount = 0;	// [#2012] NH KJW 2011.02.09 CFlowCtrl이 몇번생성되었는지 확인하여, 최종 1개만 남았을 때 SendBuffer/RecvBuffer를 삭제하도록 함

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFlowCtrl::CFlowCtrl(CLineCtrl *line)
{
	// [#2012] NH KJW 2011.02.09 4K -> 100K 변경에 따라 new로 생성하도록 수정.
	//memset(m_RecvData, 0, sizeof(m_RecvData));
	//memset(m_SendData, 0, sizeof(m_SendData));

	if( m_nInstanceCount == 0 )
	{
		NHDEBUG(1, (_T("CFlowCtrl() m_SendData = new BYTE[]\n")));
		m_SendData = NULL;	// [###2]
		m_SendData = new BYTE[NETBUF_SEND_SIZE];
		memset(m_SendData, 0, NETBUF_SEND_SIZE);

		NHDEBUG(1, (_T("CFlowCtrl() m_RecvData = new BYTE[]\n")));
		m_RecvData = NULL;	// [###2]
		m_RecvData = new BYTE[NETBUF_RECV_SIZE];
		memset(m_RecvData, 0, NETBUF_RECV_SIZE);
	}
	m_nInstanceCount++;
	// end of [#2012]

	m_SendLen = 0;
	m_RecvLen = 0;

	m_pNetLine = line; // [#RWC6-55] 2019.10.22 TLS Refactor
	m_bNetworkTest = FALSE;	// [#585] NH KSK 2009.12.03

	// Timer Thread Start..
	if (!m_hTimerWorker)
	{
		DWORD	dwThreadID;

		// start Timer thread.
		gbTimerDoing = TRUE;

		m_hTimerWorker = CreateThread(NULL, 
									  0, 
									  FlowTimer_WorkerThread,
									  this, // [#2042] NH KJW 2011.04.04
									  0,
									  &dwThreadID);
	}
}

CFlowCtrl::~CFlowCtrl()
{
	// 타이머 스레드를 종료한다.
	if (m_hTimerWorker)
	{
		DWORD	dwExitCode;

		// stop Timer thread.
		gbTimerDoing = FALSE;

		// wait exit.
		Sleep(20);

		dwExitCode = STILL_ACTIVE;
		while (dwExitCode != STILL_ACTIVE)
			GetExitCodeThread(m_hTimerWorker, &dwExitCode);

		CloseHandle(m_hTimerWorker);
		m_hTimerWorker = NULL;
	}

	// [#2012] NH KJW 2011.02.09
	if( m_nInstanceCount == 1 )
	{
		// [###2]
		if (m_SendData != NULL)
		{
			NHDEBUG(1, (_T("~CFlowCtrl() delete [] m_SendData\n")));
			delete [] m_SendData;
			m_SendData = NULL;
		}

		if (m_RecvData != NULL)
		{
			NHDEBUG(1, (_T("~CFlowCtrl() delete [] m_RecvData\n")));
			delete [] m_RecvData;
			m_RecvData = NULL;
		}
		// end of [###2]
	}
	m_nInstanceCount--;
	// end of [#2012]
}

// [#RWC6-55] 2019.10.22 TLS Refactor
// Sets the current transport type for the protocol handler
void CFlowCtrl::SetLine(CLineCtrl *line)
{
	if (line == NULL)
	{
		return;
	}

	m_pNetLine = line;
}

// [#89] KSK 2008.04.14
// V01.02.29 Add function REVIEW8
int CFlowCtrl::RecvData(LPBYTE pRecvBuf, int *Length, int WaitSec)
{
	NHDEBUG(1, (L"CFlowCtrl::RecvData():m_RecvLen(%d), *Length(%d)\n", m_RecvLen, *Length));
	if (m_RecvLen != 0)
	{
		memcpy(pRecvBuf, m_RecvData, __min(m_RecvLen, *Length));
		*Length = __min(m_RecvLen, *Length);
		return 0;
	}
	else
		*Length = 0;	// V01.02.25 NZ	
	return 1;
}
// end of [#89]

BOOL CFlowCtrl::CloseLine()
{
	return TRUE;
}

// [#122] NH AIREAT 2008.04.22 파라미터 추가
int	CFlowCtrl::RMSConnectOpen(LPCTSTR pDestInfo, LPCTSTR port, LPCTSTR option, LPCTSTR OpenType)
{
	return 1;
}

int	CFlowCtrl::RMSConnectClose()
{
	return 1;
}

BOOL CFlowCtrl::RMSConnectCheck()
{
	return FALSE;
}

int CFlowCtrl::RMSConnectAccept()
{
	return 0;
}

int CFlowCtrl::GetQueue(BYTE *ctl_code)
{
	return get_queue((char*)ctl_code);
}

int	CFlowCtrl::ClearQueue()
{
	return clear_queue();
}

int	CFlowCtrl::PutEvent(BYTE recv_evt)
{
	return put_event((char)recv_evt);
}

// [#595] NH KSK 2009.12.09
int	CFlowCtrl::Check_Queue(int Check_Kind, BYTE Check_Data)
{
	return check_queue(Check_Kind, (char)Check_Data);
}
// end of [#595]

void CFlowCtrl::SetCheckTimer(DWORD Sec)
{
	if (m_hTimerWorker)
	{
		if (Sec == 0)
			gdwWaitTime = 0;
		else
			gdwWaitTime = (Sec * 1000) + GetTickCount();

NHDEBUG(1, (_T("SetTimer : %d\n"), Sec));
	}
}

// [#2075] NH KSK 2011.06.27
BOOL CFlowCtrl::RKTConnectCheck()
{
	return FALSE;
}

int	CFlowCtrl::RKTConnectOpen(LPCTSTR pDestInfo, LPCTSTR port, LPCTSTR option)
{
	return 1;
}

int	CFlowCtrl::RKTConnectClose()
{
	return 1;
}
// end of [#2075]


// Flow Ctrl에서 사용할 Timer Thread.
DWORD WINAPI  FlowTimer_WorkerThread(LPVOID lpParam)
{
	while(gbTimerDoing)
	{
		if (gdwWaitTime)
		{
			if (GetTickCount() > gdwWaitTime)
			{
NHDEBUG(1, (_T("OCCUR TIMEOUT - PUT EVENT\n")));
				put_event(TIMEOUT_EVT);
				gdwWaitTime = 0;
			}
		}
		
		Sleep(500);
	}

	return 0;
}
