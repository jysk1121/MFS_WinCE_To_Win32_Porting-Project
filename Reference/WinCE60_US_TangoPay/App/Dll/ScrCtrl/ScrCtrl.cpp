#include "stdafx.h"
#include ".\Scr\ScrCtrl.h"
#include "FlashLiteView.h"
#include "PicassoView.h"

//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------
//#define NH_DEBUG		// Debug Message On/Off
#include ".\Common\NHDbgApi.h"

// type of message
#define DBG_CALL	1
#define DBG_INFO	1

//------------------------------------------------------------------
//	Define Global Variable
//------------------------------------------------------------------

//------------------------------------------------------------------
//	Implementation
//------------------------------------------------------------------

/*-------------------------------------------------------------------
 CLASS    NAME: CScrCtrl
 FUNCTION NAME: CScrCtrl()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : constructor
-------------------------------------------------------------------*/
CScrCtrl::CScrCtrl()
{
//	NHUIDBG(DBG_CALL, (_T("Before RefCount(%d)\n"), g_nRefCount));
}

/*-------------------------------------------------------------------
 CLASS    NAME: CScrCtrl
 FUNCTION NAME: ~CScrCtrl()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : 
 DESCRIPTION  : destructor
-------------------------------------------------------------------*/
CScrCtrl::~CScrCtrl()
{
//	NHUIDBG(DBG_CALL, (_T("Before RefCount(%d)\n"), g_nRefCount));
}

/*-------------------------------------------------------------------
 CLASS    NAME: CScrCtrl
 FUNCTION NAME: CreateScreenEngine()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : CScrCtrl Object pointer
 PARAMETER    :   type : CScrCtrl type (FlashLite, NHGDI
				     x : the initial hrizontal position of the window.
				     y : the initial vertical position of the window.
				 width : the width of the window.
				height : the height of the window.
 DESCRIPTION  : Create CScrCtrl Object
-------------------------------------------------------------------*/
CScrCtrl* CScrCtrl::CreateScreenEngine(SCREEN_TYPE type, int x, int y, int width, int height)
{
	NHUIDBG(DBG_CALL, (_T("type(%d), x(%d), y(%d), widht(%d), height(%d)\n"), type, x, y, width, height));

	CScrCtrl	*pScrCtrl;

	// Flash Lite 30
	if (type == SCR_TYPE_FLASH_LITE)
	{
		pScrCtrl = (CScrCtrl*) new CFlashLiteView(x, y, width, height);

		NHUIDBG(DBG_INFO, (_T("Created Flash Lite Object\n")));

		return pScrCtrl;
	}
	// Nautilus Hyosung Drawing Engine
	else if (type == SCR_TYPE_PICASSO)
	{
		pScrCtrl = (CScrCtrl*) new CPicassoView(x, y, width, height);

		NHUIDBG(DBG_INFO, (_T("Created NH GDI Object\n")));

		return pScrCtrl;
	}

	return NULL;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CScrCtrl
 FUNCTION NAME: DestroyScreenEngine()
 WRITER       : AIREAT (2009.09.18)
 RETURN TYPE  : none
 PARAMETER    : pScrCtrl : pointer of the CScrCtrl Object.
 DESCRIPTION  : Destroy CScrCtrl Object
-------------------------------------------------------------------*/
void CScrCtrl::DestroyScreenEngine(CScrCtrl *pScrCtrl)
{
	NHUIDBG(DBG_CALL, (_T("pScrCtrl(0x%08X)\n"), pScrCtrl));

	if (pScrCtrl == NULL)
		return;

	if (pScrCtrl->GetScreenType() == SCR_TYPE_FLASH_LITE)
		delete ((CFlashLiteView*)pScrCtrl);
	else if (pScrCtrl->GetScreenType() == SCR_TYPE_PICASSO)		// KSK 2012.10.19 Bug Fix
		delete ((CPicassoView*)pScrCtrl);
	else
		delete pScrCtrl;

	pScrCtrl = NULL;
}

//------------------------------------------------------------------
//	Dll Entry Pointer
//------------------------------------------------------------------

static AFX_EXTENSION_MODULE ScrCtrlDLL = { NULL, NULL };

BOOL APIENTRY DllMain(HANDLE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		RETAILMSG(1, (L"SCRCTRL.DLL Initializing!\n"));

		// Extension DLL one-time initialization
		if (!AfxInitExtensionModule(ScrCtrlDLL, (HINSTANCE)hInstance))
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

		new CDynLinkLibrary(ScrCtrlDLL);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		RETAILMSG(1, (L"SCRCTRL.DLL Terminating!\n"));

		// Terminate the library before destructors are called
		AfxTermExtensionModule(ScrCtrlDLL);
	}

    return TRUE;
}

//void CScrCtrl::SetScreenLocale(int Locale)
//{
//}

//int CScrCtrl::GetScreenLocale()
//{
//	return 1;
//}

BOOL CScrCtrl::SetDemoMode(BOOL bDemo)
{
	return TRUE;
}

void CScrCtrl::DrawDebugMsg(int x, int y, CString strMsg)
{
}
