// EPPDLL4CRYPTERA.h : EPPDLL4CRYPTERA DLL의 기본 헤더 파일입니다.
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
#include "./EPPComm/EPPComm.h"

// CEPPDLL4CRYPTERAApp
// 이 클래스의 구현을 보려면 EPPDLL4CRYPTERA.cpp를 참조하십시오.
//

class CEPPDLL4CRYPTERAApp : public CWinApp
{
public:
	CEPPDLL4CRYPTERAApp();

	CEPPComm	m_pComm;
// 재정의입니다.
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

