#include "stdafx.h"
#include "DEV_Define.h"
#include "DEV_HOST.h"
#include "../../EagleCE_Framework/EagleCE_Framework/Util.h"
#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"


CDEV_HOST::CDEV_HOST()
{
	// DLL 인스턴스 취득
	m_hDll = LoadLibrary(EAGLECE_ATM_HOST_DLL);

	// API 취득
	LibSet_FlowInform = (Set_FlowInform)GetProcAddress(m_hDll, _T("Set_FlowInform"));
	if (LibSet_FlowInform == NULL)
		LOG(Error, _T("Set_FlowInform Function Load failed"));

	LibSync_Send_Host = (Sync_Send_Host)GetProcAddress(m_hDll, _T("Sync_Send_Host"));
	if (LibSync_Send_Host == NULL)
		LOG(Error, _T("Sync_Send_Host Function Load failed"));

	LibSync_Recv_Host = (Sync_Recv_Host)GetProcAddress(m_hDll, _T("Sync_Recv_Host"));
	if (LibSync_Recv_Host == NULL)
		LOG(Error, _T("Sync_Recv_Host Function Load failed"));

	LibSync_ConnectTest_Host = (Sync_ConnectTest_Host)GetProcAddress(m_hDll, _T("Sync_ConnectTest_Host"));
	if (LibSync_ConnectTest_Host == NULL)
		LOG(Error, _T("Sync_ConnectTest_Host Function Load failed"));

	LibSync_PingTest_Host = (Sync_PingTest_Host)GetProcAddress(m_hDll, _T("Sync_PingTest_Host"));
	if (LibSync_PingTest_Host == NULL)
		LOG(Error, _T("Sync_PingTest_Host Function Load failed"));

	LibAsync_PreDialStart = (Async_PreDialStart)GetProcAddress(m_hDll, _T("Async_PreDialStart"));
	if (LibAsync_PreDialStart == NULL)
		LOG(Error, _T("Async_PreDialStart Function Load failed"));

	LibSync_PreDialCancelbyUser = (Sync_PreDialCancelbyUser)GetProcAddress(m_hDll, _T("Sync_PreDialCancelByUser"));
	if (LibSync_PreDialCancelbyUser == NULL)
		LOG(Error, _T("Sync_PreDialCancelByUser Function Load failed"));

	LibSync_CloseLine = (Sync_CloseLine)GetProcAddress(m_hDll, _T("Sync_CloseLine"));
	if (LibSync_CloseLine == NULL)
		LOG(Error, _T("Sync_CloseLine Function Load failed"));

	LibSync_ConnectOpen_AMS = (Sync_ConnectOpen_AMS)GetProcAddress(m_hDll, _T("Sync_ConnectOpen_AMS"));
	if (LibSync_ConnectOpen_AMS == NULL)
		LOG(Error, _T("LibSync_ConnectOpen_AMS Function Load failed"));

	LibSync_ConnectClose_AMS = (Sync_ConnectClose_AMS)GetProcAddress(m_hDll, _T("Sync_ConnectClose_AMS"));
	if (LibSync_ConnectClose_AMS == NULL)
		LOG(Error, _T("LibSync_ConnectClose_AMS Function Load failed"));

	LibAsync_ConnectCheck_AMS = (Async_ConnectCheck_AMS)GetProcAddress(m_hDll, _T("Async_ConnectCheck_AMS"));
	if (LibAsync_ConnectCheck_AMS == NULL)
		LOG(Error, _T("LibAsync_ConnectCheck_AMS Function Load failed"));

	LibAsync_ConnectAccept_AMS = (Async_ConnectAccept_AMS)GetProcAddress(m_hDll, _T("Async_ConnectAccept_AMS"));
	if (LibAsync_ConnectAccept_AMS == NULL)
		LOG(Error, _T("LibAsync_ConnectAccept_AMS Function Load failed"));


	m_nLineType = AMS_LINE_TCP;
	m_nJournalCount = 0;
	m_nAMSProcessStatus = AMS_STATUS_INIT;

	// AMS에서 사용할 Send / Recv Buffer 생성

	m_pAMSSendBuffer = NULL;
	m_pAMSSendBuffer = new BYTE[NET_SEND_BUFF_SIZE];

	m_pAMSRecvBuffer = NULL;
	m_pAMSRecvBuffer = new BYTE[NET_RECV_BUFF_SIZE];
}


