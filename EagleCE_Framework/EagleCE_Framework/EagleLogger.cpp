#include "stdafx.h"
#include "EagleLogger.h"
#include "Util.h"
#include "CeFileFind.h"
#include <fstream>
#include <time.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CEagleLogger* CEagleLogger::m_pInstance = NULL;


/** **********************************************************
*	@brief		Constructor
*	@retval		Nothing
************************************************************/
CEagleLogger::CEagleLogger(LogLevel level, int logItems)
	: CLogger(level, logItems)
{
	memset(m_szLogPrefix, 0, sizeof(m_szLogPrefix));

	memset(m_szCurrentLogFile, 0, sizeof(m_szCurrentLogFile));

	memset(&m_tmCurrentLogTime, 0, sizeof(m_tmCurrentLogTime));

	//m_nCurrentLogIndex = 0;

	m_nMaxLogDays = 0;

	m_nMaxLogSize = 0;
}


/** **********************************************************
*	@brief		Destructor
*	@retval		Nothing
************************************************************/
CEagleLogger::~CEagleLogger()
{
}


/** **********************************************************
*	@brief		Create Instance
*	@retval		Nothing
************************************************************/
void CEagleLogger::CreateInstance()
{
	m_pInstance = new CEagleLogger(Info,
		static_cast<int>(Function)
		| static_cast<int>(LineNumber)
		| static_cast<int>(DateTime)
		| static_cast<int>(Level));
}


/** **********************************************************
*	@brief		Get Instance
*	@retval		Nothing
************************************************************/
CEagleLogger* CEagleLogger::GetInstance()
{
	return m_pInstance;
}


/** **********************************************************
*	@brief		Release Instance
*	@retval		Nothing
************************************************************/
void CEagleLogger::ReleaseInstance()
{
	if (m_pInstance)
	{
		delete m_pInstance;

		m_pInstance = NULL;
	}
}



/** **********************************************************
*	@brief		Initialize
*	@retval		Nothing
************************************************************/
void CEagleLogger::Initialize(LPCTSTR lpszLogPrefix, UINT nMaxLogDays, UINT nMaxLogSize)
{
	// Set parameter
	// The 2nd argument of _tcscpy_s is a CHARACTER count, not a byte count.
	// sizeof() reports 8192 for a TCHAR[4096] buffer, so the guard is twice the
	// real capacity. Corrected for Win32 only; the WinCE path is left as-is.
#ifdef _WIN32_WCE
	_tcscpy_s(m_szLogPrefix, sizeof(m_szLogPrefix), lpszLogPrefix);
#else
	_tcscpy_s(m_szLogPrefix, _countof(m_szLogPrefix), lpszLogPrefix);
#endif

	m_nMaxLogDays = nMaxLogDays;

	m_nMaxLogSize = nMaxLogSize;

	// Create log folder
	CreateDirectory(PATH_LOG_ABS, NULL);

	GetLocalTime(&m_tmCurrentLogTime);

	// Delete old files
	//DeleteOldFiles();

	// Get current log file
	GetCurrentLogFile();


	// Add logger
	AddOutputStream(new std::tofstream(m_szCurrentLogFile), true, Info);
}


/** **********************************************************
*	@brief		Finalize
*	@retval		Nothing
************************************************************/
void CEagleLogger::Finalize(void)
{
	// Clear logger
	ClearOutputStreams();
}


/** **********************************************************
*	@brief		Log
*	@retval		Nothing
************************************************************/
//void CEagleLogger::Log(LogLevel level, LPCTSTR file, INT line, LPCTSTR func, LPCTSTR format, ...)
void CEagleLogger::Log(LogLevel level, LPCTSTR format, ...)
{
	// 날짜가 바뀌어도 file을 바꾸지않고 계속 logging하도록 변경
//	SYSTEMTIME SystemTime;
//	GetLocalTime(&SystemTime);

	// Check the date is changed
//	if (m_tmCurrentLogTime.wYear== SystemTime.wYear
//		&& m_tmCurrentLogTime.wMonth == SystemTime.wMonth
//		&& m_tmCurrentLogTime.wDay == SystemTime.wDay)
//	{
		// Check the log file size has exceeded the maximum size
		
		//struct _stat st;
		//_tstat(m_szCurrentLogFile, &st);

		//FILE *f;
		//f = _tfopen(m_szCurrentLogFile, _T("r"));
		//fseek(f, 0, SEEK_END);
		//int nSize = ftell(f);

		//if (nSize > (int)m_nMaxLogSize)
		//{
		//	// Increase log index 
		//	//m_nCurrentLogIndex++;


		//	// Get current log file
		//	GetCurrentLogFile();


		//	// Clear logger
		//	CLogger::ClearOutputStreams();

		//	// Add logger
		//	AddOutputStream(new std::tofstream(m_szCurrentLogFile), true, Info);
		//}
//	}

	// The date is changed
	//else
	//{
	//	// Get current log time
	//	memcpy(&m_tmCurrentLogTime, &SystemTime, sizeof(SystemTime));

	//	// Initialize log index
	//	//m_nCurrentLogIndex = 0;


	//	// Delete old files
	//	//DeleteOldFiles();

	//	// Get Current Log File
	//	GetCurrentLogFile();


	//	// Clear logger
	//	CLogger::ClearOutputStreams();

	//	// Add logger
	//	AddOutputStream(new std::tofstream(m_szCurrentLogFile), true, Info);


	//}

	// Log
	va_list args;

	va_start(args, format);

	int length = _vsctprintf(format, args ) + 1;

	TCHAR* text =  new TCHAR[length];

	_vstprintf_s(text, length, format, args);

	va_end(args);


	//CLogger::Log(level, file, line, func, text);
	CLogger::Log(level, text);


	delete [] text;

}


