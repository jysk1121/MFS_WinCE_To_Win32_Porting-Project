#ifndef _COMMON_DEF_H_
#define _COMMON_DEF_H_
// ----------------------------------------------------------------------------
// Registry 조회를 위하여 관련값들을 Define함
// ----------------------------------------------------------------------------
#define		_REGKEY_DEVERROR				_T("SOFTWARE\\ATM\\ERRORCODE")

#define		_REGKEY_CDUSTATUS				_T("SOFTWARE\\ATM\\DEVINFO\\CDM")	// 2004.03.31 중국현지화
#define		_REGKEY_MCUSTATUS				_T("SOFTWARE\\ATM\\DEVINFO\\IDC")	// 20030719_1.0_1053 : 장치정보 경로
#define		_REGKEY_JPRSTATUS				_T("SOFTWARE\\ATM\\DEVINFO\\JPR")	// 20030719_1.0_1053 : 장치정보 경로
#define		_REGKEY_SPRSTATUS				_T("SOFTWARE\\ATM\\DEVINFO\\SPR")	// 20030719_1.0_1053 : 장치정보 경로


// ----------------------------------------------------------------------------
// 레지스트리 입출력을 위한 Type값 설정
// ----------------------------------------------------------------------------
#define		REG_INT			REG_DWORD_LITTLE_ENDIAN
#define		REG_STR			REG_SZ
//#define		REG_MULTI		REG_MULTI_SZ

// ----------------------------------------------------------------------------
// EVENT QUEUE관련
// ----------------------------------------------------------------------------
#define		MWI_EVENT_DEVICE				("EVENT_DEVICE")
#define		MWI_EVENT_USERDATA				("EVENT_USERDATA")

// ----------------------------------------------------------------------------
// MATRIX 설정 값관련
// ----------------------------------------------------------------------------
#define		MWI_EVENT_EXIST					1					// 이벤트 발생됨
#define		MWI_EVENT_NOTEXIST				0					// 이벤트 발생안됨

#define		MWI_EVENT_RESET					0					// 메트릭스 초기처리
#define		MWI_EVENT_OCCUR					1					// 해당 이벤트 발생됨
#define		MWI_EVENT_SETTING				2					// 해당 이벤트가 발생될것으로 설정함

// ----------------------------------------------------------------------------
// DEFINE VALUES (Default Device Timeout) : Seconds
// ----------------------------------------------------------------------------
#define		WTIME_MCUEJECT					45					// 카드 방출및 수취대기
#define		WTIME_MCUINSERT					30					// 카드 삽입대기
#define		WTIME_CHECKESCROW				30					// 입금수표 배서및일시스태킹 기본시간

#define		WTIME_SPREJECT					45					// 명세표 방출및 수취대기


// 상태조회시 TAG
#define		ST_DEVICEVSTATUS				1					// 장치상태조회	: 20030806_1.0_1059 : Define Const-Variables
#define		ST_MEDIASTATUS					2					// 매체상태조회	: 20030806_1.0_1059 : Define Const-Variables
#define		ST_PAPERSTATUS					3					// 용지상태조회	: 20030806_1.0_1059 : Define Const-Variables


// 장치 상태
#define		ST_OK							_T("OK")
#define		ST_NOTSUPP						_T("NOTSUPP")
#define		ST_UNKNOWN						_T("UNKNOWN")
#define		ST_DEVONLINE					_T("DEVONLINE")
#define		ST_DEVNODEVICE					_T("DEVNODEVICE")
#define		ST_DEVOFFLINE					_T("DEVOFFLINE")

#define		ST_HEALTHY						_T("HEALTHY")
#define		ST_FATAL						_T("FATAL")
#define		ST_NODEVICE						_T("NODEVICE")

// 매체 상태
#define		ST_NOTPRESENT					_T("NOTPRESENT")
#define		ST_PRESENT						_T("PRESENT")
#define		ST_INJAWS						_T("INJAWS")
#define		ST_JAMMED						_T("JAMMED")
#define		ST_ENTERING						_T("ENTERING")

// 용지상태
#define		ST_PAPER_FULL					_T("FULL")
#define		ST_PAPER_LOW					_T("LOW")
#define		ST_PAPER_OUT					_T("OUT")
#define		ST_PAPER_UNKNOWN				_T("UNKNOWN")

