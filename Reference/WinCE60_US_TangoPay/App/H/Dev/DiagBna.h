#ifndef __DIAG_BNA_H__
#define __DIAG_BNA_H__

#ifndef _DEVCTRL_OWNER_
#pragma comment(lib, "DevCtrl.lib")
#endif

#include ".\Common\NHCtrlThread.h"
#include "DiagFuncPointer.h"

#define	COMM_BARCODE_DATA_LENGTH	0x1C

#define		BNATYPE_MEI					1					// MEI Bill Acceptor
#define		BNATYPE_JCM					3					// JCM Bill Acceptor

enum BNA_DLL_COMMAND
{
	BNACMD_NONE = 0,
	BNACMD_OPEN,
	BNACMD_CLOSE,
	BNACMD_INITIALIZE,
	BNACMD_ACCEPT,
	BNACMD_CANCELACCEPT,
	BNACMD_STACK,
	BNACMD_EJECT,
	BNACMD_ENABLE,
	BNACMD_GETCURRENCY,
	BNACMD_GETENABLEDDENOM,
	BNACMD_GETLASTERROR,
	BNACMD_SETHANDLE,
	BNACMD_ACCEPT2,					/// cash only
	BNACMD_CANCELACCEPT2,
	BNACMD_ACCEPT3,					/// tickets only (barcode)
	BNACMD_CANCELACCEPT3,
	BNACMD_QUERYBARCODEDATA,
	BNACMD_GETEPVERSION_BILL,
	BNACMD_GETEPVERSION_VARIANT
};

class AFX_EXT_CLASS CDiagBna : public CNHCtrlThread
{
public:
	CDiagBna(int nBnaType = BNATYPE_MEI);
	~CDiagBna();

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

	BOOL				Open(BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL				Close(BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL				Initialize(BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL				Accept(BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL				CancelAccept(BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL				Stack(BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL				Eject(BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL				Enable(BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL				GetCurrency(BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL				GetEnabledDenom(BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL				GetLastError(BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL				SetHandle(BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL				Accept2(BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL				CancelAccept2(BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL				Accept3(BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL				CancelAccept3(BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL				QueryBarcodeData(BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL				GetEPVersion_Bill(BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL				GetEPVersion_Variant(BOOL bWait=TRUE, int *pRequestID=NULL);

	ULONG				GetAcceptValue();
	int					GetAcceptCount();
	int					GetRejectCount();
	CString				GetBarcodeData();
	CString				GetEPVersion_Bill();
	CString				GetEPVersion_Variant();

	/*	Attribute */
private:
	BOOL				m_bInit;			// 초기화 유무
	BOOL				m_bDoingThread;		// Thread 동작 유무
	HINSTANCE			m_hDLL;				// DLL instance handle
	DIAGNOSTICS_BNA		m_pDllFunc;			// DLL Function
	int					m_nResponseCmd;		// 호출한 함수의 결과를 저장.
	BOOL				m_bErrorExists;
	
	
	WCHAR				m_wchErrorCode[16];
	WCHAR				m_wchEPVersionBill[16];
	WCHAR				m_wchEPVersionVariant[16];
	WCHAR				m_wchBarcodeData[COMM_BARCODE_DATA_LENGTH + 1];
	ULONG				m_ulAcceptValue;
	int					m_nAcceptCount;
	int					m_nRejectCount;


protected:
	unsigned			ThreadHandlerProc(void);	// 스레드의 Handler.
};

#endif //__DIAG_BNA_H__