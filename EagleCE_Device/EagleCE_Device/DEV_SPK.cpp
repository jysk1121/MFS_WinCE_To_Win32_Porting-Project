#include "stdafx.h"
#include "DEV_SPK.h"
#include <mmdeviceapi.h>
#include <endpointvolume.h>


/************************************************************
*	@brief		생성자
*	@retval		없음
************************************************************/
CDEV_SPK::CDEV_SPK()
{
	m_VoiceObject.CoCreateInstance(CLSID_SpVoice);
}


/************************************************************
*	@brief		소멸자
*	@retval		없음
************************************************************/
CDEV_SPK::~CDEV_SPK()
{
	m_VoiceObject.Release();
}


/************************************************************
*	@brief		초기화
*	@retval		없음
************************************************************/
BOOL CDEV_SPK::Initialize(void)
{
	// 비트레이트
	SetRate(-2);

	// 언어, 성별 설정
	SetVoices(_T("Language=409"), _T("Gender=Female"));

	// 볼륨 설정
	SetVolume(100);

	m_VoiceObject->SetAlertBoundary(SPEI_PHONEME);

	m_VoiceObject->SetPriority(SPVPRI_ALERT);


	return TRUE;
}


/************************************************************
*	@brief		속도 설정
*	@retval		없음
************************************************************/
void CDEV_SPK::SetRate(int nRate)
{
	m_VoiceObject->SetRate(nRate);
}


/************************************************************
*	@brief		음성 설정
*	@retval		없음
************************************************************/
void CDEV_SPK::SetVoices(CString strLanguage, CString strGender)
{
	CComPtr<IEnumSpObjectTokens> cpIEnum;

	SpEnumTokens(SPCAT_VOICES, strLanguage, strGender, &cpIEnum);


	CComPtr<ISpObjectToken> cpIVoiceToken;

	cpIEnum->Next(1, &cpIVoiceToken, NULL);

	m_VoiceObject->SetVoice(cpIVoiceToken);
}


/************************************************************
*	@brief		볼륨 설정
*	@retval		없음
************************************************************/
void CDEV_SPK::SetVolume(int nVolume)
{
	m_VoiceObject->SetVolume(nVolume);
}


/************************************************************
*	@brief		재생
*	@retval		없음
************************************************************/
BOOL CDEV_SPK::SpeakAsync(CString strText)
{
	ULONG nStreamNumber;

	return SUCCEEDED(m_VoiceObject->Speak(strText, SPF_IS_NOT_XML | SPF_ASYNC | SPF_PURGEBEFORESPEAK, &nStreamNumber));
}


/************************************************************
*	@brief		중지
*	@retval		없음
************************************************************/
BOOL CDEV_SPK::SpeakStop()
{
	ULONG nStreamNumber;

	return SUCCEEDED(m_VoiceObject->Speak(_T(""), SPF_IS_NOT_XML | SPF_ASYNC | SPF_PURGEBEFORESPEAK, &nStreamNumber));
}
