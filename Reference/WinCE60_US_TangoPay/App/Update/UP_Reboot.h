#ifndef __UP_INIT_H__
#define __UP_INIT_H__

#include "FileMgr.h"

//////////////////////////////////////////////////////////////////////////
//	구동에 필요한 준비작업을 수행 한다.

class CUP_Reboot : public CFileMgr
{
public:
	CUP_Reboot();
	~CUP_Reboot();

private:

	// Method define
public:
	CString Process(CUpdateStateInfo &StateInfo);						// State 처리
};

CUP_Reboot	g_CUP_Reboot;

#endif // __UP_INIT_H__
