#pragma once
// [#RWC6-47] US William 2019.08.27 B4U TXN

#include <windows.h>
#include <stdio.h>

private:

	// The B4U Service
	CB4UService*		m_B4U;

	// Polling Threads
	HANDLE					m_hB4UPollingThread;
	BOOL					m_isB4UPolling;

	/**
	 * When true, the polling thread will attempt to update the QR code displayed on the
	 * screen, in addition to session information.
	 */
	BOOL					m_shouldB4UDisplayQRUpdates;

	/**
	 * The current B4U Session
	 */
	LPB4USession		m_B4USession;
	
	/**
	 * The B4U Configuration INI file read
	 */
	LPB4UIni			m_B4UIni;

	/**
	 * The B4U Journal read
	 */
	LPB4UJournal		m_B4UJournal;

	/**
	 * The status of the current B4U session
	 */
	LPB4USessionStatus m_B4USessionStatus;

	/**
	 * Creates a new B4U session and saves the state to the ivars. If the current session is
	 * valid, this method will be a NOOP
	 */
	BOOL CreateNewB4USession(bool& created, B4USessionType sessionType);

	/**
	 * Updates the B4U session status
	 */
	BOOL UpdateB4USessionStatus(B4USessionType sessionType);

	/**
	 * Starts a polling timer for the active B4U session
	 */
	void StartB4UPollingTimer();

	/**
	 * Stops the B4U polling timer
	 */
	void StopB4UPollingTimer();

	/**
	 * Destroys the current B4U session information. This should be called after a TXN
	 */
	void ResetB4UStateData();

	/**
	 * The timer tick function
	 */
	friend DWORD B4UPollingThread(void *params);

	/**
	 * True if B4U is enabled
	 * @returns bool true if B4U is enabled
	 */
	BOOL IsB4UEnabled();

	/**
	 * Displays the QR code for the current B4U session. If B4U is disabled, a NOOP
	 * @param[out] filename the file path of the saved QR code
	 * @returns the success of the operation
	 */
	BOOL DisplayB4UQrCodeIfEnabled(CString& filename);

	/**
	 * B4U txn failure screen
	 */
	FLOW_ID F_NH_B4U_RequestInfo();

	/**
	 * B4U txn start. Will show a QR code that should be scanned by the user's mobile device
	 * @returns FLOW_ID the flow ID of the next screen flow, i.e. success or fail
	 */
	FLOW_ID F_NH_B4U_QR();

	/**
	 * B4U txn failure screen
	 */
	FLOW_ID F_NH_B4U_Failed();

	/**
	 * B4U txn cancel screen
	 */
	FLOW_ID F_NH_B4U_Cancelled();

	/**
	 * B4U txn timeout screen
	 */
	FLOW_ID F_NH_B4U_Timeout();

	/**
	 * B4U ATM Authrization screen
	 */
	FLOW_ID F_NH_B4U_Authorization();

	/**
	 * B4U txn success screen + dispense + reversal + receipt
	 */
	FLOW_ID F_NH_B4U_Success();

	/**
	 * B4U txn finalizing screen + display receipt + disclaimers
	 */
	FLOW_ID F_NH_B4U_Finalize();

	/**
	 * B4U Generating transaction data for display/paper receipt
	 */
	void F_NH_B4U_GenerateTxnData(CString& strPrintData, int Type = 0);
