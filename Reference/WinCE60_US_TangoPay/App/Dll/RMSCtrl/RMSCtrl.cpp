// RmsCtrl.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include ".\Rms\RmsCtrl.h"


static AFX_EXTENSION_MODULE RmsCtrlDLL = { NULL, NULL };

BOOL APIENTRY DllMain(HANDLE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		RETAILMSG(1, (L"RMSCTRL.DLL Initializing!\n"));
		
		// Extension DLL one-time initialization
		if (!AfxInitExtensionModule(RmsCtrlDLL, (HINSTANCE)hInstance))
			return 0;

		// Insert this DLL into the resource chain
		// NOTE: If this Extension DLL is being implicitly linked to by
		//  an MFC Regular DLL (such as an ActiveX Control)
		//  instead of an MFC application, then you will want to
		//  remove this line from DllMain and put it in a separate
		//  function exported from this Extension DLL.  The Regular DLL
		//  that uses this Extension DLL should then explicitly call that
		//  function to initialize this Extension DLL.  Otherwise,
		//  the CDynLinkLibrary object will not be attached to the
		//  Regular DLL's resource chain, and serious problems will
		//  result.

		new CDynLinkLibrary(RmsCtrlDLL);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		RETAILMSG(1, (L"RMSCTRL.DLL Terminating!\n"));

		// Terminate the library before destructors are called
		AfxTermExtensionModule(RmsCtrlDLL);
	}

    return TRUE;
}

CRmsCtrl::CRmsCtrl()
{
	m_pDevCmn = NULL;
	m_pTranCmn = NULL;
	memset(&m_AddCashInfo, 0, sizeof(m_AddCashInfo));	// [#530] NH KSK 2009.06.03
	memset(&m_DateTimeInfo, 0, sizeof(m_DateTimeInfo)); //[#611] SOOK 2010.01.16 Change ATM TimeÃß°¡ 
}

CRmsCtrl::~CRmsCtrl()
{
	m_pDevCmn = NULL;
	m_pTranCmn = NULL;
}

//void CRmsCtrl::SetDevCmn(CDevCmn *pDevCmn)
void CRmsCtrl::SetDevCmn(CDevCmn *pDevCmn, CTranCmn *pTranCmn)
{
	m_pDevCmn = pDevCmn;
	m_pTranCmn = pTranCmn;
}