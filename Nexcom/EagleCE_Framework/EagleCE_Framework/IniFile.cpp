#include "stdafx.h"
#include <Afx.h>
#include <AfxColl.h>

#include "IniFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifdef _WIN32_WCE
// Win32 desktop already declares GetPrivateProfileString/WritePrivateProfileString
// (macros to ...W/...A in <winbase.h>, backed by kernel32.lib). WinCE lacks these
// APIs natively, so this file provides its own implementation for CE only.
DWORD GetPrivateProfileString(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpDefault, LPTSTR lpReturnedString, DWORD nSize, LPCTSTR lpFileName);
BOOL WritePrivateProfileString(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpString, LPCTSTR lpFileName);
BOOL InsertKeyString(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpString, LPCTSTR lpFileName);
#endif // _WIN32_WCE

/** **********************************************************
*	@brief		생성자
*	@retval		없음
************************************************************/
CIniFile::CIniFile(CString strFileName)
{
	m_strFileName = strFileName;
}


/** **********************************************************
*	@brief		문자열 읽기
*	@retval		없음
************************************************************/
CString CIniFile::ReadString(LPCTSTR m_Sec, LPCTSTR m_Ident, LPCTSTR m_Def)
{
	TCHAR Buffer[4096];
	memset(Buffer, 0, sizeof(Buffer));

	GetPrivateProfileString(m_Sec, m_Ident, m_Def, Buffer, sizeof(Buffer), m_strFileName);

	return Buffer;
}


/** **********************************************************
*	@brief		문자열 쓰기
*	@retval		없음
************************************************************/
BOOL CIniFile::WriteString(LPCTSTR  m_Sec, LPCTSTR  m_Ident, LPCTSTR  m_Val)
{
	return WritePrivateProfileString(m_Sec, m_Ident, m_Val, m_strFileName);
}


/** **********************************************************
*	@brief		섹션 리스트 읽기
*	@retval		없음
************************************************************/
BOOL CIniFile::ReadSections(CStringArray& m_Secs)
{
	LPVOID pvData = NULL;
	HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, 16385);
	_ASSERTE(NULL != hGlobal);

	pvData = GlobalLock(hGlobal);
	_ASSERTE(NULL != pvData);

	m_Secs.RemoveAll();

	if (GetPrivateProfileString(NULL, NULL, NULL, (TCHAR*)pvData, 16384, m_strFileName))
	{
		TCHAR *P = (TCHAR*)pvData;
		while (1)
		{
			// NULL 체크
			if (!P) {
				break;
			}

			if (*P == 0) {
				break;
			}

			m_Secs.Add(P);
			P += _tcsclen(P) + 1;
		}
	}
	GlobalUnlock(hGlobal);
	GlobalFree(hGlobal);
	return m_Secs.GetSize() > 0;
}

/** **********************************************************
*	@brief		섹션 읽기
*	@retval		없음
************************************************************/
BOOL CIniFile::ReadSection(LPCTSTR m_Sec, CStringArray& m_Secs)
{
	LPVOID pvData = NULL;
	HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, 16385);
	_ASSERTE(NULL != hGlobal);

	pvData = GlobalLock(hGlobal);
	_ASSERTE(NULL != pvData);

	m_Secs.RemoveAll();

	if (GetPrivateProfileString(m_Sec, NULL, NULL, (TCHAR*)pvData, 16384, m_strFileName))
	{
		TCHAR *P = (TCHAR*)pvData;
		while (1)
		{
			// NULL 체크
			if (!P) {
				break;
			}

			if (*P == 0) {
				break;
			}

			m_Secs.Add(P);
			P += _tcsclen(P) + 1;
		}
	}
	GlobalUnlock(hGlobal);
	GlobalFree(hGlobal);
	return m_Secs.GetSize() > 0;
}

CString GetFirstParam(CString& strName, TCHAR tDelimiter /*= _T(',')*/)
{
	int P = strName.Find(tDelimiter);
	TCHAR m_Res[1024] = { 0, };

	_stprintf_s(m_Res, _countof(m_Res), _T("%s"), strName);
	if (P < 0)
	{
		return m_Res;
	}
	m_Res[P] = _T('\0');

	strName.Delete(0, P + 1);
	strName.TrimLeft();
	strName.TrimRight();

	return m_Res;
}

CString GetSpecificParam(CString& strSource, BOOL iOption)
{
	CString			rtnString, szTemp;

	szTemp = strSource;

	for (int i = 0; i<iOption; i++)
	{
		rtnString = GetFirstParam(szTemp);
	}

	return rtnString;
}

