#ifndef __NH_KEYMGR_STATE_DEFINE_H__
#define __NH_KEYMGR_STATE_DEFINE_H__

// ----------------------------------------------------------------------------
//	KEYMGR STATE의 이름 정의
// ----------------------------------------------------------------------------
#define	STATE_EXIT							L""								// 빈 State 에러를 의미한다.

// ----------------------------------------------------------------------------
//	KEYMGR STATE의 이름 정의
// ----------------------------------------------------------------------------
#define STATE_KM_INIT						L"KM_INIT"
#define STATE_KM_DEINIT						L"KM_DEINIT"
#define STATE_KM_ENTERPW					L"KM_ENTERPW"

// ----------------------------------------------------------------------------
//	KEYMGR STATE의 이름 정의
// ----------------------------------------------------------------------------
#define STATE_KM_MAIN						L"KM_KEYMGRMAIN"
#define STATE_KM_MAIN2						L"KM_KEYMGRMAIN2"
#define STATE_KM_EDITMKKEY					L"KM_EDITMKKEY"
#define STATE_KM_EDITMKKEY2					L"KM_EDITMKKEY2"
#define STATE_KM_ENTERKEYIDX				L"KM_ENTERKEYIDX"
#define STATE_KM_ENTERMKKEY					L"KM_ENTERMKKEY"
#define STATE_KM_ENTERMKKEY2				L"KM_ENTERMKKEY2"
#define STATE_KM_ENTERMKKEY3				L"KM_ENTERMKKEY3"
#define STATE_KM_CHEKCKEY					L"KM_CHECKKEY"
#define STATE_KM_CHEKCKEY2					L"KM_CHECKKEY2"
#define STATE_KM_CHANGEPW					L"KM_CHANGEPW"
#define STATE_KM_SERIALNUMBER				L"KM_SERIALNUMBER"
#define	STATE_KM_REMOTEKEYINFO				L"KM_REMOTEKEYINFO"
#define STATE_KM_SELECTMKEY					L"KM_SELECTMKEY"
#define STATE_KM_SELECTPART					L"KM_SELECTPART"

//Key Mode Au¿e
#define STATE_KM_MAIN_KM					L"KM_KEYMGRMAIN_KM"
//#define	STATE_KM_INJECTMENU_KM				L"KM_INJECTMENU_KM"
#define STATE_KM_EDITMKKEY_KM				L"KM_EDITMKKEY_KM"
#define STATE_KM_CHEKCKEY_KM				L"KM_CHECKKEY_KM"
#define STATE_KM_ENTERMKKEY_KMS				L"KM_ENTERMKKEY_KMS"
#define STATE_KM_ENTERMKKEY_KMT				L"KM_ENTERMKKEY_KMT"
#define	STATE_KM_INJECTMENU_KM				L"KM_INJECTMENU_KM"

//PCI 2.0 - Removal Detection
#define	STATE_KM_ENTERID					L"KM_ENTERID"
#define	STATE_KM_SET_STATE					L"KM_SET_STATE"

#define	STATE_KM_ADD_ID						L"KM_ADD_ID"
#define	STATE_KM_CHANGE_ID					L"KM_CHANGE_ID"
#define	STATE_KM_DELETE_ID					L"KM_DELETE_ID"
#define	STATE_KM_CHANGE_STATE				L"KM_CHANGE_STATE"


#define	STATE_KM_SETFIELDMODE				L"KM_SETFIELDMODE"
#define	STATE_KM_CLEAR_EPP					L"KM_CLEAR_EPP"


#endif //__NH_KEYMGR_STATE_DEFINE_H__