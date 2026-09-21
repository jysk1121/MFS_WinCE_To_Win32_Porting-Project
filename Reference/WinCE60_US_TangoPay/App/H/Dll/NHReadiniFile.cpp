//#include "stdafx.h"
#include "NHReadiniFile.h"
//#include "winsock.h"


//#define NH_DEBUG
#include ".\Common\NHDbgApi.h"

// if conf is true. byte swap..
#define BYTE_SWAP(val) ((((val)&0xFF)<<8)|(((val)&0xFF00)>>8))

// 저장되어 있는 값을 Order에 상관없이 정확히 읽기 위해
#define READ_ORDER_SWAP(little, val) ((little)?(BYTE_SWAP(val)):(val))
// 현재 값을 Order에 상관없이 정확히 저장하기 위해
#define WRITE_ORDER_SWAP(little, val) ((little)?(BYTE_SWAP(val)):(val))


CNHReadiniFile::CNHReadiniFile()
{
	m_pStream = NULL;

	// check host endian.
	unsigned short	Temp = 0xFFFE;
	unsigned char *pTemp = (unsigned char*)&Temp;

	if (pTemp[0] == 0xFF)
		m_bLittleEndian = FALSE;
	else
		m_bLittleEndian = TRUE;

	// default file type - ascii mode.
	m_wBOMType = BOM_NONE;

	m_iniContent.Empty();
	nTokenPos = 0;
}

CNHReadiniFile::~CNHReadiniFile()
{
	Close();
}

BOOL CNHReadiniFile::Open(BYTE* pIniContents, OpModeFlags opmode, OpModeFlags filetype)
{	
	m_wBOMType = filetype;

	CHECK_TIME_START;

	if (filetype == CNHReadiniFile::modeLittleUNICODE)
	{
		m_wBOMType = BOM_UCS2_LE;

		USHORT wBOM = pIniContents[0] << 8 | pIniContents[1];
		NHDBG((L"BOM Type : [0x%x]\n", wBOM));

		if (wBOM == BOM_UTF8
			&& pIniContents[2] == BOM_UTF8_THIRDBYTE)				// UTF8 (EF BB "BF")
		{
			m_wBOMType = BOM_UTF8;

			// UTF8 -> UTF16
			// Skip BOM character (first 3 bytes)
			int len = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)&pIniContents[3], -1, NULL, 0);
			WCHAR* wchTemp = new WCHAR[len+1]();
			MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)&pIniContents[3], -1, wchTemp, len);
			m_iniContent = wchTemp;
			delete[] wchTemp;
		}
		else
		{
			m_iniContent = CString((WCHAR*)pIniContents);
		}
	}
	else
	{
		m_iniContent = CString(pIniContents);
	}

	NHDBG((L"m_iniContent.GetLength(): %d\n", m_iniContent.GetLength()));

	CHECK_TIME_END;
	
	return TRUE;
}


