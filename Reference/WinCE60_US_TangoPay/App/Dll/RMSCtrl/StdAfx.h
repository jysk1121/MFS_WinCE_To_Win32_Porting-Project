// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__20FDD155_7E3F_4FD8_B217_9F6A75BF975D__INCLUDED_)
#define AFX_STDAFX_H__20FDD155_7E3F_4FD8_B217_9F6A75BF975D__INCLUDED_

// FOR WINCE60 Define
#include ".\Common\WinCE60Def.h"

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

#ifndef _AFX_NO_OLE_SUPPORT
// OLE support available only for dynamic builds in MFC for Windows CE
#include <afxole.h>         // MFC OLE classes
#include <afxodlgs.h>       // MFC OLE dialog classes
#include <afxdisp.h>        // MFC OLE automation classes
#endif // _AFX_NO_OLE_SUPPORT


#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

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

#ifndef UNDER_CE
#  define RETAILMSG(cond, printf_exp) _tprintf printf_exp
#  define LPSHORT short*
#endif


//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__20FDD155_7E3F_4FD8_B217_9F6A75BF975D__INCLUDED_)
