// StandardCtrl.h: interface for the CStandardCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STANDARDCTRL_H__09C59415_E132_4CDC_8AA0_6892E6761D29__INCLUDED_)
#define AFX_STANDARDCTRL_H__09C59415_E132_4CDC_8AA0_6892E6761D29__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Queue.h"
#include ".\Net\FlowCtrl.h"

class CStandardCtrl : public CFlowCtrl 
{
public:
	CStandardCtrl(CLineCtrl *line);
	virtual ~CStandardCtrl();

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

#endif // !defined(AFX_STANDARDCTRL_H__09C59415_E132_4CDC_8AA0_6892E6761D29__INCLUDED_)
