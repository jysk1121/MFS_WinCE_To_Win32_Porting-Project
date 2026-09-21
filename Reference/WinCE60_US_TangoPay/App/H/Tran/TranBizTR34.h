#ifndef __TRAN_BIZ_TR34_H__
#define __TRAN_BIZ_TR34_H__

#define GET_PHASE_STRING(NUM)	\
	((NUM == 0) ? (_T("UNINIT")) :	\
	((NUM == 1) ? (_T("BIND")) :	\
	((NUM == 2) ? (_T("TRANSPORT")) :	\
	((NUM == 3) ? (_T("VERIFICATION")):	\
	((NUM == 4) ? (_T("REBIND")) :	\
	((NUM == 5) ? (_T("UNBIND")) :	(_T("UNKNOWN")) ))))))

struct TR34
{
	struct Phase {
		enum Value {
			UNINIT,
			BIND,
			TRANSPORT,
			VERIFICATION,
			REBIND,
			UNBIND
		};
	};

	struct ErrType {
		enum Value {
			SUCCESSFUL = 0,

			MISMATCH_RECORDFORMAT = 10,
			MISMATCH_APPTYPE,
			MISMATCH_BANKID,
			MISMATCH_TERMINALID,
			MISMATCH_REQRESTYPE,

			NOT_APPROVED_BIND,
			NOT_APPROVED_TRANSPORT,
			NOT_APPROVED_VERIFICATION,
			NOT_APPROVED_UNBIND
		};
	};

	struct ResCode {
		enum Value {
			SUCCESSFUL = 0,

			ATM_CTEPP_ERR		= 901,
			ATM_KEYEXCH_ERR		= 911,
			ATM_RTEPP_ERR		= 912,
			ATM_KBPK_LOAD_ERR	= 921,
			ATM_KCV_ERR			= 922,

			HOST_CTEPP_ERR		= 951,
			HOST_RTEPP_ERR		= 961,
			HOST_KCV_ERR		= 971,

			UNKNOWN				= 999
		};
	};

	struct GetCertOpt {
		static const CString PublicEncKey()				{	return _T("PUBLICENCKEY");			}
		static const CString PublicHostKey()			{	return _T("PUBLICHOSTKEY");			}
		static const CString PublicVerificationKey()	{	return _T("PUBLICVERIFICATIONKEY");	}
	};

	struct StartAuthOpt {
		static const CString InitializeEDMEx()			{	return _T("InitializeEDMEx");		}
	};
};



#define TR34_RNMODE_ASN1_FLAG				0x80
#define TR34_RNMODE_NORMAL					16
#define TR34_RNMODE_ASN1					34

#define TR34_RN_ASN1_HEADER_LENGTH			18
#define TR34_RN_DEFAULT_LENGTH				16

										//    12345678901234567890123456789012345678901234567890123456789012345678
#define TR34_KCV_EMPTY						L"      "	// 6 bytes
#define TR34_RTEPP_EMPTY					L"                                                                    "		// 68 bytes

// [#RWC6-151] US Kook 2020.07.14 TR34
typedef	struct tagSTD1TR34_Header
{
	CString RecordFormat;
	CString ApplicationType;
	CString MessageDelimiter;
	CString	BankID;
	CString FS1;
	CString TerminalID;
	CString FS2;

	CString GetStr()
	{
		RecordFormat = L"H";
		ApplicationType = L"0";
		MessageDelimiter = L".";
		BankID.Format(L"%-6.6s", MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_ROUTINGID));
		FS1 = FIELD_DELIMITER;
		TerminalID.Format(L"%-8.8s", MemGetStr(_MEM_FLD_APP_ATMINFO, _MEM_VAR_APP_TERMINALID));
		FS2 = FIELD_DELIMITER;

		return RecordFormat + ApplicationType + MessageDelimiter + BankID + FS1 + TerminalID + FS2;
	}
	void Clear()
	{
		RecordFormat = ApplicationType = MessageDelimiter = BankID = FS1 = TerminalID = FS2 = L"";
	}
}STD1TR34_Header;

// Bind
typedef	struct tagSTD1TR34_Bind_Request
{
	CString RequestType;
	CString FS3;
	CString CTepp;

	CString GetStr()
	{
		RequestType = L"F0";
		FS3 = FIELD_DELIMITER;

		return RequestType + FS3 + CTepp;
	}
}STD1TR34_BindReq;

