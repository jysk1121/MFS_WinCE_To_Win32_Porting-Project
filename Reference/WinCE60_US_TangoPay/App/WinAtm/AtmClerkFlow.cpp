/////////////////////////////////////////////////////////////////////////////
// AtmClerkCtrl.cpp : Implementation of the CWinAtmCtrl ActiveX Control class.
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "MainFrm.h"
#include "AtmClerkCtrl.h"
#include ".\Tran\B4UService.h"
#include ".\Dev\LoginManager.h"

//------------------------------------------------------------------
//	Define Debug Flag
//------------------------------------------------------------------

//#define NH_DEBUG		// Debug Message On/Off

#include ".\Common\NHDbgApi.h"

#define	DBG_CALL		1
#define DBG_INFO		1
#define FIRST_TIME_LOGIN 0

// SCREEN 사용 번호 정리 KSK 2008.6.13
/*
// 700번대는 WINCE 6.0만 사용
case 701:	ClerkQuickReplenish();				break;
case 702:	ClerkQuickDayTotal();				break;
case 703:	ClerkQuickCassetteTotal();			break;
case 704:	ClerkQuickAddCash();				break;
case 705:	ClerkQuickResult();					break;
case 706:	ClerkResetMasterPassword();			break;	[#2070] 사용 안함으로 변경
case 707:
case 708:
case 709:	후면기에서 사용
case 710:	후면기에서 사용

case 711:	ClerkExitSupervisor();				break;
case 712:	ClerkQuickConfigurationProc();		break;
case 713:	ClerkQuickConfCancelConfirmProc();	break;
case 714:	ClerkQuickCommunicationProc();		break;
case 715:	ClerkQuickTCPIPTypeProc();			break;
case 716:	ClerkQuickEotEnqOptionProc();		break;
case 717:	ClerkQuickMessageFormatProc();		break;
case 718:	ClerkQuickTelephoneNumberProc();	break;
case 719:	ClerkQuickProcessorProc();			break;
case 720:	ClerkQuickHostIpSetupProc();		break;
case 721:	ClerkQuickRemoteMonitorProc();		break;
case 722:	ClerkQuickChangeOperatorPasswordProc();	break;
case 723:	ClerkQuickATMDHCPSettingProc();		break;
case 724:	ClerkQuickAtmIpSetupProc();			break;
case 725:	ClerkQuickOperatorPasswordProc();	break;
case 726:	ClerkQuickChangeServicerPasswordProc();	break;
case 727:	ClerkQuickServicerPasswordProc();	break;
case 728:	ClerkQuickRevenueAddedFeatures1Proc();	break;
case 729:	ClerkQuickTerminalIDViewProc();		break;
case 730:	ClerkQuickRevenueAddedFeatures2Proc();	break;
case 731:	ClerkQuickRoutingIDViewProc();		break;
case 732:	ClerkQuickAcceptConfigurationProc();	break;
case 733:	ClerkQuickStandard3OptionProc();	break;
case 734:	ClerkQuickMasterPasswordProc();		break;
case 735:	ClerkQuickMasterPasswordProc();		break;
case 736:	ClerkQuickConfigResultViewProc();	break;

case 737 ~ 739: not used

case 740:	ClerkAUCountrySetup();				break;		// [#2042] AU KSK 2011.04.02 
case 741:	ClerkCardReaderSetupProc();			break;		// [#2073] NH KSK 2011.06.25
case 742:	ClerkKeyManagementProc();			break;		// [#2080] NH KSK 2011.06.27
case 743:	ClerkRKTSetupProc()					break;		// [#2080] NH KSK 2011.06.27
case 744:	ClerkJournalUploadToHostProc();		break;		// [#2076] NH KSK 2011.06.29
case 745:	ClerkAntiSkimmingSetupProc();		break;		// [#2074] NH KSK 2011.07.09
case 746:	ClerkDiagAntiSkimmingTestProc();	break;		// [#2078] NH KSK 2011.07.15
case 747:	ClerkHaloLEDSetupProc();			break;		// [#2205] US KSK 2013.06.28
case 748:	ClerkGivePaySetupProc();			break;		// [#2535] US Justin 2018.03.20 GivePay Online
case 749:	ClerkTDLOptionSetupProc();			break;		// [#2292] US Justin 2014.09.17 Add Cardtronics Additional TDL Settings
case 750: not used
case 751:	ClerkDiagnosticsRFIDCardScan();		break;		// [#2325] NH KSK 2015.01.25
case 752	ClerkDCCWithdrawalOptionProc();		break;		// [#2472] US Justin 2017.02.08 Add DCC Withdrawal Option
case 753:	ClerkPOPMoneySetupProc();			break;		// [#2350] US Justin 2015.06.22 Add PAI OP Setup
case 754: not used
case 756:	ClerkReceiptPaperCutSetupProc();	break;		// [#2371] US Justin 2015.09.08 Receipt Paper Cut Setup
case 757:	ClerkMoniMobileSetupProc();			break;		// [#2382] US Justin 2015.12.16 MoniMobile Setup
case 759:	ClerkHalCashSetupProc();			break;		// [#2396] US Justin 2016.02.17 Pin4
case 760: not used
case 761:	ClerkJustCashSetupProc();			break;		// [#2445] US Justin 2016.09.92 Add Just.Cash Setup
case 762:	ClerkPaypalCCASetupProc();			break;		// [#2446] US Justin 2016.09.29 Add Paypal CCA Setup
case 763:	ClerkPrintOptionProc();				break;		// [#2525] AU HJ AHN 2018.01.10 Print Option 
case 764:	ClerkCameraSetupProc();				break;		// [#2518] US Kook 2017.12.22 Support MX-2800SE
case 765:	ClerkDiagnosticsCamera();			break;		// [#2518] US Kook 2017.12.21 Support MX-2800SE
case 766:	ClerkPowerControlProc();			break;		// [#2536] NH Kook 2018.03.09 ATM Shutdown
case 767:	ClerkEMVOtherOptionsProc();			break;		// [#2549] NH Justin 2018.05.07 Add Perto Rico Option
case 768: not used
case 769:	ClerkB4USetupProc();				break;		// [#RWC6-16] Bitload 4 U. A new cryptocurrency feature for CE 6.0 ATMs

case 770:	ClerkDiagnosticsBarcodeReader();	break;		// [#GLDV-3005] US Kook 2021.10.07 Support Side Car
case 771:	ClerkDiagnosticsBillAcceptor();		break;		// [#GLDV-3005] US Kook 2021.10.07 Support Side Car
case 772:	ClerkSideCarSetupProc();			break;		// [#GLDV-3005] US Kook 2021.10.22 Support Side Car

case 773:	ClerkDispenseMixOptionProc();		break;		// [#2459] AU KSK 2016.12.23

case 774 ~ 790: not used

case 791:	ClerkDualHostDCCProc();				break;		// [#2185] US Justin 2013.04.10 Dual Host DCC Main Screen
case 792:	ClerkDualHostDCCHostIpSetupProc();	break;		// [#2185] US Justin 2013.04.10 Dual Host DCC Host Info
case 793:	ClerkDigitalReceiptOptionProc();	break;		// [#2263] CA Justin 2014.05.05 Digital Receipt Option, 2700(BMO), CANADA ONLY
case 794:	ClerkDCCCustomOptionProc();			break;		// [#2187] US Justin 2013.04.17 DCC

case 801 ~ 802: not used

case 803:	ClerkBasicCouponSettingProc();		break;
case 804:	ClerkAutoDayTotalProc();			break;
case 805:	ClerkBasicCouponTextProc();			break;			WINCE 5.0만 사용
case 806:	ClerkDeleteAllBinListProc();		break;			WINCE 5.0만 사용
case 807:	ClerkSelectMachineTypeProc();		break;
case 808:	ClerkSetDenominationProc_MX();		break;			WINCE 5.0만 사용
case 809:	ClerkExtendedBinList()				break;
case 810:	ClerkErrorCodeView();				break;

case 811:	ClerkSearchErrorCodeProc();			break;			WINCE 5,0만 사용
case 812:	ClerkErrorSumProc();				break;
case 813:	ClerkOptOutProc()					break;			
case 814:   ClerkMXPrintOptionProc();			break;			WINCE 5,0만 사용
case 815:	ClerkRMSRingCountProc();			break;
case 816:	ClerkSearchJournalbyDateProc();		break;
case 817:	ClerkPreDialOptionProc();			break;			// [#525] US KSK 2009.05.21
case 818:	ClerkRejectAnalysisProc();			break;			// [#526]
case 819:	ClerkNeedMoreTime();				break;			WINCE 5,0만 사용
case 820:	ClearkClearRejectAnalysisProc();	break;			WINCE 5,0만 사용

// [#540] HWANG 2008.07.01
case 821:	ClerkDiagonsticsMain();				break;
case 822:	ClerkDiagnosticsCardScan();			break;
case 823:	ClerkDiagnosticsModem();			break;
case 824:	ClerkDiagnosticsSensor();			break;
case 825:	ClerkDiagnosticsCDUSensorView();	break;
case 826:	ClerkDiagnosticsSPRSensorView();	break;
case 827:	ClerkDiagnosticsAuxiliaryUnit();	break;
case 828:	ClerkDiagnosticsFlicker();			break;
case 829:	ClerkDiagnosticsExitScreen();		break;
// end of [#540]

case 830:	ClerkSurchargeOwnerProc();			break;			WINCE 5.0만 사용
case 831:	ClerkBasicCoupon1Proc();			break;
case 832:	ClerkBasicCoupon2Proc();			break;
case 833:	ClerkBasicCoupon3Proc();			break;
case 834:	ClerkBasicCoupon4Proc();			break;	
case 835:	ClerkBasicCoupon5Proc();			break;
case 836:	ClerkBasicCoupon6Proc();			break;
case 837:	ClerkCommunicationIDProc();			break;			WINCE 5.0만 사용
case 838:	ClerkHostInitialStringProc();		break;			WINCE 5.0만 사용
case 839:	ClerkRMSInitialStringProc();		break;			WINCE 5.0만 사용

case 840:	ClerkTerminalIPProc();				break;			WINCE 5.0만 사용
case 841:	ClerkTerminalSubnetProc();			break;			WINCE 5.0만 사용
case 842:	ClerkTerminalGatewayProc();			break;			WINCE 5.0만 사용
case 843:	ClerkTerminalDNSProc();				break;			WINCE 5.0만 사용
case 844:	ClerkDeviceSetupProc();				break;
case 845:	ClerkCDUSetupProc();				break;
case 846:	ClerkSystemControlProc();			break;
case 847:	ClerkReceiptTailProc();				break;			WINCE 5.0만 사용 (충돌)
case 847:	ClerkChangBackgroundProc();			break;			WINCE 6.0만 사용 (충돌)
case 848:	ClerkEnhancedCouponText1Proc();		break;			WINCE 5.0만 사용 (충돌)
case 848:	ClerkDefaultBackgroundProc();		break;			WINCE 6.0만 사용 (충돌)
case 849:	ClerkEnhancedCouponText2Proc();		break;			WINCE 5.0만 사용 (충돌)
case 849:	ClerkBackScreen1Proc();				break;			WINCE 6.0만 사용 (충돌)

case 850:	ClerkHostAddress1Proc();			break;			WINCE 5.0만 사용 (충돌)
case 850:	ClerkBackScreen2Proc();				break;			WINCE 6.0만 사용 (충돌)
case 851:	ClerkHostAddress2Proc();			break;			WINCE 5.0만 사용 (충돌)
case 851:	ClerkBackScreen3Proc();				break;			WINCE 6.0만 사용 (충돌)
case 852:	ClerkEnhancedCouponText3Proc();		break;			WINCE 5.0만 사용 (충돌)
case 852:	ClerkBackScreen4Proc();				break;			WINCE 6.0만 사용 (충돌)
case 853:	ClerkEnhancedCouponText4Proc();		break;			WINCE 5.0만 사용 (충돌)
case 853:	ClerkBackScreen5Proc();				break;			WINCE 6.0만 사용 (충돌)
case 854:	ClerkRmsAddressProc();				break;			WINCE 5.0만 사용 (충돌)
case 854:	ClerkBackScreen6Proc();				break;			WINCE 6.0만 사용 (충돌)
case 855:	ClerkConfirmClearErrorSummary();	break;			WINCE 5.0만 사용
case 856:	ClerkDiagnosticsMagtekCard();		break;
case 857:	ClerkBinSearchProc();				break;			WINCE 5.0만 사용
case 858:	ClerkBinManagementProc();			break;			WINCE 5.0만 사용
case 859:	ClerkBinConfirmationProc();			break;			WINCE 5.0만 사용

case 860:	ClerkConfirmKeyMgrProc();			break;
case 861:	ClerkKeyMgrExitScreen();			break;
case 862:	ClerkViewBinListProc();				break;			WINCE 5.0만 사용
case 863:	ClerkExitMessageProc();				break;			WINCE 5.0만 사용
case 864:	ClerkSurchargeContactInfoProc();	break;			WINCE 5.0만 사용
case 865:	ClerkMarketingMessageProc();		break;			WINCE 5.0만 사용
case 866:   ClerkStoreMessageProc();			break;			WINCE 5.0만 사용
case 867:	ClerkProcessorMessageProc();		break;			WINCE 5.0만 사용
case 868:	ClerkAUSurchargeModeProc();			break;			WINCE 5.0만 사용
case 869:	ClerkBinPropertiesProc();			break;			WINCE 5.0만 사용

case 870:	ClerkStandard1OptionProc();			break;
case 871:	ClerkDiagnostics_LCDUSensorView();	break;
case 872:	ClerkDiagnostics_TCPIPProc();		break;
case 873:	ClerkDiagnostics_HostAddressProc();	break;			WINCE 5.0만 사용
case 874:	ClerkEnhancedCouponSettingProc();	break;
case 875:	ClerkAdvertisement1Proc();			break;			WINCE 5.0만 사용 (충돌)	- 934
case 875:	ClerkDiagnostics_CDURSensorView();	break;			WINCE 6.0만 사용 (충돌)
case 876:	ClerkAdvertisement2Proc();			break;			WINCE 5.0만 사용 (충돌) - 935
case 877:	ClerkAdvertisement3Proc();			break;			WINCE 5.0만 사용 (충돌)	- 936
case 878:	ClerkAdvertisement4Proc();			break;			WINCE 5.0만 사용 (충돌) - 937
case 879:	ClerkAdvertisement5Proc();			break;			WINCE 5.0만 사용 (충돌) - 938

case 880:	ClerkAdvertisement6Proc();			break;			WINCE 5.0만 사용 (충돌) - 939
case 880:	ClerkServiceProc();					break;			WINCE 6.0만 사용 (충돌)

case 881:	ClerkOptionalFunction2Proc();		break;
case 882:	ClerkDeviceOptionProc();			break;
case 883:	ClerkScheduledJournalUploadProc();	break;
case 884:	ClerkAdvertisementProc_Euro();		break;			WINCE 5.0만 사용
case 885:	ClerkAdvertisementPreviewTransactionAD();	break;	WINCE 5.0만 사용
case 886:	ClerkNoticeServiceProc();			break;			WINCE 6.0만 사용
case 887:	ClerkAdvertisementPreviewTrxADScreen();		break;	WINCE 5.0만 사용
//case 888:	ClerkMXSurchargeModeProc();			break;			사용안함
case 888:	ClerkHaloLedNMCUFlickerProc();		break;			WINCE 6.0만 사용 [#2205] NH KSK 2013.06.28 Halo LED 제어
case 889:	ClerkMX_PSurchargeModeProc();		break;			사용안함

case 890:	ClerkCoupon1TextProc_Euro();		break;			WINCE 5.0만 사용
case 891:	ClerkCoupon2TextProc_Euro();		break;			WINCE 5.0만 사용
case 892:	ClerkCoupon3TextProc_Euro();		break;			WINCE 5.0만 사용
case 893:	ClerkCoupon4TextProc_Euro();		break;			WINCE 5.0만 사용
case 894:	ClerkCoupon5TextProc_Euro();		break;			WINCE 5.0만 사용
case 895:	ClerkCoupon6TextProc_Euro();		break;			WINCE 5.0만 사용
case 896:	ClerkDiagnostics_GBM10CHSensorView();	break;			WINCE 6.0만 사용
case 897:	ClerkResetServicePasswordProc();	break;			WINCE 5.0만 사용 WINCE6.0 사용 안함
case 898:	ClerkSitemapProc();					break;			WINCE 6.0만 사용 (WINCE5.0과 중복됨 - 추후 변경 필요)

// 900번 부터는 다시한번 WINCE 5.0과 WINCE 6.0과 충돌나는 Screen 번호가 있는지 확인 필요함. (넘 많아서 여기까지만 비교했음) KSK 2010.06.09
case 899:	ClerkOpInitializeProc();			break;
case 900:	ClerkOpErrorModeProc();				break;			// [#539] NH KSK 2009.06.29
case 901:	ClerkOpModeProc();					break;
case 902:	ClerkOpInputPinProc();				break;
case 903:	ClerkReplenishProc();				break;
case 905:	ClerkOperatorPasswordProc();		break;
case 906:	ClerkServicePasswordProc();			break;
case 907:	ClerkMasterPasswordProc();			break;
case 908:	ClerkClearNvramProc();				break;
case 909:	ClerkConfirmClearAllProc();			break;

case 910:	ClerkRebootProc();					break;		// WINCE5.0만 사용
case 911:	ClerkConfirmYesNo();				break;		// WINCE5.0만 사용 [#650] NH KJW 2010.08.30 [정기버전업 V01.04.07] 공용 사용자 YES/NO Query 화면 추가
case 912:	ClerkAddCashProc();					break;
case 913:	ClerkClearLogProc();				break;		// WINCE5.0만 사용
case 915:   ClerkSelectReceiptOptionProc();		break;
case 916:	ClerkMOD10Proc();					break;
case 917:	ClerkSwInstallationProc();			break;		// WINCE5.0만 사용
case 918:	ClerkBackupNVRAM();					break;		// [#539] KSK 2009.06.29 US, CA, MX 사용 안함
case 919:	ClerkRestoreNVRAM();				break;		// [#539] KSK 2009.06.29 US, CA, MX 사용 안함

case 920:	ClerkPreBalanceProc();				break;	
case 921:	ClerkJournalProc();					break;
case 922:	ClerkViewJournalProc();				break;
case 923:	ClerkSearchJournalProc();			break;
case 924:	ClerkLastXPrintProc();				break;
//case 925:	ClerkClearJournalProc();			break;
case 926:	ClerkRMSListenningPortProc();		break;
case 927:	ClerkRMSStatusProc();				break;
case 928:	ClerkRMSSendIPProc();				break;		// [#539] KSK 2009.06.29 US, CA, MX 사용 안함
case 929:	ClerkRMSSendURLProc();				break;		// [#539] KSK 2009.06.29 US, CA, MX 사용 안함

case 930:	ClerkRMSPhoneNumberProc();			break;
case 931:	ClerkReportProc();					break;
case 932:	ClerkSWVersionProc();				break;
case 933:	ClerkConfirmClearSettingProc();		break;
case 934:	ClerkAdvertisement1Proc();			break;
case 935:	ClerkAdvertisement2Proc();			break;
case 936:	ClerkAdvertisement3Proc();			break;
case 937:	ClerkAdvertisement4Proc();			break;
case 938:	ClerkAdvertisement5Proc();			break;
case 939:	ClerkAdvertisement6Proc();			break;

case 940:	ClerkAdvertisementPreviewProc();	break;
case 941:	ClerkCustomerSetupProc();			break;
case 942:	ClerkChangeMessageProc();			break;
case 943:	ClerkReceiptHeaderTailProc();		break;
case 944:	ClerkAddBinListProc();				break;
case 945:   ClerkAUSurchargeModeProc			break;		// [#2000] SOOK 2010.09.27 호주 적용
case 946:	ClerkSurchargeModeProc();			break;
case 947:	ClerkBinListProc();					break;
case 948:	ClerkAdvertisementProc();			break;
case 949:	ClerkSelectCouponOptionProc();		break;

case 950:	ClerkEMVProc();						break;
case 951:	ClerkSystemSetupProc();				break;
case 952:	ClerkClockSetupProc();				break;
case 953:	ClerkAdjustVolumeProc();			break;
case 954:	ClerkTransLanguageProc();			break;		// [#2166] NH KSK 2012.11.13
case 956:	ClerkChangePasswordProc();			break;
case 957:	ClerkModemSetupProc();				break;
case 958:	ClerkModemTestProc();				break;		// [#539] KSK 2009.06.29 US, CA, MX 사용 안함
case 959:	ClerkPercentSurchargeProc();		break;

case 961:	ClerkHostSetupProc();				break;
case 962:	ClerkTelephoneNumberProc();			break;
case 963:   ClerkStoreMessageProc				break;
case 964:	ClerkHealthCheckMsgProc();			break;		
case 965:	ClerkRemoteMonitorProc();			break;
case 966:   ClerkConfigAtStartProc();			break;		
case 967:	ClerkTerminalNumberProc();			break;
case 968:	ClerkRoutingIDProc();				break;
case 969:	ClerkSerialNumberProc();			break;

case 970:   ClerkExitMessageProce();			break;		사용안함
case 971:	ClerkSelectHostProc();				break;
case 972:	ClerkStandard3OptionProc();			break;
case 973:	ClerkModemTypeProc();				break;
case 974:	ClerkCommunicationProc();			break;
case 975:	ClerkMessageFormatProc();			break;
case 976:	ClerkTCPIPTypeProc();				break;
case 977:	ClerkAtmIpConfigurationProc();		break;		// [#539] KSK 2009.06.29 US, CA, MX 사용 안함
case 978:	ClerkRMSTCPIPTimeoutProc();			break;		// [#2234] NH KMK 2014.01.14 RMS TCPIP Timeout
case 979:	ClerkAtmIpSetupProc();				break;

case 980:	ClerkHostIpSetupProc();				break;
case 981:	ClerkTransactionSetupProc();		break;
case 982:	ClerkSetFastCashProc();				break;
case 983:	ClerkSetDenominationProc();			break;
case 984:	ClerkEnhancedCoupon1Proc();			break;		// [#620] US KSK 2010.02.19
case 985:	ClerkEnhancedCoupon2Proc();			break;		// [#620] US KSK 2010.02.19
case 986:	ClerkEnhancedCoupon3Proc();			break;		// [#620] US KSK 2010.02.19
case 987:	ClerkEnhancedCoupon4Proc();			break;		// [#620] US KSK 2010.02.19
case 988:	ClerkEnhancedCoupon5Proc();			break;		// [#620] US KSK 2010.02.19
case 989:	ClerkEnhancedCoupon6Proc();			break;		// [#620] US KSK 2010.02.19

case 990:	ClerkReversalRetryProc();			break;
case 991:	AP USE
case 992:	ClerkSupervisorLanguageProc();		break;		// [#2166] NH KSK 2012.11.13
case 993:	ClerkLanguageProc();				break;		// [#2166] NH KSK 2012.11.13
case 994:	ClerkStandard1ExtenedProc();		break;		// [#2150] US Justin 2012.10.08 (WINCE5.0과 충돌)
case 995:	ClerkNetworkUserAddressProc();		break;
case 996:	ClerkOptionalSettingProc();			break;
case 997:	ClerkAccountsProc();				break;	
case 998:	ClerkAIDListProc();					break;
//case 999:	ClerkClearTranSeqNumProc();			break;

case 925	not used
case 999:	not used

case 1010:	ClerkLibertyXOptions();				break;		// (1010)	[#RWC6-59] US William 2019.10.03 LibertyX
case 1011:	ClerkLibertyXHostOptions();			break;		// (1011)	[#RWC6-59] US William 2019.10.03 LibertyX
case 1012:	ClerkLibertyXAPIOptions();			break;		// (1012)	[#RWC6-59] US William 2019.10.03 LibertyX

case 1013:	ClerkRemoteUpdatesOptions();	
case 1014:  ClerkRemoteUpdatesAdHoc();			break;
*/
/////////////////////////////////////////////////////////////////////////////

