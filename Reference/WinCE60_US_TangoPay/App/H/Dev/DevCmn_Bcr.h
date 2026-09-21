#ifndef __DEV_CMN_BCR_H__
#define __DEV_CMN_BCR_H__

/////////////////////////////////////////////////////////////////////////////
//	SCANNER PRINTER FUNCTION(BCR)
/////////////////////////////////////////////////////////////////////////////
// BCR SET MODE
int		fnBCR_ClearErrorCode();

// BCR GET MODE
CString	fstrBCR_GetErrorCode();
int		fnBCR_GetDeviceStatus();

// BCR OPERATION MODE
int		fnBCR_Initialize();
int		fnBCR_Deinitialize();
int		fnBCR_SendRawData(BOOL bWait = FALSE);
int		fnBCR_CancelAccept();
int		fnBCR_Reset();
CString	fstrBCR_GetBarcodeData();

#endif __DEV_CMN_BCR_H__