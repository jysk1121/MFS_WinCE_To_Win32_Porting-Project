#include "StdAfx.h"
#include "NFileIO.h"

CNFileIO::CNFileIO()
{
	m_pStream = NULL;
}

CNFileIO::~CNFileIO()
{
	Close();
}

BOOL CNFileIO::Open(LPCTSTR lpszFileName)
{
	Close();

	// file Open
	m_pStream = _wfopen(lpszFileName, _T("rt"));

	if (!m_pStream)
		return FALSE;
	
	return TRUE;
}

void CNFileIO::Close()
{
	if (m_pStream)
		fclose(m_pStream);
	
	m_pStream = NULL;
}

BOOL CNFileIO::ReadString(CString &strOneLine)
{
	int		i;
	char	*pText;
	
	///////////////////////////////////////////////////////////////////
	//	Read File
	///////////////////////////////////////////////////////////////////
	
	memset((void*)m_LineBuffer, 0, MAX_LINE_BUFFER+1);
	
	if (NULL == fgets((char*)m_LineBuffer, MAX_LINE_BUFFER, m_pStream))
		return FALSE;
	
	pText = (char*)&m_LineBuffer[0];
	for(i = 0; i < MAX_LINE_BUFFER ; i++)
	{
		if (pText[0] == 0x0D || pText[0] == 0x0A)
			pText[0] = ' ';
		
		pText++;
		
		// is null?
		if (pText[0] == 0)
			break;
	}
	
	strOneLine = m_LineBuffer;

	return TRUE;
}