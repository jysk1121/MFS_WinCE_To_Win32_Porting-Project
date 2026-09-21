#pragma once

#include <afxmt.h>

#include "DEV_DIO_Define.h"

/************************************************************
* DIO
************************************************************/
class CDEV_DIO
{
public:
	// 생성자
	CDEV_DIO();
	// 소멸자
	virtual ~CDEV_DIO();

	// Sensor Status
	BOOL m_bAudioJackInserted;

	BOOL m_bSafeDoorOpened;

	BOOL m_bTopDoorOpened;

	char m_szSensor;

	char m_szPrevSensor;

	// Is DLL Loaded
	BOOL IsDllLoaded(void);
	// 열기
	BOOL Open(HWND hWnd, int nPortNum);
	// 닫기
	BOOL Close();
	// 버전 취득
	CString GetVersion();
	// LED 설정
	BOOL SetLed(BYTE byLED_ID, BYTE byAction);
	// Sensor 조회
	BOOL GetSensor(char* pStatus);

	// Run Flicker thread
	void StartDIOThread();

	// LED 점멸
	static UINT FlickerLed(LPVOID pParam);

	// LED 점멸
	void FlickerLed(void);

	// Sensor 감시 thread
	static UINT WatchSensor(LPVOID pParam);

	// Sensor 감시 thread
	void WatchSensor(void);

	// Sensor 조회
	BOOL DIO_GetSensor(char* pStatus);

	// Write LED
	BOOL DIO_SetLED(BYTE byLED_1, BYTE byLED_2);

	// Set Flicking LED
	BOOL DIO_SetFlicking(BYTE byDev, BYTE byTime);

	BYTE GetCheckSum(LPSTR lpText, UINT nStartPos, UINT nEndPos);

	BYTE* MakeSendFormat(BYTE byCommand, BYTE* pbyData=NULL, int nDataLen=0);

//protected:

	HWND				m_hWnd;
	HANDLE				m_FlickerLedThread;
	HANDLE				m_WatchSensorThread;
	BYTE				m_SetLed[5];
	BYTE				m_FlickerLed[5];
	BYTE				m_byTemp[5];
	BYTE				m_byAction;
	BOOL				m_bExecFlag;
	BOOL				m_bThreadExit;

	CString				m_strDIO_FW_Version;

	BOOL				m_bSupportFlicking;

	CCriticalSection	m_DIOLock;
};
