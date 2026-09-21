#ifndef __NH_DBGAPI_H__
#define __NH_DBGAPI_H__

#if defined(_WIN32_WCE)
	#include ".\Common\CmnLib.h"
#endif

//#define NO_TIMECHECK

#ifndef UNDER_CE					// FOR SCREEN VIEWER / Win32 Debugging
	#include <fstream>
	#include <queue>
	#include "..\DLL\NHSyncObject.h"

	#define LOG_FILE					_T("C:\\MoniPlusCE2_Win32.LOG")
	#define BUF_SIZE					(4096 * 4)		// not sure but multiple of 4K (minimum block size) might fit for NAND write
	#define FLUSH_SIZE					(4096 * 16)		

	static std::queue<CString>			m_qLogMessages;
	static HANDLE						m_hQueueLogger;
	static DWORD						m_nQueueLoggerID;
	static DWORD _stdcall				QueueLoggerThread( void *pParam );
	static CNHCritSec					m_csQueueLogger;
	static HANDLE						m_flushMutex;

	// singleton across DLLs! got from : https://stackoverflow.com/questions/21333935/using-a-singleton-across-dll-boundary/21370524
	struct ISingleton
	{
		virtual void Push_TimeStamp_FuncName(LPCWSTR lpszFmt, ...)=0;
		virtual void Push_LogContent(LPCWSTR lpszFmt, ...)=0;
	};

	struct Singleton : public ISingleton
	{
		//  TimeStamp			         FuncName						 LogContent
		// "[04/02 17:41:26.229] [UIDBG] CScrCtrl::CreateScreenEngine >> Created NH GDI Object"
		virtual void Push_TimeStamp_FuncName(LPCWSTR lpszFmt, ...)
		{
			if (m_hQueueLogger == NULL)
			{
				m_hQueueLogger = CreateThread(NULL,	/* Must be NULL */ 
					0,								/* Stack size */ 
					QueueLoggerThread,				/* Thread Start Routine */ 
					NULL,							/* Thread Parameter */ 
					NULL,							/* Creation Flag */ 
					&m_nQueueLoggerID);				/* Thread ID */ 

				m_flushMutex = CreateMutex(NULL, FALSE, L"NHDbgApi");

				CString strMsg;
				strMsg.Format(L"QueueLogger Created. [%d], [%d]\n", m_nQueueLoggerID, &m_qLogMessages);
				m_qLogMessages.push(strMsg);
			}

			WCHAR tszInfo[BUF_SIZE] = L"";
			va_list args;
			va_start(args, lpszFmt);
			vswprintf_s(tszInfo, BUF_SIZE, lpszFmt, args);
			va_end(args);

			SYSTEMTIME	ti;
			GetLocalTime(&ti);

			CString strDateTime;
			strDateTime.Format(L"[%02d/%02d %02d:%02d:%02d.%03d] %s", ti.wMonth, ti.wDay, ti.wHour, ti.wMinute, ti.wSecond, ti.wMilliseconds, tszInfo);

			// lock the queue
			m_csQueueLogger.Enter();	// this would be released after Push_LogContent is completed
			m_qLogMessages.push(strDateTime);
		}

		//  TimeStamp			         FuncName						 LogContent
		// "[04/02 17:41:26.229] [UIDBG] CScrCtrl::CreateScreenEngine >> Created NH GDI Object"
		virtual void Push_LogContent(LPCWSTR lpszFmt, ...)
		{
			WCHAR tszInfo[BUF_SIZE] = {};
			va_list args;
			va_start(args, lpszFmt);
			vswprintf_s(tszInfo, BUF_SIZE, lpszFmt, args);
			va_end(args);

			CString strLogText(tszInfo);
			m_qLogMessages.push(strLogText);

			// unlock the queue
			m_csQueueLogger.Leave();
		}
	};

	static ISingleton &GetSingleton()
	{
		static Singleton inst;
		return inst;
	}

	static void FlushLog(CString pLogMessages)
	{
		if (WaitForSingleObject(m_flushMutex, 100) != WAIT_OBJECT_0)
		{
			Sleep(10);	// for a breakpoint
		}
		else
		{
			std::ofstream log_file;
			log_file.open(LOG_FILE, std::ios_base::out | std::ios_base::app);
			log_file << CT2A(pLogMessages);
			log_file.close();
		}

		ReleaseMutex(m_flushMutex);
	}

	DWORD _stdcall QueueLoggerThread( void *pParam )
	{
		CString strLogMessage;
		int nStackedCount = 0;

		while (true)
		{
			m_csQueueLogger.Enter();	// synchronize push/pop of vector
			while (!m_qLogMessages.empty())
			{
				strLogMessage += m_qLogMessages.front();
				m_qLogMessages.pop();
				nStackedCount++;
			}
			m_csQueueLogger.Leave();

			if (nStackedCount > 0)
			{
				FlushLog(strLogMessage);
				strLogMessage.Empty();
				nStackedCount = 0;
			}

			Sleep(10);		// less write, less load. what would be sweet spot for ATM?
		}

		return 0;
	}
