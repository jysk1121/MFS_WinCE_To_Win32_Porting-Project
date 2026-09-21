#ifndef __TRAN_BIZ_RKT_H__
#define __TRAN_BIZ_RKT_H__

// [#2075] NH KSK 2011.06.27
typedef	struct tagSTD1RKT_Authentication_Request 
{
	CString RecordFormat;
	CString ApplicationType;
	CString MessageDelimiter;
	CString	BankID;
	CString TerminalID;
	CString RequestType;
	CString EPP_SerialNumber;
	CString EPP_Sign_SK;
	CString EPP_PublicKey;
	CString EPP_Sign_PK;
}STD1RKT_AuthReq;

typedef	struct tagSTD1RKT_TMK_Request 
{
	CString RecordFormat;
	CString ApplicationType;
	CString MessageDelimiter;
	CString	BankID;
	CString TerminalID;
	CString RequestType;
	CString ResponseCode;
	CString KeyType;
	CString EPP_RSACrypt_Repp;
}STD1RKT_TMKReq;

typedef	struct tagSTD1RKT_TMK_Result_Request 
{
	CString RecordFormat;
	CString ApplicationType;
	CString MessageDelimiter;
	CString	BankID;
	CString TerminalID;
	CString RequestType;
	CString ResponseCode;
	CString EPP_KCV;
}STD1RKT_TMKResultReq;

typedef	struct tagSTD1RKT_Authentication_Resp
{
	CString RecordFormat;
	CString ApplicationType;
	CString MessageDelimiter;
	CString	BankID;
	CString TerminalID;
	CString ResponseType;
	CString ResponseCode;
	CString HSM_PublicKey;
	CString HSM_Sign_PK;
}STD1RKT_AuthResp;

typedef	struct tagSTD1RKT_TMK_Resp 
{
	CString RecordFormat;
	CString ApplicationType;
	CString MessageDelimiter;
	CString	BankID;
	CString TerminalID;
	CString ResponseType;
	CString ResponseCode;
	CString KeyType;
	CString HSM_RSACrypt_TMK;
	CString HSM_Sign_TMK;
	CString HSM_KCV;
}STD1RKT_TMKResp;

typedef	struct tagSTD1RKT_TMK_Result_Resp
{
	CString RecordFormat;
	CString ApplicationType;
	CString MessageDelimiter;
	CString	BankID;
	CString TerminalID;
	CString ResponseType;
	CString ResponseCode;
}STD1RKT_TMKResultResp;

public:
	STD1RKT_AuthReq			RKT_AuthReq;
	STD1RKT_TMKReq			RKT_TMKReq;
	STD1RKT_TMKResultReq	RKT_TMKResultReq;

	STD1RKT_AuthResp		RKT_AuthResp;
	STD1RKT_TMKResp			RKT_TMKResp;
	STD1RKT_TMKResultResp	RKT_TMKResultResp;

public:
	// RKT Proc
	BIZ_RETURN	P_BIZ_RKT_Proc(int nKeyType, BOOL bShow = TRUE);

public:
	int			RKT_Process_R0();
	int			RKT_Process_R1(int nRKTType);
	int			RKT_Process_R2();

#endif 
// end of [#2075]