/** ***************************************************************
*	@file LineTCP.cpp 
*	@date 2017/10/20
*	@author	MFS	
*	@brief 실질적인 통신을 수행하는 TCP라인을 구현한 소스파일입니다. 
********************************************************************/
#include "stdafx.h"
#include "LineTcp.h"

#include "Queue.h"
#include "../../EagleCE_Framework/EagleCE_Framework/EagleLogger.h"
#include "../../EagleCE_Framework/EagleCE_Framework/Util.h"

CString strAMSCertFile;
char	chAMSCertFile[1024] = { 0, };
//------------------------------------------------------------------
//	Define extern function
//------------------------------------------------------------------
extern int		put_data(char *recv_data, int recv_size);
extern int		put_event(char recv_evt);

//------------------------------------------------------------------
//	Define Return Value
//------------------------------------------------------------------
#define RET_OK		0 ///< RET_OK 반환값이 정상임을 의미함
#define RET_NG		1 ///< RET_NG 반환값이 NG임을 의미함. 
#define RET_NG_SSL	22 ///< RET_NG-SSL 반환값이 SSL-NG임을 의미함. 

/** ********************************************************************
* @brief TCP, SSL통신 관련 변수들을 기본값으로 초기화하는 함수입니다. @n
윈도우 소켓을 로딩하는 로직도 포함하고 있습니다. 
* @param 없음
* @retval 없음
************************************************************************/
CLineTcp::CLineTcp() : m_eStartCheckSocket(FALSE, FALSE)
{
	CLineCtrl::Register(LINE_TCP, this);  // 객체의 주소값을 전달합니다. 
 
	WSADATA wsd;                          // 윈도우 소켓 초기화에 필요한 변수를 선언합니다.  

	// Load Win sock
    if (WSAStartup(MAKEWORD(2,2), &wsd) != 0)  // 윈도우 소켓을 초기화하고 실패할 경우 로그를 남깁니다. 
		LOG(Error, _T("WSAStartup Error"));

	
	m_Socket = INVALID_SOCKET;
	m_ListenSocket = INVALID_SOCKET;
	m_pRemoteAddr = NULL;
	m_bRequestConnect = FALSE;

	m_bConnectStatus = FALSE;
	m_bUseSSL = FALSE;
	m_pSSLMethod = NULL;
	m_pSSLCtx = NULL;
	m_pSSL = NULL;

	// OpenSSL 1.1.0+ initializes the library automatically on first use;
	// SSL_library_init()/OpenSSL_add_all_algorithms()/SSL_load_error_strings() are no longer required.

	m_nTCPConnectionTimeout = 30;
	m_bUseListenSSL = FALSE;

	m_bSocketClose_Forced = FALSE;
}

/** *************************************************************
* @brief  윈도우 소켓을 언로드하고 CleanUp 함수를 호출합니다. 
* @param 없음 
* @retval 없음 
*****************************************************************/
CLineTcp::~CLineTcp()
{
	LOG(Info, _T("Line TCP Terminate"));

	CleanUp();

	m_bExitThread = TRUE;

	// Unload Win sock
	if (WSACleanup() == SOCKET_ERROR)
	{
		LOG(Error, _T("WSACleanup Error"));
	}
}
/** ***************************************************
* @brief 통신과 관련된 변수들을 모두 사용하지 않는 상태로 전환합니다.
커뮤니케이션 소켓과 리스닝 소켓을 닫는 로직도 포함하고 있습니다. 
* @param 없음 
* @retval 없음 
*******************************************************/
void CLineTcp::CleanUp()
{
	//LOG(Info, _T("CleanUp call"));

	m_bCheckSSLAccept = FALSE;
	m_bCheckConnect = FALSE;
	m_bCheckSocket = FALSE;
	m_bConnectStatus = FALSE;
	m_bRequestConnect = FALSE;
	m_bUseListenSSL = FALSE;

	// send SSL/TLS close_notify
	//if ((m_bUseSSL == TRUE) && (m_pSSL != NULL))
	//{
	//	int ret = 0;

	//	LOG(Info, _T("ssl free(0x%x) - start"), m_pSSL);

	//	ret = SSL_shutdown(m_pSSL);

	//	if (ret < 1)
	//	{
	//		shutdown(m_Socket, SD_SEND);
	//	}
	//	
	//	if (ret > 0)
	//	{
	//		SSL_free(m_pSSL);
	//	}
	//	else
	//	{
	//		SSL_clear(m_pSSL);
	//		SSL_free(m_pSSL);
	//	}

	//	// delete ssl ctx
	//	if (m_pSSLCtx != NULL)
	//	{
	//		LOG(Info, _T("ctx free(0x%x) - start"), m_pSSLCtx);
	//		SSL_CTX_free(m_pSSLCtx);
	//	}

	//	m_pSSL = NULL;
	//	m_pSSLCtx = NULL;
	//	m_pSSLMethod = NULL;

	//	ERR_clear_error();
	//	ERR_remove_state(0);
	//}

	int ret = 0;

	// SSL Connect중에 Timeout 발생시 강제로 Socket만 Close하기 위해 변수 Check (Thread에서 CleanUp호출)
	// Socket 강제 Close시에 SSL Connect 함수가 빠져 나오므로 이 후 SSL 관련 Memory를 해제하도록 함 (2번째로 Line Open에서 CleanUp 호출)
	if (m_bSocketClose_Forced == FALSE)
	{
		if (m_pSSL != NULL)
		{
			// 연결이 된 상태에서만 SSL_shutdown을 call하도록 수정 (ssl connect 함수 return이 안되어져 있는 경우 call시 비정상 종료됨
			LOG(Info, _T("ssl shutdown(0x%x) - start"), m_pSSL);

			ret = SSL_shutdown(m_pSSL);

			if (ret < 1)
			{
				shutdown(m_Socket, SD_SEND);
			}

			LOG(Info, _T("ssl free(0x%x) (%d) - start"), m_pSSL, ret);

			// ssl memory 해제
			if (ret > 0)
			{
				SSL_free(m_pSSL);
			}
			else
			{
				SSL_clear(m_pSSL);
				SSL_free(m_pSSL);
			}
		}

		// delete ssl ctx
		if (m_pSSLCtx != NULL)
		{
			LOG(Info, _T("ctx free(0x%x) - start"), m_pSSLCtx);
			SSL_CTX_free(m_pSSLCtx);
		}

		m_pSSL = NULL;
		m_pSSLCtx = NULL;
		m_pSSLMethod = NULL;

		ERR_clear_error();
		ERR_remove_state(0);
	}

	m_bSocketClose_Forced = FALSE;
	m_bUseSSL = FALSE;
		
	// close communication socket
	if (m_Socket != INVALID_SOCKET)
	{
		shutdown(m_Socket, SD_BOTH);
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
	}

	// close listen socket
	if (m_ListenSocket != INVALID_SOCKET)
	{
		shutdown(m_ListenSocket, SD_BOTH);
		closesocket(m_ListenSocket);
		m_ListenSocket = INVALID_SOCKET;
	}

	// delete addr info
	if (m_pRemoteAddr != NULL)
	{
		freeaddrinfo(m_pRemoteAddr);
		m_pRemoteAddr = NULL;
	}

	//m_pSSL = NULL;
	//m_pSSLCtx = NULL;
	//m_pSSLMethod = NULL;
}