BOOL CNHReadiniFile::Open(LPCTSTR lpszFileName, OpModeFlags opmode, OpModeFlags filetype)
{
	TCHAR	szMode[5];
	WCHAR	Temp;

	Close();


	// Read Operation
	if (modeRead == opmode)
// 		_tcscpy(szMode, (_T("rt")));
		_tcscpy(szMode, (_T("rb")));	// [#2186] US KMK 2013.04.23 유니코드 0x1A를 EOF로 인식하는 문제 수정
	else
		_tcscpy(szMode, (_T("wb")));

	// file Open
	m_pStream = _tfopen(lpszFileName, szMode);

	if (!m_pStream)
		return FALSE;
	
	if (modeRead == opmode)
	{
		// Unicode file
		if (modeASCII != filetype)
		{
			// failed
			if (!fread(&Temp, sizeof(Temp), sizeof(Temp)/sizeof(WCHAR)/* 1 */, m_pStream))
				goto ErrorReturn;

			NHDBG((L"BOM Type of '%s': [0x%x]\n", lpszFileName, Temp));

			// 0xFFFE	unicode (little endian)
			// 0xFEFF	unicode (big endian)
			USHORT wBOM = Temp & 0xFFFF;
			switch (wBOM)
			{
			case BOM_UCS2_LE:
			case BOM_UCS2_BE:
			case BOM_UTF8:					// UTF8 ("EF BB" BF)
			case BOM_UTF8_REVERSE:
				// good
				break;

			default:
				// not good
				goto ErrorReturn;
				break;
			}

			if (wBOM == BOM_UTF8 || wBOM == BOM_UTF8_REVERSE)
			{
				// read 1 more character (total 3 BOM characters)
				unsigned char chTemp;
				fread(&chTemp, sizeof(chTemp), sizeof(chTemp)/sizeof(char), m_pStream);

				if (chTemp == BOM_UTF8_THIRDBYTE)		// UTF8 (EF BB "BF")
				{
					m_wBOMType = BOM_UTF8;
					return TRUE;
				}
				else
				{
					// go one step back when it's not UTF-8.
					fseek(m_pStream, -1, SEEK_CUR);
				}
			}

			// we convert for read value.	[ 0xFEFF (65279) -> 0XFFFE (65534) ]
			m_wBOMType = (unsigned short)READ_ORDER_SWAP(m_bLittleEndian, (unsigned short)Temp);
		}
		
	}
	else	// in case of "not modeRead". It is not used currently..?
	{
		if (modeASCII == filetype)
			m_wBOMType = BOM_NONE;
		else
		{
			// we convert for read value.
			if (modeBigUNICODE == filetype)
				m_wBOMType = BOM_UCS2_BE;
			else
				m_wBOMType = BOM_UCS2_LE;

			Temp = m_wBOMType;
			Temp = (WCHAR)WRITE_ORDER_SWAP(m_bLittleEndian, (unsigned short)Temp);

			// write Unicode Header..
			int nNumWrite = fwrite((void*)&Temp, sizeof(Temp), 1, m_pStream);		// [CODESONAR] /* Ignored Return Value (ID: 192) */
		}
	}

	return TRUE;

ErrorReturn:
	Close();
	return FALSE;
}

void CNHReadiniFile::Close(void)
{
	if (m_pStream)
		fclose(m_pStream);

	m_pStream = NULL;
}

