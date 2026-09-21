#ifndef __TRAN_BIZ_HOST_ALPHI_H__
#define __TRAN_BIZ_HOST_ALPHI_H__

#define ALPHI_HEADER_LENGTH 4
#define ALPHI_MAC_STRING_LENGTH 16
#define ALPHI_MAC_HEX_LENGTH 8

typedef struct tagALPHI_ResponseRedemption
{
	CString	Bit3;				
	BOOL	Bit4_Exist;
	CString	Bit4;
	CString	Bit7;				
	CString	Bit11;			
	CString	Bit12;				
	CString	Bit13;			
	CString	Bit22;				
	CString	Bit32;	
	BOOL	Bit35_Exist;
	CString	Bit35;				
	CString	Bit37;
	BOOL	Bit38_Exist;
	CString	Bit38;				
	CString	Bit39;			
	CString	Bit41;				
	CString	Bit42;
	BOOL	Bit45_Exist;
	CString	Bit45;
	BOOL	Bit48_Exist;
	CString	Bit48;		
	BOOL	Bit59_Exist;
	CString	Bit59;			
	CString	Bit101;				
	CString	Bit128;	
} ALPHI_ResponseRedemption;

typedef struct tagALPHI_ResponseEnquiry
{
	CString	Bit3;				
	CString	Bit7;				
	CString	Bit11;			
	CString	Bit22;				
	CString	Bit32;
	BOOL	Bit35_Exist;
	CString	Bit35;				
	CString	Bit37;
	CString	Bit39;			
	CString	Bit41;				
	CString	Bit42;
	BOOL	Bit45_Exist;
	CString	Bit45;		
	BOOL	Bit48_Exist;
	CString	Bit48;	
	BOOL	Bit59_Exist;
	CString	Bit59;
	CString	Bit101;				
	CString	Bit128;	
} ALPHI_ResponseEnquiry;

typedef struct tagALPHI_ResponseReversal
{
	CString	Bit3;				
	BOOL	Bit4_Exist;
	CString	Bit4;
	CString	Bit7;				
	CString	Bit11;			
	CString	Bit12;				
	CString	Bit13;			
	CString	Bit22;				
	CString	Bit32;	
	BOOL	Bit35_Exist;
	CString	Bit35;				
	CString	Bit37;
	CString	Bit39;			
	CString	Bit41;				
	CString	Bit42;
	BOOL	Bit45_Exist;
	CString	Bit45;
	BOOL	Bit48_Exist;
	CString	Bit48;	
	CString	Bit49;
	CString	Bit60;
	CString	Bit90;			
	CString	Bit101;				
	CString	Bit128;	
} ALPHI_ResponseReversal;

typedef struct tagALPHI_ResponseLoopTest
{
	CString	Bit7;				
	CString	Bit11;			
	CString	Bit37;
	CString	Bit39;			
	CString	Bit41;				
	CString	Bit42;
	BOOL	Bit48_Exist;
	CString	Bit48;	
	CString	Bit70;
	CString	Bit101;				
	CString	Bit128;	
} ALPHI_ResponseLoopTest;

public:
	BIZ_RETURN	BIZ_ALPHI_SendHost();
	BIZ_RETURN	BIZ_ALPHI_RecvHost();

protected:
	int			BIZ_ALPHI_SendData();
	int			BIZ_ALPHI_RecvData();
	BIZ_RETURN	BIZ_ALPHI_AnalHostData();
	BIZ_RETURN	BIZ_ALPHI_AnalRedemption();
	BIZ_RETURN	BIZ_ALPHI_AnalEnquiry();
	BIZ_RETURN	BIZ_ALPHI_AnalReversal();
	BIZ_RETURN	BIZ_ALPHI_AnalLoopTest();

	int			BIZ_REDEMPTION_MakeTransMsg();
	int			BIZ_REVERSAL_MakeTransMsg();
	int			BIZ_ENQUIRY_MakeTransMsg();
	int			BIZ_LOOPTEST_MakeTransMsg();
	CString		GetVariableLengthFields(int pLL,CString pValue,BOOL pNeedUnpack);
	void		Bit3_ProcessingCode(BOOL pRedemption);
	void		Bit4_SurchargeAmount();
	void		Bit7_TransmissionDateAndTime(WORD pYear,WORD pMonth,WORD pDay,WORD pHour,WORD pMin,WORD pSec);
	void		Bit11_SystemTraceAuditNumber(BOOL pRedemption);
	void		Bit12_LocalTime(WORD pHour,WORD pMin,WORD pSec);
	void		Bit13_LocalDate(WORD pYear,WORD pMonth,WORD pDay);
	void		Bit22_PointOfService();
	void		Bit32_AcquiringInstitutionIdentificationCode();
	void		Bit35_Optional_Track2Data();
	void		Bit41_CardAcceptorTerminalIdentification();
	void		Bit42_CardAcceptorIdentificationCode();
	void		Bit45_Track2Data();
	void		Bit48_Optional_AdditionalData();
	void		Bit49_CurrencyCode();
	void		Bit62_ReservedForPrivateUse();
	void		Bit70_NetworkManagementCode();
	void		Bit90_OriginalDataElements();
	void		Bit101_File_Name();
	void		Bit104_TransactionDescription();
	CString		Bit128_MAC(CString pData);
	
	// from redemption request
	CString		m_Reversal_Bit3_Req;
	CString		m_Reversal_Bit4_Req;
	CString		m_Reversal_Bit11_Req;
	CString		m_Reversal_Bit12_Req;
	CString		m_Reversal_Bit13_Req;
	CString		m_Reversal_Bit22_Req;
	CString		m_Reversal_Bit32_Req;
	CString		m_Reversal_Bit35_Req;
	CString		m_Reversal_Bit41_Req;
	CString		m_Reversal_Bit42_Req;
	CString     m_Reversal_Bit45_Req;
	CString		m_Reversal_Bit48_Req;
	CString		m_Reversal_Bit49_Req;
	CString		m_Reversal_Bit62_Req;
	CString		m_Reversal_Bit104_Req;

	void		ClearBit();
	void		PrepareNewMessage(CString pMTI);
	BOOL		m_bGotResponseForRedeem;
	ALPHI_ResponseRedemption	m_ALPHI_ResponseRedemption; 
	ALPHI_ResponseEnquiry		m_ALPHI_ResponseEnquiry;      
	ALPHI_ResponseReversal		m_ALPHI_ResponseReversal;  
	ALPHI_ResponseLoopTest		m_ALPHI_ResponseLoopTest;

	BOOL		OptionExistBitmap(CString pBitmap,BYTE pPosition);
	int			ParseDataOfVariantLength(BOOL pAscii,INT pCountOfLL,CString &pKey, INT pOffset);
	CString		GetPrimaryBitmap6(BOOL pSendTrack1);
	BOOL		ExistAdditionalData();
	void		Bit45_Optional_Track1Data();
	BOOL		SendTrack2Data();
	BOOL		VerifyMAC(CString pData);
	BOOL		VerifyLength();
	BYTE*		HexToBytes(CString Ps_Str, int Ri_Length);
	BYTE		HexToByte(CString Ps_Hex);
#endif  // [#2497] NH woooZ 2017.09.13  ALPHI server