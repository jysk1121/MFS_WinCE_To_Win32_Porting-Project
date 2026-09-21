#ifndef __UP_INIT_H__
#define __UP_INIT_H__

#include "FileMgr.h"

//////////////////////////////////////////////////////////////////////////
//	구동에 필요한 준비작업을 수행 한다.

class CUP_FromRMS : public CFileMgr
{
public:
	CUP_FromRMS();
	~CUP_FromRMS();

private:
	BOOL	CheckFormRMS(CUpdateStateInfo &StateInfo);

	// Method define
public:
	CString Process(CUpdateStateInfo &StateInfo);						// State 처리
};

CUP_FromRMS	g_CUP_FromRMS;

#endif // __UP_INIT_H__
