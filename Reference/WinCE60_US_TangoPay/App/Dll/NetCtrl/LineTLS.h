#pragma once
#include ".\Common\NHCtrlThread.h"
#include ".\Net\LineCtrl.h"
#include ".\Dev\DevDefine.h"
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <curl/curl.h>

#ifdef UNDER_CE
#  pragma comment (lib, "ws2.lib")
#else
#  pragma comment (lib, "ws2_32.lib")
#endif

#pragma warning(push)
#pragma warning(disable:4351)	// suppress unimportant warning below
// warning C4351: new behavior: elements of array '_tls_options():Hostname / Port / Server' will be default initialized

typedef struct _tls_options {
	char	Hostname[512];
	char    Port[6];
	char    Server[518]; // Host : Port

	bool UseTLS;
	bool UseStrongCiphers;

	int		TLSVersionOptions;
	bool	UseCertValidation;
	CString	PublicKeyFileName;
	CString PrivateKeyFileName;


	_tls_options():Hostname(),Port(),Server()
	{
		UseTLS = false;
		UseStrongCiphers = true;
		UseCertValidation = false;
		TLSVersionOptions = 0;
	};

	bool operator ==(const _tls_options& a) const
	{
		return (
			   strcmp(Hostname, a.Hostname) == 0
			&& strcmp(Port, a.Port) == 0
			&& strcmp(Server, a.Server) == 0
			&& TLSVersionOptions == a.TLSVersionOptions
			&& UseCertValidation == a.UseCertValidation
			&& PublicKeyFileName == a.PublicKeyFileName
			&& PrivateKeyFileName == a.PrivateKeyFileName
		);
	}

	const char * GetTcpProtocol() const 
	{
		if (UseTLS)
		{
			return "https";
		}

		return "http";
	}

	/**
	 * Determines the method of communications which should be used
	 * with the TLS server.
	 */
	const long GetCurlTlsVersion()
	{
#ifdef NO_SECURE_TLS_MODE
		// NB: Please note the meanings and sentiment behind each of the options below. Essentially, we are drawing a "line in the sand"
		// and stating that after TLS 1.2, we will always negotiate the latest protocol version offered.
		// SSLv3: Only SSLv3 will be used
		// TLSv1.0: TLSv1.0 is the latest version of TLS offered.
		// TLSv1.1: TLSv1.1 is the latest version of TLS offered.
		// TLSv1.2: TLSv1.2 is the oldest version of TLS offered. Implies negotiation up to 1.3. Older protocols will fail
		// TLSv1.3: TLSv1.3 is the oldest version of TLS offered. Older protocols will fail
		switch (TLSVersionOptions)
		{
		case CONN_UPTO_SSL_V30:
		case CONN_UPTO_SSL_V30_USE_CERT:
		case CONN_UPTO_SSL_V30_LINE_TEST:
		case CONN_UPTO_SSL_V30_USE_CERT_LINE_TEST:
			return CURL_SSLVERSION_SSLv3;
		case CONN_UPTO_TLS_V10:
		case CONN_UPTO_TLS_V10_USE_CERT:
		case CONN_UPTO_TLS_V10_LINE_TEST:
		case CONN_UPTO_TLS_V10_USE_CERT_LINE_TEST:
			return CURL_SSLVERSION_MAX_TLSv1_0;
		case CONN_UPTO_TLS_V11:
		case CONN_UPTO_TLS_V11_USE_CERT:
		case CONN_UPTO_TLS_V11_LINE_TEST:
		case CONN_UPTO_TLS_V11_USE_CERT_LINE_TEST:
			return CURL_SSLVERSION_MAX_TLSv1_1;

		case CONN_UPTO_TLS_V12:
		case CONN_UPTO_TLS_V12_USE_CERT:
		case CONN_UPTO_TLS_V12_LINE_TEST:
		case CONN_UPTO_TLS_V12_USE_CERT_LINE_TEST:
			return CURL_SSLVERSION_TLSv1_2;
		case CONN_UPTO_TLS_V13:
		case CONN_UPTO_TLS_V13_USE_CERT:
		case CONN_UPTO_TLS_V13_LINE_TEST:
		case CONN_UPTO_TLS_V13_USE_CERT_LINE_TEST:
		default:
			return CURL_SSLVERSION_TLSv1_3;
		};
#else
		return CURL_SSLVERSION_TLSv1_2; // This will use a minimum of TLS 1.2, and offer upgrades to 1.3
#endif
	}

} TLSOptions, *LPTLSOptions;

#pragma warning(pop)


class CLineTLS :
	public CLineCtrl, public CNHCtrlThread
{
public:
	CLineTLS(void);
	~CLineTLS(void);

	/* LINECTRL INTERFACE */
public:
	virtual int	LineOpen(LPCTSTR Dest_Info,			/*ip:[xxx.xxx.xxx.xxx]*/
						 LPCTSTR dest_port=L"",		/*port:[xxxxx]*/ 
						 LPCTSTR sOption=L"",		/*SSL mode: "1" - enable, "0" - disable*/	 
						 LPCTSTR mode=L"");			/*socket mode: "1" - ready for Rms, "0" - host connect mode*/
	virtual int	LineClose();
	virtual int	LineSendData(BYTE *send_data, int len);
	virtual int LineAccept(BYTE opt);				/*accept option: '0x01' - call accept, '0x00' - call deny*/
	virtual int LineCallState(int nMode=0);
	virtual int GetLineType();						// [#RWC6-67] US William 2019.10.22 MoniView TLS

protected:
	// Response handler
	unsigned ThreadHandlerProc(void);

private:
	int activeSessions;
	CURL* curl;

	// Event used to synchronize the response handler thread and the output buffer
	CNHEvent startCheckSocket;

	// Event used to synchronize the shutdown of the TLS session.
	// NB: For some reason, if a call to BIO_read() is made after the underlying
	// BIO is reset with BIO_reset(), a TCP connection will be created. We obivously
	// don't want this, so we need to ensure that the reading thread has stopped reading
	// before we reset the BIO.
	CNHEvent stoppedReadEvent;

	/**
	 * Returns whether or not the "line" is available for a new connection
	 */
	bool IsSessionActive();

	/**
	 * Destroys the TLS/OpenSSL components and frees the connection
	 */
	void DeInitConnection();

	/**
	 * Opens the connection to the host
	 */
	int OpenSession(TLSOptions tlsOptions);

	/**
	 * Configures the connection parameters
	 */
	int ConfigureConnection(CURL* curl, TLSOptions tlsOptions);

	/**
	 * Parses the options string provided by the interface
	 * @params options[in] the options string
	 * @params hostname[in] the hostname of the server
	 * @params port[in] the port number of the host
	 * @params tlsOptions[out] The TLS version options
	 * @returns true if the operation was successful and the tlsOptions value can be used 
	 */
	static bool ParseOptionsString(CString options, CString hostname, CString port, TLSOptions &tlsOptions);
};


/**
 * GLOBAL INSTANCE
 * A global instance registers the instance with the other lines
 */
CLineTLS g_tlsLine;