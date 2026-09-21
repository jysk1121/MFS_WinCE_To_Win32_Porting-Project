#include "stdafx.h"
#include "UPFunctionKey.h"
#include ".\Common\CmnLib.h"

//  FUNCTION KEY 
#define FDK_F1			(0x10)
#define FDK_F2			(0x1B)
#define FDK_F3			(0x11)
#define FDK_F4			(0x1A)
#define FDK_F5			(0x12)
#define FDK_F6			(0x19)
#define FDK_F7			(0x13)
#define FDK_F8			(0x18)

/*-------------------------------------------------------------------
 CLASS    NAME: CUPFunctionKey
 FUNCTION NAME: CUPFunctionKey()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 생성자.
-------------------------------------------------------------------*/
CUPFunctionKey::CUPFunctionKey() : m_oWaitObject(TRUE)
{
	m_pScrCtrl = NULL;
	m_bStartKeyCheck = FALSE;

	FDKInit();
	SIUInit();

	Resume();
}

/*-------------------------------------------------------------------
 CLASS    NAME: CUPFunctionKey
 FUNCTION NAME: ~CUPFunctionKey()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 소멸자.
-------------------------------------------------------------------*/
CUPFunctionKey::~CUPFunctionKey()
{
}

/*-------------------------------------------------------------------
 CLASS    NAME: CUPFunctionKey
 FUNCTION NAME: Initialize()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 초기화를 수행한다.
-------------------------------------------------------------------*/
BOOL CUPFunctionKey::Initialize(CScrCtrl *pScrCtrl)
{
	m_pScrCtrl = pScrCtrl;

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CUPFunctionKey
 FUNCTION NAME: FunctionKeyCheck()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Function Key Check 수행 여부를 결정한다.
-------------------------------------------------------------------*/
BOOL CUPFunctionKey::FunctionKeyCheck(BOOL bEnable)
{
	m_bStartKeyCheck = bEnable;

	return TRUE;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CUPFunctionKey
 FUNCTION NAME: ThreadHandlerProc()
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Thread Handler
-------------------------------------------------------------------*/
unsigned CUPFunctionKey::ThreadHandlerProc()
{
	BYTE		InputKey;
	CNHConfig	SystemConfig;

	NH_OS_VERSION	eOSVersion = SystemConfig.GetOSVersion();

//	if (eOSVersion != NH_OS_NH2700_L &&
//		eOSVersion != NH_OS_NH2700_T)
	if (!(eOSVersion == NH_OS_NH2700_L || eOSVersion == NH_OS_NH2700_T_T))	// [#2064] NH KSK 2011.05.18
	{
		ClearFunctionKeyBuffer();

		while(1)
		{
			if (m_oWaitObject.Wait(50) != WAIT_TIMEOUT)
				continue;

			if (m_bStartKeyCheck == TRUE && m_pScrCtrl != NULL)
			{
				InputKey = GetFunctionKeyCode();
				switch(InputKey)
				{
				case FDK_F1:	m_pScrCtrl->SetKeyEvent(L"F1");	break;
				case FDK_F2:	m_pScrCtrl->SetKeyEvent(L"F2");	break;
				case FDK_F3:	m_pScrCtrl->SetKeyEvent(L"F3");	break;
				case FDK_F4:	m_pScrCtrl->SetKeyEvent(L"F4");	break;
				case FDK_F5:	m_pScrCtrl->SetKeyEvent(L"F5");	break;
				case FDK_F6:	m_pScrCtrl->SetKeyEvent(L"F6");	break;
				case FDK_F7:	m_pScrCtrl->SetKeyEvent(L"F7");	break;
				case FDK_F8:	m_pScrCtrl->SetKeyEvent(L"F8");	break;
				default:		break;
				}
			}
		}
	}

	return 0;
}