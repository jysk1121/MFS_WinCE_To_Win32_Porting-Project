#include "stdafx.h"
#include "EagleDataManager.h"

#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"

// 인스턴스
CEagleDataManager* CEagleDataManager::m_pInstance = NULL;


/** **********************************************************
*	@brief		인스턴스 생성
*	@retval		없음
************************************************************/
void CEagleDataManager::CreateInstance()
{
	m_pInstance = new CEagleDataManager();
}


/** **********************************************************
*	@brief		인스턴스 취득
*	@retval		없음
************************************************************/
CEagleDataManager* CEagleDataManager::GetInstance()
{
	return m_pInstance;
}


/** **********************************************************
*	@brief		인스턴스 해제
*	@retval		없음
************************************************************/
void CEagleDataManager::ReleaseInstance()
{
	if (m_pInstance)
	{
		delete m_pInstance;
	}
}


/** **********************************************************
*	@brief		생성자
*	@retval		없음
************************************************************/
CEagleDataManager::CEagleDataManager()
{
}


/** **********************************************************
*	@brief		소멸자
*	@retval		없음
************************************************************/
CEagleDataManager::~CEagleDataManager()
{
}


/** **********************************************************
*	@brief		초기화
*	@retval		없음
************************************************************/
BOOL CEagleDataManager::Initialize(void)
{
	LOG(Info, _T("CEagleDataManager::Initialize Start"));

	// 고객 정보
	m_Client_Info.Clear();


	// 환경파일 로드
	if (FALSE == m_Config.Load())
	{
		return FALSE;
	}

	LOG(Info, _T("Load Config files end"));

	return TRUE;
}


/** **********************************************************
*	@brief		종결화
*	@retval		없음
************************************************************/
BOOL CEagleDataManager::Finalize(void)
{
	// 환경파일 저장
	if (FALSE == m_Config.Save())
	{
		return FALSE;
	}

	return TRUE;
}

