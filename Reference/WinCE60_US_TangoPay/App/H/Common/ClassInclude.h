/////////////////////////////////////////////////////////////////////////////
//	Class Header Include
/////////////////////////////////////////////////////////////////////////////
#include "..\..\WinAtm\MainFrm.h"
#include "..\Net\Network.h"
#if (_WIN32_WCE < 0x600)
#include "..\Dll\AdaCtrlMain.h"
#else
#include "..\Ada\AdaCtrl.h"
#endif
#include "..\Dev\JnlMgr.h"		// [#3] NH AIREAT 2008.3.10
