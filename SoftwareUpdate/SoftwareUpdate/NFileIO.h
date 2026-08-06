#ifndef __NH_FILE_IO_H__
#define __NH_FILE_IO_H__

#define MAX_LINE_BUFFER	2048

class CNFileIO
{
public:
	CNFileIO();
	~CNFileIO();

public:
	BOOL	Open(LPCTSTR lpszFileName);
	void	Close();
	BOOL	ReadString(CString &strOneLine);

private:
	FILE	*m_pStream;
	char	m_LineBuffer[MAX_LINE_BUFFER+1];
};

#endif //__NH_FILE_IO_H__