// --------------------------------------------------------------------------------
// Printer(Receipt/Journal/Passbook) 관련 Media/Form/Field name설정
// --------------------------------------------------------------------------------
// [#2373] US Justin 2015.09.22 Support Coupon Image
// Journal Media and Form
#define		MEDIA_JOURNAL					_T("JournalMedia")
#define		FORM_JOURNAL					_T("Journal")
#define		FORM_JOURNAL_PRE				_T("Journal")
#define		FORM_JOURNAL_POST				_T("Journal")

// Text Receipt Media and Form
#define		MEDIA_RECEIPT					_T("ReceiptMedia")
#define		FORM_RECEIPT					_T("Slip")

// Image Print Commands
#define		CMD_PRINT_HEADER_IMAGE			_T("HEADERIMAGE")
#define		CMD_PRINT_COUPON_IMAGE_BCD		_T("COUPONIMAGE_BCD")
#define		CMD_PRINT_COUPON_IMAGE_QR		_T("COUPONIMAGE_QR")
#define		CMD_PRINT_COUPON_IMAGE_LARGEQR	_T("COUPONIMAGE_LARGEQR")	// [#2503] US Justin 2017.09.15 Add Ethereum.. Large QR Image

// Header Image and Barcode Media and Form
#define		MEDIA_RECEIPT_IMAGE				_T("ReceiptImageMedia")		// [#2377] Header Image or Barcode( height : 5)
#define		MEDIA_RECEIPT_LARGEIMAGE		_T("ReceiptLargeImageMedia")// [#2503] Header Image or Barcode( height : 11)

#define		FORM_RECEIPT_HEADER_IMAGE		_T("ReceiptHeaderImage")	// [#2373] Receipt Header Image
#define		FORM_RECEIPT_COUPON_BCD			_T("CouponImageBCD")		// [#2377] Barcode (Height : 5)
#define		FORM_RECEIPT_COUPON_QR			_T("CouponImageQR")			// [#2377] QR Code (Height : 6)
#define		FORM_RECEIPT_COUPON_LARGEQR		_T("CouponImageLargeQR")	// [#2503] QR Code (Height : 11)

#define		IMAGETYPE_HEADER				0
#define		IMAGETYPE_COUPON_BCD			1
#define		IMAGETYPE_COUPON_QR				2
#define		IMAGETYPE_COUPON_LARGEQR		3							// [#2503] US Justin 2017.09.15 Add Ethereum.. Large QR Image


#define		COUPON_IMAGE_FILE_BCD			ATM_PATH _T("\\..\\CouponImageBCD.bmp")
#define		COUPON_IMAGE_FILE_QR			ATM_PATH _T("\\..\\CouponImageQR.bmp")
// End of [#2373]

#define		QR_RECEIPT_IMAGEFILE			ATM_PATH _T("\\..\\AP_QRCode.bmp")	// [#2382] US Justin Making a constant.

#define		GPAY_QR_AUTOENROLL_IMAGEFILE	ATM_PATH _T("\\..\\GPayAutoEnrollQR.bmp")	// [#RWC6-44] GivePay AutoEnrollment QR Code Display

#define		TANGOPAY_QR_DOWNLOAD_APP_INFO	ATM_PATH _T("\\..\\TangoPayDownloadInfoQR.bmp")	// [RWC6-676] SKKim 2024.05.29

// [#2383] US Justin 2015.12.11 Mobile App
#define		MOBILEAPP_QR_IMAGEFILE			ATM_PATH _T("\\..\\MobileAppQR.bmp")
#define		MOBILEAPP_QR_IMAGEERROR			ATM_PATH _T("\\..\\MobileAppQRError.bmp")
											
#define		MOBILEAPP_QR_HEADER				_T("HYO");
#define		MOBILEAPP_QR_TYPE_REGISTER		0
#define		MOBILEAPP_QR_TYPE_ERRORCODE		1
#define		MOBILEAPP_QR_TYPE_CASHBALANCE	2
#define		MOBILEAPP_QR_TYPE_TERMINALID	10							// [#J007]
// End of [#2382]

