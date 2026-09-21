// EagleCE_ATM.h : PROJECT_NAME 응용 프로그램에 대한 주 헤더 파일입니다.
//

/** *********************************************************
*	@mainpage			EagleCE ATM Project
*	@section program	EagleCE ATM
***********************************************************/

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH에 대해 이 파일을 포함하기 전에 'stdafx.h'를 포함합니다."
#endif

#ifdef STANDARDSHELL_UI_MODEL
#include "resource.h"
#endif

// CEagleCE_ATMApp:
// 이 클래스의 구현에 대해서는 EagleCE_ATM.cpp을 참조하십시오.
//

class CEagleCE_ATMApp : public CWinApp
{
public:
	CEagleCE_ATMApp();
	
// 재정의입니다.
public:
	virtual BOOL InitInstance();

// 구현입니다.

	DECLARE_MESSAGE_MAP()
};

extern CEagleCE_ATMApp theApp;
