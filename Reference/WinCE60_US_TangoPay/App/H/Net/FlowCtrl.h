// FlowCtrl.h: interface for the CFlowCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FLOWCTRL_H__4B6E3B25_C207_4B47_9F94_B0D36D9E608F__INCLUDED_)
#define AFX_FLOWCTRL_H__4B6E3B25_C207_4B47_9F94_B0D36D9E608F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// [#2012] NH KJW 2011.02.09
//#define		BUFFER_SIZE		4096
//#define NETBUF_SEND_SIZE	(1024 * 4)
#define NETBUF_SEND_SIZE	(1024 * 10)				// [#2515] US Justin 2017.11.20 Increase Send Buffer Size
#define NETBUF_RECV_SIZE	(1024 * 300)
// end of [#2012]

#define		TIMER_CLEAR		0

#include "LineCtrl.h"

class CFlowCtrl
{
public:
	CFlowCtrl(CLineCtrl *line);
	virtual ~CFlowCtrl();

public:
//	BOOL		m_bDialTest;
	BOOL		m_bNetworkTest;	// [#585] NH KSK 2009.12.03

	// [#RWC6-55] 2019.10.22 TLS Refactor
	/**
	 * Sets the current transport type for the protocol handler
	 * @param line[in] the transport class for the protocol
	 */
	void	SetLine(CLineCtrl *line);

protected:
	// Line interface.
	CLineCtrl		*m_pNetLine;

public:
	// [#2012] NH KJW 2011.02.09 수신전문 최대 크기 변경 4K -> 100K
	//static BYTE	m_SendData[BUFFER_SIZE];
	//static BYTE	m_RecvData[BUFFER_SIZE];
	static PBYTE	m_SendData;
	static PBYTE	m_RecvData;
	// end of [#2012]
	static int		m_SendLen;
	static int		m_RecvLen;

	static int		m_nInstanceCount;	// [#2012] NH KJW 2011.02.09 CFlowCtrl이 몇번생성되었는지 확인하여, 최종 1개만 남았을 때 SendBuffer/RecvBuffer를 삭제하도록 함

private:
	static HANDLE		m_hTimerWorker;

protected:
	void	SetCheckTimer(DWORD Sec);
	int		GetQueue(BYTE *ctl_code);
	int		ClearQueue();
	int		PutEvent(BYTE recv_evt);
	int		Check_Queue(int Check_Kind, BYTE Check_Data);	// [#595] NH KSK 2009.12.09

public:
	virtual int		SendData(BYTE *pSendBuf, int Length, LPCTSTR pDestInfo, LPCTSTR port=L"", LPCTSTR option=L"")=0;
	virtual int		RecvData(LPBYTE pRecvBuf, int *Length, int WaitSec);	// V01.02.29 REVIEW8 [#89] KSK 2008.04.14
	virtual BOOL	CloseLine();

	// for rms
	virtual int		RMSConnectOpen(LPCTSTR pDestInfo, LPCTSTR port=L"", LPCTSTR option=L"", LPCTSTR OpenType=L"");
	virtual int		RMSConnectClose();
	virtual BOOL	RMSConnectCheck();
	virtual int		RMSConnectAccept();

	// [#2075] NH KSK 2011.06.27
	virtual BOOL	RKTConnectCheck();
	virtual int		RKTConnectOpen(LPCTSTR pDestInfo, LPCTSTR port=L"", LPCTSTR option=L"");
	virtual int		RKTConnectClose();
	// end of [#2075]
};

#endif // !defined(AFX_FLOWCTRL_H__4B6E3B25_C207_4B47_9F94_B0D36D9E608F__INCLUDED_)