/*---------------------------------------

--> example (dispaly ';' (0x003b))
      < little >  < big >
Hed		FFFE		FEFF
File	3b00		003b
RAM		3b00		003b (read fget..)
VALUE	003b		3b00 (little system)
		3b00		003b (big system)
---------------------------------------*/
BOOL CNHReadiniFile::ReadString(BOOL bAscii)
{
	unsigned char *pText;
	unsigned short	Value;
// 	unsigned char	Temp;
	int		i;

	///////////////////////////////////////////////////////////////////
	//	Read File
	///////////////////////////////////////////////////////////////////

	// read unicode file
	if (m_wBOMType == BOM_UCS2_LE || m_wBOMType == BOM_UTF8)
	{
		int	nNumWrite = 0;

		memset((void*)m_LineBuffer, 0, MAX_LINE_BUFFER+1);
		memset((void*)m_LineBufferUnicode, 0, MAX_LINE_BUFFER+1);

// 		if (NULL == fgets((char*)m_LineBuffer, MAX_LINE_BUFFER, m_pStream))
// 			return FALSE;
		if (m_wBOMType == BOM_UTF8)
		{
			// UTF8 should be read via fgets(), and converted by MultiByteToWideChar().
			if (NULL == fgets((char*)m_LineBuffer, MAX_LINE_BUFFER, m_pStream))
				return FALSE;

			int len = MultiByteToWideChar(CP_UTF8, 0, m_LineBuffer, -1, NULL, 0);
			MultiByteToWideChar(CP_UTF8, 0, m_LineBuffer, -1, m_LineBufferUnicode, len);
		}
		else
		{
			if (NULL == fgetws((WCHAR*)m_LineBufferUnicode, MAX_LINE_BUFFER, m_pStream))
				return FALSE;
		}


// 		// must read by 2byte.
// 		if (m_nFileType == LITTLE_ENDIAN)
// 			nNumWrite = fread(&Temp, sizeof(Temp), sizeof(Temp), m_pStream);		// [CODESONAR]  /* Ignored Return Value (ID: 193) */
	}
	// read ascii file
	else
	{
		memset((void*)m_LineBuffer, 0, MAX_LINE_BUFFER+1);

		if (NULL == fgets((char*)m_LineBuffer, MAX_LINE_BUFFER, m_pStream))
			return FALSE;

		pText = (unsigned char*)&m_LineBuffer[0];
		for(i = 0; i < MAX_LINE_BUFFER ; i++)
		{
			if (pText[0] == 0x0D || pText[0] == 0x0A)
				pText[0] = ' ';

			pText++;

			// is null?
			if (pText[0] == 0)
				break;
		}
	}

	
	///////////////////////////////////////////////////////////////////
	// Byte Order process. convert specific symbol('\n', '\r')
	///////////////////////////////////////////////////////////////////

	// read unicode file
	if (m_wBOMType == BOM_UCS2_LE)
	{
		WCHAR *pTextUnicode;

		// swap byte order. convert '\n'->' ', '\r'->' '
		for(i = 0; i < MAX_LINE_BUFFER ; i++)
		{
			pTextUnicode = (WCHAR*)&m_LineBufferUnicode[i];
			Value = (unsigned short)m_LineBufferUnicode[i];
			
			// little system -> swap big endian file type
			if ((m_bLittleEndian && m_wBOMType == BOM_UCS2_BE) || 
				(!m_bLittleEndian && m_wBOMType == BOM_UCS2_LE))
			{
				pTextUnicode[0] = (unsigned char)((Value & 0xff00) >> 8);
				pTextUnicode[1] = (unsigned char)(Value & 0xff);
			}
			
			//convert '\n'->' ', '\r'->' '
			if (pTextUnicode[0] == '\n' || pTextUnicode[0] == '\r')
				pTextUnicode[0] = ' ';
			if (pTextUnicode[1] == '\n' || pTextUnicode[1] == '\r')
				pTextUnicode[1] = ' ';
			
			// is null?, end swap order
			if(	(pTextUnicode[0] == 0 && pTextUnicode[1] == 0))
				break;
		}
	}
	
	return TRUE;
}

