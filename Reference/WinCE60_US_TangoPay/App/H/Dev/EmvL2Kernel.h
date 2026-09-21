#ifndef __EMV_L2_KERNEL_2010_H__
#define __EMV_L2_KERNEL_2010_H__

#pragma comment(lib, "EmvL2Kernel.lib")


/***************************************************************************
                    SCSp_ATM_EMV_Crypt.h  -  description
                         -------------------
begin                : May 06 2010
copyright            : (C) 2010 by SCSpro
***************************************************************************/


#include "EmvL2KernelDef.h"


//------------------------ Description about Extern Functions -----------------------//
#define DLL_IMPORT		__declspec(dllimport)
//-------------------------------------------------------------------

// EMV Application Selection
DLL_IMPORT int emvkrnl_Sel_ApplicationSelection(unsigned char SupportPSE, EMVst_LIST_OF_AID *AIDList, int *CandidateCnt);
DLL_IMPORT int emvkrnl_Sel_FinalAppSelection(unsigned short AidLen, unsigned char *AIDVal, unsigned char *SW);
DLL_IMPORT int emvkrnl_Sel_MakeDisplayAppList(unsigned char SupportUserConfirm, 
								   unsigned short CandidateCnt, 
								   EMV_ICC_APPS_INFORM *IccAppInform);
DLL_IMPORT void emvkrnl_Sel_RemoveCandidateList(unsigned short CandidateCnt, unsigned short AidLen, unsigned char *AIDVal);

// EMV Transaction
DLL_IMPORT int emvkrnl_Trans_InitApplication(void);
DLL_IMPORT int emvkrnl_Trans_ReadAppData(void);
DLL_IMPORT int emvkrnl_Trans_OfflineDataAuth(EMVst_CA_PUBKEY *stCAPKey);
DLL_IMPORT int emvkrnl_Trans_ProcessRestrict(void);
DLL_IMPORT int emvkrnl_Trans_CardholderVerify(unsigned short OnlinePinLen, unsigned char *EncipheredOnlinePIN);
DLL_IMPORT int emvkrnl_Trans_TerminalRiskMgmt(unsigned char SupportForceOnline);
DLL_IMPORT unsigned char emvkrnl_Trans_TerminalActionAnalysis(void);
DLL_IMPORT int emvkrnl_Trans_CardActionAnalysis(unsigned char *OnlineFlag);
DLL_IMPORT int emvkrnl_Trans_OnlineProcess(int OnlineMode, unsigned char Acquirer_CID);
DLL_IMPORT int emvkrnl_Trans_Completion(void);

DLL_IMPORT int RegistIFMCommFunc(int (CALLBACK* SMCSENDAPDU)(BYTE dCode, BYTE *apdu, UINT apduLen, BYTE *resp,int *rlen));

//-------------------------------------------------------------------
// for Tlv Function
DLL_IMPORT void emvkrnl_Tlv_GetTidTag(int tid, unsigned char *tag);
DLL_IMPORT int  emvkrnl_Tlv_GetTid(unsigned char *tag, int len);
DLL_IMPORT int	emvkrnl_Tlv_GetTag(unsigned char *buf, unsigned char *tag);
DLL_IMPORT int	emvkrnl_Tlv_GetVal(int tid, int *len, unsigned char *val);
DLL_IMPORT int	emvkrnl_Tlv_GetTidLen(int tid);
DLL_IMPORT int	emvkrnl_Tlv_GetLen(unsigned char *buf, int *vlen);
DLL_IMPORT int	emvkrnl_Tlv_StoreVal(int tid, int len, unsigned char *val);
DLL_IMPORT int	emvkrnl_Tlv_StoreValFromRecord(unsigned char *record, int type, int inlen, int source);

//-------------------------------------------------------------------
// EMV Utility
DLL_IMPORT void emvkrnl_Util_BINtoN(int val, unsigned char *data, int len);
DLL_IMPORT void emvkrnl_Util_toYYYYMMDD(unsigned char *data, unsigned char *out, int format);
DLL_IMPORT unsigned char emvkrnl_Util_GetCNRealLen(unsigned char *data, int len);
DLL_IMPORT void emvkrnl_Util_RightJustify(unsigned char *data, int len, unsigned char *dest);
DLL_IMPORT void emvkrnl_Util_Decompress(unsigned char *bySource, unsigned char *byDestination, int byCount);
DLL_IMPORT void emvkrnl_Util_Compress(unsigned char *bySource, unsigned char *byDestination, int byCount);

//-------------------------------------------------------------------
// Other Functions
DLL_IMPORT void emvkrnl_Util_GetAPDUSW(unsigned char *SW12);
DLL_IMPORT void emvkrnl_Util_GetVersion(unsigned char *KernelVer);
DLL_IMPORT void emvkrnl_Sel_DescretionaryDataList(EMVst_ICC_DESCDATA_INFORM *EmvstIccDescInform);
DLL_IMPORT void emvkrnl_Sel_SetOnlyOneAID(unsigned char ucMatchUpYN);

#endif __EMV_L2_KERNEL_2010_H__
