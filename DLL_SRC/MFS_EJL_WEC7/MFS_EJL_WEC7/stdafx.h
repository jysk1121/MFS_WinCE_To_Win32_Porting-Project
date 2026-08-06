// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 및 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.

#pragma once

// 참고 - 이 값은 대상 Windows CE OS 버전과 밀접한 관련은 없습니다.
#ifdef _WIN32_WCE
#define WINVER _WIN32_WCE
#else
#ifndef WINVER
#define WINVER 0x0600	// Win32 desktop: the value the SDK was defaulting to anyway;
                    	// declaring it only silences the warning, it must NOT lower the
                    	// target, or version-gated SDK headers (e.g. the one providing
                    	// ipexport.h / IPAddr for <Icmpapi.h>) drop out.
#endif
#endif // _WIN32_WCE

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// 일부 CString 생성자는 명시적으로 선언됩니다.

#ifdef _CE_DCOM
#define _ATL_APARTMENT_THREADED
#endif

#ifdef _WIN32_WCE
#include <ceconfig.h>
#if defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP)
#define SHELL_AYGSHELL
#endif
#endif // _WIN32_WCE

#include <afxwin.h>         // MFC 핵심 및 표준 구성 요소입니다.
#include <afxext.h>         // MFC 확장입니다.

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxole.h>         // MFC OLE 클래스입니다.
#include <afxodlgs.h>       // MFC OLE 대화 상자 클래스입니다.
#include <afxdisp.h>        // MFC 자동화 클래스입니다.
#endif // _AFX_NO_OLE_SUPPORT

#include <afxdtctl.h>		// Internet Explorer 4 공용 컨트롤에 대한 MFC 지원입니다.
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// Windows 공용 컨트롤에 대한 MFC 지원입니다.
#endif // _AFX_NO_AFXCMN_SUPPORT


#if defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP)
#ifndef _DEVICE_RESOLUTION_AWARE
#define _DEVICE_RESOLUTION_AWARE
#endif
#endif

#ifdef _DEVICE_RESOLUTION_AWARE
#include "DeviceResolutionAware.h"
#endif



#ifdef _WIN32_WCE
#if (_WIN32_WCE < 0x500) && ( defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP) )
	#pragma comment(lib, "ccrtrtti.lib")
	#ifdef _X86_	
		#if defined(_DEBUG)
			#pragma comment(lib, "libcmtx86d.lib")
		#else
			#pragma comment(lib, "libcmtx86.lib")
		#endif
	#endif
#endif

#include <altcecrt.h>
#endif // _WIN32_WCE

#include <time.h>

#ifndef _WIN32_WCE
// Pull the STL headers in here, while "new" still means new.
//
// MFS_EJL_WEC7.cpp does "#define new DEBUG_NEW" and only afterwards includes
// EJLDBCtrl.h -> DbSqlite.h -> <vector>. With that macro active the line
// "::operator new(_Count * sizeof (_Ty))" inside <xmemory> expands to
// "::operator new(THIS_FILE, __LINE__)(...)", which does not compile (C2665).
// stdafx.h is the first include in every translation unit, so parsing the
// STL here keeps those templates clean. WinCE used a different STL and never
// hit this, so it is left untouched.
#include <vector>
#include <string>
#endif	// !_WIN32_WCE
