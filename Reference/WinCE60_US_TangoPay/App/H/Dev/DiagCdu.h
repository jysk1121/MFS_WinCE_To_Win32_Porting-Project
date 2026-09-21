#ifndef __DIAG_CDU_H__
#define __DIAG_CDU_H__

#ifndef _DEVCTRL_OWNER_
#pragma comment(lib, "DevCtrl.lib")
#endif

#include ".\Common\NHCtrlThread.h"
#include "DiagFuncPointer.h"

#define MAX_CST_NUMS				4
#define CDU_SUCCESS					0
#define CDU_FAIL					-1

#define IO_COMPLETED				1	
#define CDU_OPEN_TIMEOUT			5000
#define CDU_INIT_TIMEOUT			10000
#define CDU_DISPENSING_TIMEOUT		10000

class AFX_EXT_CLASS CDiagCdu : public CNHCtrlThread
{
public:
	CDiagCdu();
	~CDiagCdu();

	/*	Method */
public:
	// ASync, Sync Device Command
	BOOL	CompleteAction(int nID, DWORD dwWaitMs = INFINITE);
	BOOL	Open(BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL	Close(BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL	Initialize(BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL	Test_Dispense(ULONG ulDispensedNotes[MAX_CST_NUMS], ULONG* ulTotalRejectedNote, ULONG ulRequestNotes[MAX_CST_NUMS] = NULL, BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL	GetVersion(USHORT *pusCstNum, ULONG ulCstIndex[MAX_CST_NUMS], TCHAR szCstCountry[4],  USHORT *pusCDUType, BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL	GetSensorInfo(LPBYTE lpbSensorData, WORD* wSizeOfData, BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL	GetAutoAdjustSensor(BOOL bWait=TRUE, int *pRequestID=NULL);		// [#RWC6-82] US Kook 2022.04.22 cdu ar auto dimming
	void	GetCurrentCDUInform(USHORT *pusCstNum, ULONG ulCstIndex[MAX_CST_NUMS], TCHAR szCstCountry[4],  USHORT *pusCDUType);
	CString	GetCountryInfos(CString countryInfo, INT flag);
	CString GetNextCountryInfos(CString countryInfo);
	BOOL	SetCDUSetupInform(USHORT usCstNum, ULONG ulCstIndex[MAX_CST_NUMS], TCHAR szCstCountry[4],  USHORT usCDUType);
	BOOL	GetLastError(BYTE *bErrorCode = NULL, BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL	IsDeviceError(); 
	CString	GetErrorString();
	void	ClearDeviceError();
	/*  */


	/*	Attribute */
public:
	BOOL				m_bInit;			// 초기화 유무
	BOOL				m_bDoingThread;		// Thread 동작 유무
	BOOL				m_bPortOpen;		// Port Open Flag
	BOOL				m_bCDUInfo;			// CDU Information [CST NUM, COUNTRY, CDU TYPE]
	HINSTANCE			m_hCDU_DLL;			// DLL instance handle
	DIAGNOSTICS_CDU		m_CDU_FUNC;			// DLL Function


	int					m_nResponseCmd;		// 호출한 함수의 결과를 저장.

	/* CDU DEVICE INFORMATION */
	USHORT				m_CSTNum;
	ULONG				m_CSTIndex[4];
	TCHAR				m_CSTCountry[4];
	USHORT				m_CSTType;

	/* DISPENSE INFORMATION */
	ULONG				m_RequestNotes[MAX_CST_NUMS];
	ULONG				m_DispensNotes[MAX_CST_NUMS];
	ULONG				m_TotalRejectedNote;
	
	/* CDU SENSOR INFORMATION */
	BYTE		m_Status[256];
	WORD		m_wSizeOfData;

	BYTE		m_ErrorCode[16];
	BOOL		m_bError;
	CString		m_strErroCode;

protected:
	unsigned ThreadHandlerProc(void);	// 스레드의 Handler.
};

#endif //__DIAG_CARD_H__