#ifndef _EMVAPI_MFS_H_
#define _EMVAPI_MFS_H_


#define ICEMV_APP_CNT			256				// VISA, MASTER, JCB, NonEMV etc..
#define MAX_LABEL_AID_LEGNTH	16				// Max Label or AID Length
#define MAX_CAKEY				248				// Max CA Plu Key Length
#define HASH_SIZE				20  			// Hash Result Size

#define MAX_RECORD				256

#pragma pack(push,1)

typedef struct 
{
	unsigned char	AidUse;
	int				AidLen;
	unsigned char	szAid[MAX_LABEL_AID_LEGNTH];
	unsigned char	AidName[MAX_LABEL_AID_LEGNTH];
	int				RecordLen;
	unsigned char	szRecord[MAX_RECORD];
}EMV_AID_CONFIG;

typedef struct
{
	int				nTotalAidListCount;
	EMV_AID_CONFIG	m_sEMV_Config[ICEMV_APP_CNT];

}EMV_AID_CONFIG_DATA;

typedef struct 
{
	int AidLen;
	unsigned char Aid[MAX_LABEL_AID_LEGNTH];
}EMV_AID;

typedef struct 
{
	int AidCount;
	EMV_AID AidList[ICEMV_APP_CNT];
}EMV_LIST_OF_AID;

typedef struct 
{
	unsigned char	US_Comm_AidUse;
	int				AidCount;
	EMV_AID			US_Comm_Aid[ICEMV_APP_CNT];
}EMV_US_COMMON_AIDLIST;

typedef struct
{
	unsigned char AppLabelLen;
	unsigned char AppLabel[MAX_LABEL_AID_LEGNTH*2];
	unsigned char AidLen;
	unsigned char AidName[MAX_LABEL_AID_LEGNTH];
	unsigned char priority;
	unsigned char PreferredLen;
	unsigned char PreferredName[MAX_LABEL_AID_LEGNTH*2];
	unsigned char IsrCodeTableIndex;
	unsigned char IsrCntryCodeLen_Alpha2; 		//5F55 Len
	unsigned char IsrCntryCode_Alpha2[2]; 		//5F55 Value
	unsigned char IsrIdNo[3]; 					//Issuer Identification Number(IIN)
	unsigned char Reserved[256];				
} ICC_APP_DESC;

typedef struct
{
	int CardHolderConfirm;
	int AppCount;
	ICC_APP_DESC IccAppDesc[ICEMV_APP_CNT];
} EMV_ICC_APPS_INFORM;


#pragma pack(pop)


// Result Code
#define EMV_RSLT_OK				1
#define	EMV_ERR_BASE 			0
#define	EMV_ERR_CARD_SW 		(EMV_ERR_BASE - 1)
#define EMV_ERR_NOT_SUPPORT 	(EMV_ERR_BASE - 2)
#define EMV_ERR_NOT_ACCEPT 		(EMV_ERR_BASE - 3)
#define EMV_ERR_TERMINATE 		(EMV_ERR_BASE - 4)

// Defined Transaction Type
#define 	TRTYPE_GOODSERVICE 		0x00			// Purchase of goods or service
#define 	TRTYPE_CASH				0x01			// Cash Disbursement
#define 	TRTYPE_CASHBACK 		0x09			// Purchase with Cash Back
#define 	TRTYPE_INQUIRY			0x30			// Inquiry
#define 	TRTYPE_PAYMENT			0x19			// Payment
#define 	TRTYPE_TRANSFER			0x40			// Transfer
#define 	TRTYPE_ADMINISTRATIVE	0x80			// Administrative  
#define 	TRTYPE_CASHDEPOSIT		0x11			// Cash Deposit  

// Define Cryptogram
#define 	AAC 					0x00			// Declined
#define 	TC  					0x40			// Approved
#define 	ARQC					0x80			// Go to online
#define 	AAR 					0xC0			// Referral

// Define Transaction Type
#define		ONLINE_TRANS			0x8F
#define		OFFLINE_TRANS   		0x4F
#define		REVERSAL_TRANS   		0x1F

// Define Issuer Response Code 
#define		ISSUER_APPROVAL_VALUE	"00"
#define		ISSUER_DECLINE_VALUE	"05"
#define		ISSUER_REFERAL_VALUE	"01"

#define		EMV_APPROVED			6
#define		EMV_DECLINED			7


// Supported TLV List
//#define	EMVTid_AcquirerId 			0 		//9F01 Acquirer Identifier  