// Clerk Procedure
int	CMainFrame::ClerkProc()
{
	if (m_pDevCmn->AtmStatus == ATM_TRAN)		return TRUE;

/////////////////////////////////////////////////////////////////////////////
	
	//if (m_pDevCmn->m_pConfig->GetOSVersion() == NH_OS_NH2700_F_T || m_pDevCmn->m_pConfig->GetOSVersion() == NH_OS_NH2700_T_T)	// [#2064] NH KSK 2011.05.18
	if (m_pDevCmn->GetDeviceType(L"SPL") == SPL_SET)																				// [#GLDV-2505] US Kook 2019.05.13
	{
		CScrCtrl *pScrCtrl = m_pDevCmn->GetScreenByType(SCR_REAR);
		if (pScrCtrl != NULL)
		{
			if (pScrCtrl->GetUserInputCheck() == TRUE)
			{
				CString strCmd, strData;
				if (pScrCtrl->GetUserInputData(strCmd, strData) == TRUE)
				{
					if (strData == L"IN_SUPUERVISOR")
					{
						m_pDevCmn->fnAPL_EnDisableHotKey(DISABLE);
						m_pDevCmn->m_nHotKeyStatus = OPER_SUPERVISOR;
					}
				}
			}
		}
	}

	nOperatorSwitchStatus = m_pDevCmn->fnSNS_GetOperatorSwitch();

	if ((m_pDevCmn->OpenKey == TRUE) && (nOperatorSwitchStatus == OPER_SUPERVISOR))
	{
		m_pDevCmn->OpenKey = FALSE;						// Custom => Clerk Mode
		// [#24] KSK 2008.04.8 V01.02.19 Merge 누락 적용
		// 2007.11.26 V01.02.19 CST VALUE를 UPDATE하기 위해 CSTCnt를 0로 만든다.
		m_pDevCmn->CSTCnt = 0;
		m_pDevCmn->fnAPL_CheckDevice();
		// End of [#24]

		m_pDevCmn->fnAPL_UnUsed(ATM_CLERK);		// [#397] [NH] psc 2008.08.01 EMV(IC), MS 거래 함수 공통화
		Main_SetAtmStatus(ATM_CLERK);

#if (APP_DIGITALMINT)
		m_pTranCmn->F_NH_DigitalMintReportDeviceEvent(DMEC_MaintenanceModeEnter);
#endif

		return TRUE;
	}
	else if (m_pDevCmn->OpenKey == TRUE)
	{
		return TRUE;
	}

	//if (m_pDevCmn->m_pConfig->GetOSVersion() == NH_OS_NH2700_F_T || m_pDevCmn->m_pConfig->GetOSVersion() == NH_OS_NH2700_T_T)	// [#2064] NH KSK 2011.05.18
	if (m_pDevCmn->GetDeviceType(L"SPL") == SPL_SET)																				// [#GLDV-2505] US Kook 2019.05.13
	{
		// OPERATOR 동작중임을 화면에 표시한다.
		m_pDevCmn->fnSCR_SetActiveMode(SCR_FRONT);
		Delay_Msg(500);
		m_pDevCmn->fnSCR_ShowScreen(3);
		Delay_Msg(1000);
	}

	m_pDevCmn->fnSCR_SetActiveMode(SCR_REAR);

	NVDump('O', 'O', "01", L"",  L"ClerkProc:1");

	// [#2166] NH KSK 2012.11.13
	m_pDevCmn->fnSCR_SetCurrentLangMode(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_SUPERVISOR_LANGUAGE));
	// end of [#2166]

	// [#2333] NH KSK 2015.03.06 MX2600SE인 경우 EPP LED GUIDE 깜박이는 현상을 없애기 위해 SPR 제어하지 않도록 수정
	if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX2600SE")
		m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_SPR, FLICKER_CONTINUE);
	// end of [#2333]
	
