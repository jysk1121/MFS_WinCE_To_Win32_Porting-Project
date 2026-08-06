#ifndef __CINIFILE_H__
#define __CINIFILE_H__

#pragma once

extern CString GetFirstParam(CString& strName, TCHAR tDelimiter = _T(','));
extern CString GetSpecificParam(CString& strName, BOOL iOption);

class CIniFile
{
public:
	// 생성자
	CIniFile(CString strFileName);

	// 문자열 읽기
	CString ReadString(LPCTSTR m_Sec, LPCTSTR m_Ident, LPCTSTR m_Def);
	// 문자열 쓰기
	BOOL WriteString(LPCTSTR m_Sec, LPCTSTR m_Ident, LPCTSTR m_Val);
	// 섹션 리스트 읽기
	BOOL ReadSections(CStringArray& m_Secs);
	// 섹션 읽기
	BOOL ReadSection(LPCTSTR m_Sec, CStringArray& m_Secs);

protected:
	CString m_strFileName;
};

#endif