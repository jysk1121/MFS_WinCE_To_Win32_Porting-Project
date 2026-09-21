
#ifndef __NH_READ_INI_FILE_H__
#define __NH_READ_INI_FILE_H__

#include ".\Common\WinCE60Def.h"
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
#define BOM_UCS2_LE				(0xFFFE)
#define BOM_UCS2_BE				(0xFEFF)
#define BOM_UTF8				(0XEFBB)		// actually it's (EF BB BF)
#define BOM_UTF8_REVERSE		(0XBBEF)		// 
#define BOM_UTF8_THIRDBYTE		(0XBF)			// 
#define BOM_NONE				(0x0000)		// used for ASCII

// ini info string define.
typedef struct tag_INIVALUE
{
	CString		Section;					// Section 값
	CString		Key;						// Key 값
	CString		Values[MAX_VALUE_COUNT];	// Value 배열
	int			nValue;						// Value 개수

	void Reset() 
	{
		Key.Empty();
		
		for (int i = 0; i < MAX_VALUE_COUNT; i++)
		{
			Values[i].Empty();
		}

		nValue = -1;
	};
}INIVALUE, *PINIVALUE;

// ini operation class.
class CNHReadiniFile
{
public:
	CNHReadiniFile();
	~CNHReadiniFile();

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
	USHORT			m_wBOMType;			// 0 - ASCII, 0xFFFE - big endian, 0xFEFF - little endian

	/* file read Buffer */	
	char			m_LineBuffer[MAX_LINE_BUFFER+1];		// for parser
	WCHAR			m_LineBufferUnicode[MAX_LINE_BUFFER+1];		// for parser

	/* ini Info. */
	INIVALUE	m_INIVALUE;

	/* string buffer for TAR */
	int					nTokenPos;
	CString				m_iniContent;

	///////////////////////////////////////
	/* member function*/
	///////////////////////////////////////

public:
	/* Open File */
	BOOL	Open(LPCTSTR lpszFileName, OpModeFlags opmode, OpModeFlags filetype);
	BOOL	Open(BYTE* pIniContents, OpModeFlags opmode, OpModeFlags filetype);

	void	Close(void);

	/* get ini value */
	PINIVALUE	ReadiniValue(void);

private:
	BOOL ReadString(BOOL bAscii);
};


#endif __NH_INI_FILE_H__