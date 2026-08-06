/** **********************************************************************************
*	@file AMSCtrl.h
*	@date 2018/01/05	
*	@author MFS
*	@brief AMS 프로토콜을 구현한 헤더파일입니다. 
**************************************************************************************/

#if !defined(AFX_AMSCTRL_H__85CB9902_649F_4DB1_9A6E_EB65C54327E8__INCLUDED_)
#define AFX_AMSCTRL_H__85CB9902_649F_4DB1_9A6E_EB65C54327E8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Queue.h"
#include "FlowCtrl.h"

//#include "../../EagleCE_Framework/EagleCE_Framework/EagleConfig.h"
#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"
#include "../../EagleCE_Framework/EagleCE_Framework/Util.h"
// #include "../../Template/pkfuncs.h"
// #include "../../EagleCE_Device/EagleCE_Device/DEV_Manager.h"

#define AMS_OK	0
#define AMS_ERR	1

#define LEN_LENGTH	5	// AMSCtrl_Define.h에 중복 정의되어 있음.

#define TIMEOUT_AMS_RECEIVE_TCPIP	10

/** ********************************************************************************
*	@class CAMSCtrl 
*	@date 2018/01/05
*	@author MFS 
*	@brief AMS 프로토콜에 맞는 FLOW로 통신할 수 있게 도와주는 클래스입니다. 
************************************************************************************/
class CAMSCtrl : public CFlowCtrl
{
public:
	CAMSCtrl(int LineType);
	virtual ~CAMSCtrl(void);

private:
	BOOL	m_bOpen;
	int		m_LineType;

public:
	virtual int		SendData(BYTE *pSendBuf, int Length, LPCTSTR pDestInfo, LPCTSTR port, LPCTSTR option);
	virtual int		RecvData(BYTE *pRecvBuf, int nBufSize);

	virtual int		AMSConnectOpen(LPCTSTR OpenType, LPCTSTR pDestInfo=_T(""), LPCTSTR port=_T("0"), LPCTSTR option=_T("0"));
	virtual int		AMSConnectClose();
	virtual BOOL	AMSConnectCheck();
	virtual int		AMSConnectAccept();
//	virtual BOOL	AMSProcStart(int nOpenType, char chSubCommand=0);

private:
	int		Flow_Matrix(int initStatus);
};

#endif // !defined(AFX_AMSCTRL_H__85CB9902_649F_4DB1_9A6E_EB65C54327E8__INCLUDED_)