#pragma once

#ifdef __WIN32_
//#define	LOGMSG(code, msg)			TRACE(msg. ...)
#define	LOGMSG(code, msg)			TRACE(msg)
#else
#define LOGMSG(code, msg)			RETAILMSG(0, (msg))
#endif

class CUtil
{
public:
	CUtil(void);
	virtual ~CUtil(void);

//	void LOGMSG(LPCTSTR msg); 
};
