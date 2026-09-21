/////////////////////////////////////////////////////////////////////////////
//	DevEMV.cpp : implementation file
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

//#define NH_DEBUG
#include ".\Common\CmnLib.h"
#include ".\Common\NHDbgApi.h"
#include ".\Common\ConstDef.h"

#include ".\Dev\DevDefine.h"
#include ".\Dev\DevCmn.h"

// In Windows, there is no GetProcAddressW, so we have to force the string
// constants to multi-byte.
#ifdef UNDER_CE
#  define _LT(x) _T(x)
#else
#  define _LT(x) x
#endif

// [#2188] NH KSK 2013.04.24 2010년 Kernel Header File -> SCSp_ATM_EMV.h로 2013년 통일 (DevCmn.h로 이동)
//#include ".\Dev\EmvL2Kernel.h"
//#include ".\Dev\EmvL2Kernel_ISO8583.h"
// end of [#2188]

#define DBG_CALL		1
#define DBG_INFO		1

CNHMWI*			g_pMwi = NULL;
int				m_nEMVL2KernelVersion = EMV_KERNEL_V5;	// [#2188] NH KSK 2013.04.24
int	CALLBACK    libSMC_SendApdu(BYTE dCode, BYTE *apdu, UINT apduLen, BYTE *resp, int *rlen);

// [#2188] NH KSK 2013.04.26 EMVL2 Kernel Function Definition
////////////////////////////////////////
// Function Type Define
typedef void			(VOID_FUNC_VOID)(void);
typedef void			(VOID_FUNC_UCHAR)(unsigned char);
typedef void			(VOID_FUNC_PUCHAR)(unsigned char*);
typedef void			(VOID_FUNC_PEMVstICCDESCDATAINFORM)(EMVst_ICC_DESCDATA_INFORM*);
typedef	void			(VOID_PEMVstLISTOFISAPDU)(EMVst_LIST_OF_ISAPDU*);
typedef void			(VOID_FUNC_INT_PUCHAR)(int, unsigned char*);
typedef void			(VOID_FUNC_PUCHAR_UINT)(unsigned char*, unsigned int);
typedef void			(VOID_FUNC_USHORT_USHORT_PUCHAR)(unsigned short, unsigned short, unsigned char*);
typedef void			(VOID_FUNC_INT_PUCHAR_INT)(int, unsigned char*, int);
typedef void			(VOID_FUNC_PUCHAR_PUCHAR_INT)(unsigned char*, unsigned char*, int);
typedef void			(VOID_FUNC_PUCHAR_INT_PUCHAR)(unsigned char*, int, unsigned char*);
typedef void			(VOID_FUNC_INT)(int);	// [#2435] NH KSK 2016.07.14 added Kernel V6

typedef	int				(INT_FUNC_VOID)(void);
typedef	int				(INT_FUNC_INT)(int);
typedef int				(INT_FUNC_UCHAR)(unsigned char);
typedef int				(INT_FUNC_PUCHAR)(unsigned char*);
typedef	int				(INT_PEMVstCAPUBKEY)(EMVst_CA_PUBKEY*);
typedef int				(INT_FUNC_USHORT_PUCHAR)(unsigned short, unsigned char*);
typedef int				(INT_FUNC_INT_UCHAR)(int, unsigned char);
typedef int				(INT_FUNC_PUCHAR_INT)(unsigned char*, int);
typedef int				(INT_FUNC_PUCHAR_IPNT)(unsigned char*, int*);
typedef int				(INT_FUNC_PUCHAR_PUCHAR)(unsigned char*, unsigned char*);
typedef int				(INT_FUNC_PEMVstLISTOFNDCAID_PINT)(EMVst_LIST_OF_NDCAID*, int*);
typedef int				(INT_FUNC_USHORT_PUCHAR_PUCHAR)(unsigned short, unsigned char*, unsigned char*);
typedef int				(INT_FUNC_INT_PINT_PUCHAR)(int, int*, unsigned char*);
typedef int				(INT_FUNC_INT_INT_PUCHAR)(int, int, unsigned char*);
typedef int				(INT_FUNC_UCHAR_EMVstLISTOFAID_PINT)(unsigned char, EMVst_LIST_OF_AID*, int*);
typedef int				(INT_FUNC_UCHAR_EMVstLISTOFAIDV5_PINT)(unsigned char, EMVst_LIST_OF_AID_V5*, int*);
typedef int				(INT_FUNC_UCHAR_USHORT_PEMVICCAPPSINFORM)(unsigned char, unsigned short, EMV_ICC_APPS_INFORM*);
typedef int				(INT_FUNC_UCHAR_USHORT_PEMVICCAPPSINFORMV5)(unsigned char, unsigned short, EMV_ICC_APPS_INFORM_V5*);
typedef int				(INT_FUNC_PUCHAR_INT_INT_INT)(unsigned char*, int, int, int);

typedef	unsigned char	(UCHAR_FUNC_VOID)(void);
typedef	unsigned char	(UCHAR_FUNC_PUCHAR_INT)(unsigned char*, int);

typedef	int				(INT_FUNC_INT_CALLBACK)(int (CALLBACK* SMCSENDAPDU)(BYTE, BYTE*, UINT, BYTE*,int*));


////////////////////////////////////////
// MB2500DLL API
typedef struct _EMVL2KernelDLL_FUNCTION_
{
	// EMV Application Selection
	VOID_FUNC_UCHAR								*pemvkrnl_Comm_Sel_SetOnlyOneAID;
	INT_FUNC_UCHAR_EMVstLISTOFAID_PINT			*pemvkrnl_V4_Sel_ApplicationSelection;
	INT_FUNC_UCHAR_EMVstLISTOFAIDV5_PINT		*pemvkrnl_V5_Sel_ApplicationSelection;
	INT_FUNC_UCHAR_USHORT_PEMVICCAPPSINFORM		*pemvkrnl_V4_Sel_MakeDisplayAppList;
	INT_FUNC_UCHAR_USHORT_PEMVICCAPPSINFORMV5	*pemvkrnl_V5_Sel_MakeDisplayAppList;
	VOID_FUNC_USHORT_USHORT_PUCHAR				*pemvkrnl_Comm_Sel_RemoveCandidateList;
	INT_FUNC_USHORT_PUCHAR_PUCHAR				*pemvkrnl_Comm_Sel_FinalAppSelection;
	VOID_FUNC_PEMVstICCDESCDATAINFORM			*pemvkrnl_Comm_Sel_DescretionaryDataList;
	INT_FUNC_PEMVstLISTOFNDCAID_PINT			*pemvkrnl_V5_Sel_NDCAIDSelection;
	VOID_FUNC_UCHAR								*pemvkrnl_V5_Sel_ASI;

	// EMV Transaction
	INT_FUNC_VOID								*pemvkrnl_Comm_Trans_InitApplication;
	INT_FUNC_VOID								*pemvkrnl_Comm_Trans_ReadAppData;
	INT_PEMVstCAPUBKEY							*pemvkrnl_Comm_Trans_OfflineDataAuth;
	INT_FUNC_VOID								*pemvkrnl_Comm_Trans_ProcessRestrict;
	INT_FUNC_USHORT_PUCHAR						*pemvkrnl_Comm_Trans_CardholderVerify;
	INT_FUNC_UCHAR								*pemvkrnl_Comm_Trans_TerminalRiskMgmt;
	UCHAR_FUNC_VOID								*pemvkrnl_Comm_Trans_TerminalActionAnalysis;
	INT_FUNC_PUCHAR								*pemvkrnl_Comm_Trans_CardActionAnalysis;
	INT_FUNC_INT_UCHAR							*pemvkrnl_Comm_Trans_OnlineProcess;
	INT_FUNC_VOID								*pemvkrnl_Comm_Trans_Completion;
	VOID_PEMVstLISTOFISAPDU						*pemvkrnl_V5_Get_ISAPDUList;

	// for Tlv Function
	VOID_FUNC_INT_PUCHAR						*pemvkrnl_Comm_Tlv_GetTidTag;
	INT_FUNC_PUCHAR_INT							*pemvkrnl_Comm_Tlv_GetTid;
	INT_FUNC_PUCHAR_PUCHAR						*pemvkrnl_Comm_Tlv_GetTag;
	INT_FUNC_INT_PINT_PUCHAR					*pemvkrnl_Comm_Tlv_GetVal;
	INT_FUNC_INT								*pemvkrnl_Comm_Tlv_GetTidLen;
	INT_FUNC_PUCHAR_IPNT						*pemvkrnl_Comm_Tlv_GetLen;
	INT_FUNC_INT_INT_PUCHAR						*pemvkrnl_Comm_Tlv_StoreVal;
	INT_FUNC_PUCHAR_INT_INT_INT					*pemvkrnl_Comm_Tlv_StoreValFromRecord;

	//-------------------------------------------------------------------
	// EMV Utility
	VOID_FUNC_INT_PUCHAR_INT					*pemvkrnl_Comm_Util_BINtoN;
	VOID_FUNC_PUCHAR_PUCHAR_INT					*pemvkrnl_Comm_Util_toYYYYMMDD;
	UCHAR_FUNC_PUCHAR_INT						*pemvkrnl_Comm_Util_GetCNRealLen;
	VOID_FUNC_PUCHAR_INT_PUCHAR					*pemvkrnl_Comm_Util_RightJustify;
	VOID_FUNC_PUCHAR_PUCHAR_INT					*pemvkrnl_Comm_Util_Decompress;
	VOID_FUNC_PUCHAR_PUCHAR_INT					*pemvkrnl_Comm_Util_Compress;

	//-------------------------------------------------------------------
	// Other Functions
	VOID_FUNC_PUCHAR							*pemvkrnl_Comm_Util_GetAPDUSW;
	VOID_FUNC_PUCHAR							*pemvkrnl_Comm_Util_GetVersion;
	VOID_FUNC_PUCHAR_UINT						*pemvkrnl_V5_Util_SetKernelLogPath;
	VOID_FUNC_VOID								*pemvkrnl_V5_Trans_Initial;
	VOID_FUNC_INT								*emvkrnl_V6_Util_SetDebugFlag;	// [#2435] NH KSK 2016.07.14 added Kernel V6

	//-------------------------------------------------------------------
	// CallBack Function
	INT_FUNC_INT_CALLBACK						*pComm_RegistIFMCommFunc;
	
} EMVL2KernelDLL_FUNCTION, *PEMVL2KernelDLL_FUNCTION;
// end of [#2188]

