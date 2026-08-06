#pragma once

#pragma warning(disable:4995)
#include <iostream>
#pragma warning(default:4995)
#include <vector>
#include <Windows.h>
#include <time.h>
#include "tstring.h"
#include <tchar.h>

/************************************************************
* Log Level
************************************************************/
typedef enum 
{
	Info = 0,
	Debug,
	Warn,
	Error
} LogLevel;


/************************************************************
* Log Item
************************************************************/
typedef enum 
{
	Filename	= 0x1,
	LineNumber	= 0x2,
	Function    = 0x4,
	DateTime	= 0x8,		
	ThreadId	= 0x10,
	Level		= 0x20
} LogItem;


/************************************************************
* Logger
************************************************************/
class CLogger
{
private:
	struct StreamInfo
	{
		std::tostream* m_stream;
		bool m_owned;
		LogLevel m_level;

		StreamInfo(std::tostream* pStream, bool owned, LogLevel level)
		{
			m_stream = pStream;
			m_owned = owned;
			m_level = level;
		}
	};

public:
	CLogger(LogLevel level, int logItems)
		: m_level(level), m_logtem(logItems)
	{
		InitializeCriticalSection(&m_cs);
	}

	~CLogger()
	{
		DeleteCriticalSection(&m_cs);
	}

	void AddOutputStream(std::tostream* os, bool own)
	{
		AddOutputStream(os, own, m_level);
	}

	void AddOutputStream(std::tostream* os, bool own, LogLevel level)
	{
		EnterCriticalSection(&m_cs);

		StreamInfo si(os, own, level);
		m_outputStreams.push_back(si);

		LeaveCriticalSection(&m_cs);
	}

	void ClearOutputStreams()
	{
		EnterCriticalSection(&m_cs);

		for(std::vector<StreamInfo>::iterator iter = m_outputStreams.begin(); iter < m_outputStreams.end(); iter++)
		{
			if (iter->m_owned)
			{
				delete iter->m_stream;
			}
		}

		m_outputStreams.clear();

		LeaveCriticalSection(&m_cs);
	}

	//void Log(LogLevel level, LPCTSTR file, INT line, LPCTSTR func, LPCTSTR format, ...)
	void Log(LogLevel level, LPCTSTR format, ...)
	{
		EnterCriticalSection(&m_cs);

		for(std::vector<StreamInfo>::iterator iter = m_outputStreams.begin(); iter < m_outputStreams.end(); iter++)
		{
			if(level < iter->m_level)
			{
				continue;
			}

			std::tostream * stream = iter->m_stream;

			if (m_logtem & static_cast<int>(DateTime))
			{
				write_datetime(stream);
			}

			if (m_logtem & static_cast<int>(ThreadId))
			{
				write<int>(stream, GetCurrentThreadId());
			}

			if (m_logtem & static_cast<int>(Level))
			{
				write_loglevel(stream, level);
			}

			//if (m_logtem & static_cast<int>(Function))
			//{
			//	write<LPCTSTR>(stream, func);
			//}

			//if (m_logtem & static_cast<int>(Filename))
			//{
			//	write<LPCTSTR>(stream, file);
			//}

			//if (m_logtem & static_cast<int>(LineNumber))
			//{
			//	write<int>(stream, line);
			//}


			va_list args;

			va_start(args, format);

			int length = _vsctprintf(format, args ) + 1;

			TCHAR* text =  new TCHAR[length];

			_vstprintf_s(text, length, format, args);

			va_end(args);

#ifdef _WIN32_WCE
			RETAILMSG(ACTIVE_DEBUG, (_T("%s\n"), text));
#endif
			write<LPCTSTR>(stream, text);

			(*stream) << std::endl;

			stream->flush();

			stream->clear();

			delete [] text;
		}

		LeaveCriticalSection(&m_cs);
	}

private:
	int m_logtem;
	LogLevel m_level;
	std::vector<StreamInfo> m_outputStreams;
	CRITICAL_SECTION m_cs;

	template <class T> inline bool write(std::tostream* strm, T data)
	{
		(*strm) << _T("[");

		(*strm) << data;

		(*strm) << _T("]");

		return true;
	}

	inline bool write_datetime(std::tostream* strm)
	{
		(*strm) << _T("[");

		SYSTEMTIME st;
		GetLocalTime(&st);

		TCHAR strDate[16] = { _T('\0') };
		TCHAR strTime[32] = { _T('\0') };

		wsprintf(strDate, _T("%04d/%02d/%02d"), st.wYear, st.wMonth, st.wDay);
		//wsprintf(strTime, _T("%02d:%02d:%02d"), st.wHour, st.wMinute, st.wSecond);
		wsprintf(strTime, _T("%02d:%02d:%02d:%03d"), st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

		(*strm) << strDate << _T(" ") << strTime;

		(*strm) << _T("]");

		return true;
	}

	inline bool write_loglevel(std::tostream* strm, LogLevel level)
	{
		(*strm) << _T("[");

		switch (level)
		{
		case Error:
			(*strm) << _T("ERROR  ");
			break;

		case Warn:
			(*strm) << _T("WARNING");
			break;

		case Info:
			(*strm) << _T("INFO   ");
			break;

		case Debug:
			(*strm) << _T("DEBUG  ");
			break;
		}

		(*strm) << _T("]");

		return true;
	}
};

