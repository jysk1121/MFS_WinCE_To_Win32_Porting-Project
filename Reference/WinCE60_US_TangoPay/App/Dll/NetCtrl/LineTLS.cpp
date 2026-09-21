#include "stdafx.h"
#include "LineTLS.h"
#include "Queue.h"

//------------------------------------------------------------------
//	Define extern function
//------------------------------------------------------------------
extern int		put_data(char *recv_data, int recv_size);
extern int		put_event(char recv_evt);

#include ".\Common\NHDbgApi.h"

#define PREFERRED_CIPHERS "TLS_AES_256_GCM_SHA384:TLS_CHACHA20_POLY1305_SHA256:TLS_AES_128_GCM_SHA256:DHE-RSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-RSA-AES128-GCM-SHA256"
#define MEDIUM_CIPHERS "TLS_AES_256_GCM_SHA384:TLS_CHACHA20_POLY1305_SHA256:TLS_AES_128_GCM_SHA256:DHE-RSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-RSA-AES128-GCM-SHA256:DHE-RSA-AES256-SHA256:DHE-RSA-AES128-SHA256:ECDHE-RSA-AES256-SHA384:ECDHE-RSA-AES128-SHA256"
#define PROSA_CIPHERS "TLS_AES_256_GCM_SHA384:TLS_CHACHA20_POLY1305_SHA256:TLS_AES_128_GCM_SHA256:DHE-RSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-RSA-AES128-GCM-SHA256:DHE-RSA-AES256-SHA256:DHE-RSA-AES128-SHA256:ECDHE-RSA-AES256-SHA384:ECDHE-RSA-AES128-SHA256:AES256-GCM-SHA384"

// These ciphers are secure, AEAD cipher suites
// Prosa requires a medium-security cipher strength
#if (APP_CUSTOM_CIBC_MX)
# define TLS_CIPHERS PROSA_CIPHERS
#else
# define TLS_CIPHERS PREFERRED_CIPHERS
#endif


// Pull data from the response 3KB at a time
#define RESPONSE_BUFF_SEGMENT_SIZE (1024 * 3)

// Timeouts
#define CONNECT_TIMEOUT_MS			(30000)	// The time allowed for a connection to establish
#define CONNECT_RETRY_INTERVAL_MS	(25)	// The time to wait in between attempts to call BIO_do_connect
#define WRITE_TIMEOUT_MS			(15000)	// The maximum time a write will wait before failing
#define WRITE_RETRY_INTERVAL_MS		(25)	// The time to delay in between attempts to call BIO_write
#define READ_TIMEOUT_MS				(120000)// The maximum time to wait before failing to read from the session
#define READ_RETRY_INTERVAL_MS		(25)	// The time to delay in between attempts to call BIO_read

const int TLS_LINE_OK = 0;
const int TLS_LINE_NOK = 1;

#define CURL_ERROR_RETURN(func, errormsg, curlCode) DeInitConnection(); \
		LogCURLErrorMessageNVRam(func, curlCode); \
		NVDump('F', 'U', func, L"", errormsg); \
		return TLS_LINE_NOK

#define GENERIC_ERROR_RETURN(func, errormsg) DeInitConnection(); \
		NVDump('F', 'U', func, L"", errormsg); \
		return TLS_LINE_NOK

static int wait_on_socket(curl_socket_t sockfd, int for_recv, long timeout_ms)
{
	struct timeval tv;
	fd_set infd, outfd, errfd;
	int res;

	tv.tv_sec = timeout_ms / 1000;
	tv.tv_usec = (timeout_ms % 1000) * 1000;

	FD_ZERO(&infd);
	FD_ZERO(&outfd);
	FD_ZERO(&errfd);

	FD_SET(sockfd, &errfd); /* always check for error */ 

	if(for_recv) {
		FD_SET(sockfd, &infd);
	}
	else {
		FD_SET(sockfd, &outfd);
	}

	/* select() returns the number of signalled sockets or -1 */ 
	res = select((int)sockfd + 1, &infd, &outfd, &errfd, &tv);
	return res;
}

/**
 * Logs the error message if one exists
 */
