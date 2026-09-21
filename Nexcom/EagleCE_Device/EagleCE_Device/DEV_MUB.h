#pragma once

#include <afxmt.h>


// 
typedef enum 
{
	ID_MBU = 0,
	ID_SIU,
	ID_CDM,
	ID_ASD,
	ID_EPP,
	ID_NFC,
	ID_BAR
};

/************************************************************
* MUB(Multi UART Board)
************************************************************/
class CDEV_MUB
{
public:
	// 持失切
	CDEV_MUB();
	// 社瑚切
	virtual ~CDEV_MUB();

	// Is DLL Loaded
	BOOL IsDllLoaded(void);
	// open
	BOOL Open(HWND hWnd, int nPortNum);
	// Close
	BOOL Close();
	// F/W Version
	BOOL GetVersion(LPSTR szVersion);
	// Reset
	BOOL Reset();
	

	// Handshake Device Command
	BOOL HandShakeDeviceCommand(BYTE byID, BYTE* pSendData, BYTE* pRecvData, int nLen, int* nOutLen);

	//BYTE m_MubSendBuff[1024];
	//BYTE m_MubRecvBuff[1024];

	//////////////////////////////////////////////////////////////////////////
	// for CDM Function
	//BOOL MUB_CDM_Reset();
	//BOOL MUB_CDM_CfgStatus(LPCDMCFGSTATUS lpCfgStatus);
	//BOOL MUB_CDM_Status(LPCDMSTATUS lpStatus);
	//BOOL MUB_CDM_MultiDispense(LPCDMCBXITEM lpCbxItem, LPCDMMULTIDISPENSE lpResult);
	//BOOL MUB_CDM_LastDispense(LPCDMLASTDISPENSE lpResult);
	//BOOL MUB_CDM_Diagnostic(LPCDMDIAGNOSTIC lpResult);
	//BOOL MUB_CDM_SetBillThickness(LPCDMSETBILLTHICKNESS lpSet, LPBYTE lpbyErrorCd);
	//BOOL MUB_CDM_GetBillThickness(BYTE byCbxNo, LPCDMGETBILLTHICKNESS lpGet);
	//BOOL MUB_CDM_SetBillSize(LPCDMSETBILLSIZE lpSet, LPBYTE lpbyErrorCd);
	//BOOL MUB_CDM_GetBillSize(BYTE byCbxNo, LPCDMGETBILLSIZE lpGet);
	//BOOL MUB_CDM_LearnBill(BYTE byCbxNo, BYTE byItemCount, LPCDMLEARN lpResult);
	//BOOL MUB_CDM_GetDLLVersion(LPBYTE major_num, LPBYTE minor_num);
	//BOOL MUB_CDM_TestDispense(LPCDMCBXITEM lpCbxItem, LPCDMMULTIDISPENSE lpResult);
	//BOOL MUB_CDM_GetRejectLog(LPCDM_REJECT_INFO lpRejectLog);
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// for SIU Function
	//BOOL MUB_DIO_GetVersion(LPSTR lpFWVer, LPSTR lpDLLVer);
	//BOOL MUB_DIO_SetLed(BYTE byLED_1, BYTE byLED_2);
	//BOOL MUB_DIO_GetSensor(char* pStatus);
	//////////////////////////////////////////////////////////////////////////


protected:
	typedef BOOL(WINAPI *DLL_OPEN)(BYTE);
	typedef BOOL(WINAPI *DLL_CLOSE)();
	typedef BOOL(WINAPI *DLL_GETVERSION)(LPSTR);
	typedef BOOL(WINAPI *DLL_RESET)();
	typedef BOOL(WINAPI *DLL_SEND_DATA)(BYTE, BYTE*, int);
	typedef int(WINAPI *DLL_READ_DATA)(BYTE, BYTE*);

	HMODULE				m_hDll;
	DLL_OPEN			Dll_Open;
	DLL_CLOSE			Dll_Close;
	DLL_GETVERSION		Dll_GetVersion;
	DLL_RESET			Dll_Reset;
	DLL_SEND_DATA		Dll_Send_Data;
	DLL_READ_DATA		Dll_Read_Data;

//	BYTE GetCheckSum(LPSTR lpText, UINT nStartPos, UINT nEndPos);

	// Send Data
	BOOL SendDeviceData(BYTE byID, BYTE* pData, int nLen);
	// Read Data
	int ReadDeviceData(BYTE byID, BYTE* pData);

	CCriticalSection	m_MUBLock;
	BOOL m_bCommError;
};
