/** ***************************************************************
*	@file JsonUtil.h
*	@brief parson 기반 JSON 파싱/생성 유틸리티 클래스입니다.
*	@details CUtil의 Wrapper_OpenZip 계열과 동일한 방식으로, parson의
*	JSON_Value, JSON_Object, JSON_Array 핸들을 그대로 노출하고,
*	CString 입출력 변환만 대신 처리해줍니다.
********************************************************************/

#ifndef __JSON_UTIL_H__
#define __JSON_UTIL_H__

#include "parson.h"

/************************************************************
* @class CJsonUtil
* @brief JSON 파싱/생성을 위한 정적 유틸리티 클래스입니다.
************************************************************/
class CJsonUtil
{
public:
	/* Parse */
	static JSON_Value*		ParseFromString(CString strJson);
	static JSON_Value*		ParseFromFile(CString strFilePath);

	/* Create (신규 문서 생성) */
	static JSON_Value*		CreateObjectDocument();
	static JSON_Value*		CreateArrayDocument();

	/* Free */
	static void				Free(JSON_Value *pRootValue);

	/* Root Accessors */
	static JSON_Object*		GetRootObject(const JSON_Value *pRootValue);
	static JSON_Array*		GetRootArray(const JSON_Value *pRootValue);

	/* Serialize */
	static CString			SerializeToString(const JSON_Value *pValue, BOOL bPretty = FALSE);
	static BOOL				SerializeToFile(const JSON_Value *pValue, CString strFilePath, BOOL bPretty = FALSE);

	/* Object Getters (없으면 strDefault/dDefault/bDefault 반환) */
	static BOOL				HasValue(const JSON_Object *pObject, CString strName);
	static CString			GetString(const JSON_Object *pObject, CString strName, CString strDefault = _T(""));
	static double			GetNumber(const JSON_Object *pObject, CString strName, double dDefault = 0.0);
	static BOOL				GetBool(const JSON_Object *pObject, CString strName, BOOL bDefault = FALSE);
	static JSON_Object*		GetObject(const JSON_Object *pObject, CString strName);
	static JSON_Array*		GetArray(const JSON_Object *pObject, CString strName);

	/* Object Setters */
	static BOOL				SetString(JSON_Object *pObject, CString strName, CString strValue);
	static BOOL				SetNumber(JSON_Object *pObject, CString strName, double dValue);
	static BOOL				SetBool(JSON_Object *pObject, CString strName, BOOL bValue);
	static BOOL				SetValue(JSON_Object *pObject, CString strName, JSON_Value *pValue);

	/* Array Helpers */
	static size_t			GetArrayCount(const JSON_Array *pArray);
	static CString			GetArrayString(const JSON_Array *pArray, size_t nIndex, CString strDefault = _T(""));
	static double			GetArrayNumber(const JSON_Array *pArray, size_t nIndex, double dDefault = 0.0);
	static JSON_Object*		GetArrayObject(const JSON_Array *pArray, size_t nIndex);
	static BOOL				AppendString(JSON_Array *pArray, CString strValue);
	static BOOL				AppendNumber(JSON_Array *pArray, double dValue);
	static BOOL				AppendValue(JSON_Array *pArray, JSON_Value *pValue);

	/* Sample : 실제 사용 예제 (문서 생성 -> 직렬화 -> 파싱 -> 값 조회 왕복 검증) */
	static BOOL				Sample();

private:
	CJsonUtil();	// 정적 유틸 클래스 - 인스턴스 생성 금지

	/* char* <-> CString 변환 내부 헬퍼 */
	static CString			ToCString(const char *pszSrc);
	static CStringA		ToCStringA(CString strSrc);
};

#endif //__JSON_UTIL_H__