CDEV_HOST::~CDEV_HOST()
{

	if (m_pAMSSendBuffer != NULL)
		delete [] m_pAMSSendBuffer;

	if (m_pAMSRecvBuffer != NULL)
		delete [] m_pAMSRecvBuffer;

	if (m_hDll)
	{
		FreeLibrary(m_hDll);

		m_hDll = NULL;
	}
}


/** **********************************************************
*	@brief		Is DLL Loaded
*	@retval		없음
************************************************************/
BOOL CDEV_HOST::IsDllLoaded()
{
	return (NULL != m_hDll);
}


/** **********************************************************
*	@brief		HOST_Set_Flow_LineType
*	@retval		없음
************************************************************/
void CDEV_HOST::HOST_Set_Flow_LineType(int CtrlType, int LineType)
{
	if (LibSet_FlowInform != NULL)
		LibSet_FlowInform(CtrlType, LineType);
}


/** **********************************************************
*	@brief		HOST_Sync_Send
*	@retval		없음
************************************************************/
int CDEV_HOST::HOST_Sync_Send(BYTE *pSendBuffer, int nSendLen, LPCTSTR sHostInfo, LPCTSTR sHostPort, LPCTSTR sSSLOption, BOOL bAMSSend/*=FALSE*/)
{
	int nReturn = 1;	// Default : Connect Fail
	
	if (LibSync_Send_Host != NULL)
		nReturn = LibSync_Send_Host(pSendBuffer, nSendLen, sHostInfo, sHostPort, sSSLOption, bAMSSend);

	return nReturn;
}


/** **********************************************************
*	@brief		HOST_Sync_Recv
*	@retval		없음
************************************************************/
int CDEV_HOST::HOST_Sync_Recv(BYTE *pRecvBuffer, int nBufSize, BOOL bAMSRecv/*=FALSE*/)
{
	int nReturn = 1;	// Default : Connect Fail

	if (LibSync_Recv_Host != NULL)
		nReturn = LibSync_Recv_Host(pRecvBuffer, nBufSize, bAMSRecv);

	return nReturn;
}


/** **********************************************************
*	@brief		HOST_Sync_ConnectTest
*	@retval		성공 0, 실패 : 그 외의 값
************************************************************/
int CDEV_HOST::HOST_Sync_ConnectTest(LPCTSTR sHostInfo, LPCTSTR sHostPort, LPCTSTR sSSLOption)
{
	int nReturn = 1;	// Default : Connect Fail

#if (NETWORK_OFFLINE_MODE)
	return 0;	// Offline Mode시에는 무조건 success로 return 처리
#endif

	if (LibSync_ConnectTest_Host != NULL)
		nReturn = LibSync_ConnectTest_Host(sHostInfo, sHostPort, sSSLOption);

	return nReturn;
}


/** **********************************************************
*	@brief		HOST_Sync_PingTest
*	@param LPCTSTR sHostInfo	서버의 아이피 주소
*	@param ICMP_ECHO_REPLY &icmpEchoReply	응답 정보
*	@retval		성공 0, 실패 : 그 외의 값
************************************************************/
int CDEV_HOST::HOST_Sync_PingTest(LPCTSTR sHostInfo, ICMP_ECHO_REPLY &icmpEchoReply)
{
	int nReturn = CONNECT_FAIL;	// Default : Connect Fail

#if (NETWORK_OFFLINE_MODE)
	return 0;	// Offline Mode시에는 무조건 success로 return 처리
#endif

	if (LibSync_PingTest_Host != NULL)
		nReturn = LibSync_PingTest_Host(sHostInfo, icmpEchoReply);

	return nReturn;
}


/** **********************************************************
*	@brief		HOST_Async_PreDialStart
*	@retval		성공 0, 실패 : 그 외의 값
************************************************************/
int CDEV_HOST::HOST_Async_PreDialStart(LPCTSTR sHostInfo)
{
	int nReturn = 1;	// Default : Connect Fail

#if (NETWORK_OFFLINE_MODE)
	return 0;	// Offline Mode시에는 무조건 success로 return 처리
#endif

	if (LibAsync_PreDialStart != NULL)
		nReturn = LibAsync_PreDialStart(sHostInfo);

	return nReturn;
}