// --------------------------------------------------------------------------------
// MCU/PBM Entry 관련 값
// --------------------------------------------------------------------------------
#define		ENTRY_ENABLED					0x01				// 엔트리발행됨
#define		ENTRY_DISABLED					0x00				// 엔트리취소됨

// --------------------------------------------------------------------------------
// 명세표/저널 인자관련
// --------------------------------------------------------------------------------
#define		SLIP_MAX_COL					40					// 명세표 라인당 컬럼수
//#define		SLIP_MAX_LINE					30					// 명세표 라인수
#define		SLIP_MAX_LINE					35					// 명세표 라인수

#define		JNL_MAX_COL						40					// 저널 라인당 컬럼수
//#define		JNL_MAX_LINE					30					// 저널 라인수
#define		JNL_MAX_LINE					35					// 저널 라인수

// --------------------------------------------------------------------------------
// 플리커(Flicker, GuideLight)관련 값
// --------------------------------------------------------------------------------
#define		FLICKER_LIGHT_ON			 	_T("QUICK")			// Flicker 깜박이게 켜기
#define		FLICKER_LIGHT_CONTINUE	 		_T("CONTINUOUS")	// Flicker 깜박이지 않게 켜기
#define		FLICKER_LIGHT_OFF				_T("OFF")			// Flicker 끄기

// --------------------------------------------------------------------------------
// ERROR TYPE
// --------------------------------------------------------------------------------
#define		DEVICE_ERROR					1
#define		FATAL_ERROR						2

// --------------------------------------------------------------------------------
// FLAG VALUE
// --------------------------------------------------------------------------------
#define		S_ON							_T("ON")
#define		S_OFF							_T("OFF")

// --------------------------------------------------------------------------------
// SWITCH NAME
// --------------------------------------------------------------------------------
#define		S_TESTKEY						_T("TESTKEY")		// 테스트 키
#define		S_LOADSWITCH					_T("LOADSWITCH")	// 불휘발램 스위치

// --------------------------------------------------------------------------------
// Method SP Use FLAG VALUE
// --------------------------------------------------------------------------------
#define		S_YES							_T("YES")
#define		S_NO							_T("NO")


#define		S_QRCODE						_T("QRCODE")		// [#2219] 2013.09.04 NH Justin Digital Receipt
#define		RCPT_PAPER						1					// [#2219] 2013.09.04 NH Justin Digital Receipt
#define		RCPT_QRCODE						2					// [#2219] 2013.09.04 NH Justin Digital Receipt
#define		RCPT_DISPLAY					3					// [#RWC6-16] Bitload 4 U. A new cryptocurrency feature for CE 6.0 ATMs


// ============================================================================

