// NHMWICtl.cpp : Implementation of the CNHMWICtrl ActiveX Control class.

#include "stdafx.h"
#include "NHMWI.h"
//#define NH_DEBUG
#include "NHMWICtl.h"
#include "NHMWIPpg.h"

#include ".\Common\NHDbgApi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CNHMWICtrl, COleControl)

/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CNHMWICtrl, COleControl)
	//{{AFX_MSG_MAP(CNHMWICtrl)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
	ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// AXEvent map : AXINAX

BEGIN_EVENTSINK_MAP(CNHMWICtrl, COleControl)
    //{{AFX_EVENTSINK_MAP(CNHMWICtrl)

// ---------------------------------------------------------------------------- 
	ON_EVENT(CNHMWICtrl, IDC_NXCARDREADER, 1 /* FatalError */, OnFatalErrorNxcardreader, VTS_BSTR VTS_I4)
	ON_EVENT(CNHMWICtrl, IDC_NXCARDREADER, 2 /* DeviceError */, OnDeviceErrorNxcardreader, VTS_BSTR VTS_I4)
	ON_EVENT(CNHMWICtrl, IDC_NXCARDREADER, 3 /* LockGranted */, OnLockGrantedNxcardreader, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXCARDREADER, 4 /* LockReleased */, OnLockReleasedNxcardreader, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXCARDREADER, 5 /* LockTimeout */, OnLockTimeoutNxcardreader, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXCARDREADER, 6 /* DeviceStatusChanged */, OnDeviceStatusChangedNxcardreader, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXCARDREADER, 7 /* MediaStatusChanged */, OnMediaStatusChangedNxcardreader, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXCARDREADER, 8 /* RetainBinStatusChanged */, OnRetainBinStatusChangedNxcardreader, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXCARDREADER, 9 /* SecurityStatusChanged */, OnSecurityStatusChangedNxcardreader, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXCARDREADER, 10 /* CardsStatusChanged */, OnCardsStatusChangedNxcardreader, VTS_I2)
	ON_EVENT(CNHMWICtrl, IDC_NXCARDREADER, 11 /* ReadComplete */, OnReadCompleteNxcardreader, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXCARDREADER, 12 /* WriteComplete */, OnWriteCompleteNxcardreader, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXCARDREADER, 13 /* EjectComplete */, OnEjectCompleteNxcardreader, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXCARDREADER, 14 /* RetainComplete */, OnRetainCompleteNxcardreader, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXCARDREADER, 15 /* ChipIOComplete */, OnChipIOCompleteNxcardreader, VTS_BSTR VTS_VARIANT)
	ON_EVENT(CNHMWICtrl, IDC_NXCARDREADER, 16 /* ChipIOFailure */, OnChipIOFailureNxcardreader, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXCARDREADER, 17 /* MediaInserted */, OnMediaInsertedNxcardreader, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXCARDREADER, 18 /* InvalidMedia */, OnInvalidMediaNxcardreader, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXCARDREADER, 19 /* InvalidTrackData */, OnInvalidTrackDataNxcardreader, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXCARDREADER, 20 /* MediaRemoved */, OnMediaRemovedNxcardreader, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXCARDREADER, 21 /* RetainBinThreshold */, OnRetainBinThresholdNxcardreader, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXCARDREADER, 22 /* Timeout */, OnTimeoutNxcardreader, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXCARDREADER, 23 /* ChipPowerStatusChanged */, OnChipPowerStatusChangedNxcardreader, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXCARDREADER, 24 /* MediaDetected */, OnMediaDetectedNxcardreader, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXCARDREADER, 25 /* ResetComplete */, OnResetCompleteNxcardreader, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXCARDREADER, 26 /* ChipPowerComplete */, OnChipPowerCompleteNxcardreader, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXCARDREADER, 27 /* AcceptCancelled */, OnAcceptCancelledNxcardreader, VTS_NONE)

// ----------------------------------------------------------------------------
	ON_EVENT(CNHMWICtrl, IDC_NXPIN, 1 /* FatalError */, OnFatalErrorNxpin, VTS_BSTR VTS_I4)
	ON_EVENT(CNHMWICtrl, IDC_NXPIN, 2 /* DeviceError */, OnDeviceErrorNxpin, VTS_BSTR VTS_I4)
	ON_EVENT(CNHMWICtrl, IDC_NXPIN, 3 /* LockGranted */, OnLockGrantedNxpin, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXPIN, 4 /* LockReleased */, OnLockReleasedNxpin, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXPIN, 5 /* LockTimeout */, OnLockTimeoutNxpin, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXPIN, 6 /* EncryptComplete */, OnEncryptCompleteNxpin, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXPIN, 7 /* DecryptComplete */, OnDecryptCompleteNxpin, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXPIN, 8 /* MACingComplete */, OnMACingCompleteNxpin, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXPIN, 9 /* KeyImported */, OnKeyImportedNxpin, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXPIN, 10 /* ReadPinComplete */, OnReadPinCompleteNxpin, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXPIN, 11 /* ReadPinCancelled */, OnReadPinCancelledNxpin, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXPIN, 12 /* KeyPressed */, OnKeyPressedNxpin, VTS_BSTR VTS_I4)
	ON_EVENT(CNHMWICtrl, IDC_NXPIN, 13 /* PinBlockComplete */, OnPinBlockCompleteNxpin, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXPIN, 14 /* ReadDataComplete */, OnReadDataCompleteNxpin, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXPIN, 15 /* ReadDataCancelled */, OnReadDataCancelledNxpin, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXPIN, 16 /* EDMInitialized */, OnEDMInitializedNxpin, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXPIN, 17 /* DeviceStatusChanged */, OnDeviceStatusChangedNxpin, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXPIN, 18 /* Timeout */, OnTimeoutNxpin, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXPIN, 19 /* IllegalKeyAccessed */, OnIllegalKeyAccessedNxpin, VTS_BSTR VTS_I4)
	ON_EVENT(CNHMWICtrl, IDC_NXPIN, 20 /* ResetComplete */, OnResetCompleteNxpin, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXPIN, 21 /* KeyDeleted */, OnKeyDeletedNxpin, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXPIN, 22 /* RandomNumberGenerated */, OnRandomNumberGeneratedNxpin, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXPIN, 23 /* VerifyLocalDESComplete */, OnVerifyLocalDESCompleteNxpin, VTS_I4)
	ON_EVENT(CNHMWICtrl, IDC_NXPIN, 24 /* StartKeyExchangeComplete */, OnStartKeyExchangeCompleteNxpin, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXPIN, 25 /* RSAPublicKeyImported */, OnRSAPublicKeyImportedNxpin, VTS_BSTR VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXPIN, 26 /* RSAIssuerSignedItemExported */, OnRSAIssuerSignedItemExportedNxpin, VTS_BSTR VTS_BSTR VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXPIN, 27 /* RSASignedDESKeyImported */, OnRSASignedDESKeyImportedNxpin, VTS_BSTR VTS_BSTR VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXPIN, 28 /* RSAKeyPairGenerated */, OnRSAKeyPairGeneratedNxpin, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXPIN, 29 /* RSAEPPSignedItemExported */, OnRSAEPPSignedItemExportedNxpin, VTS_BSTR VTS_BSTR VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXPIN, 30 /* LoadCertificateComplete */, OnLoadCertificateCompleteNxpin, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXPIN, 31 /* GetCertificateComplete */, OnGetCertificateCompleteNxpin, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXPIN, 32 /* ReplaceCertificateComplete */, OnReplaceCertificateCompleteNxpin, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXPIN, 33 /* RSAEncipheredKeyImported */, OnRSAEncipheredKeyImportedNxpin, VTS_BSTR VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXPIN, 34 /* CertificateChanged */, OnCertificateChangedNxpin, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXPIN, 35 /* KCVGenerated */, OnKCVGeneratedNxpin, VTS_BSTR VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXPIN, 36 /* SecureKeyEntryComplete */, OnSecureKeyEntryCompleteNxpin, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXPIN, 37 /* SecureKeyEntryCancelled */, OnSecureKeyEntryCancelledNxpin, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXPIN, 38 /* SecureKeyEntryIncomplete */, OnSecureKeyEntryIncompleteNxpin, VTS_NONE)

	// [#RWC6-151] US Kook 2020.07.14 TR34
	ON_EVENT(CNHMWICtrl, IDC_NXPIN, 39 /* KeyBlockImported */, OnKeyBlockImported, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXPIN, 40 /* RSAEncipheredKeyExImported */, OnRSAEncipheredKeyExImported, VTS_BSTR VTS_BSTR VTS_BSTR VTS_BSTR VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXPIN, 41 /* LoadCertificateExComplete */, OnLoadCertificateExComplete, VTS_BSTR VTS_BSTR)

// ----------------------------------------------------------------------------
	ON_EVENT(CNHMWICtrl, IDC_NXRECEIPTPRINTER, 1 /* FatalError */, OnFatalErrorNxreceiptprinter, VTS_BSTR VTS_I4)
	ON_EVENT(CNHMWICtrl, IDC_NXRECEIPTPRINTER, 2 /* DeviceError */, OnDeviceErrorNxreceiptprinter, VTS_BSTR VTS_I4)
	ON_EVENT(CNHMWICtrl, IDC_NXRECEIPTPRINTER, 6 /* ControlMediaComplete */, OnControlMediaCompleteNxreceiptprinter, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXRECEIPTPRINTER, 7 /* PrintFormComplete */, OnPrintFormCompleteNxreceiptprinter, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXRECEIPTPRINTER, 8 /* FieldWarning */, OnFieldWarningNxreceiptprinter, VTS_BSTR VTS_BSTR VTS_I2)
	ON_EVENT(CNHMWICtrl, IDC_NXRECEIPTPRINTER, 9 /* FieldError */, OnFieldErrorNxreceiptprinter, VTS_BSTR VTS_BSTR VTS_I2)
	ON_EVENT(CNHMWICtrl, IDC_NXRECEIPTPRINTER, 10 /* Timeout */, OnTimeoutNxreceiptprinter, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXRECEIPTPRINTER, 11 /* SendRawDataComplete */, OnSendRawDataCompleteNxreceiptprinter, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXRECEIPTPRINTER, 12 /* DeviceStatusChanged */, OnDeviceStatusChangedNxreceiptprinter, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXRECEIPTPRINTER, 13 /* MediaStatusChanged */, OnMediaStatusChangedNxreceiptprinter, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXRECEIPTPRINTER, 17 /* ResetComplete */, OnResetCompleteNxreceiptprinter, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXRECEIPTPRINTER, 20 /* PaperStatusChanged */, OnPaperStatusChangedNxreceiptprinter, VTS_BSTR VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXRECEIPTPRINTER, 23 /* MediaTaken */, OnMediaTakenNxreceiptprinter, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXRECEIPTPRINTER, 24 /* WaitCancelled */, OnWaitCancelledNxreceiptprinter, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXRECEIPTPRINTER, 25 /* ResetCountComplete */, OnResetCountCompleteNxreceiptprinter, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXRECEIPTPRINTER, 26 /* RetractMediaComplete */, OnRetractMediaCompleteNxreceiptprinter, VTS_I2)
	ON_EVENT(CNHMWICtrl, IDC_NXRECEIPTPRINTER, 27 /* RetractBinStatusChanged */, OnRetractBinStatusChangedNxreceiptprinter, VTS_I2 VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXRECEIPTPRINTER, 28 /* ReadFormComplete */, OnReadFormCompleteNxreceiptprinter, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXRECEIPTPRINTER, 29 /* MediaExtentsComplete */, OnMediaExtentsCompleteNxreceiptprinter, VTS_I4 VTS_I4)

