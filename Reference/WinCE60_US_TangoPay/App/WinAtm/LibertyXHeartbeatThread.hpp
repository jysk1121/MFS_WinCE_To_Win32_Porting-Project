#pragma once

#include "Common/NHDbgApi.h"
#include "Dev\Devcmn.h"
#include "Common\LibertyXConfigurationManager.h"
#include "Tran\LibertyXService.h"

// Default sleep times for reconfiguration
#ifdef _DEBUG
#  define LOOP_SLEEP_TIME_MILIS_LTX (1000 * 60)		 // 1 minute
#else
#  define LOOP_SLEEP_TIME_MILIS_LTX (1000 * 60 * 60) // 60 minutes
#endif

/**
 * Class which handles the coordination of the LibertyX heartbeat messages
 */
class CLibertyXHeartbeatRunner
{
private:
	CDevCmn* devCmn;

	void PerformBasicHeatbeat(CLibertyXService *service)
	{
		LXPingResponse pong;

		NHINFO((L"Performing LTX basic heartbeat...\r\n"));
		service->Ping(pong);
	}

	void PerformDispenseHeatbeat(CLibertyXService *service)
	{
		LXPingResponse pong;
		LXPing ping;

		CLibertyXService::PopulateDispenseInformation(ping, devCmn);

		NHINFO((L"Performing LTX basic heartbeat...\r\n"));
		service->PingDispense(ping, pong);
	}

public:
	enum 
	{
		INVALID_ARGS,
		OK
	};

	CLibertyXHeartbeatRunner(CDevCmn* devCmn)
	{
		this->devCmn = devCmn;
	}

	~CLibertyXHeartbeatRunner()
	{

	}

	DWORD Run()
	{
		NHDBG((L"Starting LibertyX heartbeat thread\r\n"));

		LXConfiguration config;
		CLibertyXConfigurationManager manager;
		CLibertyXService *service = NULL;

		// Main thread loop
		while (true)
		{	
			if (!manager.GetConfiguration(config))
			{
				NHWARN((L"Configuration for LibertyX has failed. Skipping heartbeat.\r\n"));
				Delay_Msg(LOOP_SLEEP_TIME_MILIS_LTX);
				continue;
			}

			// Pause for the required interval
			if (config.HeartbeatIntervalMinutes == 0)  // Invalid interval
			{
				Delay_Msg(LOOP_SLEEP_TIME_MILIS_LTX);
			}
			else 
			{
				Delay_Msg(config.HeartbeatIntervalMinutes * 1000 * 60);
			}

			if (!config.SellBitcoinEnabled && !config.BuyBitcoinEnabled) 
			{
				// No LTX enabled
				continue;
			}

			if (devCmn->AtmStatus != ATM_CUSTOM)
			{
				// Do do a healthcheck if the terminal is offline
				continue;
			}

			// Alloc new service
			service = new CLibertyXService(config);

			if (config.SellBitcoinEnabled)
			{
				PerformDispenseHeatbeat(service);
			}
			else if (config.BuyBitcoinEnabled)
			{
				PerformBasicHeatbeat(service);
			}

			// Delete the service
			delete service;
			service = NULL;
		}

		return CLibertyXHeartbeatRunner::OK;
	};
};

/**
 * This thread runs forever, and continually polls the date/time to determine
 * if it should initiate a remote update.
 * @param[in] args a pointer to the CDevCmn object
 */
DWORD LibertyXHeartbeatThread(LPVOID args);
DWORD LibertyXHeartbeatThread(LPVOID args)
{
	if (args == NULL)
	{
		return CLibertyXHeartbeatRunner::INVALID_ARGS;
	}

	CDevCmn* devCmn = static_cast<CDevCmn*>(args);

	CLibertyXHeartbeatRunner scheduler(devCmn);

	return scheduler.Run();
}