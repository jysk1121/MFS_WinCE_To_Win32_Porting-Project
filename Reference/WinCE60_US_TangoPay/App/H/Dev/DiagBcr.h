#ifndef __DIAG_BCR_H__
#define __DIAG_BCR_H__

#ifndef _DEVCTRL_OWNER_
#pragma comment(lib, "DevCtrl.lib")
#endif

#include ".\Common\NHCtrlThread.h"
#include "DiagFuncPointer.h"

#define BCR_READ_CHECK_TIME				3000
#define BCR_IO_COMPLETED_CHECK_TIME		200

enum BCR_DLL_COMMAND
{
	BCRCMD_NONE = 0,
	BCRCMD_OPEN,
	BCRCMD_CLOSE,
	BCRCMD_ENABLE,
	BCRCMD_DISABLE,
	BCRCMD_GETEPVERSION,
	BCRCMD_GETDATA2,
	BCRCMD_CLEARDATA,
	BCRCMD_GETLASTERRORCODE
};

class AFX_EXT_CLASS CDiagBcr : public CNHCtrlThread
{
public:
	CDiagBcr();
	~CDiagBcr();

	/*	Method */
private:
	CString				GetCommandString(int pCmdNum);
	BOOL				ExecuteCommand(int nCmdNum, BOOL bWait, int *pRequestID);

public:
	// ASync로 호출한 Method의 결과를 조회한다.
	BOOL				CompleteAction(int nID, DWORD dwWaitMs = INFINITE);

	BOOL				ErrorExists();
	CString				GetErrorCode();
	void				ClearError();
	CString				GetEPVersion();
	CString				GetBarcodeData();

	BOOL				Open(BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL				Close(BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL				Enable(BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL				Disable(BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL				GetData2(BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL				GetLastErrorCode(BOOL bWait=TRUE, int *pRequestID=NULL);	

	/*	Attribute */
private:
	BOOL				m_bInit;			// 초기화 유무
	BOOL				m_bDoingThread;		// Thread 동작 유무
	HINSTANCE			m_hDLL;				// DLL instance handle
	DIAGNOSTICS_BCR		m_pDllFunc;			// DLL Function
	int					m_nResponseCmd;		// 호출한 함수의 결과를 저장.
	BOOL				m_bErrorExists;


	WCHAR				m_wchErrorCode[16];
	WCHAR				m_wchEPVersion[16];
	WCHAR				m_wchBarcodeData[1024];

protected:
	unsigned			ThreadHandlerProc(void);	// 스레드의 Handler.
};

#endif //__DIAG_BCR_H__