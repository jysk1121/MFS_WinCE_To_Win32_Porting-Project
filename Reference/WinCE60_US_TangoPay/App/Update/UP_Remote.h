#pragma once
#include "filemgr.h"

/**
 * Loads the update version from the remote repository
 */
class CUP_Remote :
	public CFileMgr
{
public:
	CUP_Remote(void);
	~CUP_Remote(void);

public:
	/**
	 * Load the update from the remote repository.
	 * @param[in,out] StateInfo the state information shared among the update components
	 * @returns a string representing the success of the process
	 */
	CString Process(CUpdateStateInfo &StateInfo);

private:
	bool nvramOk;
};

/* Global instance */
CUP_Remote	g_CUP_Remote;