// ----------------------------------------------------------------------------
	ON_EVENT(CNHMWICtrl, IDC_NXJOURNALPRINTER, 1 /* FatalError */, OnFatalErrorNxjournalprinter, VTS_BSTR VTS_I4)
	ON_EVENT(CNHMWICtrl, IDC_NXJOURNALPRINTER, 2 /* DeviceError */, OnDeviceErrorNxjournalprinter, VTS_BSTR VTS_I4)
	ON_EVENT(CNHMWICtrl, IDC_NXJOURNALPRINTER, 3 /* LockGranted */, OnLockGrantedNxjournalprinter, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXJOURNALPRINTER, 4 /* LockReleased */, OnLockReleasedNxjournalprinter, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXJOURNALPRINTER, 5 /* LockTimeout */, OnLockTimeoutNxjournalprinter, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXJOURNALPRINTER, 6 /* ControlMediaComplete */, OnControlMediaCompleteNxjournalprinter, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXJOURNALPRINTER, 7 /* PrintFormComplete */, OnPrintFormCompleteNxjournalprinter, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXJOURNALPRINTER, 8 /* FieldWarning */, OnFieldWarningNxjournalprinter, VTS_BSTR VTS_BSTR VTS_I2)
	ON_EVENT(CNHMWICtrl, IDC_NXJOURNALPRINTER, 9 /* FieldError */, OnFieldErrorNxjournalprinter, VTS_BSTR VTS_BSTR VTS_I2)
	ON_EVENT(CNHMWICtrl, IDC_NXJOURNALPRINTER, 10 /* Timeout */, OnTimeoutNxjournalprinter, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXJOURNALPRINTER, 11 /* SendRawDataComplete */, OnSendRawDataCompleteNxjournalprinter, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXJOURNALPRINTER, 12 /* DeviceStatusChanged */, OnDeviceStatusChangedNxjournalprinter, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXJOURNALPRINTER, 13 /* MediaStatusChanged */, OnMediaStatusChangedNxjournalprinter, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXJOURNALPRINTER, 14 /* TonerStatusChanged */, OnTonerStatusChangedNxjournalprinter, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXJOURNALPRINTER, 15 /* InkStatusChanged */, OnInkStatusChangedNxjournalprinter, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXJOURNALPRINTER, 16 /* LampStatusChanged */, OnLampStatusChangedNxjournalprinter, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXJOURNALPRINTER, 17 /* ResetComplete */, OnResetCompleteNxjournalprinter, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXJOURNALPRINTER, 18 /* MediaDetected */, OnMediaDetectedNxjournalprinter, VTS_BSTR VTS_I2)
	ON_EVENT(CNHMWICtrl, IDC_NXJOURNALPRINTER, 19 /* DispensePaperComplete */, OnDispensePaperCompleteNxjournalprinter, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXJOURNALPRINTER, 20 /* PaperStatusChanged */, OnPaperStatusChangedNxjournalprinter, VTS_BSTR VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXJOURNALPRINTER, 21 /* MediaInserted */, OnMediaInsertedNxjournalprinter, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXJOURNALPRINTER, 22 /* AcceptCancelled */, OnAcceptCancelledNxjournalprinter, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXJOURNALPRINTER, 23 /* MediaTaken */, OnMediaTakenNxjournalprinter, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXJOURNALPRINTER, 24 /* WaitCancelled */, OnWaitCancelledNxjournalprinter, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXJOURNALPRINTER, 25 /* ResetCountComplete */, OnResetCountCompleteNxjournalprinter, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXJOURNALPRINTER, 26 /* RetractMediaComplete */, OnRetractMediaCompleteNxjournalprinter, VTS_I2)
	ON_EVENT(CNHMWICtrl, IDC_NXJOURNALPRINTER, 27 /* RetractBinStatusChanged */, OnRetractBinStatusChangedNxjournalprinter, VTS_I2 VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXJOURNALPRINTER, 28 /* ReadFormComplete */, OnReadFormCompleteNxjournalprinter, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXJOURNALPRINTER, 29 /* MediaExtentsComplete */, OnMediaExtentsCompleteNxjournalprinter, VTS_I4 VTS_I4)

// ---------------------------------------------------------------------------- // 2005.05.26
	ON_EVENT(CNHMWICtrl, IDC_NXDOORS, 1 /* FatalError */, OnFatalErrorNxdoors, VTS_BSTR VTS_I4)
	ON_EVENT(CNHMWICtrl, IDC_NXDOORS, 2 /* DeviceError */, OnDeviceErrorNxdoors, VTS_BSTR VTS_I4)
	ON_EVENT(CNHMWICtrl, IDC_NXDOORS, 3 /* LockGranted */, OnLockGrantedNxdoors, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXDOORS, 4 /* LockReleased */, OnLockReleasedNxdoors, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXDOORS, 5 /* LockTimeout */, OnLockTimeoutNxdoors, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXDOORS, 6 /* DeviceStatusChanged */, OnDeviceStatusChangedNxdoors, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXDOORS, 7 /* CabinetChanged */, OnCabinetChangedNxdoors, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXDOORS, 8 /* SafeChanged */, OnSafeChangedNxdoors, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXDOORS, 9 /* ShieldChanged */, OnShieldChangedNxdoors, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXDOORS, 10 /* Timeout */, OnTimeoutNxdoors, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXDOORS, 11 /* ResetComplete */, OnResetCompleteNxdoors, VTS_NONE)

// ----------------------------------------------------------------------------// 2004.01.16_1.2_2003
	ON_EVENT(CNHMWICtrl, IDC_NXCASHDISPENSER, 1 /* FatalError */, OnFatalErrorNxCashDispenser, VTS_BSTR VTS_I4)
	ON_EVENT(CNHMWICtrl, IDC_NXCASHDISPENSER, 2 /* DeviceError */, OnDeviceErrorNxCashDispenser, VTS_BSTR VTS_I4)
	ON_EVENT(CNHMWICtrl, IDC_NXCASHDISPENSER, 3 /* LockGranted */, OnLockGrantedNxCashDispenser, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXCASHDISPENSER, 4 /* LockReleased */, OnLockReleasedNxCashDispenser, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXCASHDISPENSER, 5 /* LockTimeout */, OnLockTimeoutNxCashDispenser, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXCASHDISPENSER, 6 /* Timeout */, OnTimeoutNxCashDispenser, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXCASHDISPENSER, 7 /* DenominateComplete */, OnDenominateCompleteNxCashDispenser, VTS_VARIANT)
	ON_EVENT(CNHMWICtrl, IDC_NXCASHDISPENSER, 8 /* NotDispensable */, OnNotDispensableNxCashDispenser, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXCASHDISPENSER, 9 /* NoteError */, OnNoteErrorNxCashDispenser, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXCASHDISPENSER, 10 /* InCompleteDispense */, OnInCompleteDispenseNxCashDispenser, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXCASHDISPENSER, 11 /* DispenseComplete */, OnDispenseCompleteNxCashDispenser, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXCASHDISPENSER, 12 /* CountComplete */, OnCountCompleteNxCashDispenser, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXCASHDISPENSER, 13 /* ItemsPresented */, OnItemsPresentedNxCashDispenser, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXCASHDISPENSER, 14 /* PresentComplete */, OnPresentCompleteNxCashDispenser, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXCASHDISPENSER, 15 /* ItemsTaken */, OnItemsTakenNxCashDispenser, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXCASHDISPENSER, 16 /* RetractComplete */, OnRetractCompleteNxCashDispenser, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXCASHDISPENSER, 17 /* RejectComplete */, OnRejectCompleteNxCashDispenser, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXCASHDISPENSER, 18 /* OpenShutterComplete */, OnOpenShutterCompleteNxCashDispenser, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXCASHDISPENSER, 19 /* CloseShutterComplete */, OnCloseShutterCompleteNxCashDispenser, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXCASHDISPENSER, 20 /* CashUnitError */, OnCashUnitErrorNxCashDispenser, VTS_I2)
	ON_EVENT(CNHMWICtrl, IDC_NXCASHDISPENSER, 21 /* OpenSafeDoorComplete */, OnOpenSafeDoorCompleteNxCashDispenser, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXCASHDISPENSER, 22 /* CalibrateComplete */, OnCalibrateCompleteNxCashDispenser, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXCASHDISPENSER, 23 /* CashUnitThreshold */, OnCashUnitThresholdNxCashDispenser, VTS_I2)
	ON_EVENT(CNHMWICtrl, IDC_NXCASHDISPENSER, 24 /* CashUnitChanged */, OnCashUnitChangedNxCashDispenser, VTS_I2)
	ON_EVENT(CNHMWICtrl, IDC_NXCASHDISPENSER, 25 /* ResetComplete */, OnResetCompleteNxCashDispenser, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXCASHDISPENSER, 26 /* MediaDetected */, OnMediaDetectedNxCashDispenser, VTS_I2)
	ON_EVENT(CNHMWICtrl, IDC_NXCASHDISPENSER, 27 /* DeviceStatusChanged */, OnDeviceStatusChangedNxCashDispenser, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXCASHDISPENSER, 28 /* SafeDoorStatusChanged */, OnSafeDoorStatusChangedNxCashDispenser, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXCASHDISPENSER, 29 /* DispenserStatusChanged */, OnDispenserStatusChangedNxCashDispenser, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXCASHDISPENSER, 30 /* StackerStatusChanged */, OnStackerStatusChangedNxCashDispenser, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXCASHDISPENSER, 31 /* ShutterStatusChanged */, OnShutterStatusChangedNxCashDispenser, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXCASHDISPENSER, 32 /* TransportStatusChanged */, OnTransportStatusChangedNxCashDispenser, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXCASHDISPENSER, 33 /* TransportStateStatusChanged */, OnTransportStateStatusChangedNxCashDispenser, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXCASHDISPENSER, 34 /* PositionStatusChanged */, OnPositionStatusChangedNxCashDispenser, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXCASHDISPENSER, 35 /* CountChanged */, OnCountChangedNxCashDispenser, VTS_NONE)

