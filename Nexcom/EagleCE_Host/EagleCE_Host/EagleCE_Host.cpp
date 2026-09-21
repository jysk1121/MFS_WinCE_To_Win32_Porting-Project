/** ****************************************************************************************************************************************************************
*	@file EagleCE_Host.cpp 
*	@date 2017/10/20			
*	@author MFS
*	@brief EagleCE_Host프로그램의 상위 인터페이스를 구현한 소스파일입니다. 
********************************************************************************************************************************************************************/



// EagleCE_Host.cpp : 해당 DLL의 초기화 루틴을 정의합니다.
//

#include "stdafx.h"
#include "EagleCE_Host.h"

#include "VISAIICtrl.h"
#include "StandardCtrl.h"
#include "AMSCtrl.h"

#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//TODO: 이 DLL이 MFC DLL에 대해 동적으로 링크되어 있는 경우
//		MFC로 호출되는 이 DLL에서 내보내지는 모든 함수의
//		시작 부분에 AFX_MANAGE_STATE 매크로가
//		들어 있어야 합니다.
//
//		예:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// 일반적인 함수 본문은 여기에 옵니다.
//		}
//
//		이 매크로는 MFC로 호출하기 전에
//		각 함수에 반드시 들어 있어야 합니다.
//		즉, 매크로는 함수의 첫 번째 문이어야 하며 
//		개체 변수의 생성자가 MFC DLL로
//		호출할 수 있으므로 개체 변수가 선언되기 전에
//		나와야 합니다.
//
//		자세한 내용은
//		MFC Technical Note 33 및 58을 참조하십시오.
//


// CEagleCE_HostApp

BEGIN_MESSAGE_MAP(CEagleCE_HostApp, CWinApp)
END_MESSAGE_MAP()


// CEagleCE_HostApp 생성

int				m_nCurrentFlowLine; // = VISAII_FLOW;
int				m_nCurrentLine;		// = 0;
CFlowCtrl*		m_FlowAndLine[MAX_FLOWLINE];

CEagleCE_HostApp::CEagleCE_HostApp()
{
	// TODO: 여기에 생성 코드를 추가합니다.
	// InitInstance에 모든 중요한 초기화 작업을 배치합니다.

	// 로거 생성
	CEagleLogger::CreateInstance();
	CEagleLogger::GetInstance()->Initialize(_T("EagleCE_Host"), 30, 1024 * 1024);

	m_nCurrentFlowLine = VISAII_FLOW;
	m_nCurrentLine = 0;

	for (int i=0; i<MAX_FLOWLINE; i++)
		m_FlowAndLine[i] = NULL;
}


// 유일한 CEagleCE_HostApp 개체입니다.

CEagleCE_HostApp theApp;


// CEagleCE_HostApp 초기화

BOOL CEagleCE_HostApp::InitInstance()
{
	CWinApp::InitInstance();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	return TRUE;
}

int CEagleCE_HostApp::ExitInstance()
{
	CEagleLogger::GetInstance()->Finalize();

	CEagleLogger::ReleaseInstance();

	CWinApp::ExitInstance();

	return 0;
}