/////////////////////////////////////////////////////////////////////////////
	nOperatorSwitchStatus = OPER_NONE;
	nScrMaxTimeOver = 0;

	m_OpInfo.Doing = TRUE;
	m_OpInfo.nLoginMode = 0;
	m_OpInfo.nProcNextScrNum = 0;
	m_OpInfo.nCurScrNum = 0;

	m_OpInfo.nTimeout = K_300_WAIT;				// OP Timeout 10분 -> 5분으로 수정

	m_OpInfo.nProcLoofCnt = 0;
	m_OpInfo.nMoveProcReason = MOVE_PROC_SELECT;

	m_OpInfo.strTempButtonInformation = _T("");
	m_OpInfo.strSaveButtonInformation = _T("");
	m_OpInfo.strTempClerkInformation = _T("");
	m_OpInfo.strSaveClerkInformation = _T("");
	m_OpInfo.nMsgClearCount = -1;
	m_OpInfo.strTempClerkMessage = _T("");
	m_OpInfo.strSaveClerkMessage = _T("");
	m_OpInfo.strReturn = _T("");
	m_OpInfo.strTempArray.RemoveAll();
	m_OpInfo.strFromToDateScreen = _T("");				// [#508] AIREAT 2009.02.27 : JNL Search by date
	m_OpInfo.strFromToDateFile = _T("");				// [#508] AIREAT 2009.02.27 : JNL Search by date
	m_OpInfo.strJNLSearchResult.RemoveAll();			// [#508] AIREAT 2009.02.27 : JNL Search by date

	// CDU SETUP INFORMATION Initialize
	m_OpInfo.m_pusCstNum = 0;
	m_OpInfo.m_mbCDUType = 0;
	memset(m_OpInfo.m_ulCstIndex, 0, sizeof(m_OpInfo.m_ulCstIndex));
	memset(m_OpInfo.m_szCstCountry, 0, sizeof(m_OpInfo.m_szCstCountry));
	
	//m_OpInfo.strSelectedMachineType = m_pDevCmn->m_pConfig->GetMachineType();	// 국가별 Default 기종을 Set한다.

	//[#2097] US PCS 2011.11.14