// ---------------------------------------------------------------------------- // 2005.03.30
	ON_EVENT(CNHMWICtrl, IDC_NXSENSORS, 1 /* FatalError */, OnFatalErrorNxsensors, VTS_BSTR VTS_I4)
	ON_EVENT(CNHMWICtrl, IDC_NXSENSORS, 2 /* DeviceError */, OnDeviceErrorNxsensors, VTS_BSTR VTS_I4)
	ON_EVENT(CNHMWICtrl, IDC_NXSENSORS, 6 /* DeviceStatusChanged */, OnDeviceStatusChangedNxsensors, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXSENSORS, 7 /* OperatorSwitchChanged */, OnOperatorSwitchChangedNxsensors, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXSENSORS, 8 /* TamperChanged */, OnTamperChangedNxsensors, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXSENSORS, 9 /* InternalTamperChanged */, OnInternalTamperChangedNxsensors, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXSENSORS, 10 /* SeismicChanged */, OnSeismicChangedNxsensors, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXSENSORS, 11 /* HeatChanged */, OnHeatChangedNxsensors, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXSENSORS, 14 /* ResetComplete */, OnResetCompleteNxsensors, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXSENSORS, 15 /* EnhancedAudioChanged */, OnEnhancedAudioChangedNxsensors, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXSENSORS, 12 /* ProximityChanged */, OnProximityChangedNxsensors, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXSENSORS, 13 /* AmblightChanged */, OnAmblightChangedNxsensors, VTS_BSTR)