/** *****************************************************************************************************
*	@brief 받는 주소로 SSL모드, 호스트 모드, 클라이언트 모드로 분기를 타고 소켓 연결까지 해주는 함수입니다. 
*	@param LPCTSTR Dest_Info 아이피주소  
*	@param LPCTSTR dest_port 포트번호 (`~65,535) 
*	@param LPCTSTR sOption SSL 옵션 ( 0: 사용 안함, 1: TLS1.2 사용) 
*	@param LPCTSTR mode  소켓 모드 (0:host connect mode, )
*	@retval RET_NG 1, 
*	@retval RET_OK 0
*	@todo 리턴값이 반대인 로직이 있음. 
*	@todo 소켓 옵션 용어 정리
*	@todo m_eStartCheckSocket.Set();기능 파악 불가 
********************************************************************************************************/
int	CLineTcp::LineOpen(LPCTSTR Dest_Info,		/*ip:[xxx.xxx.xxx.xxx]*/
					   LPCTSTR dest_port,		/*port:[xxxxx]*/ 
					   LPCTSTR sOption,			/*SSL mode: "1" */	 
					   LPCTSTR mode)			/*socket mode: "0" - host connect mode, "1" - ready for AMS,	"2" - AMS connect mode*/
{
	CString sSSLOption = CString (sOption);

	int		nServerMode = _ttoi(mode);
	int		nUseSSL = _ttoi(sSSLOption);

	BOOL	bUseCertFile = FALSE;
	BOOL	bIsAMSMode = FALSE;		// [PCI-SSF Fix] AMS ???? ??? ???θ? ???? ĳ?? - ??? ????? ????? ????? ???? ???
	m_bUseSSL = FALSE;	// Accept 하기 전에는 UseSSL이 초기화 되지 않기 때문에 초기화 로직 추가함.
	m_bUseListenSSL = FALSE;
	m_bSocketClose_Forced = FALSE;

	// AMS Listening시의 Port 정보 제거
	if (nServerMode == 1)
	{
		// Server Mode에서는 Port 정보는 제외 시킴
		LOG(Info, _T("LineOpen Call Server Mode (SSL Option : %s)"), sSSLOption);
	}
	else
	{
		// Client Mode에서만 IP / Port 정보 표시
		LOG(Info, _T("LineOpen Call (Host : %s, Port : %s, SSL Option : %s, Mode : %s)"), Dest_Info, dest_port, sSSLOption, mode);
	}


	if (nServerMode == 2)
	{
		// AMS Client Mode인 경우에는 CertFile을 사용하도록 함
		bUseCertFile = TRUE;
		bIsAMSMode = TRUE;
		m_nTCPConnectionTimeout = 30;
		nServerMode = 0;					// Client Mode로 변환
	}
	else
	{
		m_nTCPConnectionTimeout = 30;		// Default로 Setting
	}

	if ((m_Socket != INVALID_SOCKET) || (m_ListenSocket != INVALID_SOCKET))
	{
		LOG(Error, _T("m_Socket is error condition"));
		CleanUp();
		return RET_NG;
	}
	
	// Init value
	m_bRequestConnect = FALSE;
	m_bConnectStatus = FALSE;

	// Client Mode
	if (nServerMode == 0)
	{
		// 1. resolve address
		m_pRemoteAddr = ResolveAddress(Dest_Info, dest_port);
		if (m_pRemoteAddr == NULL)
		{
			LOG(Error, _T("m_pRemoteAddr is null"));
			return RET_NG;
		}

		// 2. Create Socket
		m_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (m_Socket == INVALID_SOCKET)
		{
			LOG(Error, _T("m_Socket is invalid"));
			CleanUp();
			return RET_NG;
		}

		// 3. Set socket option
		{
			BOOL	bDontLinger = TRUE;
			if (setsockopt(m_Socket, SOL_SOCKET, SO_DONTLINGER, (char*)&bDontLinger, sizeof(BOOL)) != 0)
			{
				LOG(Error, _T("setsockopt SO_DONTLINGER is invalid"));
				CleanUp();
				return RET_NG;
			}

			BOOL	bKeepAlive = TRUE;
			if (setsockopt(m_Socket, SOL_SOCKET, SO_KEEPALIVE, (char*)&bKeepAlive, sizeof(BOOL)) != 0)
			{
				LOG(Error, _T("setsockopt SO_KEEPALIVE is invalid"));
				CleanUp();
				return RET_NG;
			}
		}

		// 4. Connect
		m_bCheckConnect = TRUE;
		m_eStartCheckSocket.Set();

		if (connect(m_Socket, m_pRemoteAddr->ai_addr, m_pRemoteAddr->ai_addrlen) == SOCKET_ERROR)
		{
			LOG(Error, _T("connect is failed"));
			CleanUp();
			return RET_NG;
		}

		LOG(Info, (_T("TCP/IP Connect OK")));

		// 5. USE SSL Option
		if (nUseSSL > 0)
		{
			m_pSSLMethod = TLS_client_method();				// Support TLS1.2 (range narrowed below via SSL_CTX_set_options)

			if (m_pSSLMethod == NULL)
			{
				LOG(Error, (_T("m_pSSLMethod is null")));
				CleanUp();
				return RET_NG_SSL;
			}

			m_pSSLCtx = SSL_CTX_new(m_pSSLMethod);
			if (m_pSSLCtx == NULL)
			{
				LOG(Error, (_T("m_pSSLCtx is null")));
				CleanUp();
				return RET_NG_SSL;
			}

			SSL_CTX_set_options(m_pSSLCtx, SSL_OP_NO_SSLv2|SSL_OP_NO_SSLv3|SSL_OP_NO_TLSv1|SSL_OP_NO_TLSv1_1);

			// [PCI-SSF Fix] 취약한 Cipher Suite(RC4/3DES/EXPORT/NULL/MD5 등) 배제
			if (SSL_CTX_set_cipher_list(m_pSSLCtx, "HIGH:!aNULL:!eNULL:!EXPORT:!RC4:!3DES:!MD5:!PSK") != 1)
			{
				LOG(Error, (_T("SSL_CTX_set_cipher_list is failed")));
				CleanUp();
				return RET_NG_SSL;
			}

			// [PCI-SSF Fix] AMS 뿐 아니라 표준 Host(Triton) 연결도 인증서 검증 대상으로 포함
			bUseCertFile = TRUE;

			// AMS Client Mode인 경우 AMS 인증서 Load 로직 추가
			if (bUseCertFile == TRUE)
			{
				BOOL bResult = TRUE;
				LPCTSTR pCertFileName = bIsAMSMode ? AMS_CERTIFICATE_FILE : HOST_ROOTCA_CERTIFICATE_FILE;

				strAMSCertFile.Format(_T("%s%s"), CUtil::GetAppPath(), pCertFileName);
				WideCharToMultiByte(CP_ACP, 0, strAMSCertFile, -1, chAMSCertFile, strAMSCertFile.GetLength(), NULL, NULL);

				if (SSL_CTX_load_verify_locations(m_pSSLCtx, chAMSCertFile, NULL) != 1)
				{
					LOG(Error, _T("Load CA file is failed (%s)"), strAMSCertFile);
					bResult = FALSE;
				}

				// Deliberately not calling SSL_CTX_set_default_verify_paths(): that
				// call adds the OS/OpenSSL default CA bundle on top of the CA file
				// just loaded above, so a certificate from any publicly-trusted CA
				// would also pass verification. Trust must be pinned to the single
				// CA file distributed with the terminal (rootCA.pem / ams-certificate.pem).

				// [PCI-SSF Fix] 인증서 검증을 SSL Handshake 단계에서 강제 (기존에는 SSL_connect 성공 후 SSL_get_verify_result로만 사후 확인)
				SSL_CTX_set_verify(m_pSSLCtx, SSL_VERIFY_PEER, NULL);

				//if (bResult == TRUE)
				//{
				//	LOG(Info, _T("AMS Cert File is loaded"));
				//}
			}

			m_pSSL = SSL_new(m_pSSLCtx);
			if (m_pSSL == NULL)
			{
				LOG(Error, (_T("m_pSSL is null")));
				CleanUp();
				return RET_NG_SSL;
			}

			if (SSL_set_fd(m_pSSL, m_Socket) < 1)
			{
				LOG(Error, (_T("SSL_set_fd is error")));
				CleanUp();
				return RET_NG_SSL;
			}

			SSL_set_connect_state(m_pSSL);
			SSL_set_shutdown(m_pSSL, SSL_SENT_SHUTDOWN|SSL_RECEIVED_SHUTDOWN);


			// set flag to check timeout
			int nReturn = SSL_connect(m_pSSL);

			if (nReturn <= 0)
			{
				//m_bUseSSL = TRUE;	// SSL_connect fail시 CleanUp()을 수행하기 위해 다시 Set을 함 (SSL_connect timeout시에 강제 종료를 위해 FALSE 처리하므로)
				
				LOG(Error, _T("SSL_connect error (%d)"), nReturn);

				CleanUp();
				return RET_NG_SSL;
			}
			else
			{
				//SSL 접속은 성공.
				//SSL 검증 로직 추가
				//SSL_VER_23_USE_CERTIFY=10 or SSL_VER_3_USE_CERTIFY=11 일 경우 이 로직을 탐.

				LOG(Info, _T("SSL Connect is OK"));

				if (bUseCertFile == TRUE)
				{
					if (m_pSSL == NULL)
					{
						CleanUp();
						return RET_NG_SSL;
					}

					int ret = SSL_get_verify_result(m_pSSL);
						
					if (ret == X509_V_OK)
					{
						// [PCI-SSF Fix] 신뢰 체인 검증(ret==X509_V_OK, 즉 rootCA.pem/ams-certificate.pem으로 로드한
						// CA 목록 안에 발급 체인이 있음)이 성공하면 유효한 것으로 판단한다.
						// Dest_Info가 IP 주소로 전달되는 운영 환경 특성상 CN/SAN을 호스트명과
						// 문자열로 비교하는 방식은 쓰지 않고, CA 신뢰 체인 검증 결과만으로 판단한다.
						// (CN은 운영 확인/감사 로그 용도로만 남겨둔다.)
						LOG(Info, _T("SSL_get_verify_result is OK (trusted by loaded CA)"));

						X509* peerCertificate = SSL_get_peer_certificate(m_pSSL);
						if (peerCertificate != NULL)
						{
							X509_NAME* name = X509_get_subject_name(peerCertificate);
							int common_name_loc = X509_NAME_get_index_by_NID(name, NID_commonName, -1);

							if (common_name_loc >= 0)
							{
								X509_NAME_ENTRY* common_name_entry = X509_NAME_get_entry(name, common_name_loc);
								ASN1_STRING* common_name_asn1 = X509_NAME_ENTRY_get_data(common_name_entry);

								if (common_name_asn1 != NULL)
								{
									char	szCommonName[256] = { 0, };
									int		nCommonNameLen = ASN1_STRING_length(common_name_asn1);

									if (nCommonNameLen > 0 && nCommonNameLen < sizeof(szCommonName))
									{
										CString strPeerCN;

										memcpy(szCommonName, ASN1_STRING_data(common_name_asn1), nCommonNameLen);
										strPeerCN.Format(_T("%S"), szCommonName);

										LOG(Info, _T("Peer Certificate Common Name (%s)"), strPeerCN);
									}
								}
							}

							X509_free(peerCertificate);
						}
					}
					else //SSL CERT. 인증실패
					{
						if (bUseCertFile == TRUE)
						{
							// AMS와의 연결시도시에는 인증서 Expired에 대해서는 Skip하도록 처리
							// 인터넷 확인 결과 Web Broser 종류에 따라 유효기간 에러코드가 다름
							// Google Crome - CRT_NOT_VALID, Explore - X509_V_ERR_CERT_HAS_EXPIRED or CRT_NOT VALID 이므로 여러개의 조건을 Check하도록 함
							if ((ret == X509_V_ERR_CERT_HAS_EXPIRED) || (ret == X509_V_ERR_CRL_HAS_EXPIRED)||(ret == X509_V_ERR_CRL_NOT_YET_VALID)||(ret == X509_V_ERR_CERT_NOT_YET_VALID))
							{
								LOG(Info, _T("AMS SSL_get_verify_result is failed (%S)(%d) - skip reason - exipred cert."), X509_verify_cert_error_string(ret), ret);
							}
							else
							{
								LOG(Error, _T("AMS SSL_get_verify_result is failed (%S)(%d)"), X509_verify_cert_error_string(ret), ret);
								CleanUp();
								return RET_NG_SSL;
							}
						}
						else
						{
							LOG(Error, _T("SSL_get_verify_result is failed (%S)(%d)"), X509_verify_cert_error_string(ret), ret);
							CleanUp();
							return RET_NG_SSL;
						}
					}
				}
			}
			m_bUseSSL = TRUE;
						
		} //SSL END

		// 6. Start Check Socket
		m_bCheckConnect = FALSE;
		m_bCheckSocket = TRUE;

		m_bConnectStatus = TRUE;
		put_event(CONNECT_EVT);
	}
	// Server Mode
	else
	{
		// 1. Create Socket
		m_ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (m_ListenSocket == INVALID_SOCKET)
		{
			LOG(Error, (_T("m_ListenSocket is invalid")));
			CleanUp();
			return RET_NG;
		}

		// 2. Set socket option
		{
			BOOL	bDontLinger = TRUE;
			if (setsockopt(m_ListenSocket, SOL_SOCKET, SO_DONTLINGER, (char*)&bDontLinger, sizeof(BOOL)) != 0)
			{
				LOG(Error, (_T("setsockopt SO_DONTLINGER is invalid")));
				CleanUp();
				return RET_NG;
			}

			BOOL	bKeepAlive = TRUE;
			if (setsockopt(m_ListenSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&bKeepAlive, sizeof(BOOL)) != 0)
			{
				LOG(Error, (_T("setsockopt SO_KEEPALIVE is invalid")));
				CleanUp();
				return RET_NG;
			}
		}

		// 3. Bind
		{
			SOCKADDR_IN		ServerAddr;

			memset(&ServerAddr, 0, sizeof(SOCKADDR_IN));
			ServerAddr.sin_family = AF_INET;
			ServerAddr.sin_port = htons((unsigned short)_ttoi(dest_port));
			ServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);

			if (bind(m_ListenSocket, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr)) == SOCKET_ERROR)
			{
				LOG(Error, (_T("bind is failed")));
				CleanUp();
				return RET_NG;
			}
		}

		// 4. Listen
		if (listen(m_ListenSocket, 5) == SOCKET_ERROR)
		{
			LOG(Error, (_T("listen is failed")));
			CleanUp();
			return RET_NG;
		}

		LOG(Info, (_T("Listen is OK")));

		if (nUseSSL > 0)
		{
			// 5. USE SSL Option
			CString strCertFile, strPrivKeyFile;
			char szCert_File[MAX_PATH];
			char szPrivKey_File[MAX_PATH];
			
			memset(szCert_File, NULL, MAX_PATH);
			memset(szPrivKey_File, NULL, MAX_PATH);
			strCertFile.Format(_T("%s%s"), CUtil::GetAppPath(), TERMINAL_CERTIFICATE_FILE);
			strPrivKeyFile.Format(_T("%s%s"), CUtil::GetAppPath(), TERMINAL_PRIVATEKEY_FILE);

			WideCharToMultiByte(CP_ACP, 0, strCertFile, -1, szCert_File, strCertFile.GetLength(), NULL, NULL);
			WideCharToMultiByte(CP_ACP, 0, strPrivKeyFile, -1, szPrivKey_File, strPrivKeyFile.GetLength(), NULL, NULL);

			m_pSSLMethod = TLS_server_method();				// Support TLS1.2 (range narrowed below via SSL_CTX_set_options)

			if (m_pSSLMethod == NULL)
			{
				LOG(Error, (_T("m_pSSLMethod is null")));
				CleanUp();
				return RET_NG_SSL;
			}

			m_pSSLCtx = SSL_CTX_new(m_pSSLMethod);
			if (m_pSSLCtx == NULL)
			{
				LOG(Error, (_T("m_pSSLCtx is null")));
				CleanUp();
				return RET_NG_SSL;
			}

			SSL_CTX_set_options(m_pSSLCtx, SSL_OP_NO_SSLv2|SSL_OP_NO_SSLv3|SSL_OP_NO_TLSv1|SSL_OP_NO_TLSv1_1);

			// [PCI-SSF Fix] 취약한 Cipher Suite(RC4/3DES/EXPORT/NULL/MD5 등) 배제
			if (SSL_CTX_set_cipher_list(m_pSSLCtx, "HIGH:!aNULL:!eNULL:!EXPORT:!RC4:!3DES:!MD5:!PSK") != 1)
			{
				LOG(Error, (_T("SSL_CTX_set_cipher_list is failed")));
				CleanUp();
				return RET_NG_SSL;
			}

			if (SSL_CTX_use_certificate_file(m_pSSLCtx, szCert_File, SSL_FILETYPE_PEM) <= 0)
			{
				LOG(Error, (_T("SSL_CTX_use_certificate_file is failed.")));
				CleanUp();
				return RET_NG_SSL;
			}
			if (SSL_CTX_use_PrivateKey_file(m_pSSLCtx, szPrivKey_File, SSL_FILETYPE_PEM) <= 0) 
			{
				LOG(Error, (_T("SSL_CTX_use_PrivateKey_file is failed.")));
				CleanUp();
				return RET_NG_SSL;
			}
			if (!SSL_CTX_check_private_key(m_pSSLCtx)) 
			{
				LOG(Error, (_T("SSL_CTX_check_private_key is failed.")));
				CleanUp();
				return RET_NG_SSL;
			}
			m_bUseListenSSL = TRUE;

			LOG(Info, (_T("SSL Listen is OK")));
		}

		// 5. Start Check Socket
		m_bCheckSocket = TRUE;
		m_eStartCheckSocket.Set();
	}

	LOG(Info, _T("CLineTcp LineOpen Call end"));

	return RET_OK;
}

