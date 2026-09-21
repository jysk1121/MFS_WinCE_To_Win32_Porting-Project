#ifndef __TRAN_BIZ_PROC_CA_H__
#define __TRAN_BIZ_PROC_CA_H__

private:
	//////////////////////////////////////
	// HELPER FUNCTION

public:

	//////////////////////////////////////
	// WORKING WITH HOST
public:

	//////////////////////////////////////
	// BUSINESS PROCESS
public:
	BOOL	BIZ_CA_WelcomeProc(BOOL bForceShowing = FALSE);

protected:
	// CA Select Transaction

	BIZ_RETURN	P_CA_NOR_SelectTransaction();	// Select Transaction CA Only

#endif 