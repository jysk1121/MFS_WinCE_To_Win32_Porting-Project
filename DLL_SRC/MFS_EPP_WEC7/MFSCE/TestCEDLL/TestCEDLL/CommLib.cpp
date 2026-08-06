#include "StdAfx.h"
#include "CommLib.h"

CCommLib::CCommLib(void)
{
	m_hDLL = NULL;
	m_hDLL = LoadLibrary(L"MFSEPPDLL4CE.dll");
}

CCommLib::~CCommLib(void)
{
}

int CCommLib::OpenDevice(int nPortNum, DWORD dwBaudRate)
{
	typedef int (*_OPENDEVICE)(int, DWORD);
	_OPENDEVICE OPENDEVICE = NULL;
	int nRet = 0;
	CString str;

	if(m_hDLL)
	{
		OPENDEVICE = (_OPENDEVICE)GetProcAddress(m_hDLL, L"EPPOpenDevice");
		if(OPENDEVICE)
			nRet = OPENDEVICE(nPortNum, dwBaudRate);
		else
			nRet = 0xFFFE;
	}
	else
		nRet = 0xFFFF;
	return nRet;
}

int CCommLib::CloseDevice(void)
{
	typedef int (*_CLOSEDEVICE)();
	_CLOSEDEVICE CLOSEDEVICE = NULL;
	int nRet = 0;

	if(m_hDLL)
	{
		CLOSEDEVICE = (_CLOSEDEVICE)GetProcAddress(m_hDLL, L"EPPCloseDevice");
		if(CLOSEDEVICE)
			nRet = CLOSEDEVICE();
		else
			nRet = 0xFFFE;
	}
	else
		nRet = 0xFFFF;
	return nRet;
}

int CCommLib::ExecuteCommand(int nCommand, LPBYTE pData, DWORD dwLen, LPBYTE pResp, DWORD &dwRet, BOOL bCheckStatus, BOOL bWaitResponse)
{
	typedef int (*_EXECUTECOMMAND)(int, LPBYTE, DWORD, LPBYTE, DWORD&, BOOL, BOOL);
	_EXECUTECOMMAND EXECUTECOMMAND = NULL;
	int nRet = 0;
	CString str;

	if(m_hDLL)
	{
		EXECUTECOMMAND = (_EXECUTECOMMAND)GetProcAddress(m_hDLL, L"EPPExecuteCommand");
		if(EXECUTECOMMAND)
			nRet = EXECUTECOMMAND(nCommand, pData, dwLen, pResp, dwRet, bCheckStatus, bWaitResponse);
		else
			nRet = 0xFFFE;
	}
	else
		nRet = 0xFFFF;
	return nRet;
}

int CCommLib::RegistEventCallBackFunction(EventCallBackFunction pFunc)
{
	int nRet = 0;
	typedef int (*_REGISTEVENTCALLBACKFUNCTION)(EventCallBackFunction);
	_REGISTEVENTCALLBACKFUNCTION REGISTEVENTCALLBACKFUNCTION = NULL;

	if(m_hDLL)
	{
		REGISTEVENTCALLBACKFUNCTION = (_REGISTEVENTCALLBACKFUNCTION)GetProcAddress(m_hDLL, L"EPPRegistEventCallBackFunction");
		if(REGISTEVENTCALLBACKFUNCTION)
			nRet = REGISTEVENTCALLBACKFUNCTION(pFunc);
		else
			nRet = 0xFFFE;
	}
	else
		nRet = 0xFFFF;
	return nRet;
}

/*
int CCommLib::GetCommandResultData(LPBYTE lpData, int nBuffLeng, int* nLength)
{
	int nRet = 0;
	typedef int (*_GETCOMMANDRESULTDATA)(LPBYTE, int, int*);
	_GETCOMMANDRESULTDATA GETCOMMANDRESULTDATA = NULL;

	if(m_hDLL)
	{
		GETCOMMANDRESULTDATA = (_GETCOMMANDRESULTDATA)GetProcAddress(m_hDLL, "EPPGetCommandResultData");
		if(GETCOMMANDRESULTDATA)
			nRet = GETCOMMANDRESULTDATA(lpData, nBuffLeng, nLength);
		else
			nRet = 0xFFFE;
	}
	else
		nRet = 0xFFFF;
	return nRet;
}
*/