/** **************************************************************
* @brief 라인을 닫습니다. 
* @param 없음 
* @retval RET_OK 0
* @todo m_eMothodUUse의 기능 파악 불가 
******************************************************************/
int	CLineTcp::LineClose()
{
	
	m_bConnectStatus = FALSE;

	if (m_Socket == INVALID_SOCKET && m_ListenSocket == INVALID_SOCKET)
		return RET_OK;   //이미 Close된 상태이므로 RET_OK 리턴함

	LOG(Info, _T("LineClose Call"));

	m_bCheckSocket = FALSE;
	m_bCheckConnect = FALSE;
	CleanUp();

	if (m_eMethodUse.Wait(1100) == WAIT_OBJECT_0)
		m_eMethodUse.Set();

	LOG(Info, _T("LineClose Call end"));

	return RET_OK;
}

/** ***********************************************************
* @brief 서버모드에서 ACCEPT를 진행합니다. 
* @param BYTE opt 0x01 - CALL ACCEPT
opt 0x00 CALL DENY
* @retval RET_NG 1
* @retval RET_OK 0
* @todo 리턴값이 반대인 부분이 있음. @n 
m_eMethodUse의 역할 파악 불가.
***************************************************************/
int CLineTcp::LineAccept(BYTE opt)
{
	LOG(Info, _T("LineAccept Call (%d)"), opt);

	//m_bRequestConnect = FALSE;

	if ((m_ListenSocket == INVALID_SOCKET) || (m_Socket != INVALID_SOCKET))				// already accepted. we support only one connection.
	{
		m_bRequestConnect = FALSE;
		LOG(Error, _T("m_ListenSock is invalid or m_Socket is already existed"));
		return RET_OK;	// 리턴값이 반대임. ㅠㅠ;;
	}

	{
		// Accept
		CAutoLock	Sync(m_eMethodUse);

		SOCKADDR_IN		ClientAddr;
		int				nClientAddrLen;

		memset(&ClientAddr, 0, sizeof(ClientAddr));
		nClientAddrLen = sizeof(ClientAddr);
		m_Socket = accept(m_ListenSocket, (SOCKADDR*)&ClientAddr, &nClientAddrLen);

		if (m_Socket == INVALID_SOCKET)
		{
			m_bRequestConnect = FALSE;
			LOG(Error, _T("LineAccept() m_Socket is invalid"));
			return RET_OK;	// 리턴값이 반대임. ㅠㅠ;;
		}

		LOG(Info, _T("LineAccept() Accept is OK"));
		// Client가 SSL Accept를 수행하지 않을 경우를 대비해 TimeOut 값 설정함.

		// SSL 구성
		if (m_bUseListenSSL == TRUE)
		{
			m_pSSL = SSL_new(m_pSSLCtx);
			if (m_pSSL == NULL)
			{
				LOG(Error, (_T("m_pSSL is null")));
				CleanUp();
				return RET_OK;	// 리턴값이 반대임.
			}

			if (SSL_set_fd(m_pSSL, m_Socket) < 1)
			{
				LOG(Error, (_T("SSL_set_fd is error")));
				CleanUp();
				return RET_OK;	// 리턴값이 반대임.
			}
		}
	}

	// SSL_Accept시에 Client가 SSL 연결을 하지 않을 경우 Holding되는 현상 해결을 위해 CAutoLock을 해제한다.
	if (m_bUseListenSSL == TRUE)
	{
		if (m_pSSL == NULL)
		{
			LOG(Error, (_T("SSL_accept m_pSSL is null - error")));
			CleanUp();
			return RET_OK;	// 리턴값이 반대임.
		}

		m_bCheckSSLAccept = TRUE;

		if (SSL_accept(m_pSSL) < 1)
		{
			LOG(Error, (_T("SSL_accept is error")));
			CleanUp();
			return RET_OK;	// 리턴값이 반대임.
		}

		m_bCheckSSLAccept = FALSE;
		m_bUseSSL = TRUE;
	}

	//////////////////////////////
	m_bRequestConnect = FALSE;
	m_bConnectStatus = TRUE;
	put_event(CONNECT_EVT);

	LOG(Info, _T("LineAccept Call end"));

	return RET_NG;	// 리턴값이 반대임. ㅠㅠ;;
}

