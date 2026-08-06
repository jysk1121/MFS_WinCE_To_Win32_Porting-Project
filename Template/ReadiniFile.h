
#ifndef __READ_INI_FILE_H__
#define __READ_INI_FILE_H__

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

#ifndef ushort
typedef unsigned short ushort;
#endif

// buffer define
#define MAX_STRING_LEN		500//100
#define MAX_VALUE_COUNT		10
#define MAX_LINE_BUFFER		2048//(255)

// system endian define.
#define LITTLE_ENDIAN		(0xFFFE)
#define BIG_ENDIAN			(0xFEFF)

#define ASCII_FILE			(0)
#define UNICODE_FILE		(1)	// unicode..

// ini info string define.
typedef struct tag_INIVALUE
{
	CString		Section;					// Section 값
	CString		Key;						// Key 값
	CString		Values[MAX_VALUE_COUNT];	// Value 배열
	int			nValue;						// Value 개수
}INIVALUE, *PINIVALUE;

// ini operation class.
class CReadiniFile
{
public:
	CReadiniFile();
	~CReadiniFile();

	///////////////////////////////////////
	/* member variable */
	///////////////////////////////////////	
public:
	// mode flag.
	enum OpModeFlags
	{
		//op mode
		modeRead = 0x0,
		modeWrite = 0x1,

		// file type
		modeASCII = 0x0,
		modeBigUNICODE = 0x1,
		modeLittleUNICODE = 0x2
	};

public:
	/* file stream */
	FILE			*m_pStream;
	BOOL			m_bLittleEndian;	// 0 - big endian, 1 - litte endian
	ushort			m_nFileType;		// 0 - ASCII, 0xFFFE - big endian, 0xFEFF - little endian

	/* file read Buffer */	
	char			m_LineBuffer[MAX_LINE_BUFFER+1];		// for parser

	/* ini Info. */
	INIVALUE	m_INIVALUE;

	///////////////////////////////////////
	/* member function*/
	///////////////////////////////////////

public:
	/* Open File */
	BOOL	Open(LPCTSTR lpszFileName, OpModeFlags opmode, OpModeFlags filetype);
	void	Close(void);

	/* get ini value */
	PINIVALUE	ReadiniValue(void);

private:
	BOOL ReadString(BOOL bAscii);
};


#endif __READ_INI_FILE_H__