/** **********************************************************
*	@brief		HOST_Sync_PreDialCancelbyUser
*	@retval		성공 0, 실패 : 그 외의 값
************************************************************/
void CDEV_HOST::HOST_Sync_PreDialCancelbyUser()
{

#if (NETWORK_OFFLINE_MODE)
	return;
#endif

	if (LibSync_PreDialCancelbyUser != NULL)
		LibSync_PreDialCancelbyUser();
}


/** **********************************************************
*	@brief		HOST_Sync_CloseLine
*	@retval		성공 0, 실패 : 그 외의 값
************************************************************/
void CDEV_HOST::HOST_Sync_CloseLine()
{

#if (NETWORK_OFFLINE_MODE)
	return;
#endif

	if (LibSync_CloseLine != NULL)
		LibSync_CloseLine();
}


/** **********************************************************
*	@brief		AMS_Sync_Communication
*	@retval		성공 : TRUE, 실패 : FALSE
************************************************************/
BOOL CDEV_HOST::AMS_Sync_Communication(int nOpenType, CString strSendCommand/*=AMS_REQ_STATUS*/)
{
	BOOL bResult = FALSE;

#if (NETWORK_OFFLINE_MODE)
	return TRUE;	// Offline Mode시에는 무조건 success로 return 처리
#endif

	bResult = AMS_Proc_Comm(nOpenType, strSendCommand);

	return bResult;
}


/** **********************************************************
*	@brief		AMS_Sync_ConnectOpen
*	@param LPCTSTR pDestInfo	AMS 정보
*	@param LPCTSTR port			AMS port 번호
*	@param LPCTSTR option		SSL 사용 여부 
*	@param LPCTSTR OpenType		Listen인지 Connect인지 
*	@retval	0: 성공, 그외: 실패
************************************************************/
int CDEV_HOST::AMS_Sync_ConnectOpen(LPCTSTR OpenType, LPCTSTR pDestInfo/*=_T("")*/, LPCTSTR port/*=_T("0")*/, LPCTSTR option/*=_T("")*/)
{
	int nReturn = 1;	// Default : Connect Fail

#if (NETWORK_OFFLINE_MODE)
	return 0;	// Offline Mode시에는 무조건 success로 return 처리
#endif

 	if (LibSync_ConnectOpen_AMS != NULL)
 		nReturn = LibSync_ConnectOpen_AMS(OpenType, pDestInfo, port, option);

	return nReturn;
}


/** **********************************************************
*	@brief		AMS_Sync_ConnectClose
*	@retval	0: 성공, 그외: 실패
************************************************************/
int CDEV_HOST::AMS_Sync_ConnectClose()
{
	int nReturn = 1;	// Default : Connect Fail

#if (NETWORK_OFFLINE_MODE)
	return 0;	// Offline Mode시에는 무조건 success로 return 처리
#endif

	if (LibSync_ConnectClose_AMS != NULL)
 		nReturn = LibSync_ConnectClose_AMS();

	return nReturn;
}


/** **********************************************************
*	@brief		AMS_Async_ConnectCheck
*	@retval	TRUE: 성공, FALSE: 실패
************************************************************/
BOOL CDEV_HOST::AMS_Async_ConnectCheck()
{
	BOOL bReturn = FALSE;	// Default : Connect Fail

#if (NETWORK_OFFLINE_MODE)
	return FALSE;	// Offline Mode시에는 무조건 success로 return 처리
#endif

	if (LibAsync_ConnectCheck_AMS != NULL)
		bReturn = LibAsync_ConnectCheck_AMS();

	return bReturn;
}


/** **********************************************************
*	@brief		AMS_Async_ConnectAccept
*	@retval	0: 성공, 그외: 실패
************************************************************/
int CDEV_HOST::AMS_Async_ConnectAccept()
{
	int nReturn = 1;	// Default : Connect Fail

#if (NETWORK_OFFLINE_MODE)
	return 0;	// Offline Mode시에는 무조건 success로 return 처리
#endif

	if (LibAsync_ConnectAccept_AMS != NULL)
		nReturn = LibAsync_ConnectAccept_AMS();

	return nReturn;
}