/** ***************************************
* @brief 연결 상태를 반환하는 함수입니다. 
* @param int nMode 소켓 모드
* @retval BOOL m_bConnectStatus 
* @retval BOOL m_bRequestConnect
*******************************************/
int CLineTcp::LineCallState(int nMode)
{
	if (nMode == 0)
		return (int)m_bRequestConnect;

	return m_bConnectStatus;
}


/** ****************************************
* @brief 연결된 소켓으로 데이터를 보내는 함수입니다. 
* @param BYTE *send_data 
* @param int len 
* @retval RET_NG 1
* @retval RET_OK 0
********************************************/
int	CLineTcp::LineSendData(BYTE *send_data, int len)
{
	int	nSent = 0;
	int	ret;

	//LOG(Info, _T("LineSendData Call"));

	if (m_Socket == INVALID_SOCKET)
	{
		LOG(Error, _T("LineSendData() m_Socket is invalid"));
		return RET_NG;
	}

	while(nSent < len)
	{
		if (m_bUseSSL == TRUE)
		{
			// m_pSSL이 null인 경우 openssl 비정상 종료되는 현상 대응
			if (m_pSSL != NULL)
			{
				ret = SSL_write(m_pSSL, (void*)&send_data[nSent], (len-nSent));
			}
			else
			{
				ret = SOCKET_ERROR;
				LOG(Error, _T("LineSendData - m_pSSL is null - Error"));
			}
		}
		else
		{
			ret = send(m_Socket, (char*)&send_data[nSent], (len-nSent), 0);
		}

		if (ret == SOCKET_ERROR)
		{
			LOG(Error, _T("LineSendData Socket Error"));
			CleanUp();
			return RET_NG;
		}
		nSent += ret;
	}

	//LOG(Error, _T("LineSendData Call end"));

	return RET_OK;
}

