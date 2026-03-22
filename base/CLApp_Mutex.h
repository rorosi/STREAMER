/**********************************************************
* Copyright(C) 2022 CAN-Lab Inc.
* Garnet Kim <garnet@can-lab.co.kr> - 2022.10.04
* FileName : CLApp_Mutex.h
* Auther   : Garnet Kim
* Modify   : 2022-10-04
**********************************************************/
#ifndef _CLAPP_MUTEX_H_
#define _CLAPP_MUTEX_H_

#include <pthread.h>

class CLAppCondition;

class CLAppMutex
{
	friend class CLAppCondition;

public:
	CLAppMutex();
	virtual ~CLAppMutex();

	int lock();
	int tryLock();
	int unlock();

private:
	pthread_mutex_t mLock;
};


class CLAppAutoLock
{
public:
	CLAppAutoLock(CLAppMutex* pLock, CLAppCondition* pCond=NULL);
	~CLAppAutoLock();

private:
	CLAppMutex* mpLock;
	CLAppCondition* mpCond;
};

class CLAppAutoUnlock
{
public:
	CLAppAutoUnlock(CLAppMutex* pLock, CLAppCondition* pCond=NULL);
	~CLAppAutoUnlock();

private:
	CLAppMutex* mpLock;
	CLAppCondition* mpCond;
};

#define CL_THREAD_SAFE_BLOCK(pLock) CLAppAutoLock __block((pLock))
#define CL_THREAD_UNSAFE_BLOCK(pLock) CLAppAutoUnlock __block((pLock))
#define CL_THREAD_SAFE_BLOCK_WITH_COND(pLock, pCond) CLAppAutoLock __block((pLock), (pCond))
#define CL_THREAD_UNSAFE_BLOCK_WITH_COND(pLock, pCond) CLAppAutoUnlock __block((pLock), (pCond))

#endif	//_CLAPP_MUTEX_H_