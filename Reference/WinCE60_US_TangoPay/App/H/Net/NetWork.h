// NetWork.h: interface for the CNetWork class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NETWORK_H__1253B7C5_3AD4_46EF_9B90_17BC0B6AB7D3__INCLUDED_)
#define AFX_NETWORK_H__1253B7C5_3AD4_46EF_9B90_17BC0B6AB7D3__INCLUDED_

#ifndef _NETCTRL_OWNER_
#pragma comment(lib, "NetCtrl.lib")
#endif

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FlowCtrl.h"

typedef enum _FLOW_LINE
{
	VISAII_FLOW = 0,
	STANDARD_FLOW,
	ACKCONTROLLED_FLOW,
	NHRMS_FLOW,
	HTTP_POST_FLOW = 5,		// [#2347] US JUSTIN 2015.05.27 Change EVENT_NOTICE to HTTP POST
	MAX_FLOWLINE = 6	// [#2313] US JUSTIN 2014.11.20 Send ATM Event to a host   5=>6
};

typedef	struct _tcp_info {
	CString		IpAddr1;
	CString		IpAddr2;
	CString		port1;
	CString		port2;
	CString		SSLOption;
}TCP_INFO;

// [#2326] US Kook 2015.07.22 Support MoniAir, structure for RASEntry registration.
// from https://social.msdn.microsoft.com/Forums/en-US/ba8ba763-9dca-45cf-8a25-f4f829be282a/windows-ce-60-gprs-settings?forum=windowsmobiledev
#define __AT_CMD_LEN__ 331

typedef struct {
	WORD wHeader;					// 0x00(L:2), Val = 0x30, Unknown
	WORD wWaitForCreditCardSec;		// 0x02(L:2), Val = 0x0, Wait For Credit Card Tone Seconds,
	WORD wCancelTimeOut;			// 0x04(L:2), Val = 78 00 -> 0x78
	WORD wReserved1;				// 0x06(L:2), Val = 0x0,
	BYTE bFlowControl;				// 0x08(L:1), Val = 0x0,
	// F.C. = None, Val = 0x0,
	// F.C. = H.W., Val = 0x10,
	// F.C. = S.W., Val = 0x20,

	BYTE bWaitForDialTone;			// 0x09(L:1), Val = 0x1, Wait For Dial Tone Before Dialing
	// Enable Wait For Dial Tone Before Dialing, Val = 0x1
	// Disable Wait For Dial Tone Before Dialing, Val = 0x3

	WORD wReserved2;				// 0x0A(L:2), Val = 0x0,
	UINT uBaudRate;					// 0x0C(L:4), Val = 00 c2 01 00, 0x0001c200 = 115200
	WORD wTerminal;					// 0x10(L:2), Val = 00(L^) 00(H) -> 0x0, digital (d2)(d1)(d0),
	// d2:Manual Dial,
	// d1:Use terminal window before dialing,
	// d0:Use terminal window after dialing,

	BYTE bDataBits;					// 0x12(L:1), Val = 0x8,
	BYTE bStopBits;					// 0x13(L:1), Val = 0x0,
	// StopBits = 1, Val = 0x0
	// StopBits = 1.5,Val = 0x1
	// StopBits = 2, Val = 0x2

	WORD wParity;					// 0x14(L:2), Val = 00(^L) 00(H) -> 0x0
	// Parity = None, Val = 0x0
	// Parity = Odd, Val = 0x1
	// Parity = Even, Val = 0x2
	// Parity = Mark, Val = 0x3
	// Parity = Space,Val = 0x4

	WCHAR wszAtCmd[__AT_CMD_LEN__];	// 0x16(L:662),
	// __AT_CMD_LEN__ ->331, Why "331" -> Reg DevCfg TotalLen : 684(0~0x2AB)
	// 684(0~0x2AB) - 22(0x0~0x15->0x16) = 662(0x16~0x2AB->0x296)
	// 662(0x16~0x2AB->0x296) / sizeof(WCHAR)
} sDevConfig;
// end of [#2326]