/** *******************************************************************
* @brief 아이피 주소와 포트 주소를 형식에 맞게 반환시켜주는 함수입니다.
* @param LPCTSTR lpszAddr 아이피 주소 
* @param LPCTSTR lpszPort 포트 주소 
* @retval addrinfo* 주소 관련 구조체 
***********************************************************************/
addrinfo* CLineTcp::ResolveAddress(LPCTSTR lpszAddr, LPCTSTR lpszPort)
{
    struct addrinfo hints,  *pAddr = NULL;
    int             rc;
	char	szAddr[256] ={0,};
	char	szPort[256] ={0,};
	CString strIpAddr;

	// URL로 입력한 것을 IP Addr로 변환하는 과정
	IPAddr ipAddress ;
	char szHost[MAX_PATH]  = { 0, };
	TCHAR tszIPAddr[MAX_PATH]  = { 0, } ;
	struct in_addr iaDest;		// Internet address structure
	LPHOSTENT pHost;			// Pointer to host entry structure

	WideCharToMultiByte( CP_ACP, 0, lpszAddr, -1, szHost, sizeof(szHost), NULL, FALSE );
	ipAddress = inet_addr(szHost);
	iaDest.s_addr = inet_addr(szHost);
	if (iaDest.s_addr == INADDR_NONE)
	{
		pHost = gethostbyname(szHost);
		if( pHost )
		{
			char *pIP ;
			iaDest.S_un.S_addr = *(DWORD *)(*pHost->h_addr_list) ;
			pIP = inet_ntoa(iaDest);
			if(strlen(pIP))
				MultiByteToWideChar( CP_ACP, 0, pIP , -1, tszIPAddr, sizeof(tszIPAddr) );
		}

		if( _tcslen(tszIPAddr) )
			LOG(Info, _T("tszIPAddr (%s)"), tszIPAddr);

		strIpAddr.Format(_T("%s"), tszIPAddr);
	}
	///////////////////////////////////////////////////////////////////
	else
	{
		strIpAddr = lpszAddr;
	}

	// check IP ADDRESS 
//	strIpAddr = lpszAddr;
	if (strIpAddr.GetLength() <= 15)
	{
		int		nDot = 0;
		BOOL	bNumeric = TRUE;
		
		for (int i = 0; i < strIpAddr.GetLength(); i++)
		{
			if (strIpAddr[i] == '.')
				nDot++;
			else if (strIpAddr[i] < '0' || strIpAddr[i] > '9')
				bNumeric = FALSE;
		}

		// replace '000' -> '0'
		if (nDot == 3 && bNumeric == TRUE)
		{
			int		nStartIndex, nLastIndex;
			CString strTemp, strNewIpAddr;

			nStartIndex = 0;
			nLastIndex = strIpAddr.Find('.');
			while (nLastIndex != -1)
			{
				strTemp.Format(L"%d", _ttoi((LPCTSTR)strIpAddr.Mid(nStartIndex, nLastIndex)));

				if (strNewIpAddr.GetLength() > 0)
					strNewIpAddr += _T(".");
				strNewIpAddr += strTemp;

				nStartIndex = nLastIndex+1;
				nLastIndex = strIpAddr.Find('.', nStartIndex);
				if (nLastIndex == -1 && nStartIndex < strIpAddr.GetLength())
					nLastIndex = strIpAddr.GetLength() - 1;
			}
			strIpAddr = strNewIpAddr;
		}
	}

	// check port range
	int	nPort = _ttoi(lpszPort);
	if (nPort < 0 || nPort > 65535)
	{
		return NULL;
	}
	LOG(Info, _T("IpAddr:[%s]"), strIpAddr);

	// convert char to unicode
	WideCharToMultiByte(CP_ACP, 0, strIpAddr, -1, szAddr, sizeof(szAddr), NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, lpszPort, -1, szPort, sizeof(szPort), NULL, NULL);

	// make hint info
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags  = ((szAddr) ? 0 : AI_PASSIVE);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

	// get address
    rc = getaddrinfo(szAddr, szPort, &hints, &pAddr);
    if (rc != 0)
    {
        return NULL;
    }

    return pAddr;
}

