/** ***************************************************************
*	@file JsonUtil.cpp
*	@brief parson 기반 JSON 파싱/생성 유틸리티 소스파일입니다.
********************************************************************/
#include "stdafx.h"
#include "JsonUtil.h"
#include "EagleLogger.h"

CJsonUtil::CJsonUtil()
{
}

/** *****************************************************************
* @brief const char* 를 CString(TCHAR) 으로 변환합니다. NULL이면 빈 문자열.
***********************************************************************/
CString CJsonUtil::ToCString(const char *pszSrc)
{
	if (pszSrc == NULL)
		return _T("");

	CString strDest;
	int nLen = MultiByteToWideChar(CP_UTF8, 0, pszSrc, -1, NULL, 0);

	if (nLen <= 0)
		return _T("");

	MultiByteToWideChar(CP_UTF8, 0, pszSrc, -1, strDest.GetBuffer(nLen), nLen);
	strDest.ReleaseBuffer();

	return strDest;
}

/** *****************************************************************
* @brief CString(TCHAR) 을 UTF-8 CStringA 로 변환합니다.
***********************************************************************/
CStringA CJsonUtil::ToCStringA(CString strSrc)
{
	CStringA strDest;
	int nLen = WideCharToMultiByte(CP_UTF8, 0, strSrc, -1, NULL, 0, NULL, NULL);

	if (nLen <= 0)
		return "";

	WideCharToMultiByte(CP_UTF8, 0, strSrc, -1, strDest.GetBuffer(nLen), nLen, NULL, NULL);
	strDest.ReleaseBuffer();

	return strDest;
}

/* ==================== Parse ==================== */

JSON_Value* CJsonUtil::ParseFromString(CString strJson)
{
	return json_parse_string(ToCStringA(strJson));
}

JSON_Value* CJsonUtil::ParseFromFile(CString strFilePath)
{
	return json_parse_file(ToCStringA(strFilePath));
}

/* ==================== Create ==================== */

JSON_Value* CJsonUtil::CreateObjectDocument()
{
	return json_value_init_object();
}

JSON_Value* CJsonUtil::CreateArrayDocument()
{
	return json_value_init_array();
}

/* ==================== Free ==================== */

void CJsonUtil::Free(JSON_Value *pRootValue)
{
	if (pRootValue != NULL)
		json_value_free(pRootValue);
}

/* ==================== Root Accessors ==================== */

JSON_Object* CJsonUtil::GetRootObject(const JSON_Value *pRootValue)
{
	return json_value_get_object(pRootValue);
}

JSON_Array* CJsonUtil::GetRootArray(const JSON_Value *pRootValue)
{
	return json_value_get_array(pRootValue);
}

/* ==================== Serialize ==================== */

CString CJsonUtil::SerializeToString(const JSON_Value *pValue, BOOL bPretty)
{
	if (pValue == NULL)
		return _T("");

	char *pszSerialized = bPretty ? json_serialize_to_string_pretty(pValue) : json_serialize_to_string(pValue);

	if (pszSerialized == NULL)
		return _T("");

	CString strResult = ToCString(pszSerialized);

	json_free_serialized_string(pszSerialized);

	return strResult;
}

BOOL CJsonUtil::SerializeToFile(const JSON_Value *pValue, CString strFilePath, BOOL bPretty)
{
	if (pValue == NULL)
		return FALSE;

	JSON_Status status = bPretty ?
		json_serialize_to_file_pretty(pValue, ToCStringA(strFilePath)) :
		json_serialize_to_file(pValue, ToCStringA(strFilePath));

	return (status == JSONSuccess);
}

/* ==================== Object Getters ==================== */

BOOL CJsonUtil::HasValue(const JSON_Object *pObject, CString strName)
{
	if (pObject == NULL)
		return FALSE;

	return json_object_has_value(pObject, ToCStringA(strName)) != 0;
}

CString CJsonUtil::GetString(const JSON_Object *pObject, CString strName, CString strDefault)
{
	if (pObject == NULL)
		return strDefault;

	const char *pszValue = json_object_get_string(pObject, ToCStringA(strName));

	if (pszValue == NULL)
		return strDefault;

	return ToCString(pszValue);
}

double CJsonUtil::GetNumber(const JSON_Object *pObject, CString strName, double dDefault)
{
	if (pObject == NULL || !HasValue(pObject, strName))
		return dDefault;

	return json_object_get_number(pObject, ToCStringA(strName));
}

BOOL CJsonUtil::GetBool(const JSON_Object *pObject, CString strName, BOOL bDefault)
{
	if (pObject == NULL)
		return bDefault;

	int nValue = json_object_get_boolean(pObject, ToCStringA(strName));

	if (nValue < 0)	// -1 : not found / not boolean
		return bDefault;

	return (nValue != 0);
}

JSON_Object* CJsonUtil::GetObject(const JSON_Object *pObject, CString strName)
{
	if (pObject == NULL)
		return NULL;

	return json_object_get_object(pObject, ToCStringA(strName));
}

JSON_Array* CJsonUtil::GetArray(const JSON_Object *pObject, CString strName)
{
	if (pObject == NULL)
		return NULL;

	return json_object_get_array(pObject, ToCStringA(strName));
}

/* ==================== Object Setters ==================== */

BOOL CJsonUtil::SetString(JSON_Object *pObject, CString strName, CString strValue)
{
	if (pObject == NULL)
		return FALSE;

	return json_object_set_string(pObject, ToCStringA(strName), ToCStringA(strValue)) == JSONSuccess;
}

