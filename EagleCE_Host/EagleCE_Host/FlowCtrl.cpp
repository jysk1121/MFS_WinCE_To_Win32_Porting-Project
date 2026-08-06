/** ************************************************************************************************************
*	@file FlowCtrl.cpp 
*	@date 2017/10/20	
*	@author MFS
*	@brief VISAII의 플로우와 Standard 플로우를 만드는 틀을 구현한 소스파일입니다. 
****************************************************************************************************************/



// FlowCtrl.cpp: implementation of the CFlowCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Queue.h"
#include "FlowCtrl.h"
#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern int		get_queue(char *ctl_code);
extern int		clear_queue();
extern int		put_event(char recv_evt);
extern int		check_queue(int check_kind, char check_data);

//////////////////////////////////////////////////////////////////////
// Global Variable
BOOL	gbTimerDoing = TRUE;		// FlowTimer_WorkerThread 동작 여부. 
DWORD	gdwWaitTime = 0;			// Timer 설정. Time out이 발생하면 0으로 설정된다.

//////////////////////////////////////////////////////////////////////
// Function Define.
DWORD WINAPI  FlowTimer_WorkerThread(LPVOID lpParam);

//////////////////////////////////////////////////////////////////////
HANDLE	CFlowCtrl::m_hTimerWorker = NULL;

int		CFlowCtrl::m_nInstanceCount = 0;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFlowCtrl::CFlowCtrl()
{
	// [PCI-SSF Fix] 송수신 버퍼를 인스턴스마다 독립적으로 할당한다 (더 이상 static 공유 아님)
	m_SendData = new BYTE[NETBUF_SEND_SIZE];
	memset(m_SendData, 0, NETBUF_SEND_SIZE);

	m_RecvData = new BYTE[NETBUF_RECV_SIZE];
	memset(m_RecvData, 0, NETBUF_RECV_SIZE);

	m_nInstanceCount++;

	m_SendLen = 0;
	m_RecvLen = 0;

	m_pNetLine = NULL;
	m_bNetworkTest = FALSE;

	// Timer Thread Start.. (프로세스 전역 1개만 사용하는 공유 워커이므로 static 유지)
	if (!m_hTimerWorker)
	{
		DWORD	dwThreadID;

		// start Timer thread.
		gbTimerDoing = TRUE;

		m_hTimerWorker = CreateThread(NULL,
									  0,
									  FlowTimer_WorkerThread,
									  this,
									  0,
									  &dwThreadID);
	}
}

CFlowCtrl::~CFlowCtrl()
{
	if (m_nInstanceCount > 0)
		m_nInstanceCount--;

	// [PCI-SSF Fix] 마지막 인스턴스가 소멸될 때만 공유 타이머 워커 스레드를 정리한다.
	// (기존 코드는 인스턴스가 하나라도 소멸되면 무조건 정지시켜, 동시에 살아있는
	//  다른 CFlowCtrl 파생 인스턴스의 타이머까지 함께 끊기는 문제가 있었다.)
	if (m_nInstanceCount == 0 && m_hTimerWorker)
	{
		// stop Timer thread.
		gbTimerDoing = FALSE;

		// [PCI-SSF Fix] 기존엔 dwExitCode를 STILL_ACTIVE로 초기화한 직후 "!= STILL_ACTIVE" 조건을
		// 검사해서 루프가 한 번도 돌지 않고(조건이 처음부터 거짓) 스레드 종료를 기다리지 않은 채
		// 바로 CloseHandle 하던 버그가 있었다. WaitForSingleObject로 실제 스레드 종료를 대기한다
		// (워커 스레드 내부 폴링 주기가 500ms이므로 최대 2초 대기 후에도 못 끝나면 로그만 남기고 진행).
		if (WaitForSingleObject(m_hTimerWorker, 2000) != WAIT_OBJECT_0)
		{
			LOG(Error, _T("FlowTimer_WorkerThread did not exit within timeout"));
		}

		CloseHandle(m_hTimerWorker);
		m_hTimerWorker = NULL;
	}

	// [PCI-SSF Fix] 인스턴스 소유 버퍼이므로 항상 해제한다.
	if (m_SendData != NULL)
	{
		delete [] m_SendData;
		m_SendData = NULL;
	}

	if (m_RecvData != NULL)
	{
		delete [] m_RecvData;
		m_RecvData = NULL;
	}
}

int CFlowCtrl::RecvData(BYTE *pRecvBuf, int nBufSize)
{
	// [PCI-SSF Fix] 목적지 버퍼(pRecvBuf) 크기를 넘어서는 복사를 방지
	int nCopyLen = m_RecvLen;

	if (nCopyLen > nBufSize)
	{
		LOG(Error, _T("CFlowCtrl::RecvData m_RecvLen(%d) > nBufSize(%d) - truncated"), m_RecvLen, nBufSize);
		nCopyLen = nBufSize;
	}

	if (nCopyLen > 0)
		memcpy(pRecvBuf, m_RecvData, nCopyLen);

	return nCopyLen;
}


int	CFlowCtrl::PreDialStart(CString strHostPhoneNumber)
{
	return 0;
}


int	CFlowCtrl::PreDialCancelByUser()
{
	return 0;
}


BOOL CFlowCtrl::CloseLine()
{
	return TRUE;
}


/** ********************************************************************
* @brief PingTest
* @param LPCTSTR sHostInfo	서버의 아이피 주소
* @param ICMP_ECHO_REPLY &icmpEchoReply	응답 정보
* @param UCHAR &cTTL		Time To Live
* @retval 0 성공 
* @retval nResult 에러코드가 정의됨. 
************************************************************************/
int CFlowCtrl::PingTest(LPCTSTR sHostInfo, ICMP_ECHO_REPLY &icmpEchoReply)
{
	int nRet = CONNECT_FAIL;

	nRet = m_pNetLine->TestPing(sHostInfo, icmpEchoReply);

	return nRet;
}


int	CFlowCtrl::AMSConnectOpen(LPCTSTR OpenType, LPCTSTR pDestInfo, LPCTSTR port, LPCTSTR option)
{
	return 1;
}

int	CFlowCtrl::AMSConnectClose()
{
	return 1;
}

BOOL CFlowCtrl::AMSConnectCheck()
{
	return FALSE;
}

int CFlowCtrl::AMSConnectAccept()
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

int	CFlowCtrl::Check_Queue(int Check_Kind, BYTE Check_Data)
{
	return check_queue(Check_Kind, (char)Check_Data);
}

void CFlowCtrl::SetCheckTimer(DWORD Sec)
{
	if (m_hTimerWorker)
	{
		if (Sec == 0)
			gdwWaitTime = 0;
		else
			gdwWaitTime = (Sec * 1000) + GetTickCount();
	}
}

// Flow Ctrl에서 사용할 Timer Thread.
DWORD WINAPI  FlowTimer_WorkerThread(LPVOID lpParam)
{

	while(gbTimerDoing)
	{
		if (gdwWaitTime)
		{
			if (GetTickCount() > gdwWaitTime)
			{
				put_event(TIMEOUT_EVT);
				gdwWaitTime = 0;
			}
		}
		
		Sleep(500);
	}

	return 0;
}