// ---------------------------------------------------------------------------- // 2005.03.30
	ON_EVENT(CNHMWICtrl, IDC_NXGUIDLIGHTS, 1 /* FatalError */, OnFatalErrorNxguidlights, VTS_BSTR VTS_I4)
	ON_EVENT(CNHMWICtrl, IDC_NXGUIDLIGHTS, 2 /* DeviceError */, OnDeviceErrorNxguidlights, VTS_BSTR VTS_I4)
	ON_EVENT(CNHMWICtrl, IDC_NXGUIDLIGHTS, 3 /* LockGranted */, OnLockGrantedNxguidlights, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXGUIDLIGHTS, 4 /* LockReleased */, OnLockReleasedNxguidlights, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXGUIDLIGHTS, 5 /* LockTimeout */, OnLockTimeoutNxguidlights, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXGUIDLIGHTS, 6 /* DeviceStatusChanged */, OnDeviceStatusChangedNxguidlights, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXGUIDLIGHTS, 7 /* GuidLightChanged */, OnGuidLightChangedNxguidlights, VTS_BSTR VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXGUIDLIGHTS, 8 /* ResetComplete */, OnResetCompleteNxguidlights, VTS_NONE)

	// [#2325] NH KSK 2015.01.20
	ON_EVENT(CNHMWICtrl, IDC_NXRFIDREADER, 1 /* FatalError */, OnFatalErrorNxrfidreader, VTS_BSTR VTS_I4)
	ON_EVENT(CNHMWICtrl, IDC_NXRFIDREADER, 2 /* DeviceError */, OnDeviceErrorNxrfidreader, VTS_BSTR VTS_I4)
	ON_EVENT(CNHMWICtrl, IDC_NXRFIDREADER, 3 /* LockGranted */, OnLockGrantedNxrfidreader, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXRFIDREADER, 4 /* LockReleased */, OnLockReleasedNxrfidreader, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXRFIDREADER, 5 /* LockTimeout */, OnLockTimeoutNxrfidreader, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXRFIDREADER, 6 /* DeviceStatusChanged */, OnDeviceStatusChangedNxrfidreader, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXRFIDREADER, 7 /* MediaStatusChanged */, OnMediaStatusChangedNxrfidreader, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXRFIDREADER, 8 /* RetainBinStatusChanged */, OnRetainBinStatusChangedNxrfidreader, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXRFIDREADER, 9 /* SecurityStatusChanged */, OnSecurityStatusChangedNxrfidreader, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXRFIDREADER, 10 /* CardsStatusChanged */, OnCardsStatusChangedNxrfidreader, VTS_I2)
	ON_EVENT(CNHMWICtrl, IDC_NXRFIDREADER, 11 /* ReadComplete */, OnReadCompleteNxrfidreader, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXRFIDREADER, 12 /* WriteComplete */, OnWriteCompleteNxrfidreader, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXRFIDREADER, 13 /* EjectComplete */, OnEjectCompleteNxrfidreader, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXRFIDREADER, 14 /* RetainComplete */, OnRetainCompleteNxrfidreader, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXRFIDREADER, 15 /* ChipIOComplete */, OnChipIOCompleteNxrfidreader, VTS_BSTR VTS_VARIANT)
	ON_EVENT(CNHMWICtrl, IDC_NXRFIDREADER, 16 /* ChipIOFailure */, OnChipIOFailureNxrfidreader, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXRFIDREADER, 17 /* MediaInserted */, OnMediaInsertedNxrfidreader, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXRFIDREADER, 18 /* InvalidMedia */, OnInvalidMediaNxrfidreader, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXRFIDREADER, 19 /* InvalidTrackData */, OnInvalidTrackDataNxrfidreader, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXRFIDREADER, 20 /* MediaRemoved */, OnMediaRemovedNxrfidreader, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXRFIDREADER, 21 /* RetainBinThreshold */, OnRetainBinThresholdNxrfidreader, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXRFIDREADER, 22 /* Timeout */, OnTimeoutNxrfidreader, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXRFIDREADER, 23 /* ChipPowerStatusChanged */, OnChipPowerStatusChangedNxrfidreader, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXRFIDREADER, 24 /* MediaDetected */, OnMediaDetectedNxrfidreader, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXRFIDREADER, 25 /* ResetComplete */, OnResetCompleteNxrfidreader, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXRFIDREADER, 26 /* ChipPowerComplete */, OnChipPowerCompleteNxrfidreader, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXRFIDREADER, 27 /* AcceptCancelled */, OnAcceptCancelledNxrfidreader, VTS_NONE)
	// end of [#2325]

	// [#GLDV-3005] US Kook 2021.10.25 Support Side Car
	// BCR EVENTS
	ON_EVENT(CNHMWICtrl, IDC_NXBARCODEREADER, 1 /* FatalError */, OnFatalErrorNxBarcodeReader, VTS_BSTR VTS_I4)
	ON_EVENT(CNHMWICtrl, IDC_NXBARCODEREADER, 2 /* DeviceError */, OnDeviceErrorNxBarcodeReader, VTS_BSTR VTS_I4)
	//ON_EVENT(CNHMWICtrl, IDC_NXBARCODEREADER, 3 /* LockGranted */, OnLockGrantedNxBarcodeReader, VTS_NONE)
	//ON_EVENT(CNHMWICtrl, IDC_NXBARCODEREADER, 4 /* LockReleased */, OnLockReleasedNxBarcodeReader, VTS_NONE)
	//ON_EVENT(CNHMWICtrl, IDC_NXBARCODEREADER, 5 /* LockTimeout */, OnLockTimeoutNxBarcodeReader, VTS_NONE)
	//ON_EVENT(CNHMWICtrl, IDC_NXBARCODEREADER, 6 /* ControlMediaComplete */, OnControlMediaCompleteNxBarcodeReader, VTS_BSTR)
	//ON_EVENT(CNHMWICtrl, IDC_NXBARCODEREADER, 7 /* PrintFormComplete */, OnPrintFormCompleteNxBarcodeReader, VTS_NONE)
	//ON_EVENT(CNHMWICtrl, IDC_NXBARCODEREADER, 8 /* FieldWarning */, OnFieldWarningNxBarcodeReader, VTS_BSTR VTS_BSTR VTS_I2)
	//ON_EVENT(CNHMWICtrl, IDC_NXBARCODEREADER, 9 /* FieldError */, OnFieldErrorNxBarcodeReader, VTS_BSTR VTS_BSTR VTS_I2)
	ON_EVENT(CNHMWICtrl, IDC_NXBARCODEREADER, 10 /* Timeout */, OnTimeoutNxBarcodeReader, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXBARCODEREADER, 11 /* SendRawDataComplete */, OnSendRawDataCompleteNxBarcodeReader, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXBARCODEREADER, 12 /* DeviceStatusChanged */, OnDeviceStatusChangedNxBarcodeReader, VTS_BSTR)
	//ON_EVENT(CNHMWICtrl, IDC_NXBARCODEREADER, 13 /* MediaStatusChanged */, OnMediaStatusChangedNxBarcodeReader, VTS_BSTR)
	//ON_EVENT(CNHMWICtrl, IDC_NXBARCODEREADER, 14 /* TonerStatusChanged */, OnTonerStatusChangedNxBarcodeReader, VTS_BSTR)
	//ON_EVENT(CNHMWICtrl, IDC_NXBARCODEREADER, 15 /* InkStatusChanged */, OnInkStatusChangedNxBarcodeReader, VTS_BSTR)
	//ON_EVENT(CNHMWICtrl, IDC_NXBARCODEREADER, 16 /* LampStatusChanged */, OnLampStatusChangedNxBarcodeReader, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXBARCODEREADER, 17 /* ResetComplete */, OnResetCompleteNxBarcodeReader, VTS_NONE)
	//ON_EVENT(CNHMWICtrl, IDC_NXBARCODEREADER, 18 /* MediaDetected */, OnMediaDetectedNxBarcodeReader, VTS_BSTR VTS_I2)
	//ON_EVENT(CNHMWICtrl, IDC_NXBARCODEREADER, 19 /* DispensePaperComplete */, OnDispensePaperCompleteNxBarcodeReader, VTS_NONE)
	//ON_EVENT(CNHMWICtrl, IDC_NXBARCODEREADER, 20 /* PaperStatusChanged */, OnPaperStatusChangedNxBarcodeReader, VTS_BSTR VTS_BSTR)
	//ON_EVENT(CNHMWICtrl, IDC_NXBARCODEREADER, 21 /* MediaInserted */, OnMediaInsertedNxBarcodeReader, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXBARCODEREADER, 22 /* AcceptCancelled */, OnAcceptCancelledNxBarcodeReader, VTS_NONE)
	//ON_EVENT(CNHMWICtrl, IDC_NXBARCODEREADER, 23 /* MediaTaken */, OnMediaTakenNxBarcodeReader, VTS_NONE)
	//ON_EVENT(CNHMWICtrl, IDC_NXBARCODEREADER, 24 /* WaitCancelled */, OnWaitCancelledNxBarcodeReader, VTS_NONE)
	//ON_EVENT(CNHMWICtrl, IDC_NXBARCODEREADER, 25 /* ResetCountComplete */, OnResetCountCompleteNxBarcodeReader, VTS_NONE)
	//ON_EVENT(CNHMWICtrl, IDC_NXBARCODEREADER, 26 /* RetractMediaComplete */, OnRetractMediaCompleteNxBarcodeReader, VTS_I2)
	//ON_EVENT(CNHMWICtrl, IDC_NXBARCODEREADER, 27 /* RetractBinStatusChanged */, OnRetractBinStatusChangedNxBarcodeReader, VTS_I2 VTS_BSTR)
	//ON_EVENT(CNHMWICtrl, IDC_NXBARCODEREADER, 28 /* ReadFormComplete */, OnReadFormCompleteNxBarcodeReader, VTS_BSTR)
	//ON_EVENT(CNHMWICtrl, IDC_NXBARCODEREADER, 29 /* MediaExtentsComplete */, OnMediaExtentsCompleteNxBarcodeReader, VTS_I4 VTS_I4)


	// BNA EVENTS
	ON_EVENT(CNHMWICtrl, IDC_NXBILLACCEPTOR, 1 /* FatalError */, OnFatalErrorNxCashAcceptor, VTS_BSTR VTS_I4)
	ON_EVENT(CNHMWICtrl, IDC_NXBILLACCEPTOR, 2 /* DeviceError */, OnDeviceErrorNxCashAcceptor, VTS_BSTR VTS_I4)
	//ON_EVENT(CNHMWICtrl, IDC_NXBILLACCEPTOR, 3 /* LockGranted */, OnLockGrantedNxCashAcceptor, VTS_NONE)
	//ON_EVENT(CNHMWICtrl, IDC_NXBILLACCEPTOR, 4 /* LockReleased */, OnLockReleasedNxCashAcceptor, VTS_NONE)
	//ON_EVENT(CNHMWICtrl, IDC_NXBILLACCEPTOR, 5 /* LockTimeout */, OnLockTimeoutNxCashAcceptor, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXBILLACCEPTOR, 6 /* Timeout */, OnTimeoutNxCashAcceptor, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXBILLACCEPTOR, 7 /* StartCashInComplete */, OnStartCashInCompleteNxCashAcceptor, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXBILLACCEPTOR, 8 /* AcceptCashComplete */, OnAcceptCashCompleteNxCashAcceptor, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXBILLACCEPTOR, 9 /* StoreCashComplete */, OnStoreCashCompleteNxCashAcceptor, VTS_I4)
	ON_EVENT(CNHMWICtrl, IDC_NXBILLACCEPTOR, 10 /* RollbackCashComplete */, OnRollbackCashCompleteNxCashAcceptor, VTS_NONE)
	//ON_EVENT(CNHMWICtrl, IDC_NXBILLACCEPTOR, 11 /* OpenShutterComplete */, OnOpenShutterCompleteNxCashAcceptor, VTS_BSTR)
	//ON_EVENT(CNHMWICtrl, IDC_NXBILLACCEPTOR, 12 /* CloseShutterComplete */, OnCloseShutterCompleteNxCashAcceptor, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXBILLACCEPTOR, 13 /* RetractComplete */, OnRetractCompleteNxCashAcceptor, VTS_NONE)
	//ON_EVENT(CNHMWICtrl, IDC_NXBILLACCEPTOR, 14 /* OpenSafeDoorComplete */, OnOpenSafeDoorCompleteNxCashAcceptor, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXBILLACCEPTOR, 15 /* ResetComplete */, OnResetCompleteNxCashAcceptor, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXBILLACCEPTOR, 16 /* CashUnitThreshold */, OnCashUnitThresholdNxCashAcceptor, VTS_I2)
	ON_EVENT(CNHMWICtrl, IDC_NXBILLACCEPTOR, 17 /* CashUnitChanged */, OnCashUnitChangedNxCashAcceptor, VTS_I2)
	ON_EVENT(CNHMWICtrl, IDC_NXBILLACCEPTOR, 18 /* CashUnitError */, OnCashUnitErrorNxCashAcceptor, VTS_I2)
	ON_EVENT(CNHMWICtrl, IDC_NXBILLACCEPTOR, 19 /* ItemsTaken */, OnItemsTakenNxCashAcceptor, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXBILLACCEPTOR, 20 /* ItemsRefused */, OnItemsRefusedNxCashAcceptor, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXBILLACCEPTOR, 21 /* ItemsInserted */, OnItemsInsertedNxCashAcceptor, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXBILLACCEPTOR, 22 /* MediaDetected */, OnMediaDetectedNxCashAcceptor, VTS_I2)
	ON_EVENT(CNHMWICtrl, IDC_NXBILLACCEPTOR, 23 /* StatusChanged */, OnStatusChangedNxCashAcceptor, VTS_BSTR)
	//ON_EVENT(CNHMWICtrl, IDC_NXBILLACCEPTOR, 24 /* SafeDoorStatusChanged */, OnSafeDoorStatusChangedNxCashAcceptor, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXBILLACCEPTOR, 25 /* AcceptorStatusChanged */, OnAcceptorStatusChangedNxCashAcceptor, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXBILLACCEPTOR, 26 /* StackerStatusChanged */, OnStackerStatusChangedNxCashAcceptor, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXBILLACCEPTOR, 27 /* ReaderStatusChanged */, OnReaderStatusChangedNxCashAcceptor, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXBILLACCEPTOR, 28 /* ShutterStatusChanged */, OnShutterStatusChangedNxCashAcceptor, VTS_BSTR VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXBILLACCEPTOR, 29 /* PositionStatusChanged */, OnPositionStatusChangedNxCashAcceptor, VTS_BSTR VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXBILLACCEPTOR, 30 /* TransportStatusChanged */, OnTransportStatusChangedNxCashAcceptor, VTS_BSTR VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXBILLACCEPTOR, 31 /* AcceptCancelled */, OnAcceptCancelledNxCashAcceptor, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXBILLACCEPTOR, 32 /* CreateP6SignatureComplete */, OnCreateP6SignatureCompleteNxCashAcceptor, VTS_I2 VTS_BSTR VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXBILLACCEPTOR, 33 /* GetP6SignatureComplete */, OnGetP6SignatureCompleteNxCashAcceptor, VTS_I2 VTS_BSTR VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXBILLACCEPTOR, 34 /* P6Inputed */, OnP6InputedNxCashAcceptor, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXBILLACCEPTOR, 35 /* ItemsPresented */, OnItemsPresentedNxCashAcceptor, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXBILLACCEPTOR, 36 /* NoteError */, OnNoteErrorNxCashAcceptor, VTS_BSTR)
	ON_EVENT(CNHMWICtrl, IDC_NXBILLACCEPTOR, 37 /* SetCashUnitInfoComplete */, OnSetCashUnitInfoCompleteNxCashAcceptor, VTS_NONE)
	ON_EVENT(CNHMWICtrl, IDC_NXBILLACCEPTOR, 38 /* SetCashInLimitComplete */, OnSetCashInLimitCompleteNxCashAcceptor, VTS_NONE)
	// TODO: register BillAcceptor events
	// end of [#GLDV-3005]

	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// Dispatch map

