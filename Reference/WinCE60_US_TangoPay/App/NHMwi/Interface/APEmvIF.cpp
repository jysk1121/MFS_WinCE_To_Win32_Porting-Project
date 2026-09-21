// APEMVIF.cpp

#include "stdafx.h"
#include "..\NHMwi.h"
//#define NH_DEBUG
#include "..\NHMwiCtl.h"
#include "..\NHMwiPpg.h"
#include ".\Common\NHDbgApi.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// Method : Called From AP
///////////////////////////////////////////////////////////////////////////////////////////////////
BSTR CNHMWICtrl::EMV_SendRecvMsgWithIFM(long ProtocolID, LPCTSTR szSendData, long nWaitSec)
{
	CString strResult = EMVINF_SendRecvMsgWithIFM(ProtocolID, szSendData, nWaitSec);
	
	return strResult.AllocSysString();
}