#define	EMVTid_AddTerminalCapa 		1 		//9F40 Additional Terminal Capability  
#define EMVTag_AddTerminalCapa		(unsigned char*)"9F40"	

//#define	EMVTid_AmountAuthB 			2 		//8100 Amount Authorized (Binary)  

#define	EMVTid_AmountAuthN 			3 		//9F02 Amount Authorized (Numeric)  
#define	EMVTag_AmountAuthN 			(unsigned char*)"9F02"	//

//#define	EMVTid_AmountOtherB 		4 		//9F04 Amount Other (Binary)  

#define	EMVTid_AmountOtherN 		5 		//9F03 Amount Other (Numeric)  
#define	EMVTag_AmountOtherN 		(unsigned char*)"9F03" 		//

//#define	EMVTid_AmountRefCurB 		6 		//9F3A Amount Reference Currency  
//#define	EMVTid_AmountTrans 			7 		//DF01 Amount Transaction  

#define	EMVTid_AC 					8 		//9F26 Application Cryptogram  
#define	EMVTag_AC 					(unsigned char*)"9F26" 	//

//#define	EMVTid_AppCurCod 			9 		//9F42 Application Currency Code  
//#define	EMVTid_AppCurExp 			10 		//9F44 Application Currency Exponent  
//#define	EMVTid_AppDisData 			11 		//9F05 Application Discretionary Data  
//#define	EMVTid_AppEffDate 			12 		//5F25 Application Effective Date  
#define	EMVTid_AppExpiredDate 		13 			//5F24 Application Expired Date  
#define	EMVTag_AppExpiredDate 		(unsigned char*)"5F24" 		//

//#define	EMVTid_AFL 					14 		//9400 Application File Locator  
//#define	EMVTid_AID 					15 		//4F00 Application Identifier  

#define	EMVTid_AIP 					16 			//8200 Application Interchange Profile  
#define	EMVTag_AIP 					(unsigned char*)"8200" 		//

//#define	EMVTid_AppLabel 			17 		//5000 Application Label  
//#define	EMVTid_AppPreferName 		18 		//9F12 Application Preferred Name  

#define	EMVTid_PAN 					19 			//5A00 Primary Account Number  
#define	EMVTag_PAN 					(unsigned char*)"5A00" 		//

#define	EMVTid_PANSeqNo 			20 			//5F34 Primary Account Sequence Number  
#define	EMVTag_PANSeqNo 			(unsigned char*)"5F34" 		//

//#define	EMVTid_AppPriorityInd 		21 		//8700 Application Priority Indicator  
//#define	EMVTid_AppRefCur 			22 		//9F3B Application Reference Currency  
//#define	EMVTid_AppRefCurExp 		23 		//9F43 Application Reference Currency Exponent  

#define	EMVTid_ATC 					24 			//9F36 Application Transaction Counter  
#define	EMVTag_ATC 					(unsigned char*)"9F36" 		//

//#define	EMVTid_AUC 					25 		//9F07 Application Usage Control  
//#define	EMVTid_ICAppVerNo 			26 		//9F08 ICC Application Version Number  

#define	EMVTid_TrAppVerNo 			27 			//9F09 Terminal Application Version Number  
#define	EMVTag_TrAppVerNo 			(unsigned char*)"9F09" 		//

#define	EMVTid_AuthRespCod 			28 			//8A00 Authorization Response Code 
#define	EMVTag_AuthRespCod 			(unsigned char*)"8A00" 		//

//#define	EMVTid_CDOL1 				29 		//8C00 CDOL1  
//#define	EMVTid_CDOL2 				30 		//8D00 CDOL2 

#define	EMVTid_HolderName 			31 			//5F20 Cardholder Name  
#define	EMVTag_HolderName 			(unsigned char*)"5F20" 		//

//#define	EMVTid_HolderNameExt 		32 		//9F0B Cardholder Name Extended  

#define	EMVTid_CVMList 				33 			//8E00 Cardholder Verification Method List 
#define	EMVTag_CVMList 				(unsigned char*)"8E00" 		//

#define	EMVTid_CVMRslt 				34 			//9F34 Cardholder Verification Method Result
#define	EMVTag_CVMRslt 				(unsigned char*)"9F34" 		//

//#define	EMVTid_CAPubKeyIdx 			35 		//8F00 CA Public Key Index  

