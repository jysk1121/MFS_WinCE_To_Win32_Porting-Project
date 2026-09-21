#ifndef __DIAG_CARD_H__
#define __DIAG_CARD_H__

#ifndef _DEVCTRL_OWNER_
#pragma comment(lib, "DevCtrl.lib")
#endif

#include ".\Common\NHCtrlThread.h"
#include "DiagFuncPointer.h"

#define MCU_SUCCESS			0
#define MCU_FAIL			-1
#define MCU_TIMEOUT			3000	// [#581] NH KSK 2009.12.02

#define MCU_IO_COMPLETED		1
#define MCU_IO_TIMEOUT		10000
#define MCU_READ_CHECK_TIME	3000
#define MCU_IO_COMPLETED_CHECK_TIME	500
#define MCU_IO_TIME_OUT			2
#define MCU_IO_FAIL				-1

class AFX_EXT_CLASS CDiagCard : public CNHCtrlThread
{
public:
	CDiagCard(int nMCUType);
	~CDiagCard();

	/*	Method */
public:
	// ASync로 호출한 Method의 결과를 조회한다.
	BOOL	CompleteAction(int nID, DWORD dwWaitMs = INFINITE);
	BOOL	Initialize(BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL	Open(BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL	Close(BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL	Enable(BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL	Disable(BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL	Read(PCARD_TRACK_DATA pTrackData, BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL	Read_Again(PCARD_TRACK_DATA pTrackData, BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL	Read_TimeOut(PCARD_TRACK_DATA pTrackData, DWORD dwTimeOut, BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL	GetSensorInfo(LPBYTE lpbSensorData, DWORD dwSize, BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL	GetLastError(BYTE bErrorCode[6], BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL	GetTrackData(PCARD_TRACK_DATA pTrackData);
	BOOL	CancleIO();
	BOOL	ICCardPowerOn(BYTE byATRData[1024], WORD* wATRLength, BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL	ICCardPowerOff(BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL	ICCardLatch(BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL	ICCardUnLatch(BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL	ICCardAccept(DWORD dwTimeOut, BOOL bWait=TRUE, int *pRequestID=NULL); //[#581] SOOK 2009.11.15 MagTek CARD SCAN시 Timeout 적용 
	BOOL	ICCardRead(PCARD_TRACK_DATA pTrackData, BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL	ICCardRemove(BOOL bWait=TRUE, int *pRequestID=NULL);

	// [#2078] NH KSK 2011.07.15
	BOOL	AntiSkimmingSetup(int nTimeThreshold, BOOL bWait=TRUE, int *pRequestID=NULL);
	BOOL	CheckAntiSkimmingSetup(BOOL bWait=TRUE, int *pRequestID=NULL);	
	BOOL	CheckAntiSkimmingStatus(BOOL bWait=TRUE, int *pRequestID=NULL);
	CString	GetAntiSkimmingVoltage();
	CString GetAntiSkimmingTemperature();
	CString	GetAntiSkimmingStatus();
	// end of [#2078]

	BOOL	IsDeviceError(); 
	CString	GetErrorString();
	void	ClearDeviceError();

	/*	Attribute */
private:

	BOOL				m_bInit;			// 초기화 유무
	BOOL				m_bPortOpen;
	BOOL				m_bDoingThread;		// Thread 동작 유무
	HINSTANCE			m_hMCU_DLL;			// DLL instance handle
	DIAGNOSTICS_MCU		m_MCU_FUNC;			// DLL Function
	int					m_nResponseCmd;		// 호출한 함수의 결과를 저장.

	int					m_MCUType;
	/* MCU SENSOR INFORMATION */
	BYTE				m_Status[256];
	WORD				m_wSizeOfData;
//	BYTE				m_ErrorCode[6];

	/* CARD TRACK DATA */
	CARD_TRACK_DATA		m_TrackData;
	DWORD				m_dwTimeOut;
	BYTE				m_byATRData[1024];
	WORD				m_wATRLength;

	/* Magtek STATUS */
	BOOL				m_bPowerOn;
	BOOL				m_bLatch;
	UINT				m_nPort;
	UINT				m_nBaud;
	UINT				m_nParity;
	UINT				m_nStop;
	UINT				m_nTraceLevel;

	BYTE				m_ErrorCode[16];
	BOOL				m_bError;
	CString				m_strErroCode;

	// [#2078] NH KSK 2011.07.15
	int					m_nAntiSkimmingTimeThreshold;
	BYTE				m_byAntiSkimmingStatus[3];		// Status[0], Status[1]은 Dummy
	BYTE				m_byVoltage[5];
	BYTE				m_byTemperature[5];
	// end of [#2078]

protected:
	unsigned ThreadHandlerProc(void);	// 스레드의 Handler.
};

#endif //__DIAG_CARD_H__