/** ********************************************************************
* @brief 프로토콜 타입을 결정하고 클래스를 생성합니다. 
* @param int CtrlType 프로토콜 타입
* @param int LineType 라인 타입 
* @retval 없음 
************************************************************************/
extern "C" void  Set_FlowInform(int CtrlType, int LineType)
{
	LOG(Info, _T("Set_FlowInform call (%d) (%d)"), CtrlType, LineType);

	// set VISAII or STANDARD TCP/IP
	if (CtrlType < VISAII_FLOW || CtrlType > MAX_FLOWLINE)
	{
		LOG(Error, _T("Set_FlowInform Ctrl Type Error (%d) (%d)"), CtrlType, LineType);
		return;
	}

	if (LineType < LINE_TCP || LineType >= MAX_LINETYPE)
	{
		LOG(Error, _T("Set_FlowInform Line Type Error (%d) (%d)"), CtrlType, LineType);
		return;
	}

	m_nCurrentFlowLine = CtrlType;
	m_nCurrentLine = LineType;

	// Protocol class create
	for (int i = VISAII_FLOW; i < MAX_FLOWLINE; i++)
	{
		// 이전에 생성한 Control Class Memory 해제 후 재 생성
		if (m_FlowAndLine[i] != NULL)
			delete m_FlowAndLine[i];

		m_FlowAndLine[i] = NULL;

		switch(i)
		{
		case VISAII_FLOW:
			LOG(Info, _T("VISA Flow Create"));
			m_FlowAndLine[i] = (CFlowCtrl*) new CVISAIICtrl(m_nCurrentLine);
			break;

		case STANDARD_FLOW:
			LOG(Info, _T("Standard Flow Create"));
			m_FlowAndLine[i] = (CFlowCtrl*) new CStandardCtrl(m_nCurrentLine);
			break;

		case AMS_FLOW:
			// AMS listens for an inbound connection, which libcurl cannot do,
			// so this flow always stays on the OpenSSL TCP line even when the
			// host flows above have been switched to LINE_CURL.
			LOG(Info, _T("AMS Flow Create"));
			m_FlowAndLine[i] = (CFlowCtrl*) new CAMSCtrl((m_nCurrentLine == LINE_CURL) ? LINE_TCP : m_nCurrentLine);
			break;

		default:
			break;
		}
	}
	LOG(Info, _T("Set_FlowInform Flow end"));
}

/** ********************************************************************
* @brief Sync_Send_Host 인터페이스입니다. 
* @param LPBYTE pSendBuffer 보내는 버퍼
* @param int nSendLen  보내는 버퍼의 길이 
* @param LPCTSTR sHostInfo 서버의 아이피 주소
* @param LPCTSTR sHostPort 서버의 포트 주소 
* @param LPCTSTR sSSLOption SSL 통신 옵션 
* @param BOOL bAMSSend AMS 통신 여부 
* @retval 0 성공 
* @retval nResult 에러코드가 정의됨.
************************************************************************/
extern "C" int  Sync_Send_Host(BYTE *pSendBuffer, int nSendLen, LPCTSTR sHostInfo, LPCTSTR sHostPort, LPCTSTR sSSLOption, BOOL bAMSSend/*=FALSE*/)
{
	// return code define
	//  1: "Modem Dial Connection Timeout" or "TCPIP Connection Timeout"
	//  2: "No Dial Tone"
	//  3: "No Answer"
	//  4: "Line Busy"
	//	5: "No Carrier(No ENQ Received)"
	//  6: "No ACK/NAK Received"
	//  7: "No Message Data Received"
	//  8: "No EOT received from HOST"
	// 10:	"NAK retry exceed"
	// 11:	"Send retry exceed"
	// 12:	"Modem Initialize fail"
	// 13:	"Modem open fail"
	// 22:	"SSL Connection failure";
	if (bAMSSend == FALSE)				// HOST 통신시에만 Log를 남기도록 변경
		LOG(Info, _T("Sync_Send_Host call (%d) (%s) (%s) (%s)"), nSendLen, sHostInfo, sHostPort, sSSLOption);

	int nResult = COMM_ERROR;

	if(TRUE == bAMSSend)
	{
		if (m_FlowAndLine[AMS_FLOW] != NULL)
			nResult = m_FlowAndLine[AMS_FLOW]->SendData(pSendBuffer, nSendLen, sHostInfo, sHostPort, sSSLOption);
	}
	else
	{
		if (m_FlowAndLine[m_nCurrentFlowLine] != NULL)
			nResult = m_FlowAndLine[m_nCurrentFlowLine]->SendData(pSendBuffer, nSendLen, sHostInfo, sHostPort, sSSLOption);
	}

	switch(nResult)
	{
		case 0:
			nResult = SUCCESS;
			break;

		case 1:
		case 2:
		case 3:
		case 4:
			nResult = CONNECT_FAIL;
			break;

		case 5:
			nResult = RECV_FAIL_ENQ;
			break;

		case 6:
			nResult = RECV_FAIL_ACK;
			break;

		case 7:
			nResult = RECV_FAIL_DATA;
			break;

		case 8:
			nResult = RECV_FAIL_EOT;
			break;

		default:
			// 그 외의 경우는 communication error로 처리
			break;
	}

	if (nResult != SUCCESS)
	{
		LOG(Error, _T("Communication is Failed : (%02d)"), nResult);
	}

	
	//LOG(Info, _T("Sync_Send_Host end"));

	return 	nResult;
}