BEGIN_DISPATCH_MAP(CNHMWICtrl, COleControl)
	//{{AFX_DISPATCH_MAP(CNHMWICtrl)
	DISP_FUNCTION(CNHMWICtrl, "TestMethod", TestMethod, VT_BSTR, VTS_BSTR VTS_BSTR VTS_BSTR)
	DISP_FUNCTION(CNHMWICtrl, "ProcSetDeviceEvent", ProcSetDeviceEvent, VT_EMPTY, VTS_BSTR VTS_BSTR VTS_BSTR)
	DISP_FUNCTION(CNHMWICtrl, "ProcSetDeviceStatus", ProcSetDeviceStatus, VT_EMPTY, VTS_BSTR VTS_BSTR VTS_BSTR)
	DISP_FUNCTION(CNHMWICtrl, "McuEntryDisable", McuEntryDisable, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "McuGetCardData", McuGetCardData, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "McuInitialize", McuInitialize, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "McuRead", McuRead, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "McuRetract", McuRetract, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "McuWrite", McuWrite, VT_I4, VTS_BSTR)
	DISP_FUNCTION(CNHMWICtrl, "SprGetPaperStatus", SprGetPaperStatus, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "JprEmbossPrint", JprEmbossPrint, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "JprGetPaperStatus", JprGetPaperStatus, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "JprInitialize", JprInitialize, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "McuDeinitialize", McuDeinitialize, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "CheckDeviceAction", CheckDeviceAction, VT_BSTR, VTS_I4 VTS_I4)
	DISP_FUNCTION(CNHMWICtrl, "McuWaitTaken", McuWaitTaken, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "SprDeinitialize", SprDeinitialize, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "SprWaitTaken", SprWaitTaken, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "ScanDeviceAction", ScanDeviceAction, VT_I4, VTS_I4 VTS_I4 VTS_I4)
	DISP_FUNCTION(CNHMWICtrl, "McuEntryEnable", McuEntryEnable, VT_I4, VTS_I4)
	DISP_FUNCTION(CNHMWICtrl, "GetErrorDevice", GetErrorDevice, VT_I4, VTS_I4)
	DISP_FUNCTION(CNHMWICtrl, "McuGetMaterialInfo", McuGetMaterialInfo, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "SprGetMaterialInfo", SprGetMaterialInfo, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "JprGetMaterialInfo", JprGetMaterialInfo, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "SprInitialize", SprInitialize, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "JprDeinitialize", JprDeinitialize, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "McuEject", McuEject, VT_I4, VTS_I4)
	DISP_FUNCTION(CNHMWICtrl, "SprPrint", SprPrint, VT_I4, VTS_BSTR)
	DISP_FUNCTION(CNHMWICtrl, "JprPrint", JprPrint, VT_I4, VTS_BSTR)
	DISP_FUNCTION(CNHMWICtrl, "JprGetDeviceStatus", JprGetDeviceStatus, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "SprGetDeviceStatus", SprGetDeviceStatus, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "SprEject", SprEject, VT_I4, VTS_I4)
	DISP_FUNCTION(CNHMWICtrl, "SprRetract", SprRetract, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "McuGetDeviceStatus", McuGetDeviceStatus, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "DorInitialize", DorInitialize, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "DorDeinitialize", DorDeinitialize, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "DorGetDeviceStatus", DorGetDeviceStatus, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "DorGetDoorStatus", DorGetDoorStatus, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "LgtInitialize", LgtInitialize, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "LgtDeinitialize", LgtDeinitialize, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "LgtSetFlicker", LgtSetFlicker, VT_I4, VTS_I4 VTS_I4)
	DISP_FUNCTION(CNHMWICtrl, "LgtSetIndicator", LgtSetIndicator, VT_I4, VTS_I4)
	DISP_FUNCTION(CNHMWICtrl, "McuCancelWaitTaken", McuCancelWaitTaken, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "McuGetErrorCode", McuGetErrorCode, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "SprGetErrorCode", SprGetErrorCode, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "JprGetErrorCode", JprGetErrorCode, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "SprClearErrorCode", SprClearErrorCode, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "JprClearErrorCode", JprClearErrorCode, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "McuClearErrorCode", McuClearErrorCode, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "DorClearErrorCode", DorClearErrorCode, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "JprGetSensorInfo", JprGetSensorInfo, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "SprGetSensorInfo", SprGetSensorInfo, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "McuGetSensorInfo", McuGetSensorInfo, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "SprClearRetractCnt", SprClearRetractCnt, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "McuClearRetractCnt", McuClearRetractCnt, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "DorGetErrorCode", DorGetErrorCode, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "SprGetRetractCnt", SprGetRetractCnt, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "McuGetRetractCnt", McuGetRetractCnt, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "ScrInitialize", ScrInitialize, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "ScrGetKeyString", ScrGetKeyString, VT_BSTR, VTS_I4)
	DISP_FUNCTION(CNHMWICtrl, "ScrSetData", ScrSetData, VT_I4, VTS_BSTR VTS_BSTR)
	DISP_FUNCTION(CNHMWICtrl, "ScrDisplayScreen", ScrDisplayScreen, VT_I4, VTS_BSTR VTS_I4 VTS_BSTR)
	DISP_FUNCTION(CNHMWICtrl, "SprCancelWaitTaken", SprCancelWaitTaken, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "LgtClearErrorCode", LgtClearErrorCode, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "LgtGetDeviceStatus", LgtGetDeviceStatus, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "LgtGetErrorCode", LgtGetErrorCode, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "ScrGetScreenData", ScrGetScreenData, VT_BSTR, VTS_I4)
	DISP_FUNCTION(CNHMWICtrl, "ScrSetScreenData", ScrSetScreenData, VT_I4, VTS_BSTR VTS_BSTR)
	DISP_FUNCTION(CNHMWICtrl, "ScrSetDisplayData", ScrSetDisplayData, VT_I4, VTS_BSTR VTS_BSTR)
	DISP_FUNCTION(CNHMWICtrl, "ScrGetKeyData", ScrGetKeyData, VT_BSTR, VTS_I4)
	DISP_FUNCTION(CNHMWICtrl, "ScrClearKeyData", ScrClearKeyData, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "GetTimeoutDevice", GetTimeoutDevice, VT_I4, VTS_I4)
	DISP_FUNCTION(CNHMWICtrl, "McuGetICData", McuGetICData, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "McuICEntryEnable", McuICEntryEnable, VT_I4, VTS_I4)
	DISP_FUNCTION(CNHMWICtrl, "McuICEntryDisable", McuICEntryDisable, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "McuICSendData", McuICSendData, VT_I4, VTS_I4 VTS_BSTR VTS_I4)
	DISP_FUNCTION(CNHMWICtrl, "McuICChipInitialize", McuICChipInitialize, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "CduInitialize", CduInitialize, VT_I4, VTS_I4)
	DISP_FUNCTION(CNHMWICtrl, "CduDeinitialize", CduDeinitialize, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "CduGetErrorCode", CduGetErrorCode, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "CduClearErrorCode", CduClearErrorCode, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "CduGetSensorInfo", CduGetSensorInfo, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "CduGetDeviceStatus", CduGetDeviceStatus, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "CduGetPosition", CduGetPosition, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "CduGetRejectCSTStatus", CduGetRejectCSTStatus, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "CduGetRecycleBoxStatus", CduGetRecycleBoxStatus, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "CduGetMaterialInfo", CduGetMaterialInfo, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "CduGetAvailWithdraw", CduGetAvailWithdraw, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "CduOpenShutter", CduOpenShutter, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "CduCloseShutter", CduCloseShutter, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "CduWaitTaken", CduWaitTaken, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "CduCancelWaitTaken", CduCancelWaitTaken, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "CduRetract", CduRetract, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "CduPresent", CduPresent, VT_I4, VTS_I4)
	DISP_FUNCTION(CNHMWICtrl, "PinInitialize", PinInitialize, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "PinDeinitialize", PinDeinitialize, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "PinGetErrorCode", PinGetErrorCode, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "PinClearErrorCode", PinClearErrorCode, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "PinEntryDisable", PinEntryDisable, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "PinGetPinKeyData", PinGetPinKeyData, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "CduGetCSTStatus", CduGetCSTStatus, VT_I4, VTS_I4)
	DISP_FUNCTION(CNHMWICtrl, "CduGetNumberOfCST", CduGetNumberOfCST, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "CduGetValueOfCash", CduGetValueOfCash, VT_I4, VTS_I4)
	DISP_FUNCTION(CNHMWICtrl, "CduGetNumberOfCash", CduGetNumberOfCash, VT_I4, VTS_I4)
	DISP_FUNCTION(CNHMWICtrl, "CduGetNumberOfSetCash", CduGetNumberOfSetCash, VT_I4, VTS_I4)
	DISP_FUNCTION(CNHMWICtrl, "CduDispenseCount", CduDispenseCount, VT_I4, VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	DISP_FUNCTION(CNHMWICtrl, "CduDispenseAmount", CduDispenseAmount, VT_I4, VTS_I4)
	DISP_FUNCTION(CNHMWICtrl, "CduDispenseAndPresentAmount", CduDispenseAndPresentAmount, VT_I4, VTS_I4  VTS_I4)
	DISP_FUNCTION(CNHMWICtrl, "PinEntryEnable", PinEntryEnable, VT_I4, VTS_I4 VTS_I4 VTS_I4 VTS_BOOL VTS_BSTR VTS_BSTR VTS_BSTR VTS_I4)
	DISP_FUNCTION(CNHMWICtrl, "PinMacingData", PinMacingData, VT_I4, VTS_BSTR)
	DISP_FUNCTION(CNHMWICtrl, "PinGetMacingData", PinGetMacingData, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "PinLoadIV", PinLoadIV, VT_I4, VTS_BSTR)
	DISP_FUNCTION(CNHMWICtrl, "PinLoadKey", PinLoadKey, VT_I4, VTS_BSTR VTS_BSTR)
	DISP_FUNCTION(CNHMWICtrl, "PinGetDeviceStatus", PinGetDeviceStatus, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "CduGetDispenseOfCST", CduGetDispenseOfCST, VT_BSTR, VTS_I4)
	DISP_FUNCTION(CNHMWICtrl, "SnsDeinitialize", SnsDeinitialize, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "SnsClearErrorCode", SnsClearErrorCode, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "SnsGetErrorCode", SnsGetErrorCode, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "SnsGetDeviceStatus", SnsGetDeviceStatus, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "SnsGetOperatorSwitch", SnsGetOperatorSwitch, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "SnsClearOperatorSwitch", SnsClearOperatorSwitch, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "ClearEventDevice", ClearEventDevice, VT_I4, VTS_I4)
	DISP_FUNCTION(CNHMWICtrl, "ClearStatusDevice", ClearStatusDevice, VT_I4, VTS_I4)
	DISP_FUNCTION(CNHMWICtrl, "GetEventDevice", GetEventDevice, VT_I4, VTS_I4)
	DISP_FUNCTION(CNHMWICtrl, "GetStatusDevice", GetStatusDevice, VT_I4, VTS_I4)
	DISP_FUNCTION(CNHMWICtrl, "CduSetMinMaxCST", CduSetMinMaxCST, VT_I4, VTS_BSTR VTS_BSTR)
	DISP_FUNCTION(CNHMWICtrl, "CduSetNumberOfCash", CduSetNumberOfCash, VT_I4, VTS_BSTR)
	DISP_FUNCTION(CNHMWICtrl, "CduSetValueOfCash", CduSetValueOfCash, VT_I4, VTS_BSTR)
	DISP_FUNCTION(CNHMWICtrl, "SnsInitialize", SnsInitialize, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "CduGetLastDispensedAmount", CduGetLastDispensedAmount, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "CduAddNumberOfCash", CduAddNumberOfCash, VT_I4, VTS_BSTR VTS_BSTR)
	DISP_FUNCTION(CNHMWICtrl, "ProcBackupTrace", ProcBackupTrace, VT_I4, VTS_BSTR VTS_I4)
	DISP_FUNCTION(CNHMWICtrl, "CduDispenseAndPresentCount", CduDispenseAndPresentCount, VT_I4, VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	DISP_FUNCTION(CNHMWICtrl, "PinLoadEncryptedKey", PinLoadEncryptedKey, VT_I4, VTS_BSTR VTS_BSTR VTS_BSTR VTS_BSTR)
	DISP_FUNCTION(CNHMWICtrl, "CduGetShutterStatus", CduGetShutterStatus, VT_BOOL, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "SnsGetEnhancedAudio", SnsGetEnhancedAudio, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "CduGetLastDispensedCount", CduGetLastDispensedCount, VT_I4, VTS_I4)
	DISP_FUNCTION(CNHMWICtrl, "McuICChipPower", McuICChipPower, VT_I4, VTS_I4)
	DISP_FUNCTION(CNHMWICtrl, "CduGetCurrencyID", CduGetCurrencyID, VT_BSTR, VTS_I4)
	DISP_FUNCTION(CNHMWICtrl, "PinBuildPinBlock", PinBuildPinBlock, VT_I4, VTS_BSTR VTS_BSTR VTS_I2 VTS_BSTR VTS_BSTR VTS_BSTR)
	DISP_FUNCTION(CNHMWICtrl, "CduSetCurrencyID", CduSetCurrencyID, VT_I4, VTS_BSTR)
	DISP_FUNCTION(CNHMWICtrl, "RKT_ExportRSAIssuerSignedItem", RKT_ExportRSAIssuerSignedItem, VT_I4, VTS_BSTR VTS_BSTR)
// [#GLDV-2797] AU Kook 2020.05.11 Support Host PK Removal
//	DISP_FUNCTION(CNHMWICtrl, "RKT_ImportRSAPublicKey", RKT_ImportRSAPublicKey, VT_I4, VTS_BSTR VTS_BSTR)
	DISP_FUNCTION(CNHMWICtrl, "RKT_ImportRSAPublicKey", RKT_ImportRSAPublicKey, VT_I4, VTS_BSTR VTS_BSTR VTS_BSTR)
// end of [#GLDV-2797]
	DISP_FUNCTION(CNHMWICtrl, "RKT_StartKeyExchange", RKT_StartKeyExchange, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "RKT_ImportRSASignedDESKey", RKT_ImportRSASignedDESKey, VT_I4, VTS_I4 VTS_BSTR VTS_BSTR VTS_BSTR)
	DISP_FUNCTION(CNHMWICtrl, "RKT_GetExportedKey", RKT_GetExportedKey, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "RKT_GetExportedSignedValue", RKT_GetExportedSignedValue, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "RKT_GetEPP_RandomNumber", RKT_GetEPP_RandomNumber, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "RKT_GetEPP_KCV", RKT_GetEPP_KCV, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "EMV_SendRecvMsgWithIFM", EMV_SendRecvMsgWithIFM, VT_BSTR, VTS_I4 VTS_BSTR VTS_I4)
	DISP_FUNCTION(CNHMWICtrl, "SnsGetProximity", SnsGetProximity, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "PinInitializeEDM", PinInitializeEDM, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "RKT_GetExtraCaps", RKT_GetExtraCaps, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "PIN_GenerateKCV_512K", PIN_GenerateKCV_512K, VT_BSTR, VTS_BSTR)
	DISP_FUNCTION(CNHMWICtrl, "RFIDInitialize", RFIDInitialize, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "RFIDDeinitialize", RFIDDeinitialize, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "RFIDGetErrorCode", RFIDGetErrorCode, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "RFIDClearErrorCode", RFIDClearErrorCode, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "RFIDGetDeviceStatus", RFIDGetDeviceStatus, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "RFIDGetCardData", RFIDGetCardData, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "RFIDEntryEnable", RFIDEntryEnable, VT_I4, VTS_I4)
	DISP_FUNCTION(CNHMWICtrl, "RFIDEntryDisable", RFIDEntryDisable, VT_I4, VTS_NONE)

	// [#RWC6-151] US Kook 2020.07.20 TR34
	DISP_FUNCTION(CNHMWICtrl, "PinGetCertificate", PinGetCertificate, VT_I4, VTS_BSTR)
	DISP_FUNCTION(CNHMWICtrl, "PinInitializeEDMEx", PinInitializeEDMEx, VT_I4, VTS_BSTR VTS_BSTR)
	DISP_FUNCTION(CNHMWICtrl, "PinImportKeyBlock", PinImportKeyBlock, VT_I4, VTS_BSTR VTS_BSTR VTS_BSTR)
	DISP_FUNCTION(CNHMWICtrl, "PinLoadCertificateEx", PinLoadCertificateEx, VT_I4, VTS_BSTR VTS_BSTR VTS_BSTR)
	DISP_FUNCTION(CNHMWICtrl, "PinImportRSAEncipheredPKCS7KeyEx", PinImportRSAEncipheredPKCS7KeyEx, VT_I4, VTS_BSTR VTS_BSTR VTS_BSTR VTS_BSTR)
	DISP_FUNCTION(CNHMWICtrl, "PinGetCertificateData", PinGetCertificateData, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "PinGetCertificateState", PinGetCertificateState, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "PinStartAuthenticateSync", PinStartAuthenticateSync, VT_I4, VTS_BSTR VTS_BSTR)

	DISP_FUNCTION(CNHMWICtrl, "PinGetAuthenticateDataToSign", PinGetAuthenticateDataToSign, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "PinGetAuthenticateSigner", PinGetAuthenticateSigner, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "PinSetAuthenticateSigner", PinSetAuthenticateSigner, VT_EMPTY, VTS_BSTR)
	DISP_FUNCTION(CNHMWICtrl, "PinSetAuthenticateSigKey", PinSetAuthenticateSigKey, VT_EMPTY, VTS_BSTR)
	DISP_FUNCTION(CNHMWICtrl, "PinSetAuthenticateSignedData", PinSetAuthenticateSignedData, VT_EMPTY, VTS_BSTR)

	// [#GLDV-3005] US Kook 2021.10.25 Support Side Car
	// BCR: 163 ~ 171
	DISP_FUNCTION(CNHMWICtrl, "BcrInitialize", BcrInitialize, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "BcrDeinitialize", BcrDeinitialize, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "BcrGetErrorCode", BcrGetErrorCode, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "BcrClearErrorCode", BcrClearErrorCode, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "BcrGetDeviceStatus", BcrGetDeviceStatus, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "BcrSendRawData", BcrSendRawData, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "BcrCancelAccept", BcrCancelAccept, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "BcrReset", BcrReset, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "BcrGetBarcodeData", BcrGetBarcodeData, VT_BSTR, VTS_NONE)

	// BNA: 172 ~
	DISP_FUNCTION(CNHMWICtrl, "BnaInitialize", BnaInitialize, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "BnaDeinitialize", BnaDeinitialize, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "BnaGetErrorCode", BnaGetErrorCode, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "BnaClearErrorCode", BnaClearErrorCode, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "BnaGetDeviceStatus", BnaGetDeviceStatus, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "BnaGetExtraStatus", BnaGetExtraStatus, VT_BSTR, VTS_BSTR)
	DISP_FUNCTION(CNHMWICtrl, "BnaStartCashIn", BnaStartCashIn, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "BnaAcceptCash", BnaAcceptCash, VT_I4, VTS_I4 VTS_I4)
	// 180
	DISP_FUNCTION(CNHMWICtrl, "BnaCancelAccept", BnaCancelAccept, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "BnaStoreCash", BnaStoreCash, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "BnaRollbackCash", BnaRollbackCash, VT_I4, VTS_I4)
	DISP_FUNCTION(CNHMWICtrl, "BnaReset", BnaReset, VT_I4, VTS_I4)
	DISP_FUNCTION(CNHMWICtrl, "BnaConfigureNoteType", BnaConfigureNoteType, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "BnaSetCashUnitInfo", BnaSetCashUnitInfo, VT_I4, VTS_NONE)

	DISP_FUNCTION(CNHMWICtrl, "BnaGetLastCashInStatus", BnaGetLastCashInStatus, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "BnaGetNumberOfCashInStatus", BnaGetNumberOfCashInStatus, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "BnaGetNumberOfLogicalUnit", BnaGetNumberOfLogicalUnit, VT_I4, VTS_NONE)

	DISP_FUNCTION(CNHMWICtrl, "BnaGetCashInStatus", BnaGetCashInStatus, VT_DISPATCH, VTS_I4)
	DISP_FUNCTION(CNHMWICtrl, "BnaGetLogicalUnit", BnaGetLogicalUnit, VT_DISPATCH, VTS_I4)
	DISP_FUNCTION(CNHMWICtrl, "BnaGetAcceptorStatus", BnaGetAcceptorStatus, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "BnaGetStackerStatus", BnaGetStackerStatus, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "BnaGetMaxCashInItem", BnaGetMaxCashInItem, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "BnaGetMaxStackerItem", BnaGetMaxStackerItem, VT_I4, VTS_NONE)
	DISP_FUNCTION(CNHMWICtrl, "BnaGetLastRefusedCount", BnaGetLastRefusedCount, VT_I4, VTS_NONE)
	// end of [#GLDV-3005]

	DISP_FUNCTION(CNHMWICtrl, "BnaGetPositionStatus", BnaGetPositionStatus, VT_BSTR, VTS_NONE)	// [RWC6-676] Start SKKim 2024.05.17
	DISP_FUNCTION(CNHMWICtrl, "BnaGetRefusedStatus", BnaGetRefusedStatus, VT_I4, VTS_NONE)		// [RWC6-676] Start SKKim 2024.05.27

	//}}AFX_DISPATCH_MAP
	DISP_FUNCTION_ID(CNHMWICtrl, "AboutBox", DISPID_ABOUTBOX, AboutBox, VT_EMPTY, VTS_NONE)
END_DISPATCH_MAP()


/////////////////////////////////////////////////////////////////////////////
// Event map

BEGIN_EVENT_MAP(CNHMWICtrl, COleControl)
	//{{AFX_EVENT_MAP(CNHMWICtrl)
	EVENT_CUSTOM("TestEvent", FireTestEvent, VTS_BSTR)
	EVENT_CUSTOM("OnDisplayScreen", FireOnDisplayScreen, VTS_BSTR  VTS_I4  VTS_BSTR)
	EVENT_CUSTOM("OnDisplayData", FireOnDisplayData, VTS_BSTR  VTS_BSTR)
	EVENT_CUSTOM("OnDisplayPinpadData", FireOnDisplayPinpadData, VTS_BSTR)
	EVENT_CUSTOM("OnSafeDoorChanged", FireOnSafeDoorChanged, VTS_BSTR)		// [#RWC6-14] US William 2019.09.18 Add Safe Door Status To Journal
	//}}AFX_EVENT_MAP
END_EVENT_MAP()


/////////////////////////////////////////////////////////////////////////////
// Property pages

// TODO: Add more property pages as needed.  Remember to increase the count!
BEGIN_PROPPAGEIDS(CNHMWICtrl, 1)
	PROPPAGEID(CNHMWIPropPage::guid)
END_PROPPAGEIDS(CNHMWICtrl)


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CNHMWICtrl, "NHMWI.NHMWICtrl.1",
	0x984454bc, 0x621a, 0x41be, 0xac, 0x67, 0x63, 0x6c, 0, 0x80, 0x1a, 0xfc)


/////////////////////////////////////////////////////////////////////////////
// Type library ID and version

IMPLEMENT_OLETYPELIB(CNHMWICtrl, _tlid, _wVerMajor, _wVerMinor)


/////////////////////////////////////////////////////////////////////////////
// Interface IDs

const IID BASED_CODE IID_DNHMWI =
		{ 0x9edfdafe, 0x3023, 0x4f3f, { 0xba, 0xd8, 0xdc, 0xa1, 0xdd, 0x37, 0x17, 0xd9 } };
const IID BASED_CODE IID_DNHMWIEvents =
		{ 0x8fe81179, 0x8f28, 0x4f2f, { 0xba, 0x66, 0x2a, 0x2a, 0xed, 0xb8, 0x70, 0x35 } };


/////////////////////////////////////////////////////////////////////////////
// Control type information

static const DWORD BASED_CODE _dwNHMWIOleMisc =
	OLEMISC_INVISIBLEATRUNTIME |
	OLEMISC_ACTIVATEWHENVISIBLE |
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CNHMWICtrl, IDS_NHMWI, _dwNHMWIOleMisc)


