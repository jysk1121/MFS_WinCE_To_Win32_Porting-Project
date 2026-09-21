// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__68620D0E_FC1E_486A_8540_945EFDB3E706__INCLUDED_)
#define AFX_STDAFX_H__68620D0E_FC1E_486A_8540_945EFDB3E706__INCLUDED_

#include ".\Common\WinCE60Def.h"

// FOR WINCE60 Define
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

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


#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT


// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__68620D0E_FC1E_486A_8540_945EFDB3E706__INCLUDED_)
