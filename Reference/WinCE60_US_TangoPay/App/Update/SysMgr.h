#ifndef __SYSTEM_MANAGER_H__
#define __SYSTEM_MANAGER_H__

#include "UpdateState.h"

//////////////////////////////////////////////////////////////////////////
//	File과 관련된 작업 UTIL

class CSystemMgr: public CUpdateState
{
protected:
	CSystemMgr();
public:
	~CSystemMgr();

protected:
	/*----------------------------------------------------------------
			REGISTRY FUNTION
	----------------------------------------------------------------*/
private:
	int RegQueryValueExt(LPCTSTR hKeyName, LPCTSTR lpszValueName, DWORD dwType, DWORD dwLen, LPVOID lpszData);

public:
	int	RegGetInt(LPCTSTR hKeyName, LPCTSTR lpszValueName, int InitValue);
	CString	RegGetStr(LPCTSTR hKeyName, LPCTSTR lpszValueName, LPCTSTR InitValue);

	CString GetUpdateFileName(UpdateType eType, CString strPath);
};

#endif //__SYSTEM_MANAGER_H__