#endif

//#define NO_TIMECHECK
#ifdef UNDER_CE
	#define CHECK_TIME_START ((void)0)
	#define CHECK_TIME_END ((void)0)
#else
# ifndef NO_TIMECHECK
	#define CHECK_TIME_START NHDBG((L"-> %s\n", TEXT(__FUNCTION__))); __int64 freq, start, end; float __a; BOOL __b; if (QueryPerformanceFrequency((_LARGE_INTEGER*)&freq)) {QueryPerformanceCounter((_LARGE_INTEGER*)&start);
	#define CHECK_TIME_END  QueryPerformanceCounter((_LARGE_INTEGER*)&end); __a=(float)((double)(end - start)/freq*1000); __b=TRUE; } else __b=FALSE; if (__b) NHDBG((L"<- %s (%.2f ms)\n", TEXT(__FUNCTION__), __a));
# else
#  define CHECK_TIME_START ((void)0)
#  define CHECK_TIME_END ((void)0)
# endif
#endif


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
		
	// Logging
	// 1 - ERROR
	// 2 - WARN
	// 3 - INFO
	// 4 - DEBUG
	// 5 - TRACE
	// 6 - UI
	#define NHLOG_ERROR	1
	#define NHLOG_WARN	2
	#define NHLOG_INFO	3
	#define NHLOG_DEBUG	4
	#define NHLOG_TRACE	5
	#define NHLOG_UI	6

#ifdef _DEBUG
#	define NH_LOG_LEVEL NHLOG_DEBUG
#else
#	ifndef NH_LOG_LEVEL
//#		define NH_LOG_LEVEL NHLOG_WARN
#		define NH_LOG_LEVEL NHLOG_DEBUG		// [RWC6-676] test code
#	endif
#endif // _DEBUG

// Prevents the ScreenViewer app (non-unicode) from sending wide chars to the printf function
#ifdef UNDER_CE
#  define PRINTF_FUNC NHDebugPrintf
#else
#  define PRINTF_FUNC TRACE
#endif

#if defined(_WIN32_WCE)
	#define _NHLOG(level, token, printf_exp) \
		((void)((level <= NH_LOG_LEVEL) ? (NHDebugTimeStampPrint(), NKDbgPrintfW(TEXT("[%s] %s >> "), token, TEXT(__FUNCTION__)), PRINTF_FUNC printf_exp), 1 : 0))
#else
	#define _NHLOG(level, token, printf_exp) \
		((void)((level <= NH_LOG_LEVEL) ? (GetSingleton().Push_TimeStamp_FuncName(L"[%s] %s >> ", token, TEXT(__FUNCTION__)), GetSingleton().Push_LogContent printf_exp, PRINTF_FUNC printf_exp), 1 : 0))
#endif

// Turn each level on/off based on the levels set above. This is an optimization
// to remove the branching incurred by the _NHLOG macro.
// LOG COMMANDS
#if (NH_LOG_LEVEL >= NHLOG_ERROR)
	#define NHERROR(printf_exp)			(_NHLOG( NHLOG_ERROR,	TEXT("ERR"),   printf_exp ))
#else
	#define NHERROR(printf_exp) ((void)0)
#endif

#if (NH_LOG_LEVEL >= NHLOG_WARN)
	#define NHWARN(printf_exp)			(_NHLOG( NHLOG_WARN,	TEXT("WARN"),  printf_exp ))
#else
	#define NHWARN(printf_exp) ((void)0)
#endif

#if (NH_LOG_LEVEL >= NHLOG_INFO)
	#define NHINFO(printf_exp)			(_NHLOG( NHLOG_INFO,	TEXT("INFO"),  printf_exp ))
#else
	#define NHINFO(printf_exp) ((void)0)
#endif

#if (NH_LOG_LEVEL >= NHLOG_DEBUG)
	#define NHDEBUG(cond, printf_exp)	(_NHLOG( NHLOG_DEBUG,	TEXT("DEBUG"), printf_exp ))
	#define NHDBG(printf_exp)			(_NHLOG( NHLOG_DEBUG,	TEXT("DEBUG"), printf_exp ))
#else
	#define NHDEBUG(cond, printf_exp) ((void)0)
	#define NHDBG(printf_exp) ((void)0)
#endif

#if (NH_LOG_LEVEL >= NHLOG_TRACE)
	#define NHTRACE(printf_exp)			(_NHLOG( NHLOG_TRACE,	TEXT("TRACE"), printf_exp ))
#else
	#define NHTRACE(printf_exp) ((void)0)
#endif

#if (NH_LOG_LEVEL >= NHLOG_UI)
	#define NHUIDBG(cond, printf_exp)	(_NHLOG( NHLOG_UI,		TEXT("UIDBG"), printf_exp ))
#else
	#define NHUIDBG(cond, printf_exp) ((void)0)
#endif
// ./LOG COMMANDS

#ifdef __cplusplus
}
#endif  // __cplusplus


#endif __NH_DBGAPI_H__