class AFX_EXT_CLASS CNetWork  
{
public:
	CNetWork(int LineType, int RMSClientLineType, int RMSServerLineType);
	virtual ~CNetWork();

private:
	int				m_nCurrentFlowLine;
	int				m_nCurrentRMSClientLine;
	int				m_nCurrentRMSServerLine;
	int				m_nCurrentLine;
	CFlowCtrl*		m_FlowAndLine[MAX_FLOWLINE];
	TCP_INFO		m_tcp;

	CString			strErrorCode;
	CString			strErrorMsg;

public:
//	int			SetInformation(int CtrlType, int LineType);
	int			SetInformation(int CtrlType);
	// [#2012] NH KJW 2011.01.26
	//int		SendData(BYTE *pSendBuf, int Length, int SendSec);
	//int		RecvData(BYTE *pRecvBuf, int *Length, int RecvSec);
	//int		SendData(BYTE *pSendBuf, int Length, int SendSec, int n3rdTranType = TRANHOST_ATM);		// [#2185] NH Justin 2013.05.06	Host Type Definition.
	int			SendData(BYTE *pSendBuf, int Length, int SendSec, int n3rdTranType = TRANHOST_ATM, LPCTSTR sHostInfo=NULL);		// [#2412] NH Justin 2016.04.05 Add Extra Host Info
	// [#2054] NH KJW 2011.05.03
	//int			RecvData(BYTE *pRecvBuf, int *Length, int RecvSec, int n3rdTranType = 0);
	int			RecvData(LPBYTE pRecvBuf, int *Length, int RecvSec, int n3rdTranType = TRANHOST_ATM);	// [#2185] NH Justin 2013.05.06	Host Type Definition.
	// end of [#2054]
	// end of [#2012]

	// [#525] US KSK 2009.05.25
	int 			PreDialStart();
	int 			PreDialCancelByUser();
	// end of [#525]

	int			CloseLine();

	// RMS FUNCTION
private:
	BOOL		m_bRMSConnectOpen;

public:
	BOOL		RMSConnectCheck();
	int			RMSConnectAccept();
	int			RMSConnectOpen(int OpenType);
	int			RMSConnectClose();
	int			RMSSendData(BYTE *pSendBuf, int Length, int SendSec);
	int			RMSRecvData(BYTE *pRecvBuf, int *Length, int SendSec);

	// [#2075] NH KSK 2011.06.27 RKT 지원 (Multi Transaction 지원을 위해 함수 추가)
	BOOL		RKTConnectCheck();
	int			RKTConnectOpen();
	int			RKTConnectClose();
	int			RKTSendData(BYTE *pSendBuf, int Length, int SendSec, LPCTSTR Option);
	int			RKTRecvData(BYTE *pRecvBuf, int *Length, int RecvSec);
	// end of [#2075]

	// 상위국과의 INTERFACE를 맞추기 위해
	void		SetErrorCode(CString strErr);
	void		SetErrorMsg(CString strMsg);
	void		ClearErrorCode();
	CString		GetErrorCode();
	CString		GetErrorMsg();
	int			TestModem(LPCTSTR szTestNum, int nTestOption);
	int			TestModem2();																// [#GLDV-2683] NH Kook 2019.11.19 Modem Removal Detection
	int			TestNetwork(LPCTSTR szTestAddress, LPCTSTR szPort, LPCTSTR szSSLOption);	// [#585] NH KSK 2009.12.03
	static int	TestPing(LPCTSTR szTestAddress, int* pOutRTT);								// [#2358] US Kook 2015.07.14 support 'PING' function.
	void		RenewIPAddress();
	/**
	 * Returns the MAC address of the device or empty string if unsucessful.
	 */
	CString		GetMACAddress();	// [#RWC6-54] US William 2019.09.13 Add MAC Address to PAS	

	/**
	 * Combines all the known CA files into one for use by the networking functions
	 */
	static bool	InstallCAFiles();
};

#endif // !defined(AFX_NETWORK_H__1253B7C5_3AD4_46EF_9B90_17BC0B6AB7D3__INCLUDED_)
