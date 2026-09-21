#ifndef __UP_INIT_H__
#define __UP_INIT_H__

#include "FileMgr.h"

//////////////////////////////////////////////////////////////////////////
//	구동에 필요한 준비작업을 수행 한다.

class CUP_Validate : public CFileMgr
{
public:
	CUP_Validate();
	~CUP_Validate();

private:
	BOOL	CheckValidate(CUpdateStateInfo &StateInfo);

	// Method define
public:
	CString Process(CUpdateStateInfo &StateInfo);						// State 처리
};

CUP_Validate	g_CUP_Validate;

#endif // __UP_INIT_H__