/************************************************************************/
/* XFS Error Code for MWI                                               */
/************************************************************************/
#define 	WFS_SUCCESS							0	
#define 	WFS_ERR_ALREADY_STARTED				-1  
#define 	WFS_ERR_API_VER_TOO_HIGH			-2  
#define 	WFS_ERR_API_VER_TOO_LOW				-3  
#define 	WFS_ERR_CANCELED					-4  
#define 	WFS_ERR_CFG_INVALID_HKEY			-5  
#define 	WFS_ERR_CFG_INVALID_NAME			-6  
#define 	WFS_ERR_CFG_INVALID_SUBKEY			-7  
#define 	WFS_ERR_CFG_INVALID_VALUE			-8  
#define		WFS_ERR_CFG_KEY_NOT_EMPTY			-9   
#define		WFS_ERR_CFG_NAME_TOO_LONG			-10  
#define		WFS_ERR_CFG_NO_MORE_ITEMS			-11  
#define 	WFS_ERR_CFG_VALUE_TOO_LONG			-12 
#define		WFS_ERR_DEV_NOT_READY				-13  
#define		WFS_ERR_HARDWARE_ERROR				-14  
#define		WFS_ERR_INTERNAL_ERROR				-15  
#define		WFS_ERR_INVALID_ADDRESS				-16  
#define		WFS_ERR_INVALID_APP_HANDLE			-17  
#define 	WFS_ERR_INVALID_BUFFER				-18 
#define 	WFS_ERR_INVALID_CATEGORY			-19 
#define 	WFS_ERR_INVALID_COMMAND				-20 
#define 	WFS_ERR_INVALID_EVENT_CLASS			-21 
#define 	WFS_ERR_INVALID_HSERVICE			-22 
#define 	WFS_ERR_INVALID_HPROVIDER			-23 
#define 	WFS_ERR_INVALID_HWND				-24 
#define   	WFS_ERR_INVALID_HWNDREG				-25  
#define   	WFS_ERR_INVALID_POINTER				-26  
#define   	WFS_ERR_INVALID_REQ_ID				-27  
#define   	WFS_ERR_INVALID_RESULT				-28  
#define   	WFS_ERR_INVALID_SERVPROV			-29  
#define 	WFS_ERR_INVALID_TIMER				-30 
#define   	WFS_ERR_INVALID_TRACELEVEL			-31  
#define 	WFS_ERR_LOCKED						-32 
#define   	WFS_ERR_NO_BLOCKING_CALL			-33  
#define   	WFS_ERR_NO_SERVPROV					-34  
#define   	WFS_ERR_NO_SUCH_THREAD				-35  
#define 	WFS_ERR_NO_TIMER					-36 
#define   	WFS_ERR_NOT_LOCKED					-37  
#define   	WFS_ERR_NOT_OK_TO_UNLOAD			-38  
#define   	WFS_ERR_NOT_STARTED					-39  
#define   	WFS_ERR_NOT_REGISTERED				-40  
#define   	WFS_ERR_OP_IN_PROGRESS				-41  
#define 	WFS_ERR_OUT_OF_MEMORY				-42 
#define   	WFS_ERR_SERVICE_NOT_FOUND			-43  
#define   	WFS_ERR_SPI_VER_TOO_HIGH			-44  
#define   	WFS_ERR_SPI_VER_TOO_LOW				-45  
#define   	WFS_ERR_SRVC_VER_TOO_HIGH			-46  
#define   	WFS_ERR_SRVC_VER_TOO_LOW			-47  
#define 	WFS_ERR_TIMEOUT						-48 
#define   	WFS_ERR_UNSUPP_CATEGORY				-49  
#define   	WFS_ERR_UNSUPP_COMMAND				-50  
#define   	WFS_ERR_VERSION_ERROR_IN_SRVC		-51  
#define   	WFS_ERR_INVALID_DATA				-52  
#define   	WFS_ERR_SOFTWARE_ERROR				-53  
#define 	WFS_ERR_CONNECTION_LOST				-54 
#define 	WFS_ERR_USER_ERROR					-55 
#define 	WFS_ERR_UNSUPP_DATA					-56 
                                            
//PTR                                        
#define 	WFS_ERR_PTR_FORMNOTFOUND           -100
#define 	WFS_ERR_PTR_FIELDNOTFOUND          -101
#define 	WFS_ERR_PTR_NOMEDIAPRESENT         -102
#define 	WFS_ERR_PTR_READNOTSUPPORTED       -103
#define 	WFS_ERR_PTR_FLUSHFAIL              -104
#define 	WFS_ERR_PTR_MEDIAOVERFLOW          -105
#define 	WFS_ERR_PTR_FIELDSPECFAILURE       -106
#define 	WFS_ERR_PTR_FIELDERROR             -107
#define 	WFS_ERR_PTR_MEDIANOTFOUND          -108
#define 	WFS_ERR_PTR_EXTENTNOTSUPPORTED     -109
#define 	WFS_ERR_PTR_MEDIAINVALID           -110
#define 	WFS_ERR_PTR_FORMINVALID            -111
#define 	WFS_ERR_PTR_FIELDINVALID           -112
#define 	WFS_ERR_PTR_MEDIASKEWED            -113
#define 	WFS_ERR_PTR_RETRACTBINFULL         -114
#define 	WFS_ERR_PTR_STACKERFULL            -115
#define 	WFS_ERR_PTR_PAGETURNFAIL           -116
#define 	WFS_ERR_PTR_MEDIATURNFAIL          -117
                                            
