// LibUpdate.h : main header file for the LibUpdate DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#define DllImport			__declspec(dllimport)
#define DllExport			__declspec(dllexport)

#ifndef _LIBUPDATE_OWNER_
#define DllUseport			DllImport
#else
#define DllUseport			DllExport
#endif

#include "UpdateTypes.h"
#include "UpdateRepoConfigurationManager.h"
#include "UpdateRepoService.h"
#include "RepoLocalCache.h"
