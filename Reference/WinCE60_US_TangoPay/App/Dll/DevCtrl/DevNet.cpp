/////////////////////////////////////////////////////////////////////////////
//	DevNet.cpp : implementation file
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include ".\Common\CmnLib.h"

//#define NH_DEBUG		// Debug Message On/Off

#include ".\Common\NHDbgApi.h"
#define	DBG_CALL		1
#define DBG_INFO		1

#include ".\Common\ConstDef.h"

#include ".\Dev\DevDefine.h"
#include ".\Net\Network.h"
#include ".\Dev\DevCmn.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
//	NETWORK FUNCTION(NET) : NET SET MODE
/////////////////////////////////////////////////////////////////////////////
// Clear Error Code
int	CDevCmn::fnNET_ClearErrorCode()
{
	m_pNetWork->ClearErrorCode();
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//	NETWORK FUNCTION(NET) : NET GET MODE
/////////////////////////////////////////////////////////////////////////////
// Get Error Code
CString CDevCmn::fstrNET_GetErrorCode()
{
	CString strReturn;
	
	strReturn = m_pNetWork->GetErrorCode();

	return strReturn;
}

// Get Error Message
CString CDevCmn::fstrNET_GetErrorMsg()
{
	CString strReturn;

	strReturn = m_pNetWork->GetErrorMsg();

	return strReturn;
}

// [#525] US KSK 2009.05.25 Pre Dial Interface Function
int CDevCmn::fnNET_PreDialStart()
{
#ifdef APP_LOCAL_MODE
	return TRUE;
#endif

	int		nReturn = FALSE;
	
	nReturn = m_pNetWork->PreDialStart();
	
	return nReturn;
}

int CDevCmn::fnNET_PreDialCancelByUser()
{
#ifdef APP_LOCAL_MODE
	return TRUE;
#endif

	int		nReturn = FALSE;
	
	nReturn = m_pNetWork->PreDialCancelByUser();
	
	return nReturn;
}
// end of [#525]

/////////////////////////////////////////////////////////////////////////////
//	NETWORK FUNCTION(NET) : NET OPERATION MODE
/////////////////////////////////////////////////////////////////////////////
// Initialize

//int	CDevCmn::fnNET_SendData(BYTE* szSendData, int nLength, int nSendSec)
int	CDevCmn::fnNET_SendData(BYTE* szSendData, int nLength, int nSendSec, int n3rdTranType, LPCTSTR sHostInfo)	// [#2412] NH Justin 2016.04.05	Support Extra Host Cert)
{
NHDEBUG(1, (_T("***DevNet.cpp***CDevCmn::fnNET_SendData() \n")));

#ifdef APP_LOCAL_MODE
	return TRUE;
#endif

	int		nReturn = FALSE;
	// [#2012] NH KJW 2011.02.09 static 선언하여 1회만 생성되고, static 영역에 잡히도록 함.
	//BYTE	SendData[NETBUFFSIZE] = { 0, };
	static BYTE	SendData[NETBUF_SEND_SIZE];
	memset(SendData, 0, NETBUF_SEND_SIZE);
	// end of [#2012]

	memcpy(SendData, szSendData, nLength);

	// [#2012] NH KJW 2011.01.26 Lottery URL, SSL 지원
	nReturn = m_pNetWork->SendData(SendData, nLength, nSendSec, n3rdTranType, sHostInfo);	// [#2412] NH Justin 2016.04.05	Support Extra Host Cert)

	return nReturn;
}

// [#2054] NH KJW 2011.05.03 DevCmn -> TranCtrl로 Recv Data 넘길 때, 별도의 Unpack없이 복사하도록 수정함.
/*
// Recv Data(Recv Data, Wait Time)
// [#2012] NH KJW 2011.01.26 Lottery URL, SSL 지원
//int CDevCmn::fnNET_RecvData(CString& strRecvData, int nRecvSec)
int CDevCmn::fnNET_RecvData(CString& strRecvData, int nRecvSec, int n3rdTranType)
// end of [#2012]
{
#ifdef APP_LOCAL_MODE
	return TRUE;
#endif

	int		nReturn = FALSE;
	// [#2012] NH KJW 2011.02.09 동적할당으로 변경함.
	//BYTE	RecvData[NETBUFFSIZE];
	//int		nRecvLength = NETBUFFSIZE;	// [#89] KSK 2008.04.14 V01.02.29 REVIEW8
	int nRecvLength = NETBUF_RECV_SIZE;
	// end of [#2012]
	
	// [#2012] NH KJW 2011.02.09 동적할당으로 변경함. 
	//memset(RecvData, 0, NETBUFFSIZE);
	//nReturn = m_pNetWork->RecvData(RecvData, &nRecvLength, nRecvSec);
	memset(m_pRecvData, 0, NETBUF_RECV_SIZE);
	nReturn = m_pNetWork->RecvData((PBYTE)m_pRecvData, &nRecvLength, nRecvSec, n3rdTranType);
	// end of [#2012]
	NHDEBUG(1, (L"nReturn(%d),nRecvLength(%d)\n", nReturn, nRecvLength));

	// [#2012] NH KJW 2011.02.09 동적할당으로 변경함. 
	//strFromRecv.Format(L"%S", RecvData);
	CString strFromRecv( m_pRecvData, nRecvLength );
	// end of [#2012]
	MakeUnPack(strFromRecv, strRecvData);
NHDEBUG(1, (L"strFromRecv.GetLength()(%d), strRecvData.GetLength()(%d)\n", strFromRecv.GetLength(), strRecvData.GetLength()));

	return nReturn;
}
*/
int CDevCmn::fnNET_RecvData(LPBYTE pRecvBuffer, int *pnRecvLen, int nRecvSec, int n3rdTranType)
{
#ifdef APP_LOCAL_MODE
	return TRUE;
#endif

	int		nReturn = 0; // 0: OK, 1: m_nCurrentFlowLine min error, 2: m_nCurrentFlowLine max error, 3: m_RecvLen == 0 in CFlowCtrl::RecvData()
	
	nReturn = m_pNetWork->RecvData(pRecvBuffer, pnRecvLen, nRecvSec, n3rdTranType);
	NHDEBUG(1, (L"nReturn(%d),*pnRecvLen(%d)\n", nReturn, *pnRecvLen));

	return nReturn;
}
// end of [#2054]

int CDevCmn::fnNET_CloseLine()
{
	if (m_pNetWork != NULL)
		m_pNetWork->CloseLine();

	return 0;
}

// Modem Test Module
int	CDevCmn::fnNET_ModemTest(LPCTSTR szTestNum, int nTestOption)
{
	int nRet;

	nRet = m_pNetWork->TestModem(szTestNum, nTestOption);

	return nRet;
}

// [#GLDV-2683] NH Kook 2019.11.19 Modem Removal Detection
int	CDevCmn::fnNET_ModemTest2()
{
#ifdef UNDER_CE
#  define MODEM_TEST_COUNT	3
#else
#  define MODEM_TEST_COUNT 0
#endif
	for (int nRetryCnt = 1; nRetryCnt <= MODEM_TEST_COUNT; nRetryCnt++)
	{
		NHDEBUG(1, (_T("Checking Modem Status [%d / %d] ...\n"), nRetryCnt, MODEM_TEST_COUNT));
		// send AT command to modem and check its response.
		if (m_pNetWork->TestModem2() == TRUE)
		{
			NHDEBUG(1, (_T("-> Modem Status : OK \n")));
			return TRUE;
		}
		else
		{
			NHDEBUG(1, (_T("-> Modem Status : NG \n")));
		}
	}

	return FALSE;
}
// end of [#GLDV-2683]

// [#585] NH KSK 2009.12.03
// Network Test Module
int	CDevCmn::fnNET_NetworkTest(LPCTSTR szTestAddress, LPCTSTR szPort, LPCTSTR szSSLOption)
{
	int nRet;
	
	nRet = m_pNetWork->TestNetwork(szTestAddress, szPort, szSSLOption);
	
	return nRet;
}
// end of [#585]

// [#2358] US Kook 2015.07.14 support 'PING' function.
int CDevCmn::fnNET_PingTest(LPCTSTR szTestAddress, int* pOutRTT)
{
	return m_pNetWork->TestPing(szTestAddress, pOutRTT);
}
// end of [#2358]

BOOL CDevCmn::fbNET_RMSConnectCheck(void)
{
	return m_pNetWork->RMSConnectCheck();
}

int CDevCmn::fnNET_RMSConnectAccept(void)
{
	return m_pNetWork->RMSConnectAccept();
}

int	CDevCmn::fnNET_RMSConnectOpen(int OpenType)		// [#122] NH AIREAT 2008.04.22 파라미터 추가
{
	return m_pNetWork->RMSConnectOpen(OpenType);
}

int CDevCmn::fnNET_RMSConnectClose(void)
{
	return m_pNetWork->RMSConnectClose();
}

int	CDevCmn::fnNET_RMSSendData(BYTE *pSendData, int nLen, int nSendSec)
{
	return m_pNetWork->RMSSendData(pSendData, nLen, nSendSec);
}

int	CDevCmn::fnNET_RMSRecvData(BYTE *pRecvData, int *pnLen, int nRecvSec)
{
	return m_pNetWork->RMSRecvData(pRecvData, pnLen, nRecvSec);
}

// [#2075] NH KSK 2011.06.27
int	CDevCmn::fnNET_RKTConnectOpen()
{
	return m_pNetWork->RKTConnectOpen();
}

int	CDevCmn::fnNET_RKTConnectClose()
{
	return m_pNetWork->RKTConnectClose();
}

BOOL CDevCmn::fbNET_RKTConnectCheck()
{
	return m_pNetWork->RKTConnectCheck();
}

int	CDevCmn::fnNET_RKTSendData(BYTE *pSendData, int nLen, int nSendSec, LPCTSTR Option)
{
	return m_pNetWork->RKTSendData(pSendData, nLen, nSendSec, Option);
}

int	CDevCmn::fnNET_RKTRecvData(BYTE *pRecvData, int *pnLen, int nRecvSec)
{
	return m_pNetWork->RKTRecvData(pRecvData, pnLen, nRecvSec);
}
// end of [#2075]