/////////////////////////////////////////////////////////////////////////////
// CNHMWICtrl::CNHMWICtrlFactory::UpdateRegistry -
// Adds or removes system registry entries for CNHMWICtrl
// 웹페이지에서 사용시 보안문제 해결용
#include ".\\System\\Helpers.h"
#include "ObjSafe.h"

BOOL CNHMWICtrl::CNHMWICtrlFactory::UpdateRegistry(BOOL bRegister)
{
	// TODO: Verify that your control follows apartment-model threading rules.
	// Refer to MFC TechNote 64 for more information.
	// If your control does not conform to the apartment-model rules, then
	// you must modify the code below, changing the 6th parameter from
	// afxRegApartmentThreading to 0.

	if (bRegister)
	{
		int retval = AfxOleRegisterControlClass(
			AfxGetInstanceHandle(),
			m_clsid,
			m_lpszProgID,
			IDS_NHMWI,
			IDB_NHMWI,
			afxRegApartmentThreading,
			_dwNHMWIOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);


		HRESULT hr = CreateComponentCategory(CATID_SafeForScripting, L"Controls that are safely scriptable");
		

		if (SUCCEEDED(hr))
			RegisterCLSIDInCategory(m_clsid, CATID_SafeForScripting);

		
		hr = CreateComponentCategory(CATID_SafeForInitializing, L"Controls safely initializable from persistent data");

		if (SUCCEEDED(hr))
			RegisterCLSIDInCategory(m_clsid, CATID_SafeForInitializing);

		return retval;
	}
	else
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);

}


