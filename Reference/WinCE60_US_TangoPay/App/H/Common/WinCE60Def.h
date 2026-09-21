#ifndef __WIN_CE_60_DEF_H__
#define __WIN_CE_60_DEF_H__

// [2ND] NH AIREAT 2008.11.10
#if (defined(_WIN32_WCE) && (_WIN32_WCE >= 0x600))

	// NOTE - this is value is not strongly correlated to the Windows CE OS version being targeted
	#ifndef WINVER
	#define WINVER _WIN32_WCE
	#endif

	// for unsafe string control function
	#ifndef _CRT_SECURE_NO_DEPRECATE
	#define _CRT_SECURE_NO_DEPRECATE
	#endif

	// Exclude rarely-used stuff from Windows headers
	#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
	#endif

	#ifndef VC_EXTRALEAN
	#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
	#endif

	// turns off MFC's hiding of some common and often safely ignored warning messages
	#define _AFX_ALL_WARNINGS

	#include <ceconfig.h>
#else
	// for unsafe string control function
	#ifndef _CRT_SECURE_NO_DEPRECATE
	#define _CRT_SECURE_NO_DEPRECATE
	#endif

	// turns off MFC's hiding of some common and often safely ignored warning messages
	#define _AFX_ALL_WARNINGS

#endif
// end of [2ND]

#endif //__WIN_CE_60_DEF_H__