static void LogCURLErrorMessage(CURLcode);
void LogCURLErrorMessage(CURLcode err)
{
	NHERROR((L"CURL error (%d): %S\r\n", err, curl_easy_strerror(err)));
}

/**
 * Logs the error message if one exists
 */
static void LogCURLErrorMessageNVRam(char*, CURLcode);
void LogCURLErrorMessageNVRam(char* func, CURLcode err)
{
	CString errStr, strErrCode;
	errStr.Format(L"%S", curl_easy_strerror(err));
	strErrCode.Format(L"TER:%d", err);

	NHERROR((_T("Fatal Error: %s\r\n"), errStr));
	NVDump('F', 'U', func, strErrCode, errStr);
}

CLineTLS::CLineTLS(void) : startCheckSocket(FALSE, FALSE), stoppedReadEvent(FALSE, FALSE)
{
	NHDEBUG(DBG_CALL, (L"CALL : [%s]\n", TEXT(__FUNCTION__)));

	this->activeSessions = 0;

	CLineCtrl::Register(LINE_TLS, this);

	Resume();
}

CLineTLS::~CLineTLS(void)
{
	this->DeInitConnection();
}

int CLineTLS::GetLineType()
{
	return LINE_TLS;
}

int	CLineTLS::LineOpen(LPCTSTR Dest_Info,	/*ip:[xxx.xxx.xxx.xxx]*/
				LPCTSTR dest_port,		/*port:[xxxxx]*/ 
				LPCTSTR sOption,		/*SSL mode: "1" - enable, "0" - disable*/	 
				LPCTSTR mode)			/*socket mode: "1" - ready for Rms, "0" - host connect mode*/
{
	NHDEBUG(DBG_CALL, (L"TLS Connecting with options: dest=%s, port=%s, options=%s, mode=%s\r\n", Dest_Info, dest_port, sOption, mode));

	int listenMode = _ttoi(mode);
	if (listenMode == 1)
	{
		NHERROR((L"Listen mode not support with TLS\r\n"));
		// Listen mode not supported in TLS mode
		return TLS_LINE_NOK;
	}

	if (this->IsSessionActive())
	{
		NHERROR((L"Attempt to open TLS session while another session exists\r\n"));
		GENERIC_ERROR_RETURN("12", L"NG1");
	}

	TLSOptions tlsOptions;
	if (!CLineTLS::ParseOptionsString(sOption, Dest_Info, dest_port, tlsOptions))
	{
		NHERROR((L"Failed to parse options string\r\n"));
		GENERIC_ERROR_RETURN("12", L"NG2");
	}

	curl = curl_easy_init();
	this->ConfigureConnection(curl, tlsOptions);

	if (this->OpenSession(tlsOptions) != TLS_LINE_OK)
	{
		GENERIC_ERROR_RETURN("12", L"NG6");
	}

	// Increment the session count, once a session has been established
	this->activeSessions++;

	// Notify the thread that it should start polling for data
	put_event(CONNECT_EVT);
	startCheckSocket.Set();

	return TLS_LINE_OK;
}

int CLineTLS::LineClose()
{
	this->DeInitConnection();

	return TLS_LINE_OK;
}