/////////////////////////////////////////////////////////////////////////////
// CNHMWICtrl::CNHMWICtrl - Constructor

CNHMWICtrl::CNHMWICtrl()
{
	InitializeIIDs(&IID_DNHMWI, &IID_DNHMWIEvents);

	m_pDevCtrl	= new CDevCtrl;
	m_pDevCtrl->SetOwner(this);

	m_bMcuEntry = ENTRY_DISABLED;								// 카드부 엔트리 발행되지 않음
	m_bPinEntry = ENTRY_DISABLED;								// PINPAD 엔트리 발행되지 않음 2004.04.02

	m_nErrorDevice = DEV_NONE;									// 장애디바이스

	m_nTimeoutDevice = DEV_NONE;								// 타임아웃디바이스

	m_nEventDevice = DEV_NONE;									// 이벤트디바이스
	m_nStatusDevice = DEV_NONE;									// 상태디바이스

	// ------------------------------------------------------------------------
	// PROPERTY
	// ------------------------------------------------------------------------
																// 20031106_1.1_1092
	m_nSprExist					= ST_NOT_DETECT;				// 명세표잔류여부(잔류/동작여부) 논리플래그 : 초기값-없음
	m_nJprExist					= ST_NOT_DETECT;				// 저널 잔류여부(저널 동작여부) 논리플래그 : 초기값-없음
	m_nMcuExist					= ST_NOT_DETECT;				// 카드잔류여부(잔류/동작여부) 논리플래그 : 초기값-없음

	m_strICData.Empty();

																// 장치디바이스 오픈여부 플래그
	m_bDoorOpened				= FALSE;						// 뒷문부오픈여부(초기값:FALSE) 
	m_bJprOpened				= FALSE;						// 저널부오픈여부(초기값:FALSE) 
	m_bLightOpened				= FALSE;						// LIGHT부오픈여부(초기값:FALSE) 
	m_bMcuOpened				= FALSE;						// 카드부오픈여부(초기값:FALSE) 
	m_bSprOpened				= FALSE;						// 명세표부오픈여부(초기값:FALSE) 
	m_bCduOpened				= FALSE;						// CDU오픈여부(초기값:FALSE) 
	m_bPinOpened				= FALSE;						// PINPAD오픈여부(초기값:FALSE) 
	m_bSensorOpened				= FALSE;						// SENSOR오픈여부(초기값:FALSE) 
	m_bRFIDOpened				= FALSE;						// [#2325] NH KSK 2015.01.20
	m_bBcrOpened				= FALSE;
	m_bBnaOpened				= FALSE;

																// 장치디바이스 오픈 이벤트 수신여부 플래그 : 2005.03.30
	m_bDoorOpenedEvent			= FALSE;						// 뒷문부오픈이벤트수신여부(초기값:FALSE) 
	m_bLightOpenedEvent			= FALSE;						// LIGHT부오픈이벤트수신여부(초기값:FALSE) 
	m_bIndOpenedEvent			= FALSE;						// INDICATOR부오픈이벤트수신여부(초기값:FALSE) 
	m_bSensorOpenedEvent		= FALSE;						// SENSOR오픈이벤트수신여부(초기값:FALSE) 

																// 장치디바이스 클로우즈 이벤트 수신여부 플래그 : 2005.03.30
	m_bDoorClosedEvent			= FALSE;						// 뒷문부클로우즈이벤트수신여부(초기값:FALSE) 
	m_bLightClosedEvent			= FALSE;						// LIGHT부클로우즈이벤트수신여부(초기값:FALSE) 
	m_bIndClosedEvent			= FALSE;						// INDICATOR부클로우즈이벤트수신여부(초기값:FALSE) 
	m_bSensorClosedEvent		= FALSE;						// SENSOR클로우즈이벤트수신여부(초기값:FALSE) 

	m_bMcuWaitTaken				= FALSE;						// 카드부매체수취대기여부(초기값:FALSE) 
	m_bSprWaitTaken				= FALSE;						// 명세표부매체수취대기여부(초기값:FALSE) 
	m_bCduWaitTaken				= FALSE;						// CDU매체수취대기여부(초기값:FALSE) 

	m_nCSTMinimum4Mix6			= 0;							// 보정매수값		2004.11.05
	m_nCduDispenseAmount		= 0;							// CDU 방출금액		2004.04.01
	VariantInit( &m_vMixResult );								// CDU Mix결과 초기화
	m_bDispenseSuccecded		= FALSE;						// CDU Dispense 결과 성공여부		V01.02.25

	m_nPINEntryMode				= PINMODE_NOT_USE;				// PinPad입력모드(NORMAL/EPP etc)	2004.04.02 
	m_strPinEppCardData.Empty();
	m_strPinKeyData.Empty();
	m_strPinMacingData.Empty();
	m_strPinLastKey.Empty();
	
	m_strCurrency.Empty();										// [#514] [MX] KSK 2009.3.10
	m_GetAvailableAmount = FALSE;								// 일반 DISPENSE 명령에 대하여 장애/정상 정상처리함.

	// [#537] NH KSK 2009.06.11
	m_strEPP_ExportedKey.Empty();
	m_strEPP_Signed.Empty();
	m_strEPP_RandomNumber.Empty();		// [#2259] NH KSK 2014.03.13
	m_strEPP_KeyCheckValue.Empty();
	// end of [#537] 
	
	//-- SJKWONNOTE 2006-05-03 --ADD//
	m_pSPInform = NULL;
	m_pSPInform = (LPSPInform)GetSystemAddr(5, 4096);
	if(m_pSPInform == NULL)
		m_pSPInform = (LPSPInform)GetSystemAddr(5, 4096);

	RegSetMwiInformation();										// MWI정보기록		2004.04.01
}


/////////////////////////////////////////////////////////////////////////////
// CNHMWICtrl::~CNHMWICtrl - Destructor

CNHMWICtrl::~CNHMWICtrl()
{
	if (m_pDevCtrl	!= NULL)	{	delete m_pDevCtrl;			m_pDevCtrl = NULL;		}
	
	if	(m_pSPInform != NULL)
		FreeSystemAddr((LPVOID)m_pSPInform); //NVRAM의 pointer를 해제한다.
}


/////////////////////////////////////////////////////////////////////////////
// CNHMWICtrl::OnDraw - Drawing function

void CNHMWICtrl::OnDraw(
			CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
	// TODO: Replace the following code with your own drawing code.

	CBitmap	bitmap;
	if(!bitmap.LoadBitmap(IDB_LOGO))
	{
		pdc->FillRect(rcBounds, CBrush::FromHandle((HBRUSH)GetStockObject(WHITE_BRUSH)));
		pdc->Ellipse(rcBounds);
	}
	else
	{
		BITMAP bm;
		bitmap.GetBitmap(&bm);
		
		CDC	memDC;
		memDC.CreateCompatibleDC(pdc);

		CBitmap *pOldBitmap;
		pOldBitmap = memDC.SelectObject(&bitmap);

		pdc->BitBlt(0,
					0,
					bm.bmWidth,
					bm.bmHeight,
					&memDC,
					0,
					0,
					SRCCOPY);

		memDC.SelectObject(pOldBitmap);
		memDC.DeleteDC();	
	}

}


