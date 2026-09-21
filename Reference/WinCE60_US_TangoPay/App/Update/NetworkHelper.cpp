#include "stdafx.h"
#include "NetworkHelper.h"
#include <winsock2.h>
#include <iphlpapi.h>
#include <icmpapi.h>
#include ".\Common\NHDbgApi.h"

#pragma comment(lib, "iphlpapi.lib")

#define IP_ACQUIRE_POLL_RATE_MS	(1 * 1000)

int ping_host(const char *hostname);
int ping_host(const char *hostname)
{
	int ret = 1;
	HANDLE hIcmpFile = INVALID_HANDLE_VALUE;
	IPAddr ipaddr = INADDR_NONE;
	DWORD dwRetVal = 0;
	int replySize = sizeof(ICMP_ECHO_REPLY);
	PICMP_ECHO_REPLY reply = (PICMP_ECHO_REPLY) new BYTE[replySize]();

	IP_OPTION_INFORMATION ipInfo = {};
	ipInfo.Ttl   = 128;
	ipInfo.Tos   = 0;
	ipInfo.Flags = 0;
	ipInfo.OptionsSize = 0 ;
	ipInfo.OptionsData = NULL ;

	ipaddr = inet_addr(hostname);
	if (ipaddr == INADDR_NONE)
	{
		goto cleanup;
	}

	hIcmpFile = IcmpCreateFile();
	if (hIcmpFile == INVALID_HANDLE_VALUE)
	{
		goto cleanup;
	}    

	NHDBG((L"Sending 32B ping\r\n"));
	dwRetVal = IcmpSendEcho(hIcmpFile, ipaddr, NULL, 0, &ipInfo, reply, replySize, IP_ACQUIRE_POLL_RATE_MS);
	NHDBG((L"Echo result: %d, error: %d\r\n", dwRetVal, GetLastError()));

	if (dwRetVal != 0)
	{
		// Success!
		NHDBG((L"Reply Status: %d\r\n", reply->Status));
		if (reply->Status == IP_SUCCESS)
		{
			ret = 0;
		}
	}

	if (reply->Status != IP_REQ_TIMED_OUT)
	{
		Delay_Msg(IP_ACQUIRE_POLL_RATE_MS);
	}

cleanup:
	if (hIcmpFile == INVALID_HANDLE_VALUE)
	{
		IcmpCloseHandle(hIcmpFile);
	}

	if (reply)
	{
		memset(reply, 0x00, replySize);
		delete [] reply;
	}

	return ret;
}

int resolve_host(const char* hostname);
int resolve_host(const char* hostname)
{
	struct hostent* host = gethostbyname(hostname);
	
	if (host == NULL)
	{
		DWORD err = GetLastError();
		NHERROR((L"Hostname resolution for %S failed with error code: %d\r\n", hostname, err));

		return 1;
	}
	
	return 0;
}


NetworkHelper::NetworkHelper(void)
{
}

NetworkHelper::~NetworkHelper(void)
{
}


void NetworkHelper::ConfigureForStaticIp()
{
	AtmSetDHCP(0);
	AtmSetIpAddress((LPCTSTR)MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_IPADDR));
	AtmSetSubnetMask((LPCTSTR)MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_SUBNET));
	AtmSetGateWay((LPCTSTR)MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_GATEWAY));
	AtmSetDNS((LPCTSTR)MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_DNS), (LPCTSTR)MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ATM_DNS_2ND));
#ifndef _DEBUG // Resetting the LAN with a debugger breaks Visual Studio
	ReloadLanCard();
#endif
}

void NetworkHelper::ConfigureForDhcp()
{
	AtmSetDHCP(1);
	ReloadLanCard();
}

void NetworkHelper::ConfigureNetworkInformation()
{
	AtmSetIpAddress((LPCTSTR)MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_IPADDR));
	AtmSetSubnetMask((LPCTSTR)MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_SUBNET));
	AtmSetGateWay((LPCTSTR)MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_GATEWAY));
	AtmSetDNS((LPCTSTR)MemGetStr(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_DNS), (LPCTSTR)MemGetStr(_MEM_FLD_OPTIONALSETTING_2, _MEM_VAR_OPT2_ATM_DNS_2ND));
	AtmSetDHCP(MemGetInt(_MEM_FLD_TCPIPINFO, _MEM_VAR_TCPIP_ATM_DHCP));

	ReloadLanCard();
}


bool NetworkHelper::WaitForIpAddressOrNetworkConnection(int timeout, CString resolveHost)
{
	CString ipAddress;
	int rtt = 0;
	bool result = false;
	COleDateTime timeoutDateTime = COleDateTime::GetCurrentTime() + COleDateTimeSpan(0, 0, 0, timeout / 1000);
	int hostLen = resolveHost.GetLength();
	char *host = new char[hostLen + 1]();

	WideToMulti(host, resolveHost, hostLen);

	while (true)
	{
		if (COleDateTime::GetCurrentTime() > timeoutDateTime)
		{
			NHERROR((L"IP acquisition timed out\r\n"));
			goto cleanup;
		}

		AtmGetDhcpIpAddress(ipAddress);

		if (resolve_host(host) == 0)
		{
			NHINFO((L"Resolution successful\r\n"));
			result = true;
			goto cleanup;
		}

		Delay_Msg(IP_ACQUIRE_POLL_RATE_MS);
	}

cleanup:

	delete [] host;

	return result;
}
