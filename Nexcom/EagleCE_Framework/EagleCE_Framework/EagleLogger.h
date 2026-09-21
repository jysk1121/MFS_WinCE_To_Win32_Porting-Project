#pragma once


#include "Logger.h"

// 로그 Size를 최소화를 우해 FileW / LINE / FUNCTION 정보 제거

#ifdef UNICODE
//#define LOG(level, format, ...) CEagleLogger::GetInstance()->Log(level, __FILEW__, __LINE__, __FUNCTIONW__, format, __VA_ARGS__);
#define LOG(level, format, ...) CEagleLogger::GetInstance()->Log(level, format, __VA_ARGS__);
#else
#define LOG(level, format, ...) CEagleLogger::GetInstance()->Log(level, __FILE__, __LINE__, __FUNCTION__, format, __VA_ARGS__);
#endif


class CEagleLogger : public CLogger
{
protected:
	// Constructor
	CEagleLogger(LogLevel level, int logItems);
	
	// Destructor
	virtual ~CEagleLogger();

	// Instance
	static CEagleLogger* m_pInstance;


public:
	// Create Instance
	static void CreateInstance();

	// Get Instance
	static CEagleLogger* GetInstance();

	// Release Instance
	static void ReleaseInstance();


	// Initialize
	void Initialize(LPCTSTR lpszLogPrefix, UINT nMaxLogDays, UINT nMaxLogSize);

	// Finalize
	void Finalize(void);


	// Log
	//void Log(LogLevel level, LPCTSTR file, INT line, LPCTSTR func, LPCTSTR format, ...);
	void Log(LogLevel level, LPCTSTR format, ...);

	// Get Current Log File
	void GetCurrentLogFile(void);

	// Delete Old Files
	//void DeleteOldFiles(void);


protected:
	// Log Prefix
	TCHAR	m_szLogPrefix[4096];

	// Current Log File
	TCHAR	m_szCurrentLogFile[4096];

	// Current Log Time
	SYSTEMTIME		m_tmCurrentLogTime;

	// Current Log Index
	//UINT	m_nCurrentLogIndex;

	// Max Log Days
	UINT	m_nMaxLogDays;

	// Max Log Size
	UINT	m_nMaxLogSize;
};