/** ***********************************************************************
* @brief ssl 통신 시간제한을 체크하는 루프, 데이터를 받는 루프가 구현된 함수입니다. 데이터를 받으면 큐로 저장합니다. 
* @param 없음  
* @retval 없음
***************************************************************************/

unsigned CLineTcp::ThreadHandlerProc()
{
	struct fd_set		fdread,
						fdexcept;
    struct timeval		timeout;
	int					rc;
	SOCKET				CheckSocket;
	int					nWaitCount;

	m_bExitThread = FALSE;

	while(m_bExitThread != TRUE)
	{
		if (m_eStartCheckSocket.Wait(1000) != WAIT_OBJECT_0)
			continue;
		
		nWaitCount = 0;

		/* CHECK -- SSL CONNECTION TIMEOUT */
		while(m_bCheckConnect)
		{
			// Max 30sec
			if (nWaitCount < m_nTCPConnectionTimeout)
			{
				m_eStartCheckSocket.Wait(1000);
				nWaitCount++;
			}
			else
			{
				LOG(Error, _T("Connection timeout"));
				// SSL CONNECT 중에 Timeout 발생시 Socket Close를 해야함 (SSL_Connect 중에 SSL_ShutDown을 하면 문제가 발생함)
				//if (m_bUseSSL == TRUE)	
				//	m_bUseSSL = FALSE;

				m_bSocketClose_Forced = TRUE;

				CleanUp();
				break;
			}
		} // end of while(m_bCheckConnect)

		/* CHECK -- DATA RECV FROM REMOTE */
		while(m_bCheckSocket)
		{
			CAutoLock	Sync(m_eMethodUse);

			// select to check socket
			if (m_Socket == INVALID_SOCKET)
			{
				//LOG(Info, _T("Check Listening Socket)"));
				CheckSocket = m_ListenSocket;		// when wait for connect to remote
			}
			else
			{
				//LOG(Info, _T("Check Client Socket"));
				CheckSocket = m_Socket;				// when connected to remote
			}

			// clear set
			FD_ZERO(&fdread);
			FD_ZERO(&fdexcept);
		
			// add socket to set
			FD_SET(CheckSocket, &fdread);
			FD_SET(CheckSocket, &fdexcept);

			// set timeout value
			timeout.tv_sec = 1;
			timeout.tv_usec = 0;

			if (m_bCheckSSLAccept == TRUE)
			{
				nWaitCount++;

				if (nWaitCount > 3)	// 3 sec
				{
					// Accept Timoue으로 간주하여 종료처리
					LOG(Info, _T("Accept Timeout - Force socket closed (%d) socket return value ( 0x%x)"), nWaitCount, rc);

					m_bSocketClose_Forced = TRUE;

					CleanUp();
					break;
				}

				m_eStartCheckSocket.Wait(1000);
				rc = 0;	// SSL Accept가 완료될때까지 임의로 timeout값으로 처리하면서 select를 호출하지 않도록 함 (SSL Accept중 select 호출시 비정상 종료현상 대책)
			}
			else
			{
				rc = select(0, &fdread, NULL, &fdexcept, &timeout);
			}

			// rc '0' is timeout.
			if (rc == SOCKET_ERROR)
			{
				LOG(Info, _T("SOCKET ERROR (%d) select)"), rc);
				break;
			}
			//else if (rc != 0)
			else if (rc > 0)
			{
				if (FD_ISSET(CheckSocket, &fdread))
				{
					// Received Data from remote
					if (CheckSocket == m_Socket)
					{
						// select 1초 delay time 문제로 인해 동기화 안맞는 현상 수정 : flag를 한번 더 Check하도록 수정
						if (m_bCheckSocket == FALSE)	// AP에서 Line Close를 했으므로, Thread 대기하도록 함
						{
							break;
						}

						while(1)
						{
							// Recv Buffer 초기화
							memset(m_RecvBuf, 0, sizeof(m_RecvBuf));
							rc = SOCKET_ERROR;

							if (m_bUseSSL == TRUE)
							{
								if( (m_pSSL != NULL) && (m_Socket != INVALID_SOCKET) )
									rc = SSL_read(m_pSSL, m_RecvBuf, sizeof(m_RecvBuf));
								else
									break;
							}
							else
							{
								rc = recv(m_Socket, m_RecvBuf, sizeof(m_RecvBuf), 0);
							}

							// 
							if(rc <= 0)
							{
								LOG(Info, _T("return value ( 0x%x)"), rc);
								break;
							}

							// Read data again if ATM receives Maximum data size (CLineCtrl::m_RecvBuf[2048])
							if(rc>=sizeof(m_RecvBuf))		
							{
								put_data(m_RecvBuf, rc);
							}
							else
							{
								break;
							}
						}

						// successfully full read
						if (rc > 0)
						{
							put_data(m_RecvBuf, rc);
						}

						// an error occur
						if (rc == SOCKET_ERROR)
						{
							m_bConnectStatus = FALSE;
							put_event(DISCONNECT_EVT);
							break;
						}
						// gracefully closed
						else if (rc == 0)
						{
							m_bConnectStatus = FALSE;
							put_event(DISCONNECT_EVT);
							break;
						}
					}
					else
					{
						if (m_ListenSocket != INVALID_SOCKET)
						{
							// Request connection from remote
							if (m_bRequestConnect == FALSE)
							{
								LOG(Info, _T("Remote Connection is Detected socket return value ( 0x%x)"), rc);

								m_bRequestConnect = TRUE;
								nWaitCount = 0;	// @@@ Check Timeout
							}
							else
							{
								//LOG(Info, _T("Remote Connection is already Detected - ignored - socket return value ( 0x%x)"), rc);
							}
						}
						else
						{
							//LOG(Info, _T("Remote Connection is Detected - ignored Client Mode - socket return value ( 0x%x)"), rc);
						}

						m_eStartCheckSocket.Wait(1000);
					}
				}

				if (FD_ISSET(CheckSocket, &fdexcept))
				{
					LOG(Error, _T("SOCKET Exception)"));
					m_bConnectStatus = FALSE;
					put_event(DISCONNECT_EVT);
					break;
				}
			}
			else
			{
				//LOG(Error, _T("SOCKET FRead is not Set)"));
			}

		} // end of while(m_bCheckSocket)

		m_bCheckSSLAccept = FALSE;
		m_bCheckSocket = FALSE;
		m_bCheckConnect = FALSE;
		m_bConnectStatus = FALSE;
		m_bRequestConnect = FALSE;

	} // end of while(m_bExitThread != TRUE)

	return 0;
}

