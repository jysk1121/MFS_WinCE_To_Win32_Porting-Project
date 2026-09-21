/** ************************************************************************************************************
*	@file FlowCtrl.h
*	@date 2017/10/20	
*	@author MFS
*	@brief VISAII의 플로우와 Standard 플로우를 만드는 틀을 구현한 헤더파일입니다. 
****************************************************************************************************************/

// FlowCtrl.h: interface for the CFlowCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FLOWCTRL_H__4B6E3B25_C207_4B47_9F94_B0D36D9E608F__INCLUDED_)
#define AFX_FLOWCTRL_H__4B6E3B25_C207_4B47_9F94_B0D36D9E608F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define NETBUF_SEND_SIZE	(1024 * 16)		// AMSCtrl_Define.h에 중복 정의되어 있음.
#define NETBUF_RECV_SIZE	(1024 * 16)		// AMSCtrl_Define.h에 중복 정의되어 있음.
#define	TIMER_CLEAR		0

#include "LineCtrl.h"
/** *************************************************************************************************
@class CFlowCtrl
@date 2017/10/19
@author MFS
@brief 플로우와 관련된 클래스를 상속하기 위해 만든 틀입니다. 
*****************************************************************************************************/
class CFlowCtrl
{
public:
	CFlowCtrl();
	virtual ~CFlowCtrl();

public:
	BOOL		m_bNetworkTest;

protected:
	// Line interface.
	CLineCtrl		*m_pNetLine;

public:
	// [PCI-SSF Fix] static -> 인스턴스 멤버로 전환. CStandardCtrl/CVISAIICtrl/CAMSCtrl이
	// 동시에 존재할 때(Set_FlowInform) 서로 다른 프로토콜 흐름이 같은 송수신 버퍼를
	// 공유해 데이터가 상호 오염되는 문제를 방지한다.
	PBYTE	m_SendData;
	PBYTE	m_RecvData;
	int		m_SendLen;
	int		m_RecvLen;

	// 공유 타이머 워커 스레드(m_hTimerWorker) 생명주기 관리용 - 살아있는 인스턴스 수를 추적
	static int		m_nInstanceCount;

private:
	static HANDLE		m_hTimerWorker;

protected:
	void	SetCheckTimer(DWORD Sec);
	int		GetQueue(BYTE *ctl_code);
	int		ClearQueue();
	int		PutEvent(BYTE recv_evt);
	int		Check_Queue(int Check_Kind, BYTE Check_Data);

public:
	virtual int		SendData(BYTE *pSendBuf, int Length, LPCTSTR pDestInfo, LPCTSTR port=_T(""), LPCTSTR option=_T(""))=0;
	// [PCI-SSF Fix] 목적지 버퍼 크기(nBufSize)를 함께 받아 내부에서 클램프 - 호출자 버퍼보다 큰 데이터를 복사하는 오버플로우 방지
	virtual int		RecvData(BYTE *pRecvBuf, int nBufSize);
	virtual	int		PreDialStart(CString strHostPhoneNumber);
	virtual	int		PreDialCancelByUser();
	virtual BOOL	CloseLine();
	virtual int		PingTest(LPCTSTR sHostInfo, ICMP_ECHO_REPLY &icmpEchoReply);

	// for AMS
	virtual int		AMSConnectOpen(LPCTSTR OpenType, LPCTSTR pDestInfo=_T(""), LPCTSTR port=_T("0"), LPCTSTR option=_T("0"));
	virtual int		AMSConnectClose();
	virtual BOOL	AMSConnectCheck();
	virtual int		AMSConnectAccept();

};

#endif // !defined(AFX_FLOWCTRL_H__4B6E3B25_C207_4B47_9F94_B0D36D9E608F__INCLUDED_)