//PTR XFS 3.0 
#define 	WFS_ERR_PTR_SHUTTERFAIL            -118
#define 	WFS_ERR_PTR_MEDIAJAMMED            -119
#define 	WFS_ERR_PTR_FILE_IO_ERROR          -120
#define 	WFS_ERR_PTR_CHARSETDATA            -121
#define 	WFS_ERR_PTR_PAPERJAMMED            -122
#define 	WFS_ERR_PTR_PAPEROUT               -123
#define 	WFS_ERR_PTR_INKOUT                 -124
#define 	WFS_ERR_PTR_TONEROUT               -125
#define 	WFS_ERR_PTR_LAMPINOP               -126
#define 	WFS_ERR_PTR_SOURCEINVALID          -127
#define 	WFS_ERR_PTR_SEQUENCEINVALID        -128
#define 	WFS_ERR_PTR_MEDIASIZE              -129
                                            
//IDC                                        
#define 	WFS_ERR_IDC_MEDIAJAM               -200
#define 	WFS_ERR_IDC_NOMEDIA                -201
#define 	WFS_ERR_IDC_MEDIARETAINED          -202
#define 	WFS_ERR_IDC_RETAINBINFULL          -203
#define 	WFS_ERR_IDC_INVALIDDATA            -204
#define 	WFS_ERR_IDC_INVALIDMEDIA           -205
#define 	WFS_ERR_IDC_FORMNOTFOUND           -206
#define 	WFS_ERR_IDC_FORMINVALID            -207
#define 	WFS_ERR_IDC_DATASYNTAX             -208
#define 	WFS_ERR_IDC_SHUTTERFAIL            -209
#define 	WFS_ERR_IDC_SECURITYFAIL           -210
#define 	WFS_ERR_IDC_PROTOCOLNOTSUPP        -211
#define 	WFS_ERR_IDC_ATRNOTOBTAINED         -212
#define 	WFS_ERR_IDC_INVALIDKEY             -213
#define 	WFS_ERR_IDC_WRITE_METHOD           -214
#define 	WFS_ERR_IDC_CHIPPOWERNOTSUPP       -215
#define 	WFS_ERR_IDC_CARDTOOSHORT           -216
#define 	WFS_ERR_IDC_CARDTOOLONG            -217
                                            
//CDM                                        
#define 	WFS_ERR_CDM_INVALIDCURRENCY        -300
#define 	WFS_ERR_CDM_INVALIDTELLERID        -301
#define 	WFS_ERR_CDM_CASHUNITERROR          -302
#define 	WFS_ERR_CDM_INVALIDDENOMINATION    -303
#define 	WFS_ERR_CDM_INVALIDMIXNUMBER       -304
#define 	WFS_ERR_CDM_NOCURRENCYMIX          -305
#define 	WFS_ERR_CDM_NOTDISPENSABLE         -306
#define 	WFS_ERR_CDM_TOOMANYITEMS           -307
#define 	WFS_ERR_CDM_UNSUPPOSITION          -308
#define 	WFS_ERR_CDM_SAFEDOOROPEN           -310
#define 	WFS_ERR_CDM_SHUTTERNOTOPEN         -312
#define 	WFS_ERR_CDM_SHUTTEROPEN            -313
#define 	WFS_ERR_CDM_SHUTTERCLOSED          -314
#define 	WFS_ERR_CDM_INVALIDCASHUNIT        -315
#define 	WFS_ERR_CDM_NOITEMS                -316
#define 	WFS_ERR_CDM_EXCHANGEACTIVE         -317
#define 	WFS_ERR_CDM_NOEXCHANGEACTIVE       -318
#define 	WFS_ERR_CDM_SHUTTERNOTCLOSED       -319
#define 	WFS_ERR_CDM_PRERRORNOITEMS         -320
#define 	WFS_ERR_CDM_PRERRORITEMS           -321
#define 	WFS_ERR_CDM_PRERRORUNKNOWN         -322
#define 	WFS_ERR_CDM_ITEMSTAKEN             -323
#define 	WFS_ERR_CDM_INVALIDMIXTABLE        -327
#define 	WFS_ERR_CDM_OUTPUTPOS_NOT_EMPTY    -328
#define 	WFS_ERR_CDM_INVALIDRETRACTPOSITION -329
#define 	WFS_ERR_CDM_NOTRETRACTAREA         -330
#define 	WFS_ERR_CDM_NOCASHBOXPRESENT       -333
#define 	WFS_ERR_CDM_AMOUNTNOTINMIXTABLE    -334
#define 	WFS_ERR_CDM_ITEMSNOTTAKEN          -335
#define 	WFS_ERR_CDM_ITEMSLEFT	           -336 
                                            
