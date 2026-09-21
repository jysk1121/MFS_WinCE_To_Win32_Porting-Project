#pragma once
#if (APP_DIGITALMINT)

//
// DigitalMint Flows
//

public:
	FLOW_ID F_NH_DigitalMintStartFlow();
	FLOW_ID F_NH_DigitalMintGreetFlow();
	FLOW_ID F_NH_DigitalMintTransactionFlowAtm();
	FLOW_ID F_NH_DigitalMintTransactionFlowSidecar();
	FLOW_ID F_NH_DigitalMintDebitFlow();
	FLOW_ID F_NH_DigitalMintCashAcceptFlow();
	FLOW_ID F_NH_DigitalMintEndTransactionFlow();
	FLOW_ID F_NH_DigitalMintCancelTransactionFlow();
	FLOW_ID F_NH_DigitalMintCleanupFlow();
	FLOW_ID F_NH_DigitalMintReportDeviceStatusFlow(bool forceUpdate);
	FLOW_ID F_NH_DigitalMintReportDeviceEvent(DMKioskEventCode eventCode);

#endif