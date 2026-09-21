#ifndef __RMS_CTRL_H__
#define __RMS_CTRL_H__

#include "Queue.h"
#include ".\Net\FlowCtrl.h"

#define RMS_OK					0			// RMSDefine.h에 중복 정의 되어 있음.
#define RMS_ERR					1			// RMSDefine.h에 중복 정의 되어 있음.

#define RMS_LISTEN				1			// RMSDefine.h에 중복 정의 되어 있음.
#define RMS_CONNECT				2			// RMSDefine.h에 중복 정의 되어 있음.

class CRMSFlowCtrl : public CFlowCtrl
{
public:
	CRMSFlowCtrl(CLineCtrl* clientLine, CLineCtrl *serverLine);
	~CRMSFlowCtrl();

private:
	BOOL	m_bOpen;

	CLineCtrl *m_serverLine;
	CLineCtrl *m_clientLine;

	// The line currently being used for communications
	int		m_LineType;

public:
	virtual int		SendData(BYTE *pSendBuf, int Length, LPCTSTR pDestInfo, LPCTSTR port=L"", LPCTSTR option=L"");
	virtual int		RecvData(BYTE *pRecvBuf, int *Length, int WaitSec);

	virtual int		RMSConnectOpen(LPCTSTR pDestInfo, LPCTSTR port=L"", LPCTSTR option=L"", LPCTSTR OpenType=L"");
	virtual int		RMSConnectClose();
	virtual BOOL	RMSConnectCheck();
	virtual int		RMSConnectAccept();

private:
	int		Flow_Matrix(int initStatus);
};

#endif __RMS_CTRL_H__