//PIN                                        
#define 	WFS_ERR_PIN_KEYNOTFOUND            -400
#define 	WFS_ERR_PIN_MODENOTSUPPORTED       -401
#define 	WFS_ERR_PIN_ACCESSDENIED           -402
#define 	WFS_ERR_PIN_INVALIDID              -403
#define 	WFS_ERR_PIN_DUPLICATEKEY           -404
#define 	WFS_ERR_PIN_KEYNOVALUE             -406
#define 	WFS_ERR_PIN_USEVIOLATION           -407
#define 	WFS_ERR_PIN_NOPIN                  -408
#define 	WFS_ERR_PIN_INVALIDKEYLENGTH       -409
#define 	WFS_ERR_PIN_KEYINVALID             -410
#define 	WFS_ERR_PIN_KEYNOTSUPPORTED        -411
#define 	WFS_ERR_PIN_NOACTIVEKEYS           -412
#define 	WFS_ERR_PIN_INVALIDKEY             -413
#define 	WFS_ERR_PIN_NOTERMINATEKEYS        -414
#define 	WFS_ERR_PIN_MINIMUMLENGTH          -415
#define 	WFS_ERR_PIN_PROTOCOLNOTSUPP        -416
#define 	WFS_ERR_PIN_INVALIDDATA            -417
#define 	WFS_ERR_PIN_NOTALLOWED             -418
#define 	WFS_ERR_PIN_NOKEYRAM               -419
#define 	WFS_ERR_PIN_NOCHIPTRANSACTIVE      -420
#define 	WFS_ERR_PIN_ALGORITHMNOTSUPP       -421
#define 	WFS_ERR_PIN_FORMATNOTSUPP          -422
#define 	WFS_ERR_PIN_HSMSTATEINVALID        -423
#define 	WFS_ERR_PIN_MACINVALID             -424
#define 	WFS_ERR_PIN_PROTINVALID            -425
#define 	WFS_ERR_PIN_FORMATINVALID          -426
#define 	WFS_ERR_PIN_CONTENTINVALID         -427
                                            
//CHK                                        
#define 	WFS_ERR_CHK_FORMNOTFOUND           -500
#define 	WFS_ERR_CHK_FORMINVALID            -501
#define 	WFS_ERR_CHK_MEIDANOTFOUND          -502
#define 	WFS_ERR_CHK_MEIDAINVALID           -503
#define 	WFS_ERR_CHK_MEDIAOVERFLOW          -504
#define 	WFS_ERR_CHK_FIELDNOTFOUND          -505
#define 	WFS_ERR_CHK_FIELDINVALID           -506
#define 	WFS_ERR_CHK_FIELDERROR             -507
#define 	WFS_ERR_CHK_REQFIELDMISSING        -508
#define 	WFS_ERR_CHK_FIELDSPECFAILURE       -509
#define 	WFS_ERR_CHK_CHARSETDATA            -510
#define 	WFS_ERR_CHK_MEDIAJAM			   -511
#define 	WFS_ERR_CHK_SHUTTERFAIL            -512
                                            
//DEP                                        
#define 	WFS_ERR_DEP_DEPFULL                -600
#define 	WFS_ERR_DEP_DEPJAMMED              -601
#define 	WFS_ERR_DEP_ENVEMPTY               -602
#define 	WFS_ERR_DEP_ENVJAMMED              -603
#define 	WFS_ERR_DEP_ENVSIZE                -604
#define 	WFS_ERR_DEP_NOENV                  -605
#define 	WFS_ERR_DEP_PTRFAIL                -606
#define 	WFS_ERR_DEP_SHTNOTCLOSED           -607 
#define 	WFS_ERR_DEP_SHTNOTOPENED           -608
#define 	WFS_ERR_DEP_CONTMISSING            -609
#define 	WFS_ERR_DEP_DEPUNKNOWN             -610
#define 	WFS_ERR_DEP_CHARSETNOTSUPP         -611
#define 	WFS_ERR_DEP_TONEROUT               -612
                                            
