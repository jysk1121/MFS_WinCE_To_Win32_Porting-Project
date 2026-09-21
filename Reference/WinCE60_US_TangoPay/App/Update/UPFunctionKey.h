#ifndef __UP_FUNCTION_KEY_H__
#define __UP_FUNCTION_KEY_H__

#include ".\Scr\ScrCtrl.h"
#include ".\Dll\NHSyncObject.h"
#include ".\Common\NHCtrlThread.h"

class CUPFunctionKey : public CNHCtrlThread
{
public:
	CUPFunctionKey();
	~CUPFunctionKey();

private:
	CScrCtrl*		m_pScrCtrl;
	BOOL			m_bStartKeyCheck;
	CNHEvent		m_oWaitObject;					// Wait Object

public:
	BOOL	Initialize(CScrCtrl *pScrCtrl);
	BOOL	FunctionKeyCheck(BOOL bEnable);

private:
	unsigned ThreadHandlerProc(void);	// 스레드의 Handler.
};

#endif