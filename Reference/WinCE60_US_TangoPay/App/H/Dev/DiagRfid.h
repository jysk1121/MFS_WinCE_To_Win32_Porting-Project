#ifndef __DIAG_RFID_H__
#define __DIAG_RFID_H__

#ifndef _DEVCTRL_OWNER_
#pragma comment(lib, "DevCtrl.lib")
#endif

#include ".\Common\NHCtrlThread.h"
#include "DiagFuncPointer.h"

#define RFID_SUCCESS					0
#define RFID_FAIL						-1
#define RFID_ERROR						-2
#define RFID_TIMEOUT					3000

#define RFID_IO_COMPLETED				1
#define RFID_IO_TIMEOUT					10000
#define RFID_READ_CHECK_TIME			3000
#define RFID_IO_COMPLETED_CHECK_TIME	200
#define RFID_IO_TIME_OUT				2
#define RFID_IO_FAIL					-1

class AFX_EXT_CLASS CDiagRfid : public CNHCtrlThread
{
public:
	CDiagRfid(int nRFIDType);
	~CDiagRfid();

	/*	Method */
public:
	// ASync로 호출한 Method의 결과를 조회한다.
	BOOL	CompleteAction(int nID, DWORD dwWaitMs = INFINITE);
	BOOL	Open(BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL	Close(BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL	Enable(BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL	Disable(BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL	ReadRawData(DWORD dwTimeout, BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL	GetErrorCode(BYTE bErrorCode[16], BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL	CancelCommand(BOOL bWait=TRUE, int *pRequestID=NULL);

	void	RFID_GetTrackData(RFVDMCARDDATA sRFVDMCardData[5]);

	BOOL	IsDeviceError(); 
	CString	GetErrorString();
	void	ClearDeviceError();

	/*	Attribute */
private:

	BOOL				m_bInit;				// 초기화 유무
	BOOL				m_bPortOpen;
	BOOL				m_bDoingThread;			// Thread 동작 유무
	HINSTANCE			m_hRFID_DLL;			// DLL instance handle
	DIAGNOSTICS_RFID	m_RFID_FUNC;			// DLL Function
	int					m_nResponseCmd;			// 호출한 함수의 결과를 저장.

	int					m_RFIDType;


	/* CARD TRACK DATA */
	DWORD				m_dwTimeOut;
	int					nRecvSize;
	RFVDMCARDDATA		m_RFIDTrackData[5];

	BYTE				m_ErrorCode[16];
	BOOL				m_bError;
	CString				m_strErroCode;

protected:
	unsigned ThreadHandlerProc(void);	// 스레드의 Handler.
};

#endif //__DIAG_RFID_H__