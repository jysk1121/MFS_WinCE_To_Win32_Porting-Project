#ifndef __UP_INIT_H__
#define __UP_INIT_H__

#include "FileMgr.h"

//////////////////////////////////////////////////////////////////////////
//	구동에 필요한 준비작업을 수행 한다.

class CUP_Update : public CFileMgr
{
public:
	CUP_Update();
	~CUP_Update();

private:
//	CString	GetUpdateTitle(CString strUpdateRoot);
	CString	GetUpdateTitle(CUpdateStateInfo &StateInfo, CString strUpdateRoot);		// [#11] NH KSK 2010.10.2 update title File을 Open하지 못하는 Bug Fix
//	void	DeleteFileBeforePatch(CString strUpdateRoot);
	void	DeleteFileBeforePatch(CUpdateStateInfo &StateInfo, CString strUpdateRoot);	// [#11] NH KSK 2010.10.2 Delete Prof File을 Open하지 못하는 Bug Fix
	BOOL	UpdateFile(CUpdateStateInfo &StateInfo);

	// Method define
public:
	CString Process(CUpdateStateInfo &StateInfo);						// State 처리
};

CUP_Update	g_CUP_Update;

#endif // __UP_INIT_H__
