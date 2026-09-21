#if !defined(AFX_NHMWI_H__BBC7696C_5CBD_4935_A865_D238591D1EE3__INCLUDED_)
#define AFX_NHMWI_H__BBC7696C_5CBD_4935_A865_D238591D1EE3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// NHMWI.h : main header file for NHMWI.DLL

#if !defined( __AFXCTL_H__ )
	#error include 'afxctl.h' before including this file
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CNHMWIApp : See NHMWI.cpp for implementation.

class CNHMWIApp : public COleControlModule
{
public:
	BOOL InitInstance();
	int ExitInstance();
};

extern const GUID CDECL _tlid;
extern const WORD _wVerMajor;
extern const WORD _wVerMinor;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NHMWI_H__BBC7696C_5CBD_4935_A865_D238591D1EE3__INCLUDED)