typedef	struct tagSTD1TR34_Bind_Response
{
	CString ResponseType;
	CString FS3;
	CString ResponseCode;
	CString FS4;
	CString CThsm;
}STD1TR34_BindResp;

// Transport
typedef	struct tagSTD1TR34_Transport_Request
{
	CString RequestType;
	CString FS3;
	CString ResponseCode;
	CString FS4;
	CString RTepp;

	CString GetStr()
	{
		RequestType = L"F1";
		FS3 = FIELD_DELIMITER;
		FS4 = FIELD_DELIMITER;

		return RequestType + FS3 + ResponseCode + FS4 + RTepp;
	}
}STD1TR34_TransportReq;

typedef	struct tagSTD1TR34_Transport_Response
{
	CString ResponseType;
	CString FS3;
	CString KeyName;
	CString FS4;
	CString ResponseCode;
	CString FS5;
	CString KThsm;
}STD1TR34_TransportResp;

// Verification
typedef	struct tagSTD1TR34_Verification_Request
{
	CString RequestType;
	CString FS3;
	CString KeyName;
	CString FS4;
	CString ResponseCode;
	CString FS5;
	CString KCV;

	CString GetStr()
	{
		RequestType = L"F2";
		FS3 = FIELD_DELIMITER;
		FS4 = FIELD_DELIMITER;
		FS5 = FIELD_DELIMITER;

		return RequestType + FS3 + KeyName + FS4 + ResponseCode + FS5 + KCV.Left(6);
	}
}STD1TR34_VerificationReq;

typedef	struct tagSTD1RKT_TR34_Verification_Response
{
	CString ResponseType;
	CString FS3;
	CString ResponseCode;
}STD1TR34_VerificationResp;

// Unbind (same as Transport)
typedef	struct tagSTD1TR34_Unbind_Request
{
	CString RequestType;
	CString FS3;
	CString ResponseCode;
	CString FS4;
	CString RTepp;

	CString GetStr()
	{
		RequestType = L"F3";
		FS3 = FIELD_DELIMITER;
		FS4 = FIELD_DELIMITER;

		return RequestType + FS3 + ResponseCode + FS4 + RTepp;
	}
}STD1TR34_UnbindReq;

typedef	struct tagSTD1TR34_Unbind_Response
{
	CString ResponseType;
	CString FS3;
	CString ResponseCode;
	CString FS4;
	CString UBThsm;
}STD1TR34_UnbindResp;




private:
	STD1TR34_Header					TR34_HeaderReq, TR34_HeaderResp;

	STD1TR34_BindReq				TR34_BindReq;
	STD1TR34_BindResp				TR34_BindResp;
	STD1TR34_TransportReq			TR34_TransportReq;
	STD1TR34_TransportResp			TR34_TransportResp;
	STD1TR34_VerificationReq		TR34_VerificationReq;
	STD1TR34_VerificationResp		TR34_VerificationResp;
	STD1TR34_UnbindReq				TR34_UnbindReq;
	STD1TR34_UnbindResp				TR34_UnbindResp;

	TR34::Phase::Value				TR34_curPhase;
	int								TR34_curRNLength;

protected:
// TR34 phases
	int			TR34_InitializePhase(TR34::Phase::Value phase);
	int			TR34_Process_Preparation();
	int			TR34_Process_Bind();
	int			TR34_Process_Transport();
	int			TR34_Process_Verification();
	int			TR34_Process_Unbind();


// message procedures
	int			TR34_BuildMessage();
	int			TR34_SendMessage();
	int			TR34_RecvMessage();
	int			TR34_ParseMessage();
	int			TR34_ValidateMessage();


// utilities
	int			TR34_GetRNMode();
	int			TR34_WriteJournal(TR34::ErrType::Value eCode);
	CString		TR34_Base64Encode(CString hexData);
	CString		TR34_Base64Decode(CString base64Data);
	void		TR34_SetResCode(TR34::ResCode::Value resCode);
	int			TR34_GetResCode();
	void		TR34_ShowProcessingScreen();
	bool		TR34_IsEppBound();

public:
	BIZ_RETURN	P_BIZ_TR34_Bind_Proc(BOOL bShow);
	BIZ_RETURN	P_BIZ_TR34_Transport_Proc(BOOL bShow);
	BIZ_RETURN	P_BIZ_TR34_Unbind_Proc(BOOL bShow);


#endif
