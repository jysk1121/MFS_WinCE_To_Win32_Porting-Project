#if !defined(AFX_STDAFX_H__CBC7A5BB_8E3B_4930_B4E1_F8B4039E3FA2__INCLUDED_)
#define AFX_STDAFX_H__CBC7A5BB_8E3B_4930_B4E1_F8B4039E3FA2__INCLUDED_

// FOR WINCE60 Define
#include ".\Common\WinCE60Def.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// stdafx.h : include file for standard system include files,
//      or project specific include files that are used frequently,
//      but are changed infrequently

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxctl.h>         // MFC support for ActiveX Controls
#include <afxext.h>         // MFC extensions
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Comon Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

// Delete the two includes below if you do not wish to use the MFC
//  database classes
//#include <afxdb.h>			// MFC database classes
//#include <afxdao.h>			// MFC DAO database classes

#include <afxmt.h>			// For Using Mutex : by teo

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
//#  undef _T
//#  define _T(str) str

# define LPSHORT short*
#endif


#include <afxtempl.h>		// for Using CArray : by teo 2004.03.31

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__CBC7A5BB_8E3B_4930_B4E1_F8B4039E3FA2__INCLUDED_)
