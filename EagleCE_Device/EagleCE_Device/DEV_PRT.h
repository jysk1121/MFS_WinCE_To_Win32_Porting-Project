#pragma once

#include <afxmt.h>

#include "DEV_PRT_Define.h"

class CDEV_PRT
{
public:
	CDEV_PRT();

	virtual ~CDEV_PRT();


	// Is DLL Loaded
	BOOL IsDllLoaded(void);


	// API
	BOOL PTR_Open(HWND hWnd, LPCTSTR lpName);

	// Get Print Status
	BOOL GetPrintStatus(BYTE byPrintStatus[4]);

	int PTR_Close();

	int PTR_Reset(int nPortType = EAGLE_PRT_TYPE_COM);

	int PTR_RTQueryStatus(int nPortType, char * pszStatus);
	
	int PTR_SetMode(int nPortType, int nPrintMode);

	int PTR_StartDoc();

	CString PTR_GetVersionInfo();

	int PTR_SetMotionUnit(int nPortType, int nHorizontalMU, int nVerticalMU);

	int PTR_P_SetAreaAndDirection(int nPortType, int nOrgx, int nOrgy, int nWidth, int nHeight, int nDirection);

	int PTR_SetLineSpacing(int nPortType, int nDistance);

	int PTR_SetRightSpacing(int nPortType, int nDistance);

	int PTR_P_Textout(int nPortType, char *pszData, int nOrgx, int nOrgy, int nWidthTimes, int nHeightTimes, int nFontType, int nFontStyle);

	int PTR_S_Textout(int nPortType, char *pszData, int nOrgx, int nWidthTimes, int nHeightTimes, int nFontType, int nFontStyle);

	int PTR_CutPaper(int nPortType, int nMode, int nDistance);

	int PTR_FeedLine(int nPortType);

	int PTR_S_SetBitmapFlash(int nPortType, char *pszPaths[], int nCount);

	int PTR_S_PrintBitmapFlash(int nPortType, int nID, int nOrgx, int nMode);

	int PTR_S_SetBitmapRAM(int nPortType, char *pszPaths, int nID);

	int PTR_S_PrintBitmapRAM(int nPortType, int nID, int nOrgx, int nMode);

	int PTR_P_PrintBitmapRAM(int nPortType, int nID, int nOrgx, int nOrgy, int nMode);

	int PTR_S_DownloadPrintBmp(int nPortType, char *pszPath, int nOrgx, int nDensityMode);

	int PTR_S_SetAreaWidth(int nPortType, int nWidth);

	int PTR_P_Print(int nPortType);

	int PTR_P_Clear(int nPortType);

	// TEST Command
	int PTR_WriteFile(char *chSendData, int nSendLen);


	// Paper 상태 조회
	int	PTR_GetPaperStatus(void);

	// Printer 상태 조회
	int	PTR_GetStatus(void);

	// PTR Quiry Status Start
	void PTR_QuiryStatusStart();

	// PTR Quiry Status Stop
	void PTR_QuiryStatusStop();

	// 에러코드 취득
	CString GetLastError(void);

	BOOL	m_bPTRExecuteQuiryStatus;

	CString m_strPRT_FW_Version;

protected:
	typedef HANDLE(WINAPI *POS_Open)( LPCTSTR lpName, 
										int nComBaudrate,
										int nComDataBits, 
										int nComStopBits, 
										int nComParity, 
										int nParam );

	typedef int (__stdcall *POS_Close)(void);

	typedef int (__stdcall *POS_Reset)(void);

	typedef int (__stdcall *POS_SetMode)(int nPrintMode);

	typedef int (__stdcall *POS_SetMotionUnit)(int nHorizontalMU,int nVerticalMU);

	typedef int (__stdcall *POS_SetCharSetAndCodePage)(int nCharSet,int nCodePage);

	typedef int (__stdcall *POS_FeedLine)(void);

	typedef int (__stdcall *POS_SetLineSpacing)(int nDistance);

	typedef int (__stdcall *POS_SetRightSpacing)(int nDistance);

	typedef int (__stdcall *POS_PreDownloadBmpToRAM)(char *pszPath,int nID);

	typedef int (__stdcall *POS_PreDownloadBmpsToFlash)(char *pszPaths[],int nCount);

	typedef int (__stdcall *POS_QueryStatus)(char *pszStatus,int nTimeouts);

	typedef int (__stdcall *POS_RTQueryStatus)(char *pszStatus);

	typedef int (__stdcall *POS_NETQueryStatus)(char *ipAddress,char *pszStatus);

	typedef int (__stdcall *POS_KickOutDrawer)(int nID,int nOnTimes,int nOffTimes);

	typedef int (__stdcall *POS_CutPaper)(int nMode,int nDistance);

	typedef bool (__stdcall *POS_StartDoc)(void);

	typedef bool (__stdcall *POS_EndDoc)(void);

	typedef bool (__stdcall *POS_EndSaveFile)(void);

	typedef bool (__stdcall *POS_BeginSaveFile)(LPCTSTR lpFileName,bool bToPrinter);

	typedef int (__stdcall *POS_SetASB)(int nItems);

	typedef int (__stdcall *POS_GetASB)(char *pStatus);

	typedef int (__stdcall *POS_S_SetAreaWidth)(int nWidth);

	typedef int (__stdcall *POS_S_TextOut)( char *pszString,			 
											int nOrgx,									 
											int nWidthTimes,									 
											int nHeightTimes,									 
											int nFontType,									 
											int nFontStyle);