#ifdef _WIN32_WCE
DWORD GetPrivateProfileString(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpDefault, LPTSTR lpReturnedString, DWORD nSize, LPCTSTR lpFileName)
{
	TCHAR ch;
	bool search = true;
	bool find = false;
	int state = 0;
	int idx = 0;
	FILE* fp = _tfopen(lpFileName, _T("rt"));

	if (fp == NULL)
		search = false;

	while (search && 1 == _ftscanf(fp, _T("%c"), &ch))
	{
		switch (state)
		{
		case 0:	// 첫글자
			if (ch == _T(';'))
			{
				state = 10;	// 주석 넘기기
			}
			else if (ch == _T('['))
			{
				idx = 0;
				memset(lpReturnedString, 0, nSize);
				state = 20;	// AppName 비교
			}
			break;

		case 10:
			if (ch == _T('\n'))
			{
				state = 0;
			}
			break;

		case 20:				///< Search lpAppName
			if (ch == _T(']'))
			{
				lpReturnedString[idx++] = 0;
				if (_tcscmp(lpReturnedString, lpAppName) == 0)
				{
					idx = 0;
					memset(lpReturnedString, 0, nSize);
					state = 30;
				}
				else
					state = 10;
			}
			else
			{
				lpReturnedString[idx++] = ch;
			}
			break;

		case 30:
			if (ch == _T('\n'))
			{
				state = 31;
			}
			break;
			
		case 31:
			if (ch == _T('['))
			{
				search = false;
			}
			else if (ch == _T(';'))
			{
				state = 30;
			}
			else if (ch == _T('\n'))
			{
				state = 31;
			}
			else
			{
				idx = 0;
				memset(lpReturnedString, 0, nSize);
				lpReturnedString[idx++] = ch;
				state = 32;
			}
			break;

		case 32:		///< Search Key Name
			if (ch == _T('='))
			{
				lpReturnedString[idx++] = 0;
				if (_tcscmp(lpReturnedString, lpKeyName) == 0)
				{
					idx = 0;
					memset(lpReturnedString, 0, nSize);
					state = 40;
				}
				else
				{
					state = 30;
				}
			}
			else
			{
				lpReturnedString[idx++] = ch;
			}

			break;

		case 40:		///<	Get Value
			if (ch == _T('\n') || ch == EOF)
			{
				lpReturnedString[idx++] = 0;
				search = false;
				find = true;
				goto search_end;
			}
			else
			{
				lpReturnedString[idx++] = ch;
			}
			break;
		}
	}

search_end:

	if (fp != NULL)
		fclose(fp);

	if (!find)
		_tcscpy(lpReturnedString, lpDefault);

	return _tcslen(lpReturnedString);
}

BOOL WritePrivateProfileString(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpString, LPCTSTR lpFileName)
{
	CString out;
	CString str;
	CString strAppName, strKeyName;
	TCHAR ch;
	bool search = true;
	bool find = false;
	bool app = false;
	int state = 0;
	BOOL bInsertKeySuccess = FALSE;

	strAppName = lpAppName;
	strKeyName = lpKeyName;

	FILE* fp = _tfopen(lpFileName, _T("rt"));

	if (fp == NULL)
	{
		search = false;
	}
	else
	{
		while (1 == _ftscanf(fp, _T("%c"), &ch))
			out += ch;
		
		fclose(fp);
	}

	for (int i = 0; search && i < out.GetLength(); i++)
	{
		ch = out[i];

		switch (state)
		{
		case 0:	// 첫글자
			if (ch == _T(';'))
			{
				state = 10;	// 주석 넘기기
			}
			else if (ch == _T('['))
			{
				str.Empty();
				state = 20;       // AppName 비교
			}
			break;

		case 10:
			if (ch == _T('\n'))
			{
				state = 0;
			}
			break;

		case 20:
			if (ch == _T(']'))
			{
				if (strAppName.CompareNoCase(str) == 0)
				{
					app = true;
					str.Empty();
					state = 30;
				}
				else
				{
					state = 10;
				}
			}
			else if (ch == _T('\n'))
			{
				state = 0;
			}
			else
			{
				str += ch;
			}
			break;

		case 30:	// AppName이 일치시에만
			if (ch == _T('\n'))
			{
				state = 31;
			}
			break;

		case 31:
			if (ch == _T('['))
			{
				// Section을 찾은 다음 다시 다른 Section이 시작되면 초기화 처리
				app = false;
				str.Empty();
				state = 20;
			}
			else if (ch == _T(';'))
			{
				str.Empty();
				state = 30;
			}
			else if (ch == _T('\n'))
			{
				;
			}
			else
			{
				str += ch;		// Key값 삽입
				state = 32;
			}
			break;

		case 32:				// Key Search
			if (ch == _T('='))
			{
				if (strKeyName.CompareNoCase(str) == 0)
				{
					str.Empty();
					state = 40;
				}
				else
				{			
					str.Empty();
					state = 30;
				}
			}
			else if (ch == _T('\n'))
			{
				// 다른 key를 찾음
				str.Empty();
				state = 30;
			}
			else
			{
				str += ch;
			}
			break;

		case 40: // KeyName이 일치한 후
			if (ch == _T('\n'))
			{
				out.Delete(i - str.GetLength(), str.GetLength());
				out.Insert(i - str.GetLength(), lpString);

				find = true;

				goto Search_End;
			}
			else
			{
				str += ch;
			}
			break;
		}
	}

	if (!find) // AppName도KeyName도찾지못했다면
	{
		if (out.GetLength() > 0)
		{
			ch = out[out.GetLength()-1];
			if (ch != _T('\n'))
				out += _T('\n');
		}

		if(app)
		{
			bInsertKeySuccess = InsertKeyString(lpAppName, lpKeyName, lpString, lpFileName);
			if(TRUE == bInsertKeySuccess)
				return TRUE;
		}
		else
		{
			str.Format(_T("[%s]\n"), lpAppName);
			out += str;
			str.Format(_T("%s=%s\n"), lpKeyName, lpString);
			out += str;
		}
	}

Search_End:

	fp = _tfopen(lpFileName, _T("wt"));

	if (fp == NULL)
		return FALSE;

	// 값이 %인 경우에는 File Wirte시에 %%로 해줘야 저장됨.
	out.Replace(_T("%"), _T("%%"));

	_ftprintf(fp, out);
	fclose(fp);

	return TRUE;
} 