BOOL CJsonUtil::SetNumber(JSON_Object *pObject, CString strName, double dValue)
{
	if (pObject == NULL)
		return FALSE;

	return json_object_set_number(pObject, ToCStringA(strName), dValue) == JSONSuccess;
}

BOOL CJsonUtil::SetBool(JSON_Object *pObject, CString strName, BOOL bValue)
{
	if (pObject == NULL)
		return FALSE;

	return json_object_set_boolean(pObject, ToCStringA(strName), bValue ? 1 : 0) == JSONSuccess;
}

BOOL CJsonUtil::SetValue(JSON_Object *pObject, CString strName, JSON_Value *pValue)
{
	if (pObject == NULL)
		return FALSE;

	return json_object_set_value(pObject, ToCStringA(strName), pValue) == JSONSuccess;
}

/* ==================== Array Helpers ==================== */

size_t CJsonUtil::GetArrayCount(const JSON_Array *pArray)
{
	if (pArray == NULL)
		return 0;

	return json_array_get_count(pArray);
}

CString CJsonUtil::GetArrayString(const JSON_Array *pArray, size_t nIndex, CString strDefault)
{
	if (pArray == NULL)
		return strDefault;

	const char *pszValue = json_array_get_string(pArray, nIndex);

	if (pszValue == NULL)
		return strDefault;

	return ToCString(pszValue);
}

double CJsonUtil::GetArrayNumber(const JSON_Array *pArray, size_t nIndex, double dDefault)
{
	if (pArray == NULL || nIndex >= json_array_get_count(pArray))
		return dDefault;

	return json_array_get_number(pArray, nIndex);
}

JSON_Object* CJsonUtil::GetArrayObject(const JSON_Array *pArray, size_t nIndex)
{
	if (pArray == NULL)
		return NULL;

	return json_array_get_object(pArray, nIndex);
}

BOOL CJsonUtil::AppendString(JSON_Array *pArray, CString strValue)
{
	if (pArray == NULL)
		return FALSE;

	return json_array_append_string(pArray, ToCStringA(strValue)) == JSONSuccess;
}

BOOL CJsonUtil::AppendNumber(JSON_Array *pArray, double dValue)
{
	if (pArray == NULL)
		return FALSE;

	return json_array_append_number(pArray, dValue) == JSONSuccess;
}

BOOL CJsonUtil::AppendValue(JSON_Array *pArray, JSON_Value *pValue)
{
	if (pArray == NULL)
		return FALSE;

	return json_array_append_value(pArray, pValue) == JSONSuccess;
}

/* ==================== Sample ==================== */

/** *****************************************************************************************************
*	@brief CJsonUtil 실제 사용 예제입니다. JSON 문서를 생성 -> 직렬화 -> 파싱 -> 값 조회까지
*	한 번에 왕복시켜 정상 동작 여부를 검증합니다.
*	@details 아래와 같은 JSON을 만들어서 다시 읽어들입니다.
*	{"TransactionId":"TXN-00001","Amount":125000,"Approved":true,"Items":["cash","receipt"]}
*	@retval TRUE  왕복 검증 성공
*	@retval FALSE 파싱 실패 또는 값 불일치
********************************************************************************************************/
BOOL CJsonUtil::Sample()
{
	/* 1. 문서 생성 */
	JSON_Value *pDoc = CreateObjectDocument();
	JSON_Object *pObj = GetRootObject(pDoc);

	SetString(pObj, _T("TransactionId"), _T("TXN-00001"));
	SetNumber(pObj, _T("Amount"), 125000.0);
	SetBool(pObj, _T("Approved"), TRUE);

	JSON_Value *pItemsValue = CreateArrayDocument();
	JSON_Array *pItemsArray = GetRootArray(pItemsValue);
	AppendString(pItemsArray, _T("cash"));
	AppendString(pItemsArray, _T("receipt"));

	SetValue(pObj, _T("Items"), pItemsValue);	// pItemsValue의 소유권이 pDoc으로 넘어감 (별도 Free 불필요)

	/* 2. 직렬화 */
	CString strJson = SerializeToString(pDoc, TRUE);

	LOG(Info, _T("CJsonUtil::Sample - built json:\r\n%s"), strJson);

	Free(pDoc);

	/* 3. 파싱 */
	JSON_Value *pParsed = ParseFromString(strJson);

	if (pParsed == NULL)
	{
		LOG(Error, _T("CJsonUtil::Sample - ParseFromString failed"));
		return FALSE;
	}

	/* 4. 값 조회 */
	JSON_Object *pParsedObj = GetRootObject(pParsed);

	CString strTransactionId = GetString(pParsedObj, _T("TransactionId"));
	double dAmount = GetNumber(pParsedObj, _T("Amount"));
	BOOL bApproved = GetBool(pParsedObj, _T("Approved"));

	JSON_Array *pParsedItems = GetArray(pParsedObj, _T("Items"));
	CString strFirstItem = GetArrayString(pParsedItems, 0);

	LOG(Info, _T("CJsonUtil::Sample - parsed TransactionId=%s Amount=%.0f Approved=%d Items[0]=%s (ItemCount=%d)"),
		strTransactionId, dAmount, bApproved, strFirstItem, GetArrayCount(pParsedItems));

	Free(pParsed);

	/* 5. 왕복 검증 */
	BOOL bResult = (strTransactionId == _T("TXN-00001"))
				&& (dAmount == 125000.0)
				&& (bApproved == TRUE)
				&& (strFirstItem == _T("cash"));

	LOG(bResult ? Info : Error, _T("CJsonUtil::Sample - result %s"), bResult ? _T("OK") : _T("FAILED"));

	return bResult;
}