#if (US_VERSION || CA_VERSION || MX_VERSION)	// [#2180] NH Justin 2013.03.12  Add 5000SE to CA,MX as well
	// KSK 2011.11.28 NH1800SE or WallMount인 경우에만 NVRAM 기종 정보값을 설정한다.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// [#2128] US PCS 2012.03.13
	// NH1800SE MX4000W는 동일
	// NH2700L,NH2700B는 MX5000SE와 동일. 
	// 따라서 NH2700T / NH1500SE / 5200SE 을 제외하고는 기종을 수동으로 선택해야 한다.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//if (m_pDevCmn->m_pConfig->GetMachineType() == L"NH2700T")
	//if (m_pDevCmn->m_pConfig->GetMachineType() == L"NH2700T" || m_pDevCmn->m_pConfig->GetMachineType() == L"NH1500SE" || m_pDevCmn->m_pConfig->GetMachineType() == L"MX5200SE")	// [#2315] Justin 2014.12.12 Add 5200SE
	//if (m_pDevCmn->m_pConfig->GetMachineType() == L"NH2700T" || m_pDevCmn->m_pConfig->GetMachineType() == L"NH1500SE")	// [#2383] Justin 2015.12.18 Support 5300SE (Same configuration as 5200SE)
	if( IsUniqueATMType(m_pDevCmn->m_pConfig->GetMachineType()) == TRUE )		// [#2571] NH Justin 2018.08.03
		m_OpInfo.strSelectedMachineType = m_pDevCmn->m_pConfig->GetMachineType();
	else 
		m_OpInfo.strSelectedMachineType = MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND);
#else
	m_OpInfo.strSelectedMachineType = m_pDevCmn->m_pConfig->GetMachineType();	// 국가별 Default 기종을 Set한다