BOOL InsertKeyString(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpString, LPCTSTR lpFileName)
{
	CString out;
	CString str;
	TCHAR ch;
	bool search = true;
	bool find = false;
	bool app = false;
	int state = 0;
	FILE* fp = _tfopen(lpFileName, _T("rt"));

	if (fp == NULL)
	{
		search = false;
	}
	else
	{
		while (1 == _ftscanf(fp, _T("%c"), &ch))
			out += ch;
		fclose(fp);
	}

	for (int i = 0; search && i < out.GetLength(); i++)
	{
		ch = out[i];

		switch (state)
		{
		case 0:	// 첫글자
			if (ch == _T(';'))
			{
				state = 10;	// 주석 넘기기
			}
			else if (ch == _T('['))
			{
				str.Empty();
				state = 20;       // AppName 비교
			}
			break;

		case 10:
			if (ch == _T('\n'))
			{
				state = 0;
			}
			break;

		case 20:
			if (ch == _T(']'))
			{
				if (str.Compare(lpAppName) == 0)
				{
					app = true;
					str.Empty();
					state = 30;
				}
				else
				{
					state = 10;
				}
			}
			else if (ch == _T('\n'))
			{
				state = 0;
			}
			else
			{
				str += ch;
			}
			break;

		case 30:	// AppName이 일치시에만
			if (ch == _T('\n'))
			{
				state = 31;
			}
			break;

		case 31:
			if (ch == _T('['))
			{
				// Section을 찾은 다음 다시 다른 Section이 시작되면 초기화 처리
			//	app = false;
				str.Empty();
				state = 20;
				if(app)
				{
					str.Format(_T("%s=%s\n"), lpKeyName, lpString);
					out.Insert(i - 1, str);
					goto Search_End;
				}
			}
			else if (ch == _T(';'))
			{
				str.Empty();
				state = 30;
			}
			else if (ch == _T('\n'))
			{
				;
			}
			else
			{
				str += ch;		// Key값 삽입
				state = 32;
			}
			break;

		case 32:				// Key Search
			if (ch == _T('='))
			{
				if (str.Compare(lpKeyName) == 0)
				{
					str.Empty();
					state = 40;
				}
				else
				{			
					str.Empty();
					state = 30;
				}
			}
			else if (ch == _T('\n'))
			{
				// 다른 key를 찾음
				str.Empty();
				state = 30;
			}
			else
			{
				str += ch;
			}
			break;

		case 40: // KeyName이 일치한 후
			if (ch == _T('\n'))
			{
				out.Delete(i - str.GetLength(), str.GetLength());
				out.Insert(i - str.GetLength(), lpString);

				find = true;

				goto Search_End;
			}
			else
			{
				str += ch;
			}
			break;
		}
	}

	if (out.GetLength() > 0)
	{
		ch = out[out.GetLength()-1];
		if (ch != _T('\n'))
			out += _T('\n');
	}

	str.Format(_T("%s=%s\n"), lpKeyName, lpString);
	out += str;

Search_End:

	fp = _tfopen(lpFileName, _T("wt"));

	if (fp == NULL)
		return FALSE;

	_ftprintf(fp, out);
	fclose(fp);

	return TRUE;
}
#endif // _WIN32_WCE