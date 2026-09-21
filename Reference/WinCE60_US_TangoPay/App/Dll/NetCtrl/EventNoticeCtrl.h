// EventNoticeCtrl.h: interface for the CEventNoticeCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EVENTNOTICECTRL_H__INCLUDED_)
#define AFX_EVENTNOTICECTRL_H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Queue.h"
#include ".\Net\FlowCtrl.h"

class CEventNoticeCtrl :public CFlowCtrl
{
public:
	CEventNoticeCtrl(CLineCtrl *line);
	virtual ~CEventNoticeCtrl();

public:
	virtual int		SendData(BYTE *pSendBuf, int Length, LPCTSTR pDestInfo, LPCTSTR port, LPCTSTR option);
	virtual BOOL	CloseLine();

private:
	int				Flow_Matrix(int nDataTimeOut, LPCTSTR strTerminate = NULL, int initStatus = INIT_STS, int nMode = 0 );
																																	// [#2574] US Justin Add Data Time Out
};

#endif // !defined(AFX_EVENTNOTICECTRL_H__INCLUDED_)