	typedef int (__stdcall *POS_S_DownloadAndPrintBmp)(char *pszPath,int nOrgx,int nMode);

	typedef int (__stdcall *POS_S_PrintBmpInRAM)(int nID,int nOrgx,int nMode);

	typedef int (__stdcall *POS_S_PrintBmpInFlash)(int nID,int nOrgx,int nMode);

	typedef int (__stdcall *POS_S_SetBarcode)( char *pszInfoBuffer,
												int nOrgx,
												int nType,
												int nWidthX,
												int nHeight,
												int nHriFontType,
												int nHriFontPosition,
												int nBytesToPrint);

	typedef int (__stdcall *POS_PL_SetArea)( int nOrgx,
												int nOrgy,
												int nWidth,
												int nHeight,
												int nDirection);

	typedef int (__stdcall *POS_PL_TextOut)( char *pszString,
												int nOrgx,
												int nOrgy,
												int nWidthTimes,
												int nHeightTimes,
												int nFontType,
												int nFontStyle);

	typedef int (__stdcall *POS_PL_DownloadAndPrintBmp)(char *pszPath,int nOrgx,int nOrgy,int nMode);

	typedef int (__stdcall *POS_PL_PrintBmpInRAM)(int nID,int nOrgx,int nOrgy,int nMode);

	typedef int (__stdcall *POS_PL_SetBarcode)( char *pszInfoBuffer,
												int nOrgx,
												int nOrgy,
												int nType,
												int nWidthX,
												int nHeight,
												int nHriFontType,
												int nHriFontPosition,
												int nBytesToPrint);

	typedef int (__stdcall *POS_PL_Print)(void);

	typedef int (__stdcall *POS_PL_Clear)(void);

	typedef int (__stdcall *POS_WriteFile)(HANDLE hPort,char *pszData,int nBytesToWrite);

	typedef int (__stdcall *POS_ReadFile)( HANDLE hPort,
											char *pszData,
											int nBytesToRead,
											int nTimeouts);

	typedef HANDLE (__stdcall *POS_SetHandle)(HANDLE hNewHandle);

	typedef int (__stdcall *POS_GetVersionInfo)(int *pnMajor,int *pnMinor);

	POS_Open							LibPOS_Open;
	POS_Close							LibPOS_Close;
	POS_Reset							LibPOS_Reset;
	POS_SetMode							LibPOS_SetMode;
	POS_SetMotionUnit					LibPOS_SetMotionUnit;
	POS_SetCharSetAndCodePage			LibPOS_SetCharSetAndCodePage;
	POS_FeedLine						LibPOS_FeedLine;
	POS_SetLineSpacing					LibPOS_SetLineSpacing;
	POS_SetRightSpacing					LibPOS_SetRightSpacing;
	POS_PreDownloadBmpToRAM				LibPOS_PreDownloadBmpToRAM;
	POS_PreDownloadBmpsToFlash			LibPOS_PreDownloadBmpsToFlash;
	POS_QueryStatus						LibPOS_QueryStatus;
	POS_RTQueryStatus					LibPOS_RTQueryStatus;
	POS_NETQueryStatus					LibPOS_NETQueryStatus;
	POS_KickOutDrawer					LibPOS_KickOutDrawer;
	POS_CutPaper						LibPOS_CutPaper;
	POS_StartDoc						LibPOS_StartDoc;
	POS_EndDoc							LibPOS_EndDoc;
	POS_EndSaveFile						LibPOS_EndSaveFile;
	POS_BeginSaveFile					LibPOS_BeginSaveFile;
	POS_SetASB							LibPOS_SetASB;
	POS_GetASB							LibPOS_GetASB;
	POS_S_SetAreaWidth					LibPOS_S_SetAreaWidth;
	POS_S_TextOut						LibPOS_S_TextOut;
	POS_S_DownloadAndPrintBmp			LibPOS_S_DownloadAndPrintBmp;
	POS_S_PrintBmpInRAM					LibPOS_S_PrintBmpInRAM;
	POS_S_PrintBmpInFlash				LibPOS_S_PrintBmpInFlash;
	POS_S_SetBarcode					LibPOS_S_SetBarcode;
	POS_PL_SetArea						LibPOS_PL_SetArea;
	POS_PL_TextOut						LibPOS_PL_TextOut;
	POS_PL_DownloadAndPrintBmp			LibPOS_PL_DownloadAndPrintBmp;
	POS_PL_PrintBmpInRAM				LibPOS_PL_PrintBmpInRAM;
	POS_PL_SetBarcode					LibPOS_PL_SetBarcode;
	POS_PL_Print						LibPOS_PL_Print;
	POS_PL_Clear						LibPOS_PL_Clear;
	POS_WriteFile						LibPOS_WriteFile;
	POS_ReadFile						LibPOS_ReadFile;
	POS_SetHandle						LibPOS_SetHandle;
	POS_GetVersionInfo					LibPOS_GetVersionInfo;


	// Printer 상태
	static UINT PrinterStatus(LPVOID pParam);

	// Printer 상태
	void PrinterStatus(void);

	HMODULE							m_hDll;

	HANDLE							m_hPort;

	HWND							m_hWnd;

	HANDLE							m_StatusThread;

	char							m_byPreviousPrintStatus[4];

	char							m_byPrintStatus[4];

	int								m_nPaperStatus;
	
	int								m_nPTRStatus;

	BOOL							m_bPortOpened;

	// 에러 코드
	CString							m_strLastError;

	CCriticalSection				m_PTRLock;

	BOOL							m_bPrinterIdle;
};