/** ********************************************************************
* @brief Sync_Recv_Host 인터페이스입니다. 
* @param LPBYTE pRecvBuffer 받는 버퍼
* @param BOOL bAMSRecv AMS 통신 여부 
* @retval 받는 버퍼의 길이
************************************************************************/
extern "C" int  Sync_Recv_Host(BYTE *pRecvBuffer, int nBufSize, BOOL bAMSRecv/*=FALSE*/)
{
	if (bAMSRecv == FALSE)
		LOG(Info, _T("Sync_Recv_Host call"));

	int nRecvLength = 0;

	if(TRUE == bAMSRecv)
	{
		if (m_FlowAndLine[AMS_FLOW] != NULL)
			nRecvLength = m_FlowAndLine[AMS_FLOW]->RecvData(pRecvBuffer, nBufSize);
	}
	else
	{
		if (m_FlowAndLine[m_nCurrentFlowLine] != NULL)
			nRecvLength = m_FlowAndLine[m_nCurrentFlowLine]->RecvData(pRecvBuffer, nBufSize);
	}

	//LOG(Info, _T("Sync_Recv_Host end"));

	return 	nRecvLength;
}


/** ********************************************************************
* @brief Sync_ConnectTest_Host 인터페이스입니다. 
* @param LPCTSTR sHostInfo 서버의 아이피 주소
* @param LPCTSTR sHostPort 서버의 포트 주소 
* @param LPCTSTR sSSLOption SSL 통신 옵션 
* @retval 0 성공 
* @retval nResult 에러코드가 정의됨. 
************************************************************************/
extern "C" int  Sync_ConnectTest_Host(LPCTSTR sHostInfo, LPCTSTR sHostPort, LPCTSTR sSSLOption)
{
	LOG(Info, _T("Sync_ConnectTest_Host call"));

	int nRecvLength = 0;

	if (m_FlowAndLine[m_nCurrentFlowLine] != NULL)
		nRecvLength = m_FlowAndLine[m_nCurrentFlowLine]->SendData((BYTE*)"NETWORK_TEST", 4, sHostInfo, sHostPort, sSSLOption);

	LOG(Info, _T("Sync_Recv_Host end"));

	return 	nRecvLength;
}


/** ********************************************************************
* @brief Sync_PingTest_Host 인터페이스입니다. 
* @param LPCTSTR sHostInfo	서버의 아이피 주소
* @param ICMP_ECHO_REPLY &icmpEchoReply	응답 정보
* @param UCHAR &cTTL		Time To Live
* @retval 0 성공 
* @retval nResult 에러코드가 정의됨. 
************************************************************************/
extern "C" int  Sync_PingTest_Host(LPCTSTR sHostInfo, ICMP_ECHO_REPLY &icmpEchoReply)
{
	LOG(Info, _T("Sync_PingTest_Host call (%s)"), sHostInfo);

	int nReturn = CONNECT_FAIL;

	if (m_FlowAndLine[m_nCurrentFlowLine] != NULL)
		nReturn = m_FlowAndLine[m_nCurrentFlowLine]->PingTest(sHostInfo, icmpEchoReply);

	LOG(Info, _T("Sync_PingTest_Host end"));

	return 	nReturn;
}


