#include "stdafx.h"
#include "ReadiniFile.h"

// if conf is true. byte swap..
#define BYTE_SWAP(val) ((((val)&0xFF)<<8)|(((val)&0xFF00)>>8))

// 저장되어 있는 값을 Order에 상관없이 정확히 읽기 위해
#define READ_ORDER_SWAP(little, val) ((little)?(BYTE_SWAP(val)):(val))

// 현재 값을 Order에 상관없이 정확히 저장하기 위해
#define WRITE_ORDER_SWAP(little, val) ((little)?(BYTE_SWAP(val)):(val))


CReadiniFile::CReadiniFile()
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
	m_nFileType = 0;
}

CReadiniFile::~CReadiniFile()
{
	Close();
}

BOOL CReadiniFile::Open(LPCTSTR lpszFileName, OpModeFlags opmode, OpModeFlags filetype)
{
	TCHAR	szMode[5];
	WCHAR	Temp;

	Close();


	// Read Operation
	if (modeRead == opmode)
		_tcscpy(szMode, (_T("rt")));
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
			
			// 0xFFFE	unicode (little endian)
			// 0xFEFF	unicode (big endian)
			if ((((int)Temp & 0xFFFF) != LITTLE_ENDIAN) && 
				(((int)Temp & 0xFFFF) != BIG_ENDIAN))
				goto ErrorReturn;
			
			// we convert for read value.
			m_nFileType = (unsigned short)READ_ORDER_SWAP(m_bLittleEndian, (unsigned short)Temp);
		}
		
	}
	else
	{
		if (modeASCII == filetype)
			m_nFileType = ASCII_FILE;
		else
		{
			// we convert for read value.
			if (modeBigUNICODE == filetype)
				m_nFileType = BIG_ENDIAN;
			else
				m_nFileType = LITTLE_ENDIAN;

			Temp = m_nFileType;
			Temp = (WCHAR)WRITE_ORDER_SWAP(m_bLittleEndian, (unsigned short)Temp);

			// write Unicode Header..
			int nNumWrite = fwrite((void*)&Temp, sizeof(Temp), 1, m_pStream);
		}
	}

	return TRUE;

ErrorReturn:
	Close();
	return FALSE;
}

void CReadiniFile::Close(void)
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
BOOL CReadiniFile::ReadString(BOOL bAscii)
{
	unsigned char *pText;
	unsigned short	Value;
	unsigned char	Temp;
	int		i;

	///////////////////////////////////////////////////////////////////
	//	Read File
	///////////////////////////////////////////////////////////////////

	// read unicode file
	if (m_nFileType)
	{
		int	nNumWrite = 0;

		memset((void*)m_LineBuffer, 0, MAX_LINE_BUFFER+1);

		if (NULL == fgets((char*)m_LineBuffer, MAX_LINE_BUFFER, m_pStream))
			return FALSE;

		// must read by 2byte.
		if (m_nFileType == LITTLE_ENDIAN)
			nNumWrite = fread(&Temp, sizeof(Temp), sizeof(Temp), m_pStream);
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

	// read ascii file
	if (m_nFileType)
	{
		// swap byte order. convert '\n'->' ', '\r'->' '
		for(i = 0; i < MAX_LINE_BUFFER ; i++)
		{
			pText = (unsigned char*)&m_LineBuffer[i];
			Value = (unsigned short)m_LineBuffer[i];
			
			// little system -> swap big endian file type
			if ((m_bLittleEndian && m_nFileType == BIG_ENDIAN) || 
				(!m_bLittleEndian && m_nFileType == LITTLE_ENDIAN))
			{
				pText[0] = (unsigned char)((Value & 0xff00) >> 8);
				pText[1] = (unsigned char)(Value & 0xff);
			}
			
			//convert '\n'->' ', '\r'->' '
			if (pText[0] == '\n' || pText[0] == '\r')
				pText[0] = ' ';
			if (pText[1] == '\n' || pText[1] == '\r')
				pText[1] = ' ';
			
			// is null?, end swap order
			if(	(pText[0] == 0 && pText[1] == 0))
				break;
		}
	}
	
	return TRUE;
}

PINIVALUE CReadiniFile::ReadiniValue(void)
{
	TCHAR	chOld = (TCHAR)0x8;
	TCHAR	chNew = '\n';
	CString	strOneLine;
	int		nSearchIndex, nFirstIndex, nLastIndex;

	// don't open file
	if (!m_pStream)
		return FALSE;

	m_INIVALUE.nValue = -1;

	// Read One Line
	while(1)
	{
		if (!ReadString(TRUE))
			break;
		
		if (m_nFileType)
			strOneLine = (WCHAR*)m_LineBuffer;
		else
			strOneLine = m_LineBuffer;


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
					m_INIVALUE.Values[0] = _T("0");
				else
					m_INIVALUE.Values[0] = _T("1");

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
				m_INIVALUE.Key.MakeUpper();
				
				if (m_INIVALUE.nValue == -1)
					m_INIVALUE.nValue++;
				
				// trim
				strOneLine = strOneLine.Mid(nFirstIndex+1, (strOneLine.GetLength()-nFirstIndex+1));
				strOneLine.TrimLeft(_T("\t \n"));
				
				if (strOneLine.GetLength() != 0)
				{
					// SetValue
					nSearchIndex = strOneLine.Find(_T("\""));
					if (nSearchIndex != -1)
					{
						nFirstIndex = nLastIndex = -1;
						nFirstIndex = nSearchIndex;
						
						nLastIndex = strOneLine.Find(_T("\""), nFirstIndex+1);
						
						if (nFirstIndex != -1 && nLastIndex != -1 && (nLastIndex - nFirstIndex -1) >= 0)
						{
							m_INIVALUE.Values[m_INIVALUE.nValue] = strOneLine.Mid(nFirstIndex+1, (nLastIndex - nFirstIndex -1));
							m_INIVALUE.nValue++;
						}
					}
					else
					{
						nFirstIndex = 0;
						nSearchIndex = strOneLine.FindOneOf(_T(", \t\n"));
						while(nSearchIndex != -1)
						{
							if (nSearchIndex == 0 || m_INIVALUE.nValue >= MAX_VALUE_COUNT)
								break;
							
							// end
							if (strOneLine.GetLength() == 0)
								break;

							// comment
							if (strOneLine.GetAt(0) == ';' || strOneLine.GetAt(0) == '#')
								break;
							
							m_INIVALUE.Values[m_INIVALUE.nValue] = strOneLine.Left(nSearchIndex);
							m_INIVALUE.nValue++;
							
							// trim
							strOneLine = strOneLine.Mid(nSearchIndex+1, (strOneLine.GetLength()-nSearchIndex+1));
							strOneLine.TrimLeft(_T("\t \n"));
							
							// next
							nSearchIndex = strOneLine.FindOneOf(_T(", \t\n"));
						}
						
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
