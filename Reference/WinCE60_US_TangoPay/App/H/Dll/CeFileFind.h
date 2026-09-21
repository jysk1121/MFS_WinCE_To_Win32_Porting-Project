#if !defined _CEFILEFIND_H_
#define _CEFILEFIND_H_

// CeFileFind.h : header file

#include <afxwin.h>

#pragma warning(disable:4251) // [#2032] NH KJW 2011.03.15 AFX_CLASS_EXPORT CNHConfig클래스의 멤버가 템플릿일 경우 나는 warning이며, private으로 내부만 사용시에는 문제없어 disable함.
/////////////////////////////////////////////////////////////////////////////
// CCeFileFind window

// [#2032] NH KJW 2011.03.15 CWnd상속 불필요하여 삭제함
//class AFX_CLASS_EXPORT CCeFileFind : public CWnd
class AFX_CLASS_EXPORT CCeFileFind
// end of [#2032]
{
// Construction
public:
	CCeFileFind( );

public:
// Operations
	void Close();
	virtual BOOL FindNextFile( );
	virtual BOOL FindFile( LPCTSTR pstrName = NULL);

public:
// Attributes

	//Gets the length of the found file, in bytes. 
	DWORD	GetLength() const;
				
	//Gets the name, including the extension, of the found file 
	CString GetFileName() const;				

	//Gets the whole path of the found file. 
	CString GetFilePath() const;			

	//Gets the whole path of the found file. 
	CString GetRoot() const;			

	// to get the time the specified file was created
	virtual BOOL GetCreationTime( FILETIME* pFileTime ) const;
	virtual BOOL GetCreationTime( CTime& refTime ) const;


	//Gets the time that the file was last accessed. 
	virtual BOOL GetLastAccessTime( CTime& refTime ) const;
	virtual BOOL GetLastAccessTime( FILETIME* pFileTime ) const;


	//Gets the time the file was last changed and saved. 
	virtual BOOL GetLastWriteTime( FILETIME* pFileTime ) const;
	virtual BOOL GetLastWriteTime( CTime& refTime ) const;

	//Indicates the desired file attributes of the file to be found. 
	virtual BOOL MatchesMask( DWORD dwMask ) const;
	
	//Determines if the name of the found file has the name "." or "..", 
	//indicating that is actually a directory. 
	virtual BOOL IsDots( ) const;
	
	//Determines if the found file is read-only. 
	BOOL IsReadOnly( ) const;

	//Determines if the found file is a directory. 
	BOOL IsDirectory( ) const;
				
	//Determines if the found file is compressed. 
	BOOL IsCompressed( ) const;

	//Determines if the found file is a system file. 
	BOOL IsSystem( ) const;

	//Determines if the found file is hidden. 
	BOOL IsHidden( ) const;

	//Determines if the found file is temporary. 
	BOOL IsTemporary( ) const;

	//Determines if the found file is normal (in other words, has no other attributes). 
	BOOL IsNormal( ) const;

	//Determines if the found file is archived. 
	BOOL IsArchived( ) const;



// Implementation
public:
	virtual ~CCeFileFind();

private:

	LPWIN32_FIND_DATA m_pfiledata;
	LPWIN32_FIND_DATA m_pNextdata;
	CString	m_csRoot;
	HANDLE m_hFileHandle;

	char	m_chDirSeparator;

	void AssertDoneNext() const;
};

#endif // !defined _CEFILEFIND_H_