/** ********************************************************************
* @brief PreDialStart Pre dialing시 사용하는 함수. 
* @param LPCTSTR sHostInfo 서버의 Phone Number
* @retval 0 성공 
* @retval nResult 에러코드가 정의됨. 
************************************************************************/
extern "C" int Async_PreDialStart(LPCTSTR sHostInfo)
{
	return m_FlowAndLine[m_nCurrentFlowLine]->PreDialStart(sHostInfo);
}


/** ********************************************************************
* @brief PreDialCancelByUser Pre dialing 중 고객에 의해 Line Close시에 사용하는 함수
* @retval 0 성공 
* @retval nResult 에러코드가 정의됨. 
************************************************************************/
extern "C" int Sync_PreDialCancelByUser()
{
	return m_FlowAndLine[m_nCurrentFlowLine]->PreDialCancelByUser();
}


/** ********************************************************************
* @brief CloseLine Network Line Close 함수
* @retval TRUE 성공 
* @retval FALSE 실패 
************************************************************************/
extern "C" BOOL Sync_CloseLine()
{
	return m_FlowAndLine[m_nCurrentFlowLine]->CloseLine();
}


/** ***********************************************************************************
*	@brief AMS 접속을 위해 네트웍을 Server 기능을 수행하게 한다.\n
			TCP일경우 port 번호를 주어야 한다.\n
			option은 나중에 SSL 사용 여부 이다.
*	@param LPCTSTR pDestInfo	AMS 정보
*	@param LPCTSTR port			AMS port 번호
*	@param LPCTSTR option		SSL 사용 여부 
*	@param LPCTSTR OpenType		Listen인지 Connect인지 
*	@retval 0		성공 
*	@retval 그외	장애
***************************************************************************************/
extern "C" int Sync_ConnectOpen_AMS(LPCTSTR OpenType, LPCTSTR pDestInfo/*=_T("")*/, LPCTSTR port/*=_T("")*/, LPCTSTR option/*=_T("0")*/)
{
	if (NULL == m_FlowAndLine[AMS_FLOW])
	{
		LOG(Error, _T("m_FlowAndLine[AMS_FLOW] is NULL"));
		return FALSE;
	}

	int nReturn = m_FlowAndLine[AMS_FLOW]->AMSConnectOpen(OpenType, pDestInfo, port, option);

	return nReturn;
}


/** ***********************************************************************************
*	@brief AMS ConnectClose
*	@retval 0		성공 
*	@retval 그외	장애
***************************************************************************************/
extern "C" int Sync_ConnectClose_AMS()
{
	if (NULL == m_FlowAndLine[AMS_FLOW])
	{
		LOG(Error, _T("m_FlowAndLine[AMS_FLOW] is NULL"));
		return FALSE;
	}

	int nReturn = m_FlowAndLine[AMS_FLOW]->AMSConnectClose();

	return nReturn;
}


/** ***********************************************************************************
*	@brief AMS ConnectCheck
*	@retval TRUE	연결 성공 
*	@retval FALSE	연결 실패
***************************************************************************************/
extern "C" BOOL Async_ConnectCheck_AMS()
{
	if (NULL == m_FlowAndLine[AMS_FLOW])
	{
		LOG(Error, _T("m_FlowAndLine[AMS_FLOW] is NULL"));
		return FALSE;
	}

	int nReturn = m_FlowAndLine[AMS_FLOW]->AMSConnectCheck();

	return nReturn;
}


/** ***********************************************************************************
*	@brief AMS ConnectAccept
*	@retval 0		성공 
*	@retval 그외	장애
***************************************************************************************/
extern "C" int Async_ConnectAccept_AMS()
{
	if (NULL == m_FlowAndLine[AMS_FLOW])
	{
		LOG(Error, _T("m_FlowAndLine[AMS_FLOW] is NULL"));
		return FALSE;
	}

	int nReturn = m_FlowAndLine[AMS_FLOW]->AMSConnectAccept();

	return nReturn;
}
