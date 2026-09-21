// VISAIICtrl.h: interface for the CVISAIICtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VISAIICTRL_H__85CB9902_649F_4DB1_9A6E_EB65C54327E8__INCLUDED_)
#define AFX_VISAIICTRL_H__85CB9902_649F_4DB1_9A6E_EB65C54327E8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Queue.h"
#include ".\Net\FlowCtrl.h"

class CVISAIICtrl : public CFlowCtrl 
{
public:
	CVISAIICtrl(CLineCtrl *line);
	virtual ~CVISAIICtrl();

// [#525] US KSK 2009.05.25
public:
	int			PreDialStart(CString CurrentPhone);
	int 		PreDialCancelByUser();

private:
	BOOL		m_bPreDialStart;
// end of [#525]

public:
	virtual int		SendData(BYTE *pSendBuf, int Length, LPCTSTR pDestInfo, LPCTSTR port, LPCTSTR option);
	//virtual int		RecvData(BYTE *pRecvBuf, int *Length, int WaitSec);
	virtual BOOL	CloseLine();

	// [#2075] NH KSK 2011.06.27
	virtual BOOL	RKTConnectCheck();
	virtual int		RKTConnectOpen(LPCTSTR pDestInfo, LPCTSTR port, LPCTSTR option);
	virtual int		RKTConnectClose();
	// end of [#2075]

private:
//	int				Flow_Matrix();
	int				Flow_Matrix(int initStatus = INIT_STS, int nMode = 0);	// [#2075] NH KSK 2011.06.27
};

#endif // !defined(AFX_VISAIICTRL_H__85CB9902_649F_4DB1_9A6E_EB65C54327E8__INCLUDED_)
