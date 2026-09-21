#pragma once

#include "DEV_EPP_Define.h"

class CDEV_EPP
{
public:
	// 생성자
	CDEV_EPP();
	// 소멸자
	virtual ~CDEV_EPP();

	// Is DLL Loaded
	BOOL IsDllLoaded(void);

	// 핀 번호 취득
	CString GetPinNumber();

	BOOL EPP_OpenPort(HWND hWnd, int nPortNum);
	BOOL EPP_ClosePort();
	BOOL EPP_Reset();
	BOOL EPP_GetVersion(BYTE TYPE, LPEPPRESULT lpResult);
	BOOL EPP_GetStatus(LPEPPSTATUS lpStatus);
	BOOL EPP_SelfTest();
	BOOL EPP_ExternalAuth();
	BOOL EPP_ChangePassword();
	BOOL EPP_NORMALKEYEVENT();
	BOOL EPP_NORMALKEYEVENTSTOP();
	BOOL EPP_SETLOCALTIME();
	BOOL EPP_GETPIN(LPEPPGETPIN lpeppgetpin);
	BOOL EPP_GETPINCANCEL();
	BOOL EPP_PEKEXCHANGE(LPEXCHANGEPEK lpexchangepek);
	BOOL EPP_HOSTUNBIND();
	BOOL EPP_HOSTBIND(LPEPPHOSTBIND lpepphostbind);
	BOOL EPP_BUZZER(BOOL bOn);
	BOOL EPP_RSAKeyREPLACE(LPEXCHANGERSAKEY lpexchangersakey);

	// 에러코드 취득
	CString GetLastError(void);

	// for test MFS EPP
	static int EventControlFunc(unsigned short usEventID, unsigned short usParam, byte *lpByte, unsigned short usDataLength);
	int eventHandler(unsigned short usEventID, unsigned short usParam, byte *lpByte, unsigned short usDataLength);
	int PressKeyHandler(unsigned short usParam, byte *lpByte, unsigned short usDataLength);
	BYTE CalcLRC(unsigned char *pData, DWORD dwLen);
	//

protected:
	typedef BOOL(WINAPI *MFSCommEPP_OpenPort)(HWND hwnd, int nPortNum);
	typedef void(WINAPI *MFSCommEPP_ClosePort)();
	typedef BOOL(WINAPI *MFSCommEPP_Reset)();
	typedef BOOL(WINAPI *MFSCommEPP_GetVersion)(BYTE TYPE, LPEPPRESULT lpResult);
	typedef BOOL(WINAPI *MFSCommEPP_GetStatus)(LPEPPSTATUS lpStatus);
	typedef BOOL(WINAPI *MFSCommEPP_SelfTest)();
	typedef BOOL(WINAPI *MFSCommEPP_ExternalAuth)();
	typedef BOOL(WINAPI *MFSCommEPP_ChangePassword)();
	typedef BOOL(WINAPI *MFSCOMMEPP_NORMALKEYEVENT)();
	typedef BOOL(WINAPI *MFSCOMMEPP_NORMALKEYEVENTSTOP)();
	typedef BOOL(WINAPI *MFSCOMMEPP_SETLOCALTIME)();
	typedef BOOL(WINAPI *MFSCOMMEPP_GETPIN)(LPEPPGETPIN lpeppgetpin);
	typedef BOOL(WINAPI *MFSCOMMEPP_GETPINCANCEL)();
	typedef BOOL(WINAPI *MFSCOMMEPP_PEKEXCHANGE)(LPEXCHANGEPEK lpexchangepek);
	typedef BOOL(WINAPI *MFSCOMMEPP_HOSTUNBIND)();
	typedef BOOL(WINAPI *MFSCOMMEPP_HOSTBIND)(LPEPPHOSTBIND lpepphostbind);
	typedef BOOL(WINAPI *MFSCOMMEPP_BUZZER)(BOOL bOn);
	typedef BOOL(WINAPI *MFSCOMMEPP_RSAKeyREPLACE)(LPEXCHANGERSAKEY lpexchangersakey);

	MFSCommEPP_OpenPort LibMFSCommEPP_OpenPort;
	MFSCommEPP_ClosePort LibMFSCommEPP_ClosePort;
	MFSCommEPP_Reset LibMFSCommEPP_Reset;
	MFSCommEPP_GetVersion LibMFSCommEPP_GetVersion;
	MFSCommEPP_GetStatus LibMFSCommEPP_GetStatus;
	MFSCommEPP_SelfTest LibMFSCommEPP_SelfTest;
	MFSCommEPP_ExternalAuth LibMFSCommEPP_ExternalAuth;
	MFSCommEPP_ChangePassword LibMFSCommEPP_ChangePassword;
	MFSCOMMEPP_NORMALKEYEVENT LibMFSCOMMEPP_NORMALKEYEVENT;
	MFSCOMMEPP_NORMALKEYEVENTSTOP LibMFSCOMMEPP_NORMALKEYEVENTSTOP;
	MFSCOMMEPP_SETLOCALTIME LibMFSCOMMEPP_SETLOCALTIME;
	MFSCOMMEPP_GETPIN LibMFSCOMMEPP_GETPIN;
	MFSCOMMEPP_GETPINCANCEL LibMFSCOMMEPP_GETPINCANCEL;
	MFSCOMMEPP_PEKEXCHANGE LibMFSCOMMEPP_PEKEXCHANGE;
	MFSCOMMEPP_HOSTUNBIND LibMFSCOMMEPP_HOSTUNBIND;
	MFSCOMMEPP_HOSTBIND LibMFSCOMMEPP_HOSTBIND;
	MFSCOMMEPP_BUZZER LibMFSCOMMEPP_BUZZER;
	MFSCOMMEPP_RSAKeyREPLACE LibMFSCOMMEPP_RSAKeyREPLACE;

	HMODULE		m_hDll;

	// 에러 코드
	CString m_strLastError;
};