/** **********************************************************
*	@brief		Get Current Log File
*	@retval		Nothing
************************************************************/
void CEagleLogger::GetCurrentLogFile(void)
{
	CString strCurrentLogFile;

	strCurrentLogFile.Format(_T("%s\\%s_%04d_%02d_%02d_%02d_%02d_%02d.log"), 
		PATH_LOG_ABS,
		m_szLogPrefix,
		m_tmCurrentLogTime.wYear,
		m_tmCurrentLogTime.wMonth,
		m_tmCurrentLogTime.wDay,
		m_tmCurrentLogTime.wHour,
		m_tmCurrentLogTime.wMinute,
		m_tmCurrentLogTime.wSecond);

	_tcscpy_s(m_szCurrentLogFile, 256, strCurrentLogFile);
}


/** **********************************************************
*	@brief		Delete Old Files
*	@retval		Nothing
************************************************************/
//void CEagleLogger::DeleteOldFiles(void)
//{
//	// Get current time
//	//time_t tCurrentTime;
//	//time(&tCurrentTime);
//
//	//tm tmCurrentTime;
//	//localtime_s(&tmCurrentTime, &tCurrentTime);
//
//	SYSTEMTIME tmCurrentTime;
//	GetLocalTime(&tmCurrentTime);
//
//
//	// Set find file string
//	CString strFindFile;
//	strFindFile.Format(_T("%s\\*.*"), PATH_LOG);		// 위치 변경
//
//	// Find file
//	CCeFileFind finder;
//	BOOL bFind = finder.FindFile(strFindFile);
//	while (bFind)
//	{
//		bFind = finder.FindNextFile();
//
//		if (finder.IsDots())
//			continue;
//
//		// Get log date from log file
//		CString strLogFile = finder.GetFileName();
//		CString strLogDate = _T("");
//		
//		int nTokenPos = 0;
//		CString strToken;
//		do
//		{
//			strToken = strLogFile.Tokenize(_T("_"), nTokenPos);
//			if (strToken.SpanIncluding(_T("0123456789")) == strToken
//				&& strToken.GetLength() == 8)
//			{
//				strLogDate = strToken;
//				break;
//			}
//		}
//		while (!strToken.IsEmpty());
//
//		if (strLogDate.IsEmpty())
//			continue;
//
//		SYSTEMTIME tmLogDate;
//		memset(&tmLogDate, 0, sizeof(tmLogDate));
//		tmLogDate.wYear = _ttoi(strLogDate.Left(4)) - 1900;
//		tmLogDate.wMonth = _ttoi(strLogDate.Mid(4, 2)) - 1;
//		tmLogDate.wDay = _ttoi(strLogDate.Mid(6, 2));
//
//		//time_t tLogDate = mktime(&tmLogDate);
//
//		// Get current date
//		SYSTEMTIME tmCurrentDate;
//		memset(&tmCurrentDate, 0, sizeof(tmCurrentDate));
//		tmCurrentDate.wYear = tmCurrentTime.wYear;
//		tmCurrentDate.wMonth = tmCurrentTime.wMonth;
//		tmCurrentDate.wDay = tmCurrentTime.wDay;
//		
//		//time_t tCurrentDate = mktime(&tmCurrentDate);
//
//		// Calculating the difference between two dates
//		int nDiffDays = 0/*(int)difftime(tmCurrentDate, tmLogDate) / 3600 / 24*/;
//
//		// Check the log file date has exceeded the maximum days
//		if (nDiffDays > (int)m_nMaxLogDays)
//			DeleteFile(finder.GetFilePath());
//	}
//}
