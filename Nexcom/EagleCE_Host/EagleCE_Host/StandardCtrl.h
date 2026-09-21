/** *************************************************************
*	@file StandardCtrl.h	
*	@date 2017/10/20
*	@author MFS
*	@brief Standard플로우를 구현한 헤더파일입니다.
***************************************************************/

// StandardCtrl.h: interface for the CStandardCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STANDARDCTRL_H__09C59415_E132_4CDC_8AA0_6892E6761D29__INCLUDED_)
#define AFX_STANDARDCTRL_H__09C59415_E132_4CDC_8AA0_6892E6761D29__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Queue.h"
#include "FlowCtrl.h"

#include "../../EagleCE_Framework/EagleCE_Framework/EagleConfig.h"

/** ********************************************************************************
*	@class CStandardCtrl
*	@date 2017/10/19
*	@author MFS 
*	@brief Standard프로토콜에 맞는 플로우로 통신할 수 있게 도와주는 클래스입니다. 
************************************************************************************/
class CStandardCtrl : public CFlowCtrl 
{
public:
	CStandardCtrl(int LineType);
	virtual ~CStandardCtrl();

public:
	virtual int		SendData(BYTE *pSendBuf, int Length, LPCTSTR pDestInfo, LPCTSTR port, LPCTSTR option);
	virtual BOOL	CloseLine();

private:
	int				Flow_Matrix(int initStatus = INIT_STS, int nMode = 0);

	CEagleConfig	m_EagleConfig;

};

#endif // !defined(AFX_STANDARDCTRL_H__09C59415_E132_4CDC_8AA0_6892E6761D29__INCLUDED_)