// [#2188] NH KSK 2013.04.24
HINSTANCE					m_hEMVL2KernelDLL = NULL;
EMVL2KernelDLL_FUNCTION		m_EMVL2KernelDll_FUNC;
// end of [#2188]

// [#2452] NH KSK 2016.11.10 HexaDump함수 확장을 위해 (File Write 기능 추가) CmnLib의 Trace.cpp로 이동
/*
void HexaDump_byCha(BYTE *pData, int nLen)
{
#ifdef NH_DEBUG
	RETAILMSG(1, (L"\n-----------------------------------------------------------\n"));

	for (int i = 1; i <= nLen; i++)
	{
		RETAILMSG(1, (L"0x%02X ", pData[i-1]));
		if ((i % 10) == 0)
			RETAILMSG(1, (L"\n"));
	}

	RETAILMSG(1, (L"\n-----------------------------------------------------------\n"));
#endif
}
*/
// end of [#2452]

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnEMV_Initialize
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : --
-------------------------------------------------------------------*/
int CDevCmn::fnEMV_Initialize()
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnEMV_Initialize]\n"));

	g_pMwi = m_pMwi;

	// [#2188] NH KSK 2013.04.24 EMVL2Kernel.dll Loading
	memset(&m_EMVL2KernelDll_FUNC, 0, sizeof(m_EMVL2KernelDll_FUNC));

	switch(m_nEMVL2KernelVersion)
	{
	case EMV_KERNEL_V4:
		{
			m_hEMVL2KernelDLL = ::LoadLibrary(_T(".\\EmvL2Kernel.dll"));

			if (m_hEMVL2KernelDLL == NULL)
			{
				RETAILMSG(1, (L"LoadLibrary EmvL2Kernel.dll Error\n"));
				return -1;
			}

			RETAILMSG(1, (L"LoadLibrary EmvL2Kernel.dll OK\n"));

			// Version4에 대해서 Function Pointer Get
			m_EMVL2KernelDll_FUNC.pemvkrnl_V4_Sel_ApplicationSelection			= (INT_FUNC_UCHAR_EMVstLISTOFAID_PINT*)GetProcAddress(m_hEMVL2KernelDLL,		_LT("?emvkrnl_Sel_ApplicationSelection@@YAHEPAUEMVst_LIST_OF_AID@@PAH@Z"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_V4_Sel_ApplicationSelection == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_V4_Sel_ApplicationSelection\n"));

			// pemvkrnl_V4_Sel_MakeDisplayAppList;			// 사용 안함
			m_EMVL2KernelDll_FUNC.pemvkrnl_V4_Sel_MakeDisplayAppList			= (INT_FUNC_UCHAR_USHORT_PEMVICCAPPSINFORM*)GetProcAddress(m_hEMVL2KernelDLL,	_LT("?emvkrnl_Sel_MakeDisplayAppList@@YAHEGPAUEMV_ICC_APPS_INFORM@@@Z"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_V4_Sel_ApplicationSelection == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_V4_Sel_MakeDisplayAppList\n"));

			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Sel_RemoveCandidateList			 = (VOID_FUNC_USHORT_USHORT_PUCHAR*)GetProcAddress(m_hEMVL2KernelDLL,			_LT("?emvkrnl_Sel_RemoveCandidateList@@YAXGGPAE@Z"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Sel_RemoveCandidateList == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_Comm_Sel_RemoveCandidateList\n"));

			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Sel_FinalAppSelection			= (INT_FUNC_USHORT_PUCHAR_PUCHAR*)GetProcAddress(m_hEMVL2KernelDLL,				_LT("?emvkrnl_Sel_FinalAppSelection@@YAHGPAE0@Z"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Sel_FinalAppSelection == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_Comm_Sel_FinalAppSelection\n"));

			// pemvkrnl_Comm_Sel_DescretionaryDataList;		// 사용 안함
			// pemvkrnl_V5_Sel_NDCAIDSelection;				// 사용 안함
			//m_EMVL2KernelDll_FUNC.pemvkrnl_V5_Sel_ASI		// V4 사용 안함

			// EMV Transaction
			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_InitApplication			= (INT_FUNC_VOID*)GetProcAddress(m_hEMVL2KernelDLL,								_LT("?emvkrnl_Trans_InitApplication@@YAHXZ"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_InitApplication == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_Comm_Trans_InitApplication\n"));

			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_ReadAppData				= (INT_FUNC_VOID*)GetProcAddress(m_hEMVL2KernelDLL,								_LT("?emvkrnl_Trans_ReadAppData@@YAHXZ"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_ReadAppData == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_Comm_Trans_ReadAppData\n"));

			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_OfflineDataAuth			= (INT_PEMVstCAPUBKEY*)GetProcAddress(m_hEMVL2KernelDLL,						_LT("?emvkrnl_Trans_OfflineDataAuth@@YAHPAUEMVst_CA_PUBKEY@@@Z"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_OfflineDataAuth == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_Comm_Trans_OfflineDataAuth\n"));

			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_ProcessRestrict			= (INT_FUNC_VOID*)GetProcAddress(m_hEMVL2KernelDLL,								_LT("?emvkrnl_Trans_ProcessRestrict@@YAHXZ"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_ProcessRestrict == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_Comm_Trans_ProcessRestrict\n"));

			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_CardholderVerify			= (INT_FUNC_USHORT_PUCHAR*)GetProcAddress(m_hEMVL2KernelDLL,					_LT("?emvkrnl_Trans_CardholderVerify@@YAHGPAE@Z"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_CardholderVerify == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_Comm_Trans_CardholderVerify\n"));

			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_TerminalRiskMgmt			= (INT_FUNC_UCHAR*)GetProcAddress(m_hEMVL2KernelDLL,							_LT("?emvkrnl_Trans_TerminalRiskMgmt@@YAHE@Z"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_TerminalRiskMgmt == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_Comm_Trans_TerminalRiskMgmt\n"));

			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_TerminalActionAnalysis	= (UCHAR_FUNC_VOID*)GetProcAddress(m_hEMVL2KernelDLL,							_LT("?emvkrnl_Trans_TerminalActionAnalysis@@YAEXZ"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_TerminalActionAnalysis == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_Comm_Trans_TerminalActionAnalysis\n"));

			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_CardActionAnalysis		= (INT_FUNC_PUCHAR*)GetProcAddress(m_hEMVL2KernelDLL,							_LT("?emvkrnl_Trans_CardActionAnalysis@@YAHPAE@Z"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_CardActionAnalysis == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_Comm_Trans_CardActionAnalysis\n"));

			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_OnlineProcess				= (INT_FUNC_INT_UCHAR*)GetProcAddress(m_hEMVL2KernelDLL,						_LT("?emvkrnl_Trans_OnlineProcess@@YAHHE@Z"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_OnlineProcess == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_Comm_Trans_OnlineProcess\n"));

			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_Completion				= (INT_FUNC_VOID*)GetProcAddress(m_hEMVL2KernelDLL,								_LT("?emvkrnl_Trans_Completion@@YAHXZ"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_Completion == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_Comm_Trans_Completion\n"));

			// pemvkrnl_V5_Get_ISAPDUList;					// 사용 안함

			// for Tlv Function
			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Tlv_GetTidTag					= (VOID_FUNC_INT_PUCHAR*)GetProcAddress(m_hEMVL2KernelDLL,						_LT("?emvkrnl_Tlv_GetTidTag@@YAXHPAE@Z"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Tlv_GetTidTag == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_Comm_Tlv_GetTidTag\n"));

			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Tlv_GetTid						= (INT_FUNC_PUCHAR_INT*)GetProcAddress(m_hEMVL2KernelDLL,						_LT("?emvkrnl_Tlv_GetTid@@YAHPAEH@Z"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Tlv_GetTid == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_Comm_Tlv_GetTid\n"));

			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Tlv_GetTag						= (INT_FUNC_PUCHAR_PUCHAR*)GetProcAddress(m_hEMVL2KernelDLL,					_LT("?emvkrnl_Tlv_GetTag@@YAHPAE0@Z"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Tlv_GetTag == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_Comm_Tlv_GetTag\n"));

			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Tlv_GetVal						= (INT_FUNC_INT_PINT_PUCHAR*)GetProcAddress(m_hEMVL2KernelDLL,					_LT("?emvkrnl_Tlv_GetVal@@YAHHPAHPAE@Z"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Tlv_GetVal == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_Comm_Tlv_GetVal\n"));

			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Tlv_GetTidLen					= (INT_FUNC_INT*)GetProcAddress(m_hEMVL2KernelDLL,								_LT("?emvkrnl_Tlv_GetTidLen@@YAHH@Z"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Tlv_GetTidLen == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_Comm_Tlv_GetTidLen\n"));

			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Tlv_GetLen						= (INT_FUNC_PUCHAR_IPNT*)GetProcAddress(m_hEMVL2KernelDLL,						_LT("?emvkrnl_Tlv_GetLen@@YAHPAEPAH@Z"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Tlv_GetLen == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_Comm_Tlv_GetLen\n"));

			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Tlv_StoreVal					= (INT_FUNC_INT_INT_PUCHAR*)GetProcAddress(m_hEMVL2KernelDLL,					_LT("?emvkrnl_Tlv_StoreVal@@YAHHHPAE@Z"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Tlv_StoreVal == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_Comm_Tlv_StoreVal\n"));

			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Tlv_StoreValFromRecord			= (INT_FUNC_PUCHAR_INT_INT_INT*)GetProcAddress(m_hEMVL2KernelDLL,				_LT("?emvkrnl_Tlv_StoreValFromRecord@@YAHPAEHHH@Z"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Tlv_StoreValFromRecord == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_Comm_Tlv_StoreValFromRecord\n"));

			//-------------------------------------------------------------------
			// EMV Utility
			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Util_BINtoN						= (VOID_FUNC_INT_PUCHAR_INT*)GetProcAddress(m_hEMVL2KernelDLL,					_LT("?emvkrnl_Util_BINtoN@@YAXHPAEH@Z"));
			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Util_toYYYYMMDD					= (VOID_FUNC_PUCHAR_PUCHAR_INT*)GetProcAddress(m_hEMVL2KernelDLL,				_LT("?emvkrnl_Util_toYYYYMMDD@@YAXPAE0H@Z"));
			// pemvkrnl_Comm_Util_GetCNRealLen;				// 사용 안함
			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Util_RightJustify				= (VOID_FUNC_PUCHAR_INT_PUCHAR*)GetProcAddress(m_hEMVL2KernelDLL,				_LT("?emvkrnl_Util_RightJustify@@YAXPAEH0@Z"));
			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Util_Decompress					= (VOID_FUNC_PUCHAR_PUCHAR_INT*)GetProcAddress(m_hEMVL2KernelDLL,				_LT("?emvkrnl_Util_Decompress@@YAXPAE0H@Z"));
			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Util_Compress					= (VOID_FUNC_PUCHAR_PUCHAR_INT*)GetProcAddress(m_hEMVL2KernelDLL,				_LT("?emvkrnl_Util_Compress@@YAXPAE0H@Z"));		

			//-------------------------------------------------------------------
			// Other Functions
			// pemvkrnl_Comm_Util_GetAPDUSW;				// 사용 안함
			// pemvkrnl_Comm_Util_GetVersion;				// 사용 안함
			// pemvkrnl_V5_Util_SetKernelLogPath;			// 사용 안함

			//-------------------------------------------------------------------
			// CallBack Function
			m_EMVL2KernelDll_FUNC.pComm_RegistIFMCommFunc					= (INT_FUNC_INT_CALLBACK*)GetProcAddress(m_hEMVL2KernelDLL,							_LT("?RegistIFMCommFunc@@YAHP6AHEPAEI0PAH@Z@Z"));		
			if (m_EMVL2KernelDll_FUNC.pComm_RegistIFMCommFunc == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pComm_RegistIFMCommFunc\n"));

		}
		break;

	case EMV_KERNEL_V5:
	case EMV_KERNEL_V6:
	case EMV_KERNEL_V7:		// [#GLDV-2522] NH Kook 2019.07.01 Support EMV Kernel V7
		{
			// [#2435] NH KSK 2016.07.14 Added EMV Kernel V6
			//			m_hEMVL2KernelDLL = ::LoadLibrary(_T(".\\EmvL2Kernel5.dll"));

			int loadedKernelVersion = -1;

			if (m_nEMVL2KernelVersion == EMV_KERNEL_V5)
			{
				m_hEMVL2KernelDLL = ::LoadLibrary(_T(".\\EmvL2Kernel5.dll"));
				loadedKernelVersion = 5;
			}
			else if (m_nEMVL2KernelVersion == EMV_KERNEL_V6)
			{
				m_hEMVL2KernelDLL = ::LoadLibrary(_T(".\\EmvL2Kernel6.dll"));
				loadedKernelVersion = 6;
			}
			// end of [#2435]
			// [#GLDV-2522] NH Kook 2019.07.01 Support EMV Kernel V7
			else
			{
				m_hEMVL2KernelDLL = ::LoadLibrary(_T(".\\EmvL2Kernel7.dll"));
				loadedKernelVersion = 7;
			}
			// end of [#GLDV-2522]

			if (m_hEMVL2KernelDLL == NULL)
			{
				RETAILMSG(1, (L"LoadLibrary EmvL2KErnel5.dll or EmvL2KErnel6.dll Error\n"));
				return -1;
			}

			RETAILMSG(1, (L"LoadLibrary EmvL2KErnel%d.dll OK\r\n", loadedKernelVersion));

			// Version5 or V6에 대해서 Function Pointer Get
			// EMV Application Selection
			// pemvkrnl_Comm_Sel_SetOnlyOneAID;				// 사용 안함
			m_EMVL2KernelDll_FUNC.pemvkrnl_V5_Sel_ApplicationSelection			= (INT_FUNC_UCHAR_EMVstLISTOFAIDV5_PINT*)GetProcAddress(m_hEMVL2KernelDLL,		_LT("emvkrnl_Sel_ApplicationSelection"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_V5_Sel_ApplicationSelection == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_V5_Sel_ApplicationSelection\n"));

			// pemvkrnl_V4_Sel_MakeDisplayAppList;			// 사용 안함
			m_EMVL2KernelDll_FUNC.pemvkrnl_V5_Sel_MakeDisplayAppList			= (INT_FUNC_UCHAR_USHORT_PEMVICCAPPSINFORMV5*)GetProcAddress(m_hEMVL2KernelDLL, _LT("emvkrnl_Sel_MakeDisplayAppList"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_V5_Sel_ApplicationSelection == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_V5_Sel_ApplicationSelection\n"));

			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Sel_RemoveCandidateList			 = (VOID_FUNC_USHORT_USHORT_PUCHAR*)GetProcAddress(m_hEMVL2KernelDLL,			_LT("emvkrnl_Sel_RemoveCandidateList"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Sel_RemoveCandidateList == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_Comm_Sel_RemoveCandidateList\n"));

			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Sel_FinalAppSelection			= (INT_FUNC_USHORT_PUCHAR_PUCHAR*)GetProcAddress(m_hEMVL2KernelDLL,				_LT("emvkrnl_Sel_FinalAppSelection"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Sel_FinalAppSelection == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_Comm_Sel_FinalAppSelection\n"));

			// pemvkrnl_Comm_Sel_DescretionaryDataList;		// 사용 안함
			// pemvkrnl_V5_Sel_NDCAIDSelection;				// 사용 안함
			m_EMVL2KernelDll_FUNC.pemvkrnl_V5_Sel_ASI							= (VOID_FUNC_UCHAR*)GetProcAddress(m_hEMVL2KernelDLL,							_LT("emvkrnl_Sel_ASI"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_V5_Sel_ASI == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_V5_Sel_ASI\n"));

			// EMV Transaction
			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_InitApplication			= (INT_FUNC_VOID*)GetProcAddress(m_hEMVL2KernelDLL,								_LT("emvkrnl_Trans_InitApplication"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_InitApplication == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_Comm_Trans_InitApplication\n"));

			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_ReadAppData				= (INT_FUNC_VOID*)GetProcAddress(m_hEMVL2KernelDLL,								_LT("emvkrnl_Trans_ReadAppData"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_ReadAppData == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_Comm_Trans_ReadAppData\n"));

			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_OfflineDataAuth			= (INT_PEMVstCAPUBKEY*)GetProcAddress(m_hEMVL2KernelDLL,						_LT("emvkrnl_Trans_OfflineDataAuth"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_OfflineDataAuth == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_Comm_Trans_OfflineDataAuth\n"));

			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_ProcessRestrict			= (INT_FUNC_VOID*)GetProcAddress(m_hEMVL2KernelDLL,								_LT("emvkrnl_Trans_ProcessRestrict"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_ProcessRestrict == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_Comm_Trans_ProcessRestrict\n"));

			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_CardholderVerify			= (INT_FUNC_USHORT_PUCHAR*)GetProcAddress(m_hEMVL2KernelDLL,					_LT("emvkrnl_Trans_CardholderVerify"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_CardholderVerify == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_Comm_Trans_CardholderVerify\n"));

			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_TerminalRiskMgmt			= (INT_FUNC_UCHAR*)GetProcAddress(m_hEMVL2KernelDLL,							_LT("emvkrnl_Trans_TerminalRiskMgmt"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_TerminalRiskMgmt == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_Comm_Trans_TerminalRiskMgmt\n"));

			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_TerminalActionAnalysis	= (UCHAR_FUNC_VOID*)GetProcAddress(m_hEMVL2KernelDLL,							_LT("emvkrnl_Trans_TerminalActionAnalysis"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_TerminalActionAnalysis == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_Comm_Trans_TerminalActionAnalysis\n"));

			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_CardActionAnalysis		= (INT_FUNC_PUCHAR*)GetProcAddress(m_hEMVL2KernelDLL,							_LT("emvkrnl_Trans_CardActionAnalysis"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_CardActionAnalysis == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_Comm_Trans_CardActionAnalysis\n"));

			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_OnlineProcess				= (INT_FUNC_INT_UCHAR*)GetProcAddress(m_hEMVL2KernelDLL,						_LT("emvkrnl_Trans_OnlineProcess"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_OnlineProcess == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_Comm_Trans_OnlineProcess\n"));

			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_Completion				= (INT_FUNC_VOID*)GetProcAddress(m_hEMVL2KernelDLL,								_LT("emvkrnl_Trans_Completion"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_Completion == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_Comm_Trans_Completion\n"));

			// pemvkrnl_V5_Get_ISAPDUList;					// 사용 안함

			// for Tlv Function
			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Tlv_GetTidTag					= (VOID_FUNC_INT_PUCHAR*)GetProcAddress(m_hEMVL2KernelDLL,						_LT("emvkrnl_Tlv_GetTidTag"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Tlv_GetTidTag == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_Comm_Tlv_GetTidTag\n"));

			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Tlv_GetTid						= (INT_FUNC_PUCHAR_INT*)GetProcAddress(m_hEMVL2KernelDLL,						_LT("emvkrnl_Tlv_GetTid"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Tlv_GetTid == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_Comm_Tlv_GetTid\n"));

			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Tlv_GetTag						= (INT_FUNC_PUCHAR_PUCHAR*)GetProcAddress(m_hEMVL2KernelDLL,					_LT("emvkrnl_Tlv_GetTag"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Tlv_GetTag == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_Comm_Tlv_GetTag\n"));

			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Tlv_GetVal						= (INT_FUNC_INT_PINT_PUCHAR*)GetProcAddress(m_hEMVL2KernelDLL,					_LT("emvkrnl_Tlv_GetVal"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Tlv_GetVal == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_Comm_Tlv_GetVal\n"));

			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Tlv_GetTidLen					= (INT_FUNC_INT*)GetProcAddress(m_hEMVL2KernelDLL,								_LT("emvkrnl_Tlv_GetTidLen"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Tlv_GetTidLen == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_Comm_Tlv_GetTidLen\n"));

			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Tlv_GetLen						= (INT_FUNC_PUCHAR_IPNT*)GetProcAddress(m_hEMVL2KernelDLL,						_LT("emvkrnl_Tlv_GetLen"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Tlv_GetLen == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_Comm_Tlv_GetLen\n"));

			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Tlv_StoreVal					= (INT_FUNC_INT_INT_PUCHAR*)GetProcAddress(m_hEMVL2KernelDLL,					_LT("emvkrnl_Tlv_StoreVal"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Tlv_StoreVal == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_Comm_Tlv_StoreVal\n"));

			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Tlv_StoreValFromRecord			= (INT_FUNC_PUCHAR_INT_INT_INT*)GetProcAddress(m_hEMVL2KernelDLL,				_LT("emvkrnl_Tlv_StoreValFromRecord"));
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Tlv_StoreValFromRecord == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pemvkrnl_Comm_Tlv_StoreValFromRecord\n"));

			//-------------------------------------------------------------------
			// EMV Utility
			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Util_BINtoN						= (VOID_FUNC_INT_PUCHAR_INT*)GetProcAddress(m_hEMVL2KernelDLL,					_LT("emvkrnl_Util_BINtoN"));
			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Util_toYYYYMMDD					= (VOID_FUNC_PUCHAR_PUCHAR_INT*)GetProcAddress(m_hEMVL2KernelDLL,				_LT("emvkrnl_Util_toYYYYMMDD"));
			// pemvkrnl_Comm_Util_GetCNRealLen;				// 사용 안함
			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Util_RightJustify				= (VOID_FUNC_PUCHAR_INT_PUCHAR*)GetProcAddress(m_hEMVL2KernelDLL,				_LT("emvkrnl_Util_RightJustify"));
			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Util_Decompress					= (VOID_FUNC_PUCHAR_PUCHAR_INT*)GetProcAddress(m_hEMVL2KernelDLL,				_LT("emvkrnl_Util_Decompress"));
			m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Util_Compress					= (VOID_FUNC_PUCHAR_PUCHAR_INT*)GetProcAddress(m_hEMVL2KernelDLL,				_LT("emvkrnl_Util_Compress"));		
			m_EMVL2KernelDll_FUNC.pemvkrnl_V5_Trans_Initial						= (VOID_FUNC_VOID*)GetProcAddress(m_hEMVL2KernelDLL,							_LT("emvkrnl_Trans_Initial"));

			// [#2435] NH KSK 2016.07.14
			if (m_nEMVL2KernelVersion == EMV_KERNEL_V6
				|| m_nEMVL2KernelVersion == EMV_KERNEL_V7)						// [#GLDV-2522] NH Kook 2019.07.01 Support EMV Kernel V7
				m_EMVL2KernelDll_FUNC.emvkrnl_V6_Util_SetDebugFlag				= (VOID_FUNC_INT*)GetProcAddress(m_hEMVL2KernelDLL,								_LT("emvkrnl_Util_SetDebugFlag"));
			// end of [#2435]

			//-------------------------------------------------------------------
			// Other Functions
			// pemvkrnl_Comm_Util_GetAPDUSW;				// 사용 안함
			// pemvkrnl_Comm_Util_GetVersion;				// 사용 안함
			// pemvkrnl_V5_Util_SetKernelLogPath;			// 사용 안함

			//-------------------------------------------------------------------
			// CallBack Function

			// [#GLDV-2522] NH Kook 2019.07.01 Support EMV Kernel V7
			if (m_nEMVL2KernelVersion == EMV_KERNEL_V7)
				m_EMVL2KernelDll_FUNC.pComm_RegistIFMCommFunc					= (INT_FUNC_INT_CALLBACK*)GetProcAddress(m_hEMVL2KernelDLL,							_LT("emvkrnl_RegistIFMCommFunc"));		
			else
			// end of [#GLDV-2522]
				m_EMVL2KernelDll_FUNC.pComm_RegistIFMCommFunc					= (INT_FUNC_INT_CALLBACK*)GetProcAddress(m_hEMVL2KernelDLL,							_LT("RegistIFMCommFunc"));		
			if (m_EMVL2KernelDll_FUNC.pComm_RegistIFMCommFunc == NULL)
				RETAILMSG(1, (L"LoadEMV Function Error pComm_RegistIFMCommFunc\n"));

		}
		break;

	default:
		{
			::MessageBox(NULL, _T("ERROR"), _T("EMV L2 Kernel Version Error"), MB_OK);
			return -1;
		}
		break;
	}

	// Validation Check
	// end of [#2188]

	// [#2188] NH KSK 2013.04.26
	//RegistIFMCommFunc(libSMC_SendApdu);
	if (m_EMVL2KernelDll_FUNC.pComm_RegistIFMCommFunc != NULL)
	{
		m_EMVL2KernelDll_FUNC.pComm_RegistIFMCommFunc(libSMC_SendApdu);

//		if (m_nEMVL2KernelVersion == EMV_KERNEL_V5)
//		if (m_nEMVL2KernelVersion == EMV_KERNEL_V5 || m_nEMVL2KernelVersion == EMV_KERNEL_V6)											// [#2435] NH KSK 2016.07.014 Added EMV Kernel V6
		if (m_nEMVL2KernelVersion == EMV_KERNEL_V5 || m_nEMVL2KernelVersion == EMV_KERNEL_V6 || m_nEMVL2KernelVersion == EMV_KERNEL_V7)	// [#GLDV-2522] NH Kook 2019.07.01 Support EMV Kernel V7
		{
			if (m_EMVL2KernelDll_FUNC.pemvkrnl_V5_Trans_Initial != NULL)
				m_EMVL2KernelDll_FUNC.pemvkrnl_V5_Trans_Initial();

			// [#2435] NH KSK 2016.07.14 V6부터는 Debug / Release Dll이 하나로 통합됨
			#if (EMV_TEST_MODE)
//			if (m_nEMVL2KernelVersion == EMV_KERNEL_V6)
			if (m_nEMVL2KernelVersion == EMV_KERNEL_V6 || m_nEMVL2KernelVersion == EMV_KERNEL_V7)		// [#GLDV-2522] NH Kook 2019.07.01 Support EMV Kernel V7
			{
				if (m_EMVL2KernelDll_FUNC.emvkrnl_V6_Util_SetDebugFlag != NULL)
					m_EMVL2KernelDll_FUNC.emvkrnl_V6_Util_SetDebugFlag(1);
			}
			#endif
			// end of [#2435]
		}
	}
	// end of [#2188]

	m_bPartialAIDSupport = TRUE;	// [#2188] NH KSK 2013.05.07 Default는 SUPPORT (필드발행버전에는 무조건 SUPPORT로 해야함(필드 호환)
	return 0;
}

// [#2188] NH KSK 2013.04.24
/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnEMV_SetKernelVersion
 RETURN TYPE  : 
 PARAMETER    : Version값
 DESCRIPTION  : EMV Kernel Version을 Set하는 함수.
-------------------------------------------------------------------*/
int CDevCmn::fnEMV_SetKernelVersion(int nVersion)
{
	m_nEMVL2KernelVersion = nVersion;
	return 0;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnEMV_GetKernelVersion
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : EMV Kernel Version을 Get하는 함수.
-------------------------------------------------------------------*/
int	CDevCmn::fnEMV_GetKernelVersion()
{
	return m_nEMVL2KernelVersion;
}
// end of [#2188]

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnEMV_Sel_ApplicationSelection
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 응용 프로그램을 선택하기 위한 Candidate List를 만드는 함수.
-------------------------------------------------------------------*/
int CDevCmn::fnEMV_Sel_ApplicationSelection(unsigned char SupportPSE, EMVst_LIST_OF_AID *AIDList, int *CandidateCnt)
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnEMV_Sel_ApplicationSelection]\n"));

	int	nRes = EMV_RSLT_FAIL;

	// [#2188] NH KSK 2013.04.26
//	nRes = emvkrnl_Sel_ApplicationSelection(SupportPSE, AIDList, CandidateCnt);
	if (m_EMVL2KernelDll_FUNC.pemvkrnl_V4_Sel_ApplicationSelection != NULL)
		nRes = m_EMVL2KernelDll_FUNC.pemvkrnl_V4_Sel_ApplicationSelection(SupportPSE, AIDList, CandidateCnt);
	// end of [#2188]

	NHDEBUG(DBG_INFO, (L"[CDevCmn::fnEMV_Sel_ApplicationSelection] nRes [%d]\n", nRes));

	return nRes;
}

// [#2188] NH KSK 2013.04.26
/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnEMV_V5_Sel_ApplicationSelection
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 응용 프로그램을 선택하기 위한 Candidate List를 만드는 함수.
-------------------------------------------------------------------*/
int CDevCmn::fnEMV_V5_Sel_ApplicationSelection(unsigned char SupportPSE, EMVst_LIST_OF_AID_V5 *AIDList_V5, int *CandidateCnt,  BOOL bPartialSupport)
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnEMV_V5_Sel_ApplicationSelection]\n"));

	int	nRes = EMV_RSLT_FAIL;

	// V5인 경우에는 Partial Support가 Option 처리되어야함 (변경된 인증항목임)
	if (bPartialSupport == TRUE)
	{
		NHDEBUG(DBG_CALL, (L"[CDevCmn::pemvkrnl_V5_Sel_ASI PARTIAL_AID_SUPPOPT]\n"));
		if (m_EMVL2KernelDll_FUNC.pemvkrnl_V5_Sel_ASI != NULL)
			m_EMVL2KernelDll_FUNC.pemvkrnl_V5_Sel_ASI(PARTIAL_AID_SUPPOPT);
	}
	else
	{
		NHDEBUG(DBG_CALL, (L"[CDevCmn::pemvkrnl_V5_Sel_ASI NOT_SUPPORT]\n"));
		if (m_EMVL2KernelDll_FUNC.pemvkrnl_V5_Sel_ASI != NULL)
			m_EMVL2KernelDll_FUNC.pemvkrnl_V5_Sel_ASI(NOT_SUPPORT);
	}

	if (m_EMVL2KernelDll_FUNC.pemvkrnl_V5_Sel_ApplicationSelection != NULL)
		nRes = m_EMVL2KernelDll_FUNC.pemvkrnl_V5_Sel_ApplicationSelection(SupportPSE, AIDList_V5, CandidateCnt);

	//nRes = emvkrnl_Sel_ApplicationSelection(SupportPSE, AIDList_V5, CandidateCnt);

	NHDEBUG(DBG_INFO, (L"[CDevCmn::fnEMV_V5_Sel_ApplicationSelection] nRes [%d]\n", nRes));

	return nRes;
}
// end of [#2188]

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnEMV_Sel_MakeDisplayAppList
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Application list를 작성하는 함수.
-------------------------------------------------------------------*/
int CDevCmn::fnEMV_Sel_MakeDisplayAppList(unsigned char SupportUserConfirm, unsigned short CandidateCnt,
											 EMV_ICC_APPS_INFORM *IccAppInform)
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnEMV_Sel_MakeDisplayAppList]\n"));

	int	nRes = EMV_RSLT_FAIL;

	// [#2188] NH KSK 2013.04.26
//	nRes = emvkrnl_Sel_MakeDisplayAppList(SupportUserConfirm, CandidateCnt, IccAppInform);
	if (m_EMVL2KernelDll_FUNC.pemvkrnl_V4_Sel_MakeDisplayAppList != NULL)
		nRes = m_EMVL2KernelDll_FUNC.pemvkrnl_V4_Sel_MakeDisplayAppList(SupportUserConfirm, CandidateCnt, IccAppInform);
	// end of [#2188]

	NHDEBUG(DBG_INFO, (L"[CDevCmn::fnEMV_Sel_MakeDisplayAppList] nRes [%d]\n", nRes));

	return nRes;
}

// [#2188] NH KSK 2013.04.26
/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnEMV_V5_Sel_MakeDisplayAppList
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Application list를 작성하는 함수.
-------------------------------------------------------------------*/
int CDevCmn::fnEMV_V5_Sel_MakeDisplayAppList(unsigned char SupportUserConfirm, unsigned short CandidateCnt,
											 EMV_ICC_APPS_INFORM_V5 *IccAppInform_V5)
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnEMV_V5_Sel_MakeDisplayAppList]\n"));

	int	nRes = EMV_RSLT_FAIL;

	if (m_EMVL2KernelDll_FUNC.pemvkrnl_V5_Sel_MakeDisplayAppList != NULL)
		nRes = m_EMVL2KernelDll_FUNC.pemvkrnl_V5_Sel_MakeDisplayAppList(SupportUserConfirm, CandidateCnt, IccAppInform_V5);

	NHDEBUG(DBG_INFO, (L"[CDevCmn::fnEMV_V5_Sel_MakeDisplayAppList] nRes [%d]\n", nRes));

	return nRes;
}
// end of [#2188]

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fvEMV_Sel_RemoveCandidateList
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Selection을 실패한 AID를 Candidate List에서 삭제한 후에
				재 구성하는 함수.
-------------------------------------------------------------------*/
void CDevCmn::fvEMV_Sel_RemoveCandidateList(unsigned short CandidateCnt, unsigned short AidLen, unsigned char *AIDVal)
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fvEMV_Sel_RemoveCandidateList]\n"));

	// [#2188] NH KSK 2013.04.26
//	emvkrnl_Sel_RemoveCandidateList(CandidateCnt, AidLen, AIDVal);
	if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Sel_RemoveCandidateList != NULL)
		m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Sel_RemoveCandidateList(CandidateCnt, AidLen, AIDVal);
	// end of [#2188]
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnEMV_Sel_FinalAppSelection
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 사용자가 선택한 Application을 EMV kernel에 등록하는 함수.
-------------------------------------------------------------------*/
int CDevCmn::fnEMV_Sel_FinalAppSelection(unsigned short AidLen, unsigned char *AIDVal, unsigned char *SW)
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnEMV_Sel_FinalAppSelection ]\n"));

	int nRes = EMV_RSLT_FAIL;

	// [#2188] NH KSK 2013.04.26
//	nRes = emvkrnl_Sel_FinalAppSelection(AidLen, AIDVal, SW);
	if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Sel_FinalAppSelection != NULL)
		nRes = m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Sel_FinalAppSelection(AidLen, AIDVal, SW);
	// end of [#2188]

	NHDEBUG(DBG_INFO, (L"[CDevCmn::fnEMV_Sel_FinalAppSelection] nRes [%d]\n", nRes));

	return nRes;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnEMV_Sel_DescretionaryDataList
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : AID별 정보를 추출하는 함수 (CANADA 특이사양)
-------------------------------------------------------------------*/
void CDevCmn::fnEMV_Sel_DescretionaryDataList(EMVst_ICC_DESCDATA_INFORM *EmvIccDescInform)
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnEMV_Sel_DescretionaryDataList]\n"));

	// [#2188] NH KSK 2013.04.26
//	emvkrnl_Sel_DescretionaryDataList(EmvIccDescInform);
	if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Sel_DescretionaryDataList != NULL)
		m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Sel_DescretionaryDataList(EmvIccDescInform);
	// end of [#2188]

	NHDEBUG(DBG_INFO, (L"[CDevCmn::fnEMV_Sel_DescretionaryDataList] return \n"));
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnEMV_Trans_InitApplication
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 선택된 Application으로 GetProcessing Option을 수행한다.
-------------------------------------------------------------------*/
int CDevCmn::fnEMV_Trans_InitApplication(void)
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnEMV_Trans_InitApplication]\n"));

	int nRes = EMV_RSLT_FAIL;

	// [#2188] NH KSK 2013.04.26
//	nRes = emvkrnl_Trans_InitApplication();
	if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_InitApplication != NULL)
		nRes = m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_InitApplication();
	// end of [#2188]

	NHDEBUG(DBG_INFO, (L"[CDevCmn::fnEMV_Trans_InitApplication] nRes [%d]\n", nRes));

	return nRes;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnEMV_Trans_ReadAppData
 RETURN TYPE  : 				
 PARAMETER    : 
 DESCRIPTION  : 선택된 Application의 레코드를 읽는 함수.
-------------------------------------------------------------------*/
int	CDevCmn::fnEMV_Trans_ReadAppData(void)
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnEMV_Trans_ReadAppData]\n"));

	int nRes = EMV_RSLT_FAIL;

	// [#2188] NH KSK 2013.04.26
//	nRes = emvkrnl_Trans_ReadAppData();
	if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_ReadAppData != NULL)
		nRes = m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_ReadAppData();
	// end of [#2188]

	NHDEBUG(DBG_INFO, (L"[CDevCmn::fnEMV_Trans_ReadAppData] nRes [%d]\n", nRes));

	return nRes;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnEMV_Trans_OfflineDataAuth
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Offline Authentification을 수행하는 함수
-------------------------------------------------------------------*/
int	CDevCmn::fnEMV_Trans_OfflineDataAuth(EMVst_CA_PUBKEY *stCAPKey)
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnEMV_Trans_OfflineDataAuth]\n"));

	int	nRes = EMV_RSLT_FAIL;

	// [#2188] NH KSK 2013.04.26
//	nRes = emvkrnl_Trans_OfflineDataAuth(stCAPKey);
	if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_OfflineDataAuth != NULL)
		nRes = m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_OfflineDataAuth(stCAPKey);
	// end of [#2188]

	NHDEBUG(DBG_INFO, (L"[CDevCmn::fnEMV_Trans_OfflineDataAuth] nRes [%d]\n", nRes));

	return nRes;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnEMV_Trans_ProcessRestrict
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 처리 제한
-------------------------------------------------------------------*/
int	CDevCmn::fnEMV_Trans_ProcessRestrict(void)
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnEMV_Trans_ProcessRestrict]\n"));

	int	nRes = EMV_RSLT_FAIL;

	// [#2188] NH KSK 2013.04.30
//	nRes = emvkrnl_Trans_ProcessRestrict();
	if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_ProcessRestrict != NULL)
		nRes = m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_ProcessRestrict();
	// end of [#2188]

	NHDEBUG(DBG_INFO, (L"[CDevCmn::fnEMV_Trans_ProcessRestrict] nRes [%d]\n", nRes));

	return nRes;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnEMV_Trans_CardholderVerify
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 카드 소지자 검증 처리 
				Online Enciphered PIN 입력 후 처리.
-------------------------------------------------------------------*/
int	CDevCmn::fnEMV_Trans_CardholderVerify(unsigned short OnlinePinLen, unsigned char *EncipheredOnlinePIN)
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnEMV_Trans_CardholderVerify]\n"));

	int nRes = EMV_RSLT_FAIL;

	// [#2188] NH KSK 2013.04.30
//	nRes = emvkrnl_Trans_CardholderVerify(OnlinePinLen, EncipheredOnlinePIN);
	if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_CardholderVerify != NULL)
		nRes = m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_CardholderVerify(OnlinePinLen, EncipheredOnlinePIN);
	// end of [#2188]

	NHDEBUG(DBG_INFO, (L"[CDevCmn::fnEMV_Trans_CardholderVerify] nRes [%d]\n", nRes));

	return nRes;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnEMV_Trans_TerminalRiskMgmt
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 단말기 위험 관리
-------------------------------------------------------------------*/
int	CDevCmn::fnEMV_Trans_TerminalRiskMgmt(unsigned char SupportForceOnline)
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnEMV_Trans_TerminalRiskMgmt]\n"));

	int nRes = EMV_RSLT_FAIL;

	// [#2188] NH KSK 2013.04.30
//	nRes = emvkrnl_Trans_TerminalRiskMgmt(SupportForceOnline);
	if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_TerminalRiskMgmt != NULL)
		nRes = m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_TerminalRiskMgmt(SupportForceOnline);
	// end of [#2188]

	NHDEBUG(DBG_INFO, (L"[CDevCmn::fnEMV_Trans_TerminalRiskMgmt] nRes [%d]\n", nRes));

	return nRes;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fcEMV_Trans_TerminalActionAnalysis
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
unsigned char	CDevCmn::fcEMV_Trans_TerminalActionAnalysis(void)
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fcEMV_Trans_TerminalActionAnalysis]\n"));

	unsigned char	cRes = EMV_RSLT_FAIL;

	// [#2188] NH KSK 2013.04.30
//	cRes = emvkrnl_Trans_TerminalActionAnalysis();
	if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_TerminalActionAnalysis != NULL)
		cRes = m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_TerminalActionAnalysis();
	// end of [#2188]

	NHDEBUG(DBG_INFO, (L"[CDevCmn::fcEMV_Trans_TerminalActionAnalysis] cRes [0x%X]\n", cRes));

	return cRes;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnEMV_Trans_CardActionAnalysis
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Card Action Analysis
-------------------------------------------------------------------*/
int	CDevCmn::fnEMV_Trans_CardActionAnalysis(unsigned char *OnlineFlag)
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnEMV_Trans_CardActionAnalysis]\n"));

	int	nRes = EMV_RSLT_FAIL;

	// [#2188] NH KSK 2013.04.30
//	nRes = emvkrnl_Trans_CardActionAnalysis(OnlineFlag);
	if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_CardActionAnalysis != NULL)
		nRes = m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_CardActionAnalysis(OnlineFlag);
	// end of [#2188]

	NHDEBUG(DBG_INFO, (L"[CDevCmn::fnEMV_Trans_CardActionAnalysis] nRes [%d] OnlineFlag[%02X]\n", nRes, *OnlineFlag));

	return nRes;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnEMV_Trans_OnlineProcess
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Set Result of Online Process to EMV Kernel
-------------------------------------------------------------------*/
int	CDevCmn::fnEMV_Trans_OnlineProcess(int OnlineMode, unsigned char Acquirer_CID)
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnEMV_Trans_OnlineProcess]\n"));

	int	nRes = EMV_RSLT_FAIL;

	// [#2188] NH KSK 2013.04.30
//	nRes = emvkrnl_Trans_OnlineProcess(OnlineMode, Acquirer_CID);
	if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_OnlineProcess != NULL)
		nRes = m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_OnlineProcess(OnlineMode, Acquirer_CID);
	// end of [#2188]

	NHDEBUG(DBG_INFO, (L"[CDevCmn::fnEMV_Trans_OnlineProcess] nRes [%d]\n", nRes));

	return nRes;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnEMV_Trans_Completion
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : Complete Process
-------------------------------------------------------------------*/
int	CDevCmn::fnEMV_Trans_Completion(void)
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnEMV_Trans_Completion]\n"));

	int	nRes = EMV_RSLT_FAIL;

	// [#2188] NH KSK 2013.04.30
//	nRes = emvkrnl_Trans_Completion();
	if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_Completion != NULL)
		nRes = m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Trans_Completion();
	// end of [#2188]

	NHDEBUG(DBG_INFO, (L"[CDevCmn::fnEMV_Trans_Completion] nRes [%d]\n", nRes));

	return nRes;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fvEMV_Tlv_GetTidTag
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CDevCmn::fvEMV_Tlv_GetTidTag(int tid, unsigned char *tag)
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fvEMV_Tlv_GetTidTag]\n"));

	// [#2188] NH KSK 2013.04.30
//	emvkrnl_Tlv_GetTidTag(tid, tag);
	if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Tlv_GetTidTag != NULL)
		m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Tlv_GetTidTag(tid, tag);
	// end of [#2188]
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnEMV_Tlv_GetTag
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
int	CDevCmn::fnEMV_Tlv_GetTag(unsigned char *buf, unsigned char *tag)
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnEMV_Tlv_GetTag]\n"));

	int	nRes = EMV_RSLT_FAIL;

	// [#2188] NH KSK 2013.04.30
//	nRes = emvkrnl_Tlv_GetTag(buf, tag);
	if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Tlv_GetTag != NULL)
		nRes = m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Tlv_GetTag(buf, tag);
	// end of [#2188]

	NHDEBUG(DBG_INFO, (L"[CDevCmn::fnEMV_Tlv_GetTag] nRes [%d]\n", nRes));

	return nRes;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnEMV_Tlv_GetVal
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
int	CDevCmn::fstrEMV_Tlv_GetVal(int nID, int *len, unsigned char *pData)
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnEMV_Tlv_GetVal] nID(%d)\n", nID));

	int nRes = EMV_RSLT_FAIL;
	*len = 0;

	// [#2188] NH KSK 2013.04.30
//	nRes = emvkrnl_Tlv_GetVal(nID, len, pData);
	if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Tlv_GetVal != NULL)
		nRes = m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Tlv_GetVal(nID, len, pData);
	// end of [#2188]

	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnEMV_Tlv_GetVal] nRes [%d]\n", nRes));
	
	return nRes;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnEMV_Tlv_GetTidLen
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
int	CDevCmn::fnEMV_Tlv_GetTidLen(int tid)
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnEMV_Tlv_GetTidLen]\n"));

	int	nRes = EMV_RSLT_FAIL;

	// [#2188] NH KSK 2013.04.30
//	nRes = emvkrnl_Tlv_GetTidLen(tid);
	if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Tlv_GetTidLen != NULL)
		nRes = m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Tlv_GetTidLen(tid);
	// end of [#2188]

	NHDEBUG(DBG_INFO, (L"[CDevCmn::fnEMV_Tlv_GetTidLen] nRes [%d]\n", nRes));

	return nRes;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnEMV_Tlv_StoreVal
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
int	CDevCmn::fnEMV_Tlv_StoreVal(int tid, int len, unsigned char *val)
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnEMV_Tlv_StoreVal]\n"));

	int	nRes = EMV_RSLT_FAIL;

	// [#2188] NH KSK 2013.04.30
//	nRes = emvkrnl_Tlv_StoreVal(tid, len, val);
	if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Tlv_StoreVal != NULL)
		nRes = m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Tlv_StoreVal(tid, len, val);
	// end of [#2188]

	NHDEBUG(DBG_INFO, (L"[CDevCmn::fnEMV_Tlv_StoreVal] nRes [%d]\n", nRes));

	return nRes;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fnEMV_Tlv_StoreValFromRecord
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
int	CDevCmn::fnEMV_Tlv_StoreValFromRecord(unsigned char *record, int type, int inlen, int source)
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fnEMV_Tlv_StoreValFromRecord]\n"));

	int	nRes = EMV_RSLT_FAIL;

	// [#2188] NH KSK 2013.04.30
//	nRes = emvkrnl_Tlv_StoreValFromRecord(record, type, inlen, source);
	if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Tlv_StoreValFromRecord != NULL)
		m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Tlv_StoreValFromRecord(record, type, inlen, source);
	// end of [#2188]

	NHDEBUG(DBG_INFO, (L"[CDevCmn::fnEMV_Tlv_StoreValFromRecord] nRes [%d]\n", nRes));

	return nRes;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fvEMV_Util_BINtoN
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CDevCmn::fvEMV_Util_BINtoN(int val, unsigned char *data, int len)
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fvEMV_Util_BINtoN]\n"));

	// [#2188] NH KSK 2013.04.30
//	emvkrnl_Util_BINtoN(val, data, len);
	if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Util_BINtoN != NULL)
		m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Util_BINtoN(val, data, len);
	// end of [#2188]
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fvEMV_Util_toYYYYMMDD
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CDevCmn::fvEMV_Util_toYYYYMMDD(unsigned char *data, unsigned char *out, int format)
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fvEMV_Util_toYYYYMMDD]\n"));

	//[#2188] NH KSK 2013.04.30
//	emvkrnl_Util_toYYYYMMDD(data, out, format);
	if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Util_toYYYYMMDD != NULL)
		m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Util_toYYYYMMDD(data, out, format);
	// end of [#2188]
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fcEMV_Util_GetCNRealLen
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
unsigned char CDevCmn::fcEMV_Util_GetCNRealLen(unsigned char *data, int len)
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fcEMV_Util_GetCNRealLen]\n"));

// 	unsigned char	cRes;
	unsigned char	cRes = '\0';	// CodeSonar 지적사항

	// [#2188] NH KSK 2013.04.30
//	cRes = emvkrnl_Util_GetCNRealLen(data, len);
	if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Util_GetCNRealLen != NULL)
		cRes = m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Util_GetCNRealLen(data, len);
	// end of [#2188]

	NHDEBUG(DBG_INFO, (L"[CDevCmn::fcEMV_Util_GetCNRealLen] nRes [0x%X]\n", cRes));

	return cRes;
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fvEMV_Util_RightJustify
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CDevCmn::fvEMV_Util_RightJustify(unsigned char *data, int len, unsigned char *dest)
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fvEMV_Util_RightJustify]\n"));

	// [#2188] NH KSK 2013.04.30
//	emvkrnl_Util_RightJustify(data, len, dest);
	if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Util_RightJustify != NULL)
		m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Util_RightJustify(data, len, dest);
	// end of [#2188]
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fvEMV_Util_Decompress
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CDevCmn::fvEMV_Util_Decompress(unsigned char *bySource, unsigned char *byDestination, int byCount)
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fvEMV_Util_Decompress]\n"));

	// [#2188] NH KSK 2013.04.30
//	emvkrnl_Util_Decompress(bySource, byDestination, byCount);
	if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Util_Decompress != NULL)
		m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Util_Decompress(bySource, byDestination, byCount);
	// end of [#2188]
}

/*-------------------------------------------------------------------
 CLASS    NAME: CDevCmn
 FUNCTION NAME: fvEMV_Util_Compress
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
void CDevCmn::fvEMV_Util_Compress(unsigned char *bySource, unsigned char *byDestination, int byCount)
{
	NHDEBUG(DBG_CALL, (L"[CDevCmn::fvEMV_Util_Compress]\n"));

	// [#2188] NH KSK 2013.04.30
//	emvkrnl_Util_Compress(bySource, byDestination, byCount);
	if (m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Util_Compress != NULL)
		m_EMVL2KernelDll_FUNC.pemvkrnl_Comm_Util_Compress(bySource, byDestination, byCount);
	// end of [#2188]
}

void CDevCmn::fnEMV_Trans_Initial()
{
	if (m_EMVL2KernelDll_FUNC.pemvkrnl_V5_Trans_Initial != NULL)
		m_EMVL2KernelDll_FUNC.pemvkrnl_V5_Trans_Initial();
}

/*-------------------------------------------------------------------
 CLASS    NAME: ---
 FUNCTION NAME: libSMC_SendApdu
 RETURN TYPE  : 
 PARAMETER    : 
 DESCRIPTION  : 
-------------------------------------------------------------------*/
int	CALLBACK libSMC_SendApdu(BYTE dCode, BYTE *apdu, UINT apduLen, BYTE *resp, int *rlen)
{
	int			iResult = 0;
	CString		strUnpackMsg;								// 송신 메세지 Unpack용 데이터
	CString		strTempRecv;								// Unpack 수신 메세지 Row데이터

	HexaDump(apdu, apduLen, 3, TRUE, FILE_LOG_TYPE);	// [#2452] NH KSK 2016.11.10

	strUnpackMsg.Format(L"%s", MakeUnPack(apdu, apduLen));			// 송신 메세지를 Unpack한다.

//	NHDEBUG(DBG_CALL, (_T("[libSMC_SendApdu] dCode(0x%02x) apdu(%s) apduLen(%d) \n"), dCode, strUnpackMsg, apduLen));

	strTempRecv = g_pMwi->EMV_SendRecvMsgWithIFM(0, (LPCTSTR)strUnpackMsg, K_30_WAIT);
																	// 메세지 송신하고 응답을 수신 받으면 리턴하는 함수

//	NHDEBUG(DBG_INFO, (L"[libSMC_SendApdu] Return [%s]\n", strTempRecv));

	if (strTempRecv.GetLength() > 0)
	{
		char szTemp[1024];
		memset(szTemp, NULL, sizeof(szTemp));
		WideToMulti(szTemp, strTempRecv, sizeof(szTemp));

		*rlen = MakePack(szTemp, resp, wcslen(strTempRecv));		// 수신된 메세지를 Pack한다.

		HexaDump(resp, *rlen, 4, TRUE, FILE_LOG_TYPE);	// [#2452] NH KSK 2016.11.10

		iResult = 1;												// RSLT_SUCCESS
	}
	else
	{
		iResult = -1;												// RSLT_ERROR
	}

	return iResult;
}