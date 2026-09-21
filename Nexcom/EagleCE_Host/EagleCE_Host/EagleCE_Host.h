/** *************************************************************************************************************
*	@file EagleCE_Host.h 
*	@date 2017/10/20			
*	@author MFS
*	@brief EagleCE_Host프로그램의 상위 인터페이스를 구현한 헤더파일입니다. 
*****************************************************************************************************************/

/** *************************************************************************************************************
*	@mainpage EagleCE_Host 

*	@section INTRO 소개
	               - 메인페이지에서는 파일별 설명을 다룰 것입니다. @n
                     프로젝트는 프로토콜에 맞게 서버와 통신하기 위해 만들었습니다.

*	@section CREATEINFO 작성정보
  - 작성자      :   MFS  
  - 작성일      :   2017/10/19
*****************************************************************************************************************/

// EagleCE_Host.h : EagleCE_Host DLL의 기본 헤더 파일입니다.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH에 대해 이 파일을 포함하기 전에 'stdafx.h'를 포함합니다."
#endif

#include "resource.h"

#include "FlowCtrl.h"

typedef enum _FLOW_LINE
{
	VISAII_FLOW = 0,
	STANDARD_FLOW,
	AMS_FLOW,
	MAX_FLOWLINE
};

// CEagleCE_HostApp
// 이 클래스의 구현을 보려면 EagleCE_Host.cpp를 참조하십시오.
//

/** **************************************************************
*	@class CEagleCE_HostApp
*	@date 2017/10/20
*	@author MFS
*	@brief 프로그램의 상위 인터페이스를 구현한 클래스입니다. 
******************************************************************/

class CEagleCE_HostApp : public CWinApp
{
public:
	CEagleCE_HostApp();

public:


// 재정의입니다.
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance(); // return app exit code

	DECLARE_MESSAGE_MAP()
};

// TCP/IP HOST 함수 추가
extern "C" __declspec(dllexport) void	Set_FlowInform(int CtrlType, int LineType);
extern "C" __declspec(dllexport) int	Sync_Send_Host(BYTE *pSendBuffer, int nSendLen, LPCTSTR sHostInfo, LPCTSTR sHostPort = _T(""), LPCTSTR sSSLOption = _T("0"), BOOL bAMSSend=FALSE);
// [PCI-SSF Fix] pRecvBuffer의 실제 용량(nBufSize)을 함께 전달받아 내부에서 오버플로우 방지
extern "C" __declspec(dllexport) int	Sync_Recv_Host(BYTE *pRecvBuffer, int nBufSize, BOOL bAMSRecv=FALSE);
extern "C" __declspec(dllexport) int	Sync_ConnectTest_Host(LPCTSTR sHostInfo, LPCTSTR sHostPort = _T(""), LPCTSTR sSSLOption = _T("0"));
extern "C" __declspec(dllexport) int	Sync_PingTest_Host(LPCTSTR sHostInfo, ICMP_ECHO_REPLY &icmpEchoReply);

// Modem Only
extern "C" __declspec(dllexport) int	Async_PreDialStart(LPCTSTR sHostInfo);
extern "C" __declspec(dllexport) int	Sync_PreDialCancelByUser();
extern "C" __declspec(dllexport) BOOL	Sync_CloseLine();

// AMS
extern "C" __declspec(dllexport) int	Sync_ConnectOpen_AMS(LPCTSTR OpenType, LPCTSTR pDestInfo=_T(""), LPCTSTR port=_T(""), LPCTSTR option=_T(""));
extern "C" __declspec(dllexport) int	Sync_ConnectClose_AMS();
extern "C" __declspec(dllexport) BOOL	Async_ConnectCheck_AMS();
extern "C" __declspec(dllexport) int	Async_ConnectAccept_AMS();