#define	EMVTid_CryptInfData 		36 			//9F27 Cryptogram Information Data  
#define	EMVTag_CryptInfData 		(unsigned char*)"9F27" 		//

//#define	EMVTid_DataAuthCod 			37 		//9F45 Data Authorization Code  

#define	EMVTid_DFName 				38 			//8400 DF Name  
#define	EMVTag_DFName 				(unsigned char*)"8400" 		//

//#define	EMVTid_DefaultDDOL 			39 		//D600 Default DDOL 
//#define	EMVTid_DefaultTDOL 			40 		//D700 Default TDOL  
//#define	EMVTid_EncipherPINData 		41 		//DF02 Enciphered PIN Data  
//#define	EMVTid_DDOL 				42 		//9F49 DDOL  
//#define	EMVTid_FciIsuDisData 		43 		//BF0C FCI Issuer Discretionary Data  
//#define	EMVTid_IccDynamicNum 		44 		//9F4C ICC Dynamic Data  
//#define	EMVTid_IccPINPubKeyCert 	45 		//9F2D ICC PIN Public Key Certificate  
//#define	EMVTid_IccPINPubKeyExp 		46 		//9F2E ICC PIN Public Key Exponent  
//#define	EMVTid_IccPINPubKeyRem 		47 		//9F2F ICC PIN Public Key Remainder  
//#define	EMVTid_IccPubKeyCert 		48 		//9F46 ICC Public Key Certificate  
//#define	EMVTid_IccPubKeyExp 		49 		//9F47 ICC Public Key Exponent  
//#define	EMVTid_IccPubKeyRem 		50 		//9F48 ICC Public Key Remainder  

#define	EMVTid_IFDSerialNo 			51 			//9F1E IFD Serial Number  
#define	EMVTag_IFDSerialNo 			(unsigned char*)"9F1E" 		//

//#define	EMVTid_IACDefault 			52 		//9F0D Issuer Action Code Default  
//#define	EMVTid_IACDenial 			53 		//9F0E Issuer Action Code Denial  
//#define	EMVTid_IACOnline 			54 		//9F0F Issuer Action Code Online  

#define	EMVTid_IsuAppData 			55 			//9F10 Issuer Application Date  
#define	EMVTag_IsuAppData 			(unsigned char*)"9F10" 		//

#define	EMVTid_IsuAuthData 			56 			//9100 Issuer Authorization Data  
#define	EMVTag_IsuAuthData 			(unsigned char*)"9100" 		//

//#define	EMVTid_IsuCodTblIdx 		57 		//9F11 Issuer Code Table Index  

#define	EMVTid_IsuCountryCod 		58 			//5F28 Issuer Country Code  
#define	EMVTag_IsuCountryCod 		(unsigned char*)"5F28" 		//

//#define	EMVTid_IsuPubKeyCert 		59 		//9000 Issuer Public Key Certificate  
//#define	EMVTid_IsuPubKeyExp 		60 		//9F32 Issuer Public Key Exponent  
//#define	EMVTid_IsuPubKeyRem 		61 		//9200 Issuer Public Key Remainder  

#define	EMVTid_IsuScriptId 			62 			//9F18 Issuer Script Identifier 
#define	EMVTag_IsuScriptId 			(unsigned char*)"9F18" 		//

#define	EMVTid_IsuScriptRslt 		63 			//DF03 Issuer Script Result  
#define	EMVTag_IsuScriptRslt 		(unsigned char*)"DF03" 		//

#define	EMVTid_IsuScriptTemp1 		64 			//7100 Issuer Script Template 1
#define	EMVTag_IsuScriptTemp1 		(unsigned char*)"7100" 		//

#define	EMVTid_IsuScriptTemp2 		65 			//7200 Issuer Script Template 2  
#define	EMVTag_IsuScriptTemp2 		(unsigned char*)"7200" 		//

#define	EMVTid_LangPrefer 			66 			//5F2D Language Preference  
#define	EMVTag_LangPrefer 			(unsigned char*)"5F2D" 		//

//#define	EMVTid_LastOnlineATC 		67 		//9F13 Last Online Application Transaction Counter  
//#define	EMVTid_LowOffLimit 			68 		//9F14 Lower Offline Limit  
//#define	EMVTid_MCC 					69 		//9F15 Merchant Category Code  
//#define	EMVTid_MerchantId 			70 		//9F16 Merchant Identifier  
//#define	EMVTid_MerchantNameLoc 		71 		//DF04 Merchant Name Location  
//#define	EMVTid_MsgType 				72 		//DF05 Message Type  

