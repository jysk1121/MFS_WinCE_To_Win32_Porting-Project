#pragma once

class NetworkHelper
{
public:
	NetworkHelper(void);
	~NetworkHelper(void);

	static void ConfigureForStaticIp();
	static void ConfigureForDhcp();

	/**
	 * Waits for the the system to acquire an IP address or is able to ping the provided host.
	 * @param timeout the number of milliseconds to wait before giving up
	 * @param resolveHost the hostname to resolve with DNS to detect a network connection
	 * @returns false when the operation fails or times out
	 */
	static bool WaitForIpAddressOrNetworkConnection(int timeout, CString resolveHost);

	/**
	 * Configures the ATM's networking information to the settings defined
	 * by the OP
	 */
	static void ConfigureNetworkInformation();
};