/** *******************************************************************
* @brief TestPing
* @param LPCTSTR sHostInfo	서버의 아이피 주소
* @param ICMP_ECHO_REPLY &icmpEchoReply		응답정보
* @retval RET_NG 1
* @retval RET_OK 0
***********************************************************************/
int CLineTcp::TestPing(LPCTSTR sHostInfo, ICMP_ECHO_REPLY &icmpEchoReply)
{
	int nRet = RET_NG;

	LOG(Info, _T("TestPing Call (%s)"), sHostInfo);

	int nTtl = 128;
	int nWaitTime = 1000;

	IPAddr ipAddress ;
	IP_OPTION_INFORMATION ipInfo ;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	char szHost[MAX_PATH]  = { 0, };
	TCHAR tszPCName[MAX_PATH]  = { 0, } ;
	struct in_addr iaDest;		// Internet address structure
	LPHOSTENT pHost;			// Pointer to host entry structure

//	_tcscpy( szName , _T("") );
	WideCharToMultiByte( CP_ACP, 0, sHostInfo, -1, szHost, sizeof(szHost), NULL, FALSE );
	ipAddress = inet_addr(szHost);
	iaDest.s_addr = inet_addr(szHost);
	if (iaDest.s_addr == INADDR_NONE)
	{
		LOG(Error, _T("iaDest.s_addr == INADDR_NONE"));	//*LEHTEST
		pHost = gethostbyname(szHost);
		if( pHost )
		{
			char *pIP ;
			iaDest.S_un.S_addr = *(DWORD *)(*pHost->h_addr_list) ;
			pIP = inet_ntoa(iaDest);
			if(strlen(pIP))
				MultiByteToWideChar( CP_ACP, 0, pIP , -1, tszPCName, sizeof(tszPCName) );
// 			if( _tcslen( szPCName ) )
// 				_tcscpy( szName , szPCName );
		}
	}
	else
	{
		LOG(Error, _T("iaDest.s_addr != INADDR_NONE"));	//*LEHTEST
		pHost = gethostbyaddr((const char *)&iaDest, 
			sizeof(struct in_addr), AF_INET);
		if( pHost )
		{
			MultiByteToWideChar( CP_ACP, 0, pHost->h_name, -1, tszPCName, sizeof( tszPCName ) );
// 			if( _tcslen( szPCName ) )
// 				_tcscpy( szName , szPCName );
		}
	}

	if( _tcslen(tszPCName) )
		LOG(Info, _T("szPCName (%s)"), tszPCName);
		
	if( pHost )
		ipAddress = *(DWORD *)(*pHost->h_addr_list) ;

	if (ipAddress != INADDR_NONE)
	{
		DWORD dwRet;

		hFile = IcmpCreateFile();

		// Set some reasonable default values
		ipInfo.Ttl   = nTtl ;
		ipInfo.Tos   = 0;
		ipInfo.Flags = 0;
		ipInfo.OptionsSize = 0 ;
		ipInfo.OptionsData = NULL ;
		icmpEchoReply.Status    = IP_SUCCESS ;

		if(hFile != INVALID_HANDLE_VALUE)
		{
			dwRet = IcmpSendEcho(hFile, ipAddress, NULL, 0, (PIP_OPTION_INFORMATION)&ipInfo, 
				&icmpEchoReply, sizeof(ICMP_ECHO_REPLY), nWaitTime);
		
			IcmpCloseHandle(hFile);

			if (dwRet != 0)
			{
				if (IP_SUCCESS == icmpEchoReply.Status)
				{
					nRet = RET_OK;
				}
				else
				{
					LOG(Error, _T("icmpEcho.Status is not success. icmpEchoReply.Status:[%d]"), icmpEchoReply.Status);
				}
			}
			else
			{
				LOG(Error, _T("IcmpSendEcho() is failed. GetLastError : [%d][0x%08X]"), GetLastError(), GetLastError());
			}
		}
		else
		{
			LOG(Error, _T("IcmpCreateFile() is failed."));
		}
	}

	LOG(Info, _T("TestPing Call end"));

	return nRet;
}