#define	EMVTid_POSEntryMode 		73 			//9F39 POS Entry Code  
#define	EMVTag_POSEntryMode 		(unsigned char*)"9F39" 		//

//#define	EMVTid_PDOL 				74 		//9F38 PDOL  

#define	EMVTid_ServiceCod 			75 			//5F30 Service Code  
#define	EMVTag_ServiceCod 			(unsigned char*)"5F30" 		//

//#define	EMVTid_SignDynAppData 		76 		//9F4B Signed Dynamic Application Data  
//#define	EMVTid_SignStaAppData 		77 		//9300 Signed Static Application Data  
//#define	EMVTid_StaDataAuthTList 	78 		//9F4A Static Data Authentication Tag List  
//#define	EMVTid_TACDefault 			79 		//D800 Terminal Action Code Default  
//#define	EMVTid_TACDenial 			80 		//D900 Terminal Action Code Denial  
//#define	EMVTid_TACOnline 			81 		//DA00 Terminal Action Code Online  

#define	EMVTid_TerminalCapa 		82 			//9F33 Terminal Capability  
#define	EMVTag_TerminalCapa 		(unsigned char*)"9F33" 		//

#define	EMVTid_TCountryCod 			83 			//9F1A Terminal Country Code 
#define	EMVTag_TCountryCod 			(unsigned char*)"9F1A" 		//

#define	EMVTid_TFloorLimit 			84 			//9F1B Terminal Floor Limit  
#define	EMVTag_TFloorLimit 			(unsigned char*)"9F1B" 		//

#define	EMVTid_TerminalId 			85 			//9F1C Terminal Identifier  
#define	EMVTag_TerminalId 			(unsigned char*)"9F1C" 		//

//#define	EMVTid_TRiskMgmtData 		86 		//9F1D Terminal Risk Management Data  

#define	EMVTid_TerminalType 		87 			//9F35 Terminal Type  
#define	EMVTag_TerminalType 		(unsigned char*)"9F35" 		//

#define	EMVTid_TVR 					88 			//9500 Terminal Verification Result  
#define	EMVTag_TVR 					(unsigned char*)"9500" 		//

//#define	EMVTid_Track1DisData 		89 		//9F1F Track1 Discretionary Data  
//#define	EMVTid_Track2DisData 		90 		//9F20 Track2 Discretionary Data  

#define	EMVTid_Track2EquData 		91 			//5700 Track2 Equivalent Data  
#define	EMVTag_Track2EquData 		(unsigned char*)"5700" 		//

//#define	EMVTid_TDOL 				92 		//9700 TDOL  
//#define	EMVTid_TCHashVal 			93 		//9800 Transaction Certificate Hash Value  

#define	EMVTid_TrCurCod 			94 			//5F2A Transaction Currency Code  
#define	EMVTag_TrCurCod 			(unsigned char*)"5F2A" 		//

#define EMVTid_TrCurExp 			95 			//5F36 Transaction Currency Exponent  
#define EMVTag_TrCurExp 			(unsigned char*)"5F36" 		//

#define EMVTid_TrDate 				96 			//9A00 Transaction Date  
#define EMVTag_TrDate 				(unsigned char*)"9A00" 		//

//#define EMVTid_TrRefCurCod 			97 		//9F3C Transaction Reference Currency Code  
//#define EMVTid_TrRefCurCnv 			98 		//DB00 Transaction Reference Currency Conversion  
//#define EMVTid_TrRefCurExp 			99 		//9F3D   

#define EMVTid_TrSeqCnt 			100			//9F41   
#define EMVTag_TrSeqCnt 			(unsigned char*)"9F41" 		//

#define EMVTid_TSI 					101 		//9B00   
#define EMVTag_TSI 					(unsigned char*)"9B00" 		//

#define EMVTid_TrTime 				102 		//9F21
#define EMVTag_TrTime 				(unsigned char*)"9F21" 		//

#define EMVTid_TrType 				103 		//9C00
#define EMVTag_TrType 				(unsigned char*)"9C00" 		//

#define EMVTid_UnpredictNo 			104			//9F37   
#define EMVTag_UnpredictNo 			(unsigned char*)"9F37" 		//

