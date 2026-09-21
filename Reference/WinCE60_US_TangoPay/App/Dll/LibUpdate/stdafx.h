// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently

#pragma once

// NOTE - this is value is not strongly correlated to the Windows CE OS version being targeted
#ifdef UNDER_CE
#  define WINVER _WIN32_WCE
  // FOR WINCE60 Define
#  include ".\Common\WinCE60Def.h"
#endif

// include BaseDefine.h	[#157] KSK 2008.04.22
// AU -> AU_A, AU_C로 분리 [#] AIREAT 2009.06.11
#ifdef			_ATMCE_US		// 국가코드 미국
#include "..\H\Common\BaseDef_US.h"
#elif defined	_ATMCE_MX		// 국가코드 멕시코
#include "..\H\Common\BaseDef_MX.h"
#elif defined	_ATMCE_AU		// 국가코드 호주
#include "..\H\Common\BaseDef_AU.h"
#elif defined	_ATMCE_CA		// 국가코드 캐나다
#include "..\H\Common\BaseDef_CA.h"
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

#ifdef _CE_DCOM
#define _ATL_APARTMENT_THREADED
#endif

#if UNDER_CE
#  include <ceconfig.h>
#endif
#if defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP)
#define SHELL_AYGSHELL
#endif

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxole.h>         // MFC OLE classes
#include <afxodlgs.h>       // MFC OLE dialog classes
#include <afxdisp.h>        // MFC Automation classes
#endif // _AFX_NO_OLE_SUPPORT

#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT


#if defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP)
#ifndef _DEVICE_RESOLUTION_AWARE
#define _DEVICE_RESOLUTION_AWARE
#endif
#endif

#ifdef _DEVICE_RESOLUTION_AWARE
#include "DeviceResolutionAware.h"
#endif



#if (_WIN32_WCE < 0x500) && ( defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP) )
	#pragma comment(lib, "ccrtrtti.lib")
	#ifdef _X86_	
		#if defined(_DEBUG)
			#pragma comment(lib, "libcmtx86d.lib")
		#else
			#pragma comment(lib, "libcmtx86.lib")
		#endif
	#endif
	#include <altcecrt.h>
#endif


#ifndef UNDER_CE
#  define RETAILMSG(cond, printf_exp) _tprintf printf_exp
#  define LPSHORT short*
#endif

#include ".\Common\CmnLib.h"