//TTU                                        
#define 	WFS_ERR_TTU_FIELDERROR             -701
#define 	WFS_ERR_TTU_FIELDINVALID           -702
#define 	WFS_ERR_TTU_FIELDNOTFOUND          -703
#define 	WFS_ERR_TTU_FIELDSPECFAILURE       -704
#define 	WFS_ERR_TTU_FORMINVALID            -705
#define 	WFS_ERR_TTU_FORMNOTFOUND           -706
#define 	WFS_ERR_TTU_INVALIDLED             -707
#define 	WFS_ERR_TTU_KEYCANCELED            -708
#define 	WFS_ERR_TTU_MEDIAOVERFLOW          -709
#define 	WFS_ERR_TTU_RESNOTSUPP	           -710 
#define 	WFS_ERR_TTU_CHARSETDATA            -711
#define 	WFS_ERR_TTU_KEYINVALID             -712
#define 	WFS_ERR_TTU_KEYNOTSUPPORTED        -713
#define 	WFS_ERR_TTU_NOACTIVEKEYS           -713
                                            
//SIU                                        
#define 	WFS_ERR_SIU_INVALID_PORT           -801
#define 	WFS_ERR_SIU_SYNTAX                 -802
#define 	WFS_ERR_SIU_PORT_ERROR             -803
                                                                                            
//CAM                                                                                        
#define  WFS_ERR_CAM_CAMNOTSUPP					-1000				 
#define  WFS_ERR_CAM_MEDIAFULL					-1001				 
#define  WFS_ERR_CAM_CAMINOP					-1002				 
#define  WFS_ERR_CAM_CHARSETNOTSUPP				-1003				 
                                                                                            
//CIM                                                                                        
#define  WFS_ERR_CIM_INVALIDCURRENCY			-1300				 
#define  WFS_ERR_CIM_INVALIDTELLERID			-1301				 
#define  WFS_ERR_CIM_CASHUNITERROR				-1302				 
#define  WFS_ERR_CIM_TOOMANYITEMS				-1307				 
#define  WFS_ERR_CIM_UNSUPPOSITION				-1308				 
#define  WFS_ERR_CIM_SAFEDOOROPEN				-1310				 
#define  WFS_ERR_CIM_SHUTTERNOTOPEN				-1312				 
#define  WFS_ERR_CIM_SHUTTEROPEN				-1313				 
#define  WFS_ERR_CIM_SHUTTERCLOSED				-1314				 
#define  WFS_ERR_CIM_INVALIDCASHUNIT			-1315				
#define  WFS_ERR_CIM_NOITEMS					-1316				 
#define  WFS_ERR_CIM_EXCHANGEACTIVE				-1317				 
#define  WFS_ERR_CIM_NOEXCHANGEACTIVE			-1318				 
#define  WFS_ERR_CIM_SHUTTERNOTCLOSED			-1319				 
#define  WFS_ERR_CIM_ITEMSTAKEN					-1323				 
#define  WFS_ERR_CIM_CASHINACTIVE				-1325				 
#define  WFS_ERR_CIM_NOCASHINACTIVE				-1326				 
#define  WFS_ERR_CIM_POSITION_NOT_EMPTY			-1328				 
#define  WFS_ERR_CIM_INVALIDRETRACTPOSITION		-1334				 
#define  WFS_ERR_CIM_NOTRETRACTAREA				-1335				 
                                                                                            
//CDP                                                                                        
#define  BS_ERR_CDP_MEDIAJAM					-9900				 
#define  BS_ERR_CDP_NOMEDIA						-9901				 
#define  BS_ERR_CDP_MEDIARETAINED				-9902				 
#define  BS_ERR_CDP_RETAINBINFULL				-9903				 
#define  BS_ERR_CDP_INVALIDDATA					-9904				 
#define  BS_ERR_CDP_INVALIDMEDIA				-9905				 
#define  BS_ERR_CDP_DATASYNTAX					-9906				 
#define  BS_ERR_CDP_SHUTTERFAIL					-9907				 
#define  BS_ERR_CDP_CARDTOOSHORT				-9908				 
#define  BS_ERR_CDP_CARDTOOLONG					-9909				 



///////////////////////////////////////////////////////////////////////////////
#endif // _COMMON_DEF_H_
