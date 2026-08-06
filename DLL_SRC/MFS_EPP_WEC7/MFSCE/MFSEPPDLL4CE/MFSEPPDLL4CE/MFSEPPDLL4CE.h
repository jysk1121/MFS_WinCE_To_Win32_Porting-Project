// MFSEPPDLL4CE.h : MFSEPPDLL4CE DLL의 기본 헤더 파일입니다.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH에 대해 이 파일을 포함하기 전에 'stdafx.h'를 포함합니다."
#endif

#ifdef STANDARDSHELL_UI_MODEL
#include "resource.h"
#endif
#ifdef POCKETPC2003_UI_MODEL
#include "resourceppc.h"
#endif 


#include "SerialCommDef.h"
#include "EPPComm.h"

// CMFSEPPDLL4CEApp
// 이 클래스의 구현을 보려면 MFSEPPDLL4CE.cpp를 참조하십시오.
//


class CMFSEPPDLL4CEApp : public CWinApp
{
public:
	CMFSEPPDLL4CEApp();

	CEPPComm	m_pComm;

// 재정의입니다.
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

#ifdef DLLTEST_EXPORTS
#define DLLTEST_API extern "C" __declspec(dllexport)
#else
#define DLLTEST_API extern "C" __declspec(dllimport)
#endif

DLLTEST_API int EPPOpenDevice(int nPortNum, DWORD dwBaudRate);
DLLTEST_API int EPPCloseDevice();
DLLTEST_API int EPPExecuteCommand(int nCommand, BYTE *pData, DWORD dwDataLen, BYTE *pResult, DWORD &dwResultLength, BOOL bCheckStatus, BOOL bWaitResponse);
DLLTEST_API int EPPRegistEventCallBackFunction(EventCallBackFunction pFunc);