PINIVALUE	CNHReadiniFile::ReadiniValue(void)
{
	TCHAR	chOld = (TCHAR)0x8;
	TCHAR	chNew = '\n';
	CString	strOneLine;
	int		nSearchIndex, nFirstIndex, nLastIndex;

	// don't open file
	if (!m_pStream && m_iniContent.IsEmpty())
		return FALSE;

	m_INIVALUE.Reset();
	//m_INIVALUE.nValue = -1;

	// Read One Line
	while(1)
	{
		// TAR first
		if (!m_iniContent.IsEmpty())
		{
			strOneLine = m_iniContent.Tokenize(L"\n", nTokenPos);
			if (strOneLine.IsEmpty())
				break;

 			strOneLine += L"\n";		// TAR, appends a linefeed character for compatibility
		}
		// end of TAR
		else
		{
			if (!ReadString(TRUE))
				break;

			if (m_wBOMType != BOM_NONE)
				strOneLine = m_LineBufferUnicode;
			else
				strOneLine = m_LineBuffer;
		}

		strOneLine.TrimLeft(_T("\t \n"));

		if (strOneLine.GetLength() == 0)
			continue;
		
		// skip comment
		if (strOneLine.GetAt(0) == ';' || strOneLine.GetAt(0) == '#')
			continue;
		
		
		// replace '0x8' -> '\n'
		strOneLine.Replace(chOld, chNew);
		
		//////////////////////////////////
		// find section
		nSearchIndex = strOneLine.Find('=');
		if (nSearchIndex == -1)
		{
			nFirstIndex = nLastIndex = -1;
			
			// Search Section
			nFirstIndex = strOneLine.Find('[');
			if (nFirstIndex != -1)
			{
				nLastIndex = strOneLine.Find(']', nFirstIndex+1);
			}
			
			// found section
			if (nFirstIndex != -1 && nLastIndex != -1 && (nLastIndex - nFirstIndex -1) > 0)
			{
				m_INIVALUE.Section = strOneLine.Mid(nFirstIndex+1, (nLastIndex - nFirstIndex -1));
				m_INIVALUE.Section.MakeUpper();
				
				if (m_INIVALUE.Section.GetAt(0) == '/')
					m_INIVALUE.Values[0] = L"0";
				else
					m_INIVALUE.Values[0] = L"1";

				m_INIVALUE.Section.TrimLeft('/');

				m_INIVALUE.Key = m_INIVALUE.Section;
				
				m_INIVALUE.nValue = 1;
				
				return &m_INIVALUE;
			}
			
		}
		
		////////////////////////////////
		// find key and value.
		if (strOneLine.GetLength() != 0)
		{
			nSearchIndex = strOneLine.Find('=');
			if (nSearchIndex != -1)
			{
				nFirstIndex = strOneLine.Find('=');
				if (nFirstIndex == -1)
					continue;
				
				// Set Key
				m_INIVALUE.Key = strOneLine.Left(nFirstIndex);
				m_INIVALUE.Key.MakeUpper().Trim();
				
				if (m_INIVALUE.nValue == -1)
					m_INIVALUE.nValue++;
				
				// trim
				strOneLine = strOneLine.Mid(nFirstIndex+1, (strOneLine.GetLength()-nFirstIndex+1));
				strOneLine.TrimLeft(_T("\t \n"));

				// [#CHA] 2013.02.28 DAT 로딩 시 따옴표, 콤마 처리 보완
				while (strOneLine.GetLength() != 0)
				{
					strOneLine.TrimLeft(_T("\t \n"));
					if (strOneLine.GetLength() == 0)
					{
						break;
					}

					// SetValue
					if (strOneLine.GetAt(0) == _T('\"'))
					{
						nFirstIndex = nLastIndex = -1;
						nFirstIndex = 0;
						
						nLastIndex = strOneLine.Find(_T("\""), nFirstIndex+1);
						
						if (nFirstIndex != -1 && nLastIndex != -1 && (nLastIndex - nFirstIndex -1) >= 0)
						{
							m_INIVALUE.Values[m_INIVALUE.nValue] = strOneLine.Mid(nFirstIndex+1, (nLastIndex - nFirstIndex -1));
							
							m_INIVALUE.Values[m_INIVALUE.nValue].TrimRight(L"\r\t \n");		// TAR compatibility
							m_INIVALUE.nValue++;
						}
						else
						{
							nLastIndex = strOneLine.GetLength() - 1;	
						}

						// trim
						strOneLine = strOneLine.Mid(nLastIndex+1, (strOneLine.GetLength()-nLastIndex+1));
					}
					else
					{
						nFirstIndex = 0;
						
						// Over max values
						if (m_INIVALUE.nValue >= MAX_VALUE_COUNT)
						{
							break;
						}

						// Comments
						if (strOneLine.GetAt(0) == ';' || strOneLine.GetAt(0) == '#')
						{
							break;
						}

						nSearchIndex = strOneLine.FindOneOf(_T(", \t\n"));
						
						// No more sep./end characters
						if (nSearchIndex <= 0)
						{
							// Since no contol chars were found (could be for line-length limits)
							// we should read the entire string
							nSearchIndex = strOneLine.GetLength() - 1;
						}

						// Good value!
						m_INIVALUE.Values[m_INIVALUE.nValue] = strOneLine.Left(nSearchIndex);
						m_INIVALUE.Values[m_INIVALUE.nValue].TrimRight(L"\r\t \n");	// TAR
						m_INIVALUE.nValue++;

						// trim
						strOneLine = strOneLine.Mid(nSearchIndex+1, (strOneLine.GetLength()-nSearchIndex+1));
					}
				}
			}
			
			// check validate.
			if ((m_INIVALUE.Section.GetLength() > 0) && (m_INIVALUE.nValue > 0))
				return &m_INIVALUE;
			else
				m_INIVALUE.nValue = -1;
		}
		
	}

	return NULL;
}
