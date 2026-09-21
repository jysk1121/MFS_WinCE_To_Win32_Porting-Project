#ifndef __UP_INIT_H__
#define __UP_INIT_H__

#include "FileMgr.h"

//////////////////////////////////////////////////////////////////////////
//	구동에 필요한 준비작업을 수행 한다.

class CUP_Init : public CFileMgr
{
public:
	CUP_Init();
	~CUP_Init();

	// Method define
public:
	CString Process(CUpdateStateInfo &StateInfo);						// State 처리
};

CUP_Init	g_CUP_Init;

#endif // __UP_INIT_H__
