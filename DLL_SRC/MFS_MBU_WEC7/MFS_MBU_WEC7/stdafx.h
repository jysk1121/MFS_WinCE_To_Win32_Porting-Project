// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 및 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.

#pragma once

// 참고 - 이 값은 대상 Windows CE OS 버전과 밀접한 관련은 없습니다.
#define WINVER _WIN32_WCE

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// 일부 CString 생성자는 명시적으로 선언됩니다.

#ifdef _CE_DCOM
#define _ATL_APARTMENT_THREADED
#endif

#include <ceconfig.h>
#if defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP)
#define SHELL_AYGSHELL
#endif

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

//
#define  MAX_RECEIVE			(256 + 1)
#define  MAX_SEND				(256 + 1)

//
typedef struct
{
	WORD wLength;
	BYTE byCommand;
	BYTE byDevID;
	BYTE Data[256];
} RX_PACKET;

// 
typedef enum 
{
	ID_MBU = 0,
	ID_SIU,
	ID_ASD,
	ID_BA1,
	ID_BA2,
	ID_BA3,
	ID_BAR
};

//
#define MBU_CMD_VERSION				0x30
#define MBU_CMD_RESET				0x31
#define MBU_CMD_SEND				0x32
#define MBU_CMD_RECV				0x33
	#define MBU_CMD_RECVFUNC_READ			0x30
	#define MBU_CMD_RECVFUNC_BUFF_RESET		0x31
#define MBU_CMD_MULTI_COMPORT_SET	0x34

#define SUPPORT_MULTIPORT_FUNC_VERSION		'4'
#define SUPPORT_READPORT_FUNC_VERSION		'5'

