#ifndef __EMV_L2_KERNEL_2010_ISO_8583_H__
#define __EMV_L2_KERNEL_2010_ISO_8583_H__

#pragma comment(lib, "ISO8583ce.lib")


//------------------------ Description about Extern Functions -----------------------//
#define DLL_IMPORT		__declspec(dllimport)
//-------------------------------------------------------------------

DLL_IMPORT int		EMVIso_GetData(int bit, int tid, BYTE *ptr);
DLL_IMPORT short	EMVIso_BuildISO8583Msg(int app, char *msgType, BYTE *data);
DLL_IMPORT void		EMVIso_SetAtmData(int iTransTypeLen, char *cpTransType,
									  int iAdditionalDataLen, char *cpAdditionalData,
									  int iPriAccountLen, char *cpPriAccount,
									  int iTransAccountLen, char *cpTransAccount);
DLL_IMPORT int		EMVIso_SetRecvMsg(int iApp, int iLen, char *cpRecvHostMsg);
DLL_IMPORT WORD		GetCRC(BYTE *data, int Length);
DLL_IMPORT short	EMVIso_ReversalMsg(int app, BYTE *data);
DLL_IMPORT short	EMVIso_MsrMsg(int app, 
							   int iPanLen, char* cpPan, 
							   int iAmtLen, char* cpAmt, 
							   BYTE *data);


#endif __EMV_L2_KERNEL_2010_ISO_8583_H__