/////////////////////////////////////////////////////////////////////////////
// CNHMWICtrl::DoPropExchange - Persistence support

void CNHMWICtrl::DoPropExchange(CPropExchange* pPX)
{
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);

	// TODO: Call PX_ functions for each persistent custom property.

}


/////////////////////////////////////////////////////////////////////////////
// CNHMWICtrl::OnResetState - Reset control to default state

void CNHMWICtrl::OnResetState()
{
	COleControl::OnResetState();  // Resets defaults found in DoPropExchange

	// TODO: Reset any other control state here.
}


/////////////////////////////////////////////////////////////////////////////
// CNHMWICtrl::AboutBox - Display an "About" box to the user

void CNHMWICtrl::AboutBox()
{
	CDialog dlgAbout(IDD_ABOUTBOX_NHMWI);
	dlgAbout.DoModal();
}


/////////////////////////////////////////////////////////////////////////////
// CNHMWICtrl message handlers

////////////////////////////////////////////////////////////////////////////////////////
// -------------------------------------------------------------------------------------
// Active X Control 문제 해결
// 문제점 : Invisible on Runtime설정시 ActiveX의 Window가 만들어지지 않아 핸들이 없음
// 증상   : ActiveX를 이용한 어떠한 수행도 처리되지 않음
// 해결   : 윈도를 재생성하는 과정을 수행(?)
// 참고   : MSDN Article No : Q195188
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////
void CNHMWICtrl::OnSetClientSite() 
{
    if (m_pClientSite)
        // It doesn't matter who the parent window is or what the size of
        // the window is because the control's window will be reparented
        // and resized correctly later when it's in-place activated.
        VERIFY (CreateControlWindow (::GetDesktopWindow(), CRect(0,0,0,0),
                                      CRect(0,0,0,0)));
    COleControl::OnSetClientSite();
}

// ----------------------------------------------------------------------------
// 함 수 명 : int CNHMWICtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
// DESCRIPT : 객체생성시...
// 매개변수 : 
// 반 환 값 : 
// 기타사항 : 이 메소드는 오버라이드 할 필욘 없으나, Aggregation을 위해서 함.
// ----------------------------------------------------------------------------
int CNHMWICtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (COleControl::OnCreate(lpCreateStruct) == -1)
		return -1;

	// ActiveX Creation : AXINAX
	BOOL ret = m_pPin.Create	(NULL, 0, CRect(0,0,0,0), this, IDC_NXPIN);						// NXPin
	if (!ret)
	{
		DWORD err = GetLastError();
		NHDBG((L"Error: %d\r\n", err));
	}

	ret = m_pMcu.Create	(NULL, 0, CRect(0,0,0,0), this, IDC_NXCARDREADER);				// NXCardReader
	ret = m_pSpr.Create	(NULL, 0, CRect(0,0,0,0), this, IDC_NXRECEIPTPRINTER);			// NXReceiptPrinter
//-NOUSE	m_pJpr.Create	(NULL, 0, CRect(0,0,0,0), this, IDC_NXJOURNALPRINTER);			// NXJournalPrinter			// AIREAT 2009.07.14
	ret = m_pCdu.Create	(NULL, 0, CRect(0,0,0,0), this, IDC_NXCASHDISPENSER);			// NXCashDispenser
	ret = m_pDoor.Create	(NULL, 0, CRect(0,0,0,0), this, IDC_NXDOORS);					// NXDoors
	ret = m_pLight.Create	(NULL, 0, CRect(0,0,0,0), this, IDC_NXGUIDLIGHTS);				// NXGuidelights
	ret = m_pInd.Create	(NULL, 0, CRect(0,0,0,0), this, IDC_NXINDICATORS);				// NXIndicators
	ret = m_pSensor.Create(NULL, 0, CRect(0,0,0,0), this, IDC_NXSENSORS);					// NXSensors
//-NOUSE	m_pTrace.Create	(NULL, 0, CRect(0,0,0,0), this, IDC_NXTRACE);					// NXTrace ActiveX			// AIREAT 2009.07.14
	m_pRFID.Create	(NULL, 0, CRect(0,0,0,0), this, IDC_NXRFIDREADER);				// NXRFID	[#2325] NH KSK 2015.01.20
	m_pBcr.Create	(NULL, 0, CRect(0,0,0,0), this, IDC_NXBARCODEREADER);			// NXScannerPrinter		// [#GLDV-3005] US Kook 2021.11.08 Support Side Car
	m_pBna.Create	(NULL, 0, CRect(0,0,0,0), this, IDC_NXBILLACCEPTOR);			// NXBillAcceptor		// [#GLDV-3005] US Kook 2021.10.25 Support Side Car

	return 0;
}

// ----------------------------------------------------------------------------
// 함 수 명 : BOOL CNHMWICtrl::SetEventMatrix(LPCTSTR szDeviceName, LPCTSTR szMethodName)
// DESCRIPT : 디바이스의 메소드를 호출하기 전에 메소드에 해당하는 이벤트의 메트릭스를 설정한다.
//			  설정된 정보를 이용하여 CheckDeviceAction을 수행한다.
// 매개변수 : 디바이스ID, 메소드명
// 반 환 값 : 정상설정시 TRUE
// 기타사항 : 반드시 디바이스 구동을 위한 이벤트를 발생하기에 앞서 본 함수를 사용하여
//			  디바이스 구동이벤트 메트릭스를 설정하여야 한다. 반드시.................
// ----------------------------------------------------------------------------
BOOL CNHMWICtrl::SetEventMatrix(INT nDeviceID, char* szMethodName)
{
	return m_pDevCtrl->SetEventMatrix(nDeviceID, szMethodName);
}

// ----------------------------------------------------------------------------
// DESCRIPT  : 레지스트리에 mwi정보를 갱신처리한다.
// PARAMETER : 없음
// RETURN    : NORMAL(0)
// REMARK	 : 2004.04.01
// ----------------------------------------------------------------------------
int CNHMWICtrl::RegSetMwiInformation()
{
	return NORMAL;
}

void CNHMWICtrl::WriteMwiErrorCode_NVRam(CString strEntry, char* szTemp)
{
	if(strEntry == "SPR")
	{
		NvramCopyMemory(m_pSPInform->SPErr.ErrCode_SPR, szTemp, 7);					// [#573] NH AIREAT 2009.10.09 SW NVRAM
	}
	else if(strEntry == "CDM")
	{
		NvramCopyMemory(m_pSPInform->SPErr.ErrCode_CDU, szTemp, 7);					// [#573] NH AIREAT 2009.10.09 SW NVRAM
	}
	else if(strEntry == "IDC")
	{
		NvramCopyMemory(m_pSPInform->SPErr.ErrCode_IDC, szTemp, 7);					// [#573] NH AIREAT 2009.10.09 SW NVRAM
	}
	else if(strEntry == "PINPAD")
	{
		NvramCopyMemory(m_pSPInform->SPErr.ErrCode_PIN, szTemp, 7);					// [#573] NH AIREAT 2009.10.09 SW NVRAM
	}
	else if(strEntry == "JPR")
	{
		NvramCopyMemory(m_pSPInform->SPErr.ErrCode_JPR, szTemp, 7);					// [#573] NH AIREAT 2009.10.09 SW NVRAM
	}
	else if(strEntry == "RFID")														// [#2325] NH KSK 2015.01.20 추가 Coding 필요
	{
		NvramCopyMemory(m_pSPInform->SPErr.ErrCode_IDC, szTemp, 7);
	}																				// end of [#2325]
	// [#GLDV-3005] US Kook 2021.12.30 Side Car
	else if(strEntry == DEVNM_BCR)
	{
		NvramCopyMemory(m_pSPInform->SPInfo.BCR.ErrCode, szTemp, 7);
	}
	else if(strEntry == DEVNM_BNA)
	{
		NvramCopyMemory(m_pSPInform->SPInfo.BNA.ErrCode, szTemp, 7);
	}
	// end of [#GLDV-3005]
}

//////////////////////////////////////////////////////////////////////////
// EMV KERNEL FUNCTION
//////////////////////////////////////////////////////////////////////////
CString CNHMWICtrl::EMVINF_SendRecvMsgWithIFM(long ProtocolID, LPCTSTR szSendData, long nWaitSec)
{
	NHDEBUG(1, (_T("CNHMWICtrl::EMVINF_SendRecvMsgWithIFM() ProtocolID(%d) szSendData(%s) nWaitSec(%d) \n"), ProtocolID, szSendData, nWaitSec));
	
	CString strResult;
	int		nResult = 0;
	
	// [#68] UK HWANG 2008.04.02 EMV Level2
	{
		int nSensorStatus;
		int nDeviceStatus;
		
		nDeviceStatus = m_pDevCtrl->CheckDeviceAction(DEV_MCU, K_30_WAIT);
		/* Device Error Check */
		if( nDeviceStatus != NORMAL)
		{
			strResult = McuGetErrorCode();
			NHDEBUG(1, (_T("CNHMWICtrl::EMVINF_SendRecvMsgWithIFM() MediaStatus Error(%d) \n"),  nResult));
			return L"";
		}
		
		nSensorStatus = McuGetMaterialInfo();
		/* Media Not Present Check */
		if( nSensorStatus != ST_SENSOR2 )
		{
			strResult = McuGetErrorCode();
			NHDEBUG(1, (_T("CNHMWICtrl::EMVINF_SendRecvMsgWithIFM() MediaStatus Error(%d) \n"),  nResult));
			return L"";
		}
	}
	// end of [#68]
	
	McuICSendData(ProtocolID, szSendData, nWaitSec);					// 메세지를 IFD에 송신한다.
	nResult = m_pDevCtrl->CheckDeviceAction(DEV_MCU, K_30_WAIT);
	
	NHDEBUG(1, (_T("CNHMWICtrl::EMVINF_SendRecvMsgWithIFM() CheckDeviceAction [nResult(%d)] \n"), nResult));
	
	if (nResult == NORMAL)
	{
		strResult = McuGetICData();
	}
	else
	{
		strResult = McuGetErrorCode();
		NHDEBUG(1, (_T("CNHMWICtrl::EMVINF_SendRecvMsgWithIFM() Return Error(%s) \n"),  strResult));
		return L"";
	}
	
	
	NHDEBUG(1, (_T("CNHMWICtrl::EMVINF_SendRecvMsgWithIFM() Return strResult(%s) \n"),  strResult));
	return strResult;
}