int	CLineTLS::LineSendData(BYTE *send_data, int len)
{
	if (!IsSessionActive())
	{
		// NOOP because there's nothing to deinit
		return TLS_LINE_OK;
	}

	COleDateTime timeoutDateTime;

	CURLcode res;
	curl_socket_t sockfd;
	res = curl_easy_getinfo(curl, CURLINFO_ACTIVESOCKET, &sockfd);

	timeoutDateTime = COleDateTime::GetCurrentTime() + COleDateTimeSpan(0, 0, 0, WRITE_TIMEOUT_MS / 1000);

	int total = 0;
	size_t sent = 0;
	do {
		if (COleDateTime::GetCurrentTime() > timeoutDateTime)
		{
			NHERROR((L"TLS write timeout\r\n"));
			GENERIC_ERROR_RETURN("13", L"NG0");
			break;
		}

		// Write to host
		res = curl_easy_send(curl, send_data + total, len - total, &sent);
		if (sent > 0)
		{
			total += sent;
			timeoutDateTime = COleDateTime::GetCurrentTime() + COleDateTimeSpan(0, 0, 0, WRITE_TIMEOUT_MS / 1000);
			NHDEBUG(DBG_CALL, (L"Wrote %dB to host\r\n", sent));
		}

		if (res == CURLE_AGAIN && !wait_on_socket(sockfd, 0, 60000L)) 
		{
			NHERROR((L"Did not write complete data before session closure\r\n"));
			CURL_ERROR_RETURN("13", L"NG2", res);
		}
		else if (res == CURLE_AGAIN)
		{
			// We got an error, but the library says we should try again
			Delay_Msg(WRITE_RETRY_INTERVAL_MS);
			continue;
		}
		else if (sent == 0) 
		{
			NHERROR((L"Unable to send any data! Stopping write.\r\n"));
			break;
		}
	} while (total < len);

	// If we don't end up writing everything, the session has closed prematurely
	if (total != len)
	{
		NHERROR((L"Did not write complete data before session closure\r\n"));
		CURL_ERROR_RETURN("13", L"NG1", res);
	}

	// After we write data to the session, there's a good chance
	// we the server will send something back.
	startCheckSocket.Set();

	return TLS_LINE_OK;
}

int CLineTLS::LineAccept(BYTE opt)
{
	NHERROR((L"LineAccept not implemented\r\n"));
	return TLS_LINE_NOK;
}

int CLineTLS::LineCallState(int nMode)
{
	if (nMode == 1)
	{
		//  Return true if the nMode == 1
		return (int) IsSessionActive();
	}

	// Return TRUE if the "line" is idle
	return (int) !IsSessionActive();
}

//
// Private methods
//

bool CLineTLS::IsSessionActive()
{
	return this->activeSessions > 0;
}

void CLineTLS::DeInitConnection()
{
	if (!IsSessionActive())
	{
		// NOOP because there's nothing to deinit
		return;
	}

	this->activeSessions = 0;

	NHDEBUG(DBG_INFO, (L"De-initializing TLS service\r\n"));

	// Await the read cycle completion semaphore
	if (stoppedReadEvent.Wait(CONNECT_TIMEOUT_MS) != WAIT_OBJECT_0)
	{
		NHDEBUG(DBG_CALL, (L"The attempt to synchronize the read thread has failed\r\n"));
		NVDump('F', 'U', "16", L"", L"NG0");
	}

	curl_easy_cleanup(curl);
}

int CLineTLS::OpenSession(TLSOptions tlsOptions)
{
	curl_easy_setopt(curl, CURLOPT_CONNECT_ONLY, 1L);

	CURLcode res = curl_easy_perform(curl);
	if (res != CURLE_OK)
	{
		LogCURLErrorMessageNVRam("17", res);
		return TLS_LINE_NOK;
	}

	return TLS_LINE_OK;
}

int CLineTLS::ConfigureConnection(CURL* curl, TLSOptions tlsOptions)
{
	CURLcode err = CURLE_OK;
	curl_easy_setopt(curl, CURLOPT_URL, tlsOptions.Server);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, tlsOptions.UseCertValidation ? 1L : 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, tlsOptions.UseCertValidation ? 2L : 0L);
	curl_easy_setopt(curl, CURLOPT_CAINFO, CA_ROOT_CERTS_FILE);
	if (tlsOptions.UseStrongCiphers)
	{
		curl_easy_setopt(curl, CURLOPT_SSL_CIPHER_LIST, TLS_CIPHERS);
	}
	else 
	{
		curl_easy_setopt(curl, CURLOPT_SSL_CIPHER_LIST, MEDIUM_CIPHERS);
	}
	
	curl_easy_setopt(curl, CURLOPT_SSLVERSION, tlsOptions.GetCurlTlsVersion());