#endif
	//end of [#2097]

	// Initialize LibertyX Configuration
#if (APP_LIBERTYX)
	this->m_LXConfig = new CLibertyXConfigurationManager();

	LXConfiguration lxconfig;
	if (m_LXConfig->GetConfiguration(lxconfig))
	{
		this->m_LTXAvailable = true;
	}
#endif

#if (APP_DIGITALMINT)
	DMConfiguration config;

	if (m_pTranCmn->m_DMConfigManager->GetConfiguration(config))
	{
		this->m_DMAvailable = true;
	}
#endif

#if (APP_CUSTOM_CASHDEPOT)
	if (m_pTranCmn->m_CDService != NULL)
	{
		this->m_CDAvailable = true;
	}
#endif

	// [RWC6-676] start SKKim 2024.03.28
#if (APP_TANGOPAY)
	if (m_pTranCmn->m_pTangoPayService != NULL)
		this->m_TangPayAvailable = true;
	else
		this->m_TangPayAvailable = false;
#endif
	// [RWC6-676] end SKKim 2024.03.28

/////////////////////////////////////////////////////////////////////////////

	m_OpInfo.Doing = TRUE;
	m_OpInfo.nCurScrNum = 0;
	m_OpInfo.nProcPrevScrNum = m_OpInfo.nCurScrNum;
	
	if (!m_pDevCmn->fnAPL_CheckError())
		m_OpInfo.nProcNextScrNum = 900;
	else
		m_OpInfo.nProcNextScrNum = 902;

	CString	KeyValue;

	StoreNetworkInformation();	// [#207] KSK 2008.5.20
//#if MULTIFUNCTION_BIN
		//Key Management 관련 항목
	int nPrevKeyMode=0, nPrevKeyIndex=0;
	CString strPrevKeyCheckSum, strPrevMacKeyCheckSum, strPrevSerialNo;

#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
	int nBINTotalCount = m_pDevCmn->m_BINMgr.GetTotalBinCount();