//#define EMVTid_UpOffLimit 			105 	//9F23   
//#define EMVTid_InstallmentNo 		106 	//D100   
//#define EMVTid_ApprovalNo 			107 	//D400   
//#define EMVTid_IsuPubKey 			108 	//DF06   
//#define EMVTid_StaAppData 			109 	//DF07   
//#define EMVTid_TaxAmount 			110 	//D200   
//#define EMVTid_ServiceFee 			111 	//D300   
//#define EMVTid_OriTrDate 			112 	//D500   
//#define EMVTid_MerchantNo 			113 	//DC00   
//#define EMVTid_OriApprovalNo 		114 	//DE00   
//#define EMVTid_AcquirerName 		115 	//DF08   
//#define EMVTid_HostMessage 			116 	//DF09   
//#define EMVTid_TerminalCnt 			117 	//DF0A   
//#define EMVTid_IccPubKey 			118 	//DF0B   
//#define EMVTid_DynAppData 			119 	//DF0C   

#define EMVTid_TerminalAID 			120			//9F06   
#define EMVTag_TerminalAID 			(unsigned char*)"9F06" 		//

//#define EMVTid_TermCAPubKeyIdx 		121 	//9F22   
//#define EMVTid_TargetPercent 		122 	//DF0D   
//#define EMVTid_ThresholdValue 		123 	//DF0E   
//#define EMVTid_MaxTargetPercent 	124 	//DF0F   

#define EMVTid_TCC 					125 		//9F53   
#define EMVTag_TCC 					(unsigned char*)"9F53" 		//

//#define EMVTid_HostNotice 			126 	//DF1D 
//#define EMVTid_DispControl 			127 	//DF1E   
//#define EMVTid_LOGEntry 			128 	//9F4D   

#define EMVTid_AccountType 			129 		//5F57  
#define EMVTag_AccountType 			(unsigned char*)"5F57" 		//

//#define EMVTid_ASIndicator 			130 	//DF10 Application Selection Indicator  

#define EMVTid_LAST_TAG 			225


//typedef int		(*DLL_EMVKernel_AppSelection)(unsigned char SupportPSE, EMV_LIST_OF_AID *AIDList);
//typedef int		(*DLL_EMVKernel_MakeAppLists)(unsigned char SupportUserConfirm, EMV_ICC_APPS_INFORM *IccAppInform, int *nAppCnt);
//typedef void	(*DLL_EMVKernel_RemoveAppList)(USHORT AidLen, unsigned char *AIDVal, int *nAppCnt);
//typedef int		(*DLL_EMVKernel_FinalAppSelection)(USHORT AdiLen, unsigned char *AIDVal, int *nAppCnt, unsigned char *SW);
//typedef int		(*DLL_EMVKernel_StoreValFromRecord)(unsigned char *record, int type, int inlen, int source);
//typedef int		(*DLL_EMVKernel_InitApplication)(int *nAppCnt);
//typedef int		(*DLL_EMVKernel_ReadAppData)(void);
//typedef int		(*DLL_EMVKernel_OfflineDataAuth)(void);
//typedef int		(*DLL_EMVKernel_ProcessRestrict)(void);
//typedef int		(*DLL_EMVKernel_CardholderVerify)(int OnlinePinLen, unsigned char *EncipheredOnlinePin);
//typedef int		(*DLL_EMVKernel_TerminalRiskMgmt)(unsigned char SupportForceOnline);
//typedef byte	(*DLL_EMVKernel_TerminalActionAnalysis)(void);
//typedef int		(*DLL_EMVKernel_CardActionAnalysis)(unsigned char *OnlineFlag);
//typedef int		(*DLL_EMVKernel_OnlineProcess)(unsigned char Acquirer_CID);
//typedef int		(*DLL_EMVKernel_Completion)(void);
//
//typedef int		(*DLL_EMVKernel_CDRCommFunc)(int (CALLBACK *EMV_Command)(BYTE* SendData, DWORD SendLength, BYTE* RecvData, DWORD* RecvLen));
//typedef void	(*DLL_EMVKernel_random_number)(void);
//typedef int		(*DLL_EMVKernel_Tlv_GetVal)(unsigned char *tag, int *len, unsigned char *val);
//typedef int		(*DLL_EMVKernel_Tlv_StoreVal)(unsigned char *tag, int len, unsigned char*val);
//typedef void	(*DLL_EMVKernel_Initial_Variable)(void);
//typedef void	(*DLL_EMVKernel_Util_SetKernelLogPath)(unsigned char*FilePath,unsigned char *ExtensionName);
//typedef void	(*DLL_EMVKernel_SetDebug)(BYTE mode);

#endif
