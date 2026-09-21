#pragma once

#ifdef WIN32
//#define	LOGMSG(code, msg)			TRACE(msg. ...)
#define	LOGMSG(code, msg)			TRACE(msg)
#else
#define LOGMSG(code, msg)			RETAILMSG(code, (msg))
#endif

class CUtil
{
public:
	CUtil(void);
	virtual ~CUtil(void);

//	void LOGMSG(LPCTSTR msg); 
};
