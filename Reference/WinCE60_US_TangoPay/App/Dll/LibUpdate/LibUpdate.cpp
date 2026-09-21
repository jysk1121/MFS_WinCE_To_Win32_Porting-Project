#include "stdafx.h"
#include "LibUpdate.h"

static AFX_EXTENSION_MODULE LibUpdateDLL = { NULL, NULL };

extern "C" int APIENTRY
DllMain(HANDLE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		RETAILMSG(1, (L"LibUpdate.dll Initializing!\n"));
		
		// Extension DLL one-time initialization
		if (!AfxInitExtensionModule(LibUpdateDLL, (HINSTANCE)hInstance))
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

		new CDynLinkLibrary(LibUpdateDLL);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		RETAILMSG(1, (L"LibUpdate.dll Terminating!\n"));

		// Terminate the library before destructors are called
		AfxTermExtensionModule(LibUpdateDLL);
	}
	return 1;   // ok
}