#ifdef _DEBUG
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif

	if (!tlsOptions.PrivateKeyFileName.IsEmpty()) 
	{
		int len = tlsOptions.PrivateKeyFileName.GetLength();
		char *privateKey = new char[len + 1]();
		WideToMulti(privateKey, tlsOptions.PrivateKeyFileName, len);

		curl_easy_setopt(curl, CURLOPT_SSLKEYTYPE, "DER");
		err = curl_easy_setopt(curl, CURLOPT_SSLKEY, privateKey);
		if (err != CURLE_OK)
		{
			NHERROR((L"Could not set private key: %d\r\n", err));
		}

		memset(privateKey, 0, len);
		delete [] privateKey;
	}

	if (!tlsOptions.PublicKeyFileName.IsEmpty())
	{
		int len = tlsOptions.PublicKeyFileName.GetLength();
		char *publicKey = new char[len + 1]();
		WideToMulti(publicKey, tlsOptions.PublicKeyFileName, len);

		err = curl_easy_setopt(curl, CURLOPT_SSLCERT, publicKey);
		if (err != CURLE_OK)
		{
			NHERROR((L"Could not set public key: %d\r\n", err));
		}

		memset(publicKey, 0, len);
		delete [] publicKey;
	}

	return TLS_LINE_OK;
}

bool CLineTLS::ParseOptionsString(CString options, CString hostname, CString port, TLSOptions &tlsOptions) 
{
	if (options.GetLength() == 0)
	{
		// Nothing specified == no TLS
		tlsOptions.UseTLS = false;
	}
	else
	{ 
		// TLS Options
		CString versionOptions;
		CString extraCertsString;

		int firstSeparatorLocation = options.Find(L":");
		if (firstSeparatorLocation > 0)
		{
			// Two components in string
			versionOptions = options.Left(firstSeparatorLocation);
			extraCertsString = options.Mid(firstSeparatorLocation + 1);
		}
		else
		{
			// Only one component
			versionOptions = options;
		}

		tlsOptions.TLSVersionOptions = _ttoi(versionOptions);
		tlsOptions.UseCertValidation = tlsOptions.TLSVersionOptions >= CONN_UPTO_TLS_V12_USE_CERT;

		// Check for a valid string format (i.e. min of 4 chars)
		if (extraCertsString.GetLength() > 4)
		{
			CStringArray certArray;
			// First component is pubkey filename, second component is privkey filename
			SplitString(extraCertsString, FIELD_DELIMITER, certArray);

			tlsOptions.PublicKeyFileName = certArray.GetAt(0);

			if (certArray.GetSize() > 1)
			{
				tlsOptions.PrivateKeyFileName = certArray.GetAt(1);
			}
		}
		else 
		{
			// HACK WW::PAI - If the extraCertsString isn't a file, it can downgrade the ciphers
			// Valid options are 0 - strong ciphers, 1 - medium ciphers
			int cipherOption = Asc2Int(extraCertsString);
			if (cipherOption == 1) 
			{
				tlsOptions.UseStrongCiphers = false;
			}
		}
	}

	// Disable TLS if version string is "0"
	tlsOptions.UseTLS = tlsOptions.TLSVersionOptions != 0;

#ifndef NO_SECURE_TLS_MODE
	tlsOptions.UseCertValidation = true;
	tlsOptions.TLSVersionOptions = CONN_UPTO_TLS_V12_USE_CERT;
#endif
	// Set host and port information
	int hostnameLen = hostname.GetLength() + 1;
	WideToMulti(tlsOptions.Hostname, hostname, min(511, hostnameLen));

	int portLen = port.GetLength() + 1;
	WideToMulti(tlsOptions.Port, port, min(5, portLen));

	// Set the combined server field
	sprintf_s(tlsOptions.Server, sizeof(tlsOptions.Server), "%s://%s:%s", tlsOptions.GetTcpProtocol(), tlsOptions.Hostname, tlsOptions.Port);

	return true;
}

