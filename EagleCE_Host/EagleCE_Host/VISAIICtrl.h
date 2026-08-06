/** **********************************************************************************
*	@file VISAIICtrl.h
*	@date 2017/10/20	
*	@author MFS
*	@brief VISAII프로토콜을 구현한 헤더파일입니다. 
**************************************************************************************/


// VISAIICtrl.h: interface for the CVISAIICtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VISAIICTRL_H__85CB9902_649F_4DB1_9A6E_EB65C54327E8__INCLUDED_)
#define AFX_VISAIICTRL_H__85CB9902_649F_4DB1_9A6E_EB65C54327E8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Queue.h"
#include "FlowCtrl.h"

#include "../../EagleCE_Framework/EagleCE_Framework/EagleConfig.h"
/** ********************************************************************************
*	@class CVISAIICtrl 
*	@date 2017/10/19
*	@author MFS 
*	@brief VISAII 프로토콜에 맞는 플로우로 통신할 수 있게 도와주는 클래스입니다. 
************************************************************************************/
class CVISAIICtrl : public CFlowCtrl 
{
public:
	CVISAIICtrl(int LineType);
	virtual ~CVISAIICtrl();

private:
	BOOL			m_bPreDialStart;
	CEagleConfig	m_EagleConfig;

public:
	virtual int		SendData(BYTE *pSendBuf, int Length, LPCTSTR pDestInfo, LPCTSTR port, LPCTSTR option);
	virtual int		PreDialStart(CString strHostPhoneNumber);
	virtual int		PreDialCancelByUser();
	virtual BOOL	CloseLine();


private:
	int				Flow_Matrix(int initStatus = INIT_STS, int nMode = 0);
};

#endif // !defined(AFX_VISAIICTRL_H__85CB9902_649F_4DB1_9A6E_EB65C54327E8__INCLUDED_)
