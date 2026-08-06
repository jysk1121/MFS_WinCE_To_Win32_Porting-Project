// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__68620D0E_FC1E_486A_8540_945EFDB3E706__INCLUDED_)
#define AFX_STDAFX_H__68620D0E_FC1E_486A_8540_945EFDB3E706__INCLUDED_

// FOR WINCE60 Define
//#include ".\Common\WinCE60Def.h"	// [#685] NH KSK 2011.01.24

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

#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

/************************************************************
*	@brief	SCR <-> AP Delimiter
************************************************************/
#define SCR_CMD_DELIMITER	_T("\r")
#define SCR_RES_DELIMITER	((char)0x1F)

#define S_TIMEOVER			_T("TIMEOVER")
#define S_EXIT				_T("EXIT")
#define S_CANCEL			_T("CANCEL")

/************************************************************
*	@brief	FILED DELIMITER DEFINE
************************************************************/
#define		FIELD_DELIMITER		((char)0x1C)
#define		FIELD_SPACE			((char)0x20)
#define		UNIT_DELIMITER		((char)0x1F)
#define		GROUP_SEPERATOR		((char)0x1D)

// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__68620D0E_FC1E_486A_8540_945EFDB3E706__INCLUDED_)
