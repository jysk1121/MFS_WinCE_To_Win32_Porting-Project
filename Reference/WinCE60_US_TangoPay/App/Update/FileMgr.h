#ifndef __FILE_MANAGER_H__
#define __FILE_MANAGER_H__

#include "ScreenMgr.h"

//////////////////////////////////////////////////////////////////////////
//	File과 관련된 작업 UTIL

class CFileMgr: public CScrMgr
{
protected:
	CFileMgr();
public:
	~CFileMgr();

protected:
	BOOL IsFolder(CString strFileName);
	BOOL CountFileInDirectory(LPCTSTR lpDstPath, LPCTSTR lpDstFile, int &nCount);
	BOOL CopyFileInDirectory(LPCTSTR lpSrcPath, LPCTSTR lpSrcFile, LPCTSTR lpDstPath, BOOL bShowCount, int &nCurCnt, int nTotalCnt);
	BOOL DeleteFileInDirectory(LPCTSTR lpDstPath, LPCTSTR lpDstFile, BOOL bShowCount, int &nCurCnt, int nTotalCnt);
	BOOL IsExistFile(CString strFilePath);
	BOOL UnzipFileToDirectory(LPCTSTR lpSrcFile, LPCTSTR lpDstPath);

	// Shell Functions
	bool ShDeleteDirectory(LPCTSTR lpDstPath);
	bool ShCopyDirectory(LPCTSTR lpSrcPath, LPCTSTR lpDstPath);
};

#endif //__FILE_MANAGER_H__