#endif

	while (m_OpInfo.Doing)
	{
#if SUPPORT_CHANGE_PARAMETER_JNL		//[#610] SOOK 2010.01.14 Configuration 저널 저장 
		if ( m_OpInfo.nProcNextScrNum != 899 && m_OpInfo.nProcNextScrNum != 900 && m_OpInfo.nProcNextScrNum != 901 && m_OpInfo.nProcNextScrNum != 902 //OP 초기화면 
			&& m_OpInfo.nProcNextScrNum != 858 && m_OpInfo.nProcNextScrNum != 857 && m_OpInfo.nProcNextScrNum != 855 && m_OpInfo.nProcNextScrNum != 859  // BIN 관리 
			&& m_OpInfo.nProcNextScrNum != 846 && m_OpInfo.nProcNextScrNum != 909 ) // System Control, Clear NVRAM 제외 
//			&& m_OpInfo.nProcNextScrNum != 909 && m_OpInfo.nProcNextScrNum != 910 && m_OpInfo.nProcNextScrNum != 919 && m_OpInfo.nProcNextScrNum != 933 ) //Restore NVRAM, Clear NVRAM
		{
			m_pDevCmn->SetPreviousParameter();	
	NHDEBUG(DBG_INFO, (L"SetPreviousParameter Called at ClerkProc\n"));
		}
		

	#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
		else if ( m_OpInfo.nProcNextScrNum == 858)
		{
			if ( m_OpInfo.nCurScrNum != 857 && m_OpInfo.nCurScrNum != 855 && m_OpInfo.nCurScrNum != 859 )
			{
				nBINTotalCount = m_pDevCmn->m_BINMgr.GetTotalBinCount();
			}
		}
	#endif
		//CDU SETUP 관련 정보는 OP에서만 변경 가능 하므로 여기서 예외 처리 
#endif		//end of [#610]

		for (int i = 0; i < MAX_USER_INPUT; i++)
			m_OpInfo.m_UserInputValue[i] = L"";

		m_OpInfo.bUpdateFlag = ENABLE;
		m_OpInfo.nMoveProcReason = MOVE_PROC_SELECT;
		
		// [#2024] NH KSK 2011.02.24
		m_strNVLog.Format(L"[OP]NextScr:%d", m_OpInfo.nProcNextScrNum);
		NVDump('O', 'A', "11", L"", m_strNVLog);
		// end of [#2024]

		switch	(m_OpInfo.nProcNextScrNum)
		{
//			case 706:	ClerkResetMasterPassword();				break;	// [iTM] US AIREAT 2009.12.15 // [#2070] NH KSK 2011.06.16
			case 711:	ClerkExitSupervisor();					break;	// [iTM] US AIREAT 2009.12.15

			case 898:	ClerkSitemapProc();						break;
			case 899:	ClerkOpInitializeProc();				break;	// [#539] US KSK 2009.07.06
			case 900:	ClerkOpErrorModeProc();					break;	// [#539] US KSK 2009.06.29
			case 901:	ClerkOpModeProc();						break;
			case 902:	ClerkOpInputPinProc();					break;

			/////////////////////////////////////////////////////
			/////////////////////////////////////////////////////
			//// SETTLEMENT
			case 903:	ClerkReplenishProc();					break;	// (F1)
			case 912:	ClerkAddCashProc();						break;	// (F1)
			case 701:	ClerkQuickReplenish();					break;	// [iTM] US AIREAT 2009.11.27 OP Wizard
			case 702:	ClerkQuickDayTotal();					break;	// [iTM] US AIREAT 2009.11.27 OP Wizard
			case 703:	ClerkQuickCassetteTotal();				break;	// [iTM] US AIREAT 2009.11.27 OP Wizard
			case 704:	ClerkQuickAddCash();					break;	// [iTM] US AIREAT 2009.11.27 OP Wizard
			case 705:	ClerkQuickResult();						break;	// [iTM] US AIREAT 2009.11.27 OP Wizard

			/////////////////////////////////////////////////////
			/////////////////////////////////////////////////////
			//// CUSTOMER SETUP
			case 941:	ClerkCustomerSetupProc();				break;	// (F2)
			case 942:	ClerkChangeMessageProc();				break;	// (F1)
			case 946:	ClerkSurchargeModeProc();				break;
			case 945:	ClerkAUSurchargeModeProc();				break;	// [#2000] SOOK 2010.09.27 호주 적용
			case 959:	ClerkPercentSurchargeProc();			break;	// (F6)
			case 960:	ClerkAddTionalSurchargeOptionProc();	break;	// [#2136] US/CA PCS 2012.05.30
			case 963:	ClerkTableSurchargeOptionProc();		break;	// [#2136] US/CA PCS 2012.05.30

			case 947:	ClerkBinListProc();						break;	// (F3)
			case 944:	ClerkEditBinListProc();					break;	// (F3)
			case 809:	ClerkExtendedBinList();					break;	// (F3)
				
#if (AU_VERSION) // [#622] NH KSK 2010.02.24 // [#2069] NH KSK 2011.06.13
			case 858:	ClerkBinManagementProc();				break;	// [#546] SOOK 2009.07.24 다기능 BIN LIST 기능 추가함
			case 857:   ClerkViewBinListProc();					break;	// [#546] SOOK 2009.07.24 다기능 BIN LIST 기능 추가함
			case 855:	ClerkBinPropertiesProc();				break;	// [#546] SOOK 2009.07.24 다기능 BIN LIST 기능 추가함
			case 859:	ClerkBinConfirmationProc();				break;	// [#546] SOOK 2009.07.24 다기능 BIN LIST 기능 추가함
#endif

			case 869:	ClerkAdvertisementProc();				break;	// (F4)
			case 868:	ClerkTransactionAdvProc();				break;	// (F4)
			case 862:	ClerkTranAdvertisement1Proc();			break;
			case 863:	ClerkTranAdvertisement2Proc();			break;
			case 864:	ClerkTranAdvertisement3Proc();			break;
			case 865:	ClerkTranAdvertisement4Proc();			break;
			case 866:	ClerkTranAdvertisement5Proc();			break;
			case 867:	ClerkTranAdvertisement6Proc();			break;

			case 949:	ClerkSelectCouponOptionProc();			break;	// (F2)
			case 803:	ClerkBasicCouponSettingProc();			break;
			case 831:	ClerkBasicCoupon1Proc();				break;	// (F1)
			case 832:	ClerkBasicCoupon2Proc();				break;	// (F2)
			case 833:	ClerkBasicCoupon3Proc();				break;	// (F3)
			case 834:	ClerkBasicCoupon4Proc();				break;	// (F4)
			case 835:	ClerkBasicCoupon5Proc();				break;	// (F5)
			case 836:	ClerkBasicCoupon6Proc();				break;	// (F6)
			case 837:	ClerkAusBasicCoupon1Proc();				break;	// (F1) // [#2000] SOOK 2010.10.12 호주 사양 적용 (Coupon & Advertisement)
			case 838:	ClerkAusBasicCoupon2Proc();				break;	// (F2) // [#2000] SOOK 2010.10.12 호주 사양 적용 (Coupon & Advertisement)
			case 839:	ClerkAusBasicCoupon3Proc();				break;	// (F3) // [#2000] SOOK 2010.10.12 호주 사양 적용 (Coupon & Advertisement)
			case 840:	ClerkAusBasicCoupon4Proc();				break;	// (F4) // [#2000] SOOK 2010.10.12 호주 사양 적용 (Coupon & Advertisement)
			case 841:	ClerkAusBasicCoupon5Proc();				break;	// (F5) // [#2000] SOOK 2010.10.12 호주 사양 적용 (Coupon & Advertisement)
			case 842:	ClerkAusBasicCoupon6Proc();				break;	// (F6) // [#2000] SOOK 2010.10.12 호주 사양 적용 (Coupon & Advertisement)
			case 874:	ClerkEnhancedCouponSettingProc();		break;	// (F2)
			case 984:	ClerkEnhancedCoupon1Proc();				break;	// [#620] US KSK 2010.02.19	(F3)
			case 985:	ClerkEnhancedCoupon2Proc();				break;	// [#620] US KSK 2010.02.19 (F4)
			case 986:	ClerkEnhancedCoupon3Proc();				break;	// [#620] US KSK 2010.02.19 (F5)
			case 987:	ClerkEnhancedCoupon4Proc();				break;	// [#620] US KSK 2010.02.19 (F6)
			case 988:	ClerkEnhancedCoupon5Proc();				break;	// [#620] US KSK 2010.02.19 (F7)
			case 989:	ClerkEnhancedCoupon6Proc();				break;	// [#620] US KSK 2010.02.19 (F8)

			case 948:	ClerkWelcomeAdvProc();					break;	// (F4)
			case 934:	ClerkAdvertisement1Proc();				break;	// (F3)
			case 935:	ClerkAdvertisement2Proc();				break;	// (F4)
			case 936:	ClerkAdvertisement3Proc();				break;	// (F5)
			case 937:	ClerkAdvertisement4Proc();				break;	// (F6)
			case 938:	ClerkAdvertisement5Proc();				break;	// (F7)
			case 939:	ClerkAdvertisement6Proc();				break;	// (F8)

//			case 940:	ClerkAdvertisementPreviewProc();		break;	// SUB 공통 (F5)
			case 996:	ClerkOptionalSettingProc();				break;	// (F5)
			case 916:	ClerkMOD10Proc();						break;	// (F1)
			case 997:	ClerkAccountsProc();					break;	// (F2)
			case 920:	ClerkPreBalanceProc();					break;	// (F3)
			case 915:	ClerkSelectReceiptOptionProc();			break;	// (F4)
				
#if (US_VERSION || CA_VERSION || MX_VERSION)
			case 795:	ClerkOtherTransactionProc();			break;	// (F5) // [#2365] Enable DCC+ on Canadian Version
			case 749:	ClerkTDLOptionSetupProc();				break;	// (F3) // [#2292] US Justin 2014.09.17 Add Cardtronics TDL Option // [#2365] Enable DCC+ on Canadian Version
#endif

#if (US_VERSION)
	#if (APP_JUST_CASH)
			case 761:	ClerkJustCashGeneralOptions();			break;
	#endif

			case 762:	ClerkPaypalCCASetupProc();				break;	// [#2446] US Justin 2016.09.29 Add Paypal CCA Setup
			case 753:	ClerkPOPMoneySetupProc();				break;	// (F2) // [#2350] US Justin 2015.06.22 Add PAI OP Setup
			case 759:	ClerkHalCashSetupProc();				break;	// (F4) // [#2396] US Justin 2016.02.17 Pin4
			case 1010:	ClerkLibertyXOptions();					break;	// [#RWC6-59] US William 2019.10.03 LibertyX
			case 1011:	ClerkLibertyXHostOptions();				break;	// [#RWC6-59] US William 2019.10.03 LibertyX
			case 1012:	ClerkLibertyXAPIOptions();				break;	// [#RWC6-59] US William 2019.10.03 LibertyX

	#if (APP_DIGITALMINT)
			case 1020:	ClerkDigitalMintGeneralOptions();		break;
	#endif

	#if (APP_CUSTOM_CASHDEPOT)
			case 1030:	ClerkCashDepotGeneralOptions();			break;
	#endif
#endif

// [RWC6-162] Add B4U to Canada Release
#if (APP_B4U)
			case 769:	ClerkB4USetupProc();					break;	// (F6) // [#RWC6-16] Bitload 4 U. A new cryptocurrency feature for CE 6.0 ATMs
#endif
// end of [RWC6-162]

#if (APP_GPAY_GIFTCARD_PURCHASE)
			case 748:	ClerkGivePaySetupProc();				break;	// [#2535] US Justin 2018.03.20 GivePay Online
#endif

			case 817:	ClerkPreDialOptionProc();				break;	// (F6)
			case 819:	ClerkNeedMoreTime();					break;	// (F7)
			case 950:	ClerkEMVProc();							break;	// (F8)
			case 998:	ClerkAIDListProc();						break;	// (F2)
			case 767:	ClerkEMVOtherOptionsProc();				break;	// (F6) // [#2549] NH Justin 2018.05.07 Add Perto Rico Option
			case 972:	ClerkStandard3OptionProc();				break;	// (F6)
			case 870:	ClerkStandard1OptionProc();				break;	// (F6) // [#563] CA KSK 2009.8.20
			case 994:	ClerkStandard1ExtenedProc();			break;	// (F6) // [#2150] US Justin 2012.10.08
			case 794:	ClerkDCCCustomOptionProc();				break;	// (F4) // [#2187] NH Justin 2013.04.18

#if (US_VERSION)
			case 791:	ClerkDualHostDCCProc();					break;	// [#2499] US Justin 2017.08.21
			case 792:	ClerkDualHostDCCHostIpSetupProc();		break;	// [#2499] US Justin 2017.08.21
#endif

			case 881:	ClerkOptionalFunction2Proc();			break;	// (F7)
			case 882:	ClerkDeviceOptionProc();				break;	// (F1)

// [#2459] AU KSK 2016.12.23
#if (AU_A_VERSION)
			case 773:	ClerkDispenseMixOptionProc();			break;	// (F7)
#endif
// End of [#2459]

			case 880:	ClerkServiceProc();						break;	// (F2)
			case 847:	ClerkChangBackgroundProc();				break;
			case 848:	ClerkDefaultBackgroundProc();			break;
			case 849:	ClerkBackScreen1Proc();					break;
			case 850:	ClerkBackScreen2Proc();					break;
			case 851:	ClerkBackScreen3Proc();					break;
			case 852:	ClerkBackScreen4Proc();					break;
			case 853:	ClerkBackScreen5Proc();					break;
			case 854:	ClerkBackScreen6Proc();					break;
			case 886:	ClerkNoticeServiceProc();				break;	// (F2)

#if (MX_VERSION)
			case 814:	ClerkMXPrintOptionProc();				break;	// (F3) // [#2115] MX KSK 2012.02.05
#endif

			case 793:	ClerkDigitalReceiptOptionProc();		break;	// (F4) // [#2263] US Justin 2014.05.05 Digital Receipt Option
			case 757:	ClerkMoniMobileSetupProc();				break;	// (F5) // [#2382] US Justin 2015.12.16 MoniMobile Setup
			case 971:	ClerkSelectHostProc();					break;	// (F8)
			case 974:	ClerkCommunicationProc();				break;	// (F1)
			case 975:	ClerkMessageFormatProc();				break;	// (F2)
			case 973:	ClerkModemTypeProc();					break;	// (F3)
			case 976:	ClerkTCPIPTypeProc();					break;	// (F4)
			case 990:	ClerkReversalRetryProc();				break;	// (F5)

			/////////////////////////////////////////////////////
			/////////////////////////////////////////////////////
			//// JOURNAL
			case 921:	ClerkJournalProc();						break;	// (F3)
			case 922:	ClerkViewJournalProc();					break;	// (F2)
			case 744:	ClerkJournalUploadToHostProc();			break;	// (F5) // [#2076] NH KSK 2011.06.29

			/////////////////////////////////////////////////////
			/////////////////////////////////////////////////////
			//// SYSTEM SETUP
			case 951:	ClerkSystemSetupProc();					break;	// (F4)
			case 952:	ClerkClockSetupProc();					break;	// (F1)
			case 956:	ClerkChangePasswordProc();				break;	// (F2)
			case 905:	ClerkOperatorPasswordProc();			break;	// (F1)
			case 907:	ClerkMasterPasswordProc();				break;	// (F2)
			case 906:	ClerkServicePasswordProc();				break;	// (F3)
			case 953:	ClerkAdjustVolumeProc();				break;	// (F3)
			case 957:	ClerkModemSetupProc();					break;	// (F4)
			case 979:	ClerkAtmIpSetupProc();					break;	// (F5)
			case 844:	ClerkDeviceSetupProc();					break;	// (F6)
			case 845:	ClerkCDUSetupProc();					break;	// (F1)
			case 741:	ClerkCardReaderSetupProc();				break;	// (F3) // [#2073] NH KSK 2011.06.24
			case 745:	ClerkAntiSkimmingSetupProc();			break;	// (F6) // [#2074] NH KSK 2011.07.09
			case 747:	ClerkHaloLEDSetupProc();				break;	// (F5) // [#2205] US KSK 2013.06.28
			case 756:	ClerkReceiptPaperCutSetupProc();		break;	// (F6) // [#2371] US Justin 2015.09.08 Receipt Paper Cut Setup
			case 764:	ClerkCameraSetupProc();					break;	// (F7) // [#2518] US Kook 2017.12.22 Support MX-2800SE
			case 772:	ClerkSideCarSetupProc();				break;	// (F8) // [#GLDV-3005] US Kook 2021.10.22 Support Side Car
			case 993:	ClerkLanguageProc();					break;	// (F7) // [#2166] NH KSK 2012.11.13
			case 954:	ClerkTransLanguageProc();				break;	// (F1) // [#2166] NH KSK 2012.11.13
			case 992:	ClerkSupervisorLanguageProc();			break;	// (F2) // [#2166] NH KSK 2012.11.13
			case 846:	ClerkSystemControlProc();				break;	// (F8)
			case 766:	ClerkPowerControlProc();				break;	// (F3)
			case 816:	ClerkSearchJournalbyDateProc();			break;	// (F4)
			case 908:	ClerkClearNvramProc();					break;

#if (AU_VERSION)
			case 740:	ClerkAUCountrySetup();					break;	// [#2042] AU KSK 2011.04.02
// [#2097] US PCS 2011.11.11 // [#2180] NH Justin 2013.03.12 Add 5000SE to CA,MX as well
#elif (US_VERSION || CA_VERSION || MX_VERSION)
			case 807:	ClerkSelectMachineTypeProc();			break;
#endif
// End of [#2097]

//			case 925:	ClerkClearJournalProc();				break;	// (F4) - Journal Menu에 기 존재
//			case 999:	ClerkClearTranSeqNumProc();				break;	// (F6) - Journal Menu에 기 존재
			case 1000:	ClerkDiskManagementMainProc();			break;	// (F7) - Disk Management
			case 1013:	ClerkRemoteUpdatesOptions();			break;	// (F1) - Remote Updates
			case 1014:	ClerkRemoteUpdatesAdHoc();				break;	// (F2) - Ad Hoc

			/////////////////////////////////////////////////////
			/////////////////////////////////////////////////////
			//// REPORT
			case 931:	ClerkReportProc();						break;	// (F5)
			case 810:	ClerkErrorCodeView();					break;	// (F1)
			case 812:	ClerkErrorSumProc();					break;	// (F2)
			case 932:	ClerkSWVersionProc();					break;	// (F3)
			case 813:	ClerkOptOutProc();						break;	// (F4) // [#2540] NH Justin 2018.03.14 Surcharge and DCC Acceptance Report
			case 818:	ClerkRejectAnalysisProc();				break;	// (F6)

			/////////////////////////////////////////////////////
			/////////////////////////////////////////////////////
			//// HOST SETUP
			case 961:	ClerkHostSetupProc();					break;	// (F6)
			case 742:	ClerkKeyManagementProc();				break;	// (F1) // [#2080] NH KSK 2011.06.27
			case 966:	ClerkConfigAtStartProc();				break;	// (F2) // [#2080] NH KSK 2011.06.27
			case 743:	ClerkRKTSetupProc();					break;	// (F4) // [#2080] NH KSK 2011.06.27
			case 962:	ClerkTelephoneNumberProc();				break;	// (F3) - DIALUP
			case 980:	ClerkHostIpSetupProc();					break;	// (F3) - TCP/IP
			case 965:	ClerkRemoteMonitorProc();				break;	// (F4)
			case 815:	ClerkRMSRingCountProc();				break;	// (F1) - DIALUP
			case 926:	ClerkRMSListenningPortProc();			break;	// (F1) - TCP/IP
			case 930:	ClerkRMSPhoneNumberProc();				break;	// (F2) - DIALUP
			case 927:	ClerkRMSStatusProc();					break;	// (F2) - TCP/IP

// [#2234] NH KMK 2014.01.13 MoniView Timeout
#if (AU_VERSION)
			case 978:	ClerkRMSTCPIPTimeoutProc();				break;	// (F6) - TIMEOUT for TCP/IP
#endif
// End of [#2234]

			case 883:	ClerkScheduledJournalUploadProc();		break;
			case 964:	ClerkHealthCheckMsgProc();				break;	// (F7)
			case 804:	ClerkAutoDayTotalProc();				break;	// (F8)

			/////////////////////////////////////////////////////
			/////////////////////////////////////////////////////
			//// DIAGNOSTICS
			case 821:	ClerkDiagonsticsMain();					break;	// (F1)
			case 822:	ClerkDiagnosticsCardScan();				break;	// (F2)
			case 856:	ClerkDiagnosticsMagtekCard();			break;	// (F2) - Magtek DIP
			case 746:	ClerkDiagAntiSkimmingTestProc();		break;	// (F4) // [#2078] NH KSK 2011.07.15
			case 827:	ClerkDiagnosticsAuxiliaryUnit();		break;
			case 824:	ClerkDiagnosticsSensor();				break;	// (F1)
			case 828:	ClerkDiagnosticsFlicker();				break;	// (F2)
			case 888:	ClerkHaloLedNMCUFlickerProc();			break;	// (F2) // [#2205] NH KSK 2013.06.28
			case 825:	ClerkDiagnosticsCDUSensorView();		break;	// (F1)
			case 871:	ClerkDiagnostics_LCDUSensorView();		break;	// (F1) // [#576] KSK 2009.11.06 L-CDU Image 추가
			case 875:	ClerkDiagnostics_CDURSensorView();		break;	// (F1)
			case 896:	ClerkDiagnostics_GBM10CHSensorView();	break;	// (F1) // [#2025] NH KJW 2011.02.28 GBM10CH Image 추가
			case 826:	ClerkDiagnosticsSPRSensorView();		break;	// (F2)

#if (US_VERSION || CA_VERSION) // [#2243] CA Justin 2013.12.17 Bug fix on Canadian 4000W
			case 876:	ClerkDiagnosticsWCDUSensorView();		break;	// (F1) // [#2097] US PCS 2011.11.21 
			case 877:	ClerkDiagnosticsWSPRSensorView();		break;	// (F2) // [#2097] US PCS 2011.11.21 
#endif

			case 765:	ClerkDiagnosticsCamera();				break;	// (F3) // [#2518] US Kook 2017.12.21 Support MX-2800SE
			case 751:	ClerkDiagnosticsRFIDCardScan();			break;	// (F4)
			case 770:	ClerkDiagnosticsBarcodeReader();		break;	// (F5) // [#GLDV-3005] US Kook 2021.10.07 Support Side Car
			case 771:	ClerkDiagnosticsBillAcceptor();			break;	// (F6) // [#GLDV-3005] US Kook 2021.10.07 Support Side Car
			case 823:	ClerkDiagnosticsModem();				break;	// (F7)
			case 872:	ClerkDiagnostics_TCPIPProc();			break;	// (F7) // [#585] NH KSK 2009.12.03
			case 829:	ClerkDiagnosticsExitScreen();			break;	// Diagnostics Exit & Time Out

			/////////////////////////////////////////////////////
			/////////////////////////////////////////////////////
			//// TRANSACTION SETUP
			case 981:	ClerkTransactionSetupProc();			break;	// (F8)
			case 982:	ClerkSetFastCashProc();					break;	// (F3)

// [#2115] MX KSK 2012.02.05
#if (MX_VERSION)
			case 808:	ClerkSetDenominationProc_MX();			break;	// (F4)
#else
			case 983:	ClerkSetDenominationProc();				break;	// (F4)
#endif
// End of [#2115]

// [#2472] US Justin 2017.02.08 DCC Withdrawal Option
#if (APP_DCC_WITHOPTION)
			case 752:	ClerkDCCWithdrawalOptionProc();			break;	// (F5)
#endif
// End of [#2472]

			/////////////////////////////////////////////////////
			/////////////////////////////////////////////////////
			//// QUICK SETUP											// [iTM] US KSK 2010.01.12 : Quick Configuration
			case 712:	ClerkQuickConfigurationProc();			break;
			case 714:	ClerkQuickCommunicationProc();			break;
			case 715:	ClerkQuickTCPIPTypeProc();				break;	// TCP/IP Only
			case 716:	ClerkQuickEotEnqOptionProc();			break;	// Dialup or TCP/IP VISA FRAMED
			case 717:	ClerkQuickMessageFormatProc();			break;
			case 718:	ClerkQuickTelephoneNumberProc();		break;	// DialUp Only

#if (APP_CUSTOM_PAI)
			case 719:	ClerkQuickProcessorProc();				break;
			case 721:	ClerkQuickRemoteMonitorProc();			break;
			case 722:	ClerkQuickChangeOperatorPasswordProc();	break;
			case 725:	ClerkQuickOperatorPasswordProc();		break;
			case 726:	ClerkQuickChangeServicerPasswordProc();	break;
			case 727:	ClerkQuickServicerPasswordProc();		break;
			case 728:	ClerkQuickRevenueAddedFeatures1Proc();	break;
			case 730:	ClerkQuickRevenueAddedFeatures2Proc();	break;
			case 732:	ClerkQuickAcceptConfigurationProc();	break;
#endif

			case 720:	ClerkQuickHostIpSetupProc();			break;	// TCP/IP Only
			case 723:	ClerkQuickATMDHCPSettingProc();			break;	// TCP/IP Only
			case 724:	ClerkQuickAtmIpSetupProc();				break;	// TCP/IP Only

			case 729:	ClerkQuickTerminalIDViewProc();			break;
			case 731:	ClerkQuickRoutingIDViewProc();			break;	// STANDARD 1,2,EPS
			case 733:	ClerkQuickStandard3OptionProc();		break;	// STANDARD 3
			case 735:	ClerkQuickChangeMasterPasswordProc();	break;
			case 734:	ClerkQuickMasterPasswordProc();			break;
			case 736:	ClerkQuickConfigResultViewProc();		break;

			case 713:	ClerkQuickConfCancelConfirmProc();		break;
			
			default:											break;
		}

#if SUPPORT_CHANGE_PARAMETER_JNL		//[#610] SOOK 2010.01.14 Configuration 저널 저장 
		if (m_OpInfo.nCurScrNum != 900 && m_OpInfo.nCurScrNum != 901  && m_OpInfo.nCurScrNum != 899  && m_OpInfo.nCurScrNum != 902 //OP 초기화면 
			&& m_OpInfo.nCurScrNum != 858 && m_OpInfo.nCurScrNum != 859 && m_OpInfo.nCurScrNum != 857 && m_OpInfo.nCurScrNum != 855// BIN 관리 
			&& m_OpInfo.nCurScrNum != 846 && m_OpInfo.nProcNextScrNum != 909)
			//&& m_OpInfo.nCurScrNum != 909 && m_OpInfo.nCurScrNum != 910 && m_OpInfo.nCurScrNum != 919 && m_OpInfo.nCurScrNum != 933 )//Restore NVRAM, Clear NVRAM
		{
			m_pDevCmn->SaveChangeParameter(m_OpInfo.nLoginMode);	
			NHDEBUG(DBG_INFO, (L"SaveChangeParameter Called at ClerkProc\n"));
		}

	#if (AU_VERSION)	// [#2069] NH KSK 2011.06.13
		if ( m_OpInfo.nCurScrNum != 858 && m_OpInfo.nCurScrNum != 857 && m_OpInfo.nCurScrNum != 855 && m_OpInfo.nCurScrNum != 859 )// BIN 관리 화면이 아니면 
		{
			int nCurrentBINCount = m_pDevCmn->m_BINMgr.GetTotalBinCount();
			if ( nBINTotalCount != nCurrentBINCount)
			{
				CString strOldValue, strNewValue;
				strOldValue.Format(L"%d",nBINTotalCount);
				strNewValue.Format(L"%d", nCurrentBINCount);

				m_pDevCmn->m_JNLMgr.SaveChangeParameterJnl(m_OpInfo.nLoginMode, L"TOTAL BIN COUNT", strOldValue, strNewValue); 
			}
			nBINTotalCount = nCurrentBINCount;
		}
	#endif
#endif		//end of [#610]
	}

	#if (CA_VERSION || US_VERSION || MX_VERSION)		// [#302] [MX] KSK 2008.6.17	// [#2115] MX KSK 2012.02.05
		m_pDevCmn->fnSCR_SetCurrentLangMode(MemGetInt(_MEM_FLD_OPTIONALSETTING, _MEM_VAR_OPT_DEFAULT_LANGUAGE));
	#endif		// end of [#302]

	// [#6] NH PSC 2008.03.18 configuration 실패시 OP모드 빠져나가면 바로 configuration 재시도.
	m_pDevCmn->HostOpenRetryTime = 0;

	m_pDevCmn->GetCurrentScreen()->SetEnableHideWindow(FALSE);

	m_pDevCmn->fnSCR_SetActiveMode(SCR_FRONT);

	// [#2333] NH KSK 2015.03.06 Clerk에서 Exit시 GUIDE Option으로 LED Control
	if (MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_MACHINEKIND) == L"MX2600SE")
	{
		if(MemGetInt(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_EPP_LED_GUIDE_OPTION) == 0)
			m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_SPR, FLICKER_CONTINUE);
		else
			m_pDevCmn->fnAPL_DeviceSetFlicker(DEV_SPR, FLICKER_OFF);
	}
	// end of [#2333]

	// Destroy LTX services
#if (APP_LIBERTYX)
	if (this->m_LXConfig != NULL) delete this->m_LXConfig;
	this->m_LXConfig = NULL;
#endif

	NHDEBUG(DBG_CALL, (_T("***ClerkCtrl***CMainFrame::ClerkProc() return \n")));

	return TRUE;
}
