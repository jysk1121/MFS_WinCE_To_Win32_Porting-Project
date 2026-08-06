// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 및 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.
//

#pragma once

#ifdef _WIN32_WCE
#pragma comment(linker, "/nodefaultlib:libc.lib")
#pragma comment(linker, "/nodefaultlib:libcd.lib")

// 참고 - 이 값은 대상 Windows CE OS 버전과 깊은 관련이 없습니다.
#define WINVER _WIN32_WCE

#include <ceconfig.h>
#if defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP)
#define SHELL_AYGSHELL
#endif

#ifdef _CE_DCOM
#define _ATL_APARTMENT_THREADED
#endif
#else
#ifndef WINVER
#define WINVER 0x0600	// Win32 desktop: the value the SDK was defaulting to anyway;
                    	// declaring it only silences the warning, it must NOT lower the
                    	// target, or version-gated SDK headers (e.g. the one providing
                    	// ipexport.h / IPAddr for <Icmpapi.h>) drop out.
#endif
#endif // _WIN32_WCE

#ifdef _WIN32_WCE
#include <windows.h>
#include <commctrl.h>
#endif // _WIN32_WCE

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// 일부 CString 생성자는 명시적으로 선언됩니다.

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// ATLMFC 헤더에서 거의 사용되지 않는 항목을 제외합니다.
#endif


#include <afx.h>
#include <afxwin.h>         // MFC 핵심 및 표준 구성 요소입니다.




#if defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP)
#ifndef _DEVICE_RESOLUTION_AWARE
#define _DEVICE_RESOLUTION_AWARE
#endif
#endif

#ifdef _DEVICE_RESOLUTION_AWARE
#include "DeviceResolutionAware.h"
#endif

#ifdef _WIN32_WCE
#include <altcecrt.h>
#endif // _WIN32_WCE

// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.
#include "..\\..\\Include\\Common\\Eagle_Common_Define.h"	// 모든 Project에서 참조할 수 있는 header file
