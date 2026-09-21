#ifndef __DIAG_SPR_H__
#define __DIAG_SPR_H__

#ifndef _DEVCTRL_OWNER_
#pragma comment(lib, "DevCtrl.lib")
#endif

#include ".\Common\NHCtrlThread.h"
#include "DiagFuncPointer.h"

#define MAX_HOPPER_NUMS		4
#define SPR_SUCCESS			0
#define SPR_FAIL			-1
#define IO_COMPLETED		1
#define SPR_IO_TIMEOUT		5000

class AFX_EXT_CLASS CDiagSpr : public CNHCtrlThread
{
public:
	CDiagSpr();
	~CDiagSpr();

	/*	Method */
public:
	// ASync, Sync Device Command
	BOOL	CompleteAction(int nID, DWORD dwWaitMs = INFINITE);
	BOOL	Open(BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL	Close(BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL	Initialize(BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL	GetLastError(BYTE* bErrorCode, BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL	Test_Print(BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL	Print(BYTE* pPrintData, BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL	Cutting(BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL	GetSensorInfo(LPBYTE lpbSensorData, DWORD dwSize, BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL	IsDeviceError(); 
	CString	GetErrorString();
	void	ClearDeviceError();
	/*  */


	/*	Attribute */
private:
	BOOL				m_bInit;			// 초기화 유무
	BOOL				m_bDoingThread;		// Thread 동작 유무
	BOOL				m_bPortOpen;		// Port Open Flag
	BOOL				m_bCDUInfo;			// CDU Information [CST NUM, COUNTRY, CDU TYPE]
	HINSTANCE			m_hSPR_DLL;			// DLL instance handle
	DIAGNOSTICS_SPR		m_SPR_FUNC;			// DLL Function


	int					m_nResponseCmd;		// 호출한 함수의 결과를 저장.

	/* SPR DEVICE INFORMATION */
	BOOL		m_bError;
	CString		m_strErroCode;
	BYTE		m_ErrorCode[6];
	BYTE*		m_pSPRData;
	
	/* SPR SENSOR INFORMATION */
	BYTE		m_Status[256];
	WORD		m_wSizeOfData;


protected:
	unsigned ThreadHandlerProc(void);	// 스레드의 Handler.
};

#endif //__DIAG_SPR_H__