// This function continually waits for input from the input buffer
unsigned CLineTLS::ThreadHandlerProc()
{
	NHDEBUG(DBG_CALL, (L"Starting TLS response thread\r\n"));
	COleDateTime timeoutDateTime;

	// Main Loop
	while (true)
	{
		// Notify the other threads that the read thread is in a waiting state.
		stoppedReadEvent.Set();

		NHDEBUG(DBG_CALL, (L"Waiting for a check socket event\r\n"));
		if (this->startCheckSocket.Wait(INFINITE) != WAIT_OBJECT_0)
		{
			// The event timed out before being triggered.
			// Continue polling
			continue;
		}

		// Lock the semaphore while the thread is working
		stoppedReadEvent.Reset();

		// Time to start getting responses
		NHDEBUG(DBG_CALL, (L"Waiting for TLS response data\r\n"));
		timeoutDateTime = COleDateTime::GetCurrentTime() + COleDateTimeSpan(0, 0, 0, READ_TIMEOUT_MS / 1000);

		// Create a memory response buffer
		char *buffer = new char[RESPONSE_BUFF_SEGMENT_SIZE]();

		CURLcode res = CURLE_OK;
		curl_socket_t sockfd = 0;

		size_t len = 0;
		do
		{
			// Check for application shutdown request
			if (!IsSessionActive())
			{
				NHDEBUG(DBG_CALL, (L"TLS session requested shutdown. Stopping reads\r\n"));
				len = 0;
				break;
			}

			// Grab the socket handle if we don't already have it
			if (sockfd == 0)
			{
				res = curl_easy_getinfo(curl, CURLINFO_ACTIVESOCKET, &sockfd);
			}

			// Test timeout
			if (COleDateTime::GetCurrentTime() > timeoutDateTime)
			{
				// Read timeout has occurred
				NHDEBUG(DBG_CALL, (L"Timeout while waiting for data\r\n")); 
				NVDump('F', 'U', "19", L"TIME", L"");
				put_event(TIMEOUT_EVT);
				break;
			}

			res = curl_easy_recv(curl, buffer, RESPONSE_BUFF_SEGMENT_SIZE, &len);

			if (res == CURLE_AGAIN && !wait_on_socket(sockfd, 1, 250L)) 
			{
				NHDEBUG(DBG_CALL, (L"CURL needs another go-round. Socket not ready\r\n"));
				continue;
			}
			else if (res == CURLE_AGAIN)
			{
				NHDEBUG(DBG_CALL, (L"CURL needs another go-round\r\n"));
				Delay_Msg(100);
				continue;
			}
			else if (len > 0)
			{
				NHDEBUG(DBG_CALL, (L"Read %d bytes from the call\r\n", len));

				// Write to the output queue, then clear it.
				put_data(buffer, len);
				memset(buffer, 0, RESPONSE_BUFF_SEGMENT_SIZE);

				// We got data, so reset the timer
				timeoutDateTime = COleDateTime::GetCurrentTime() + COleDateTimeSpan(0, 0, 0, READ_TIMEOUT_MS / 1000);
				continue;
			}
			else if (len == 0)
			{	
				DWORD wsaError = WSAGetLastError();
				if (wsaError == WSAEWOULDBLOCK) 
				{
					LogCURLErrorMessageNVRam("19", res);
					NHWARN((L"Received error WSAEWOULDBLOCK. Trying again\r\n"));
					continue;
				}

				if (res == CURLE_RECV_ERROR) 
				{
					LogCURLErrorMessageNVRam("19", res);
					NHWARN((L"Received error CURLE_RECV_ERROR. Stopping\r\n"));
				}

				NHERROR((L"Read %d bytes from the call: CURLE_Code: %d\r\n", len, res));
				break;
			}
		} while (true);

		// Clear and delete response buffer
		memset(buffer, 0, RESPONSE_BUFF_SEGMENT_SIZE);
		delete [] buffer;
		
		// mem BIOs are not blocking, so -1 may not indicate an error
		if (len == 0)
		{
			NHDBG((L"TLS no more data/connection closed normally\r\n"));
		}
		else if (res != CURLE_OK) // indicates a -1/-2.
		{
			// Session was closed after all data was read, continue and set the response buffer.
			LogCURLErrorMessageNVRam("19", res);
			NHERROR((L"TLS SOCKET_ERROR\r\n"));
		}

		put_event(DISCONNECT_EVT);

		// Try to clear this semaphore
		startCheckSocket.Reset();
	} // Main Loop

	return 0;
}