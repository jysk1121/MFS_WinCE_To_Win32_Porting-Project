// UseAckCtrl.h: interface for the CUseAckCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_USEACKCTRL_H__29176016_C428_4141_8DF1_00B3EC1EEC4A__INCLUDED_)
#define AFX_USEACKCTRL_H__29176016_C428_4141_8DF1_00B3EC1EEC4A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Queue.h"
#include ".\Net\FlowCtrl.h"

class CUseAckCtrl : public CFlowCtrl 
{
public:
	CUseAckCtrl(CLineCtrl *line);
	virtual ~CUseAckCtrl();

public:
	virtual int		SendData(BYTE *pSendBuf, int Length, LPCTSTR pDestInfo, LPCTSTR port, LPCTSTR option);
	//virtual int		RecvData(BYTE *pRecvBuf, int *Length, int WaitSec);

	// [#2075] NH KSK 2011.06.27
	virtual BOOL	RKTConnectCheck();
	virtual int		RKTConnectOpen(LPCTSTR pDestInfo, LPCTSTR port, LPCTSTR option);
	virtual int		RKTConnectClose();
	// end of [#2075]


private:
//	int				Flow_Matrix();
	int				Flow_Matrix(int initStatus = INIT_STS, int nMode = 0);	// [#2075] NH KSK 2011.06.27
};

#endif // !defined(AFX_USEACKCTRL_H__29176016_C428_4141_8DF1_00B3EC1EEC4A__INCLUDED_)
