/**********************************************************
* Copyright(C) 2022 CAN-Lab Inc.
* Garnet Kim <garnet@can-lab.co.kr> - 2022.10.04
* FileName : CLApp_Condition.h
* Auther   : Garnet Kim
* Modify   : 2022-10-04
**********************************************************/
#ifndef _CLAPP_CONDITION_H_
#define _CLAPP_CONDITION_H_

#include <pthread.h>

class CLAppMutex;

class CLAppCondition
{
	friend class CLAppMutex;

public:
	CLAppCondition();
	virtual ~CLAppCondition();

	void wait(CLAppMutex* pLock);
	int timedWait(CLAppMutex* pLock, unsigned int usecs);
	void signal();
	void broadcast();

private:
	pthread_cond_t mCond;
};

#endif	// _CLAPP_CONDITION_H_

