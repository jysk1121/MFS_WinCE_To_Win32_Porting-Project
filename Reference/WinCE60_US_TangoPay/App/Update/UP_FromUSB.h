#ifndef __UP_INIT_H__
#define __UP_INIT_H__

#include "FileMgr.h"

//////////////////////////////////////////////////////////////////////////
//	구동에 필요한 준비작업을 수행 한다.

enum USB_WAIT_RESULT 
{
	UPDATE_USB,
	UPDATE_SD,
	UPDATE_TIMEOUT,
	UPDATE_EXIT
};

class CUP_FromUSB : public CFileMgr
{
public:
	CUP_FromUSB();
	~CUP_FromUSB();

private:
	USB_WAIT_RESULT	WaitForSDorUSBPlugin(CString strSDPath, CString strUSBPath);
	BOOL			CheckFormUSB(CUpdateStateInfo &StateInfo, BOOL useUsb);

	// Method define
public:
	CString Process(CUpdateStateInfo &StateInfo);	// State 처리
};

CUP_FromUSB	g_CUP_FromUSB;

#endif // __UP_INIT_H__
