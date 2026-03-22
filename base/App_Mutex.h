#ifndef _APP_MUTEX_H_
#define _APP_MUTEX_H_

#include <pthread.h>

class AppCondition;

class AppMutex
{
	friend class AppCondition;

public:
	AppMutex();
	virtual ~AppMutex();

	int lock();
	int tryLock();
	int unlock();

private:
	pthread_mutex_t mLock;
};


class AppAutoLock
{
public:
	AppAutoLock(AppMutex* pLock, AppCondition* pCond=NULL);
	~AppAutoLock();

private:
	AppMutex* mpLock;
	AppCondition* mpCond;
};

class AppAutoUnlock
{
public:
	AppAutoUnlock(AppMutex* pLock, AppCondition* pCond=NULL);
	~AppAutoUnlock();

private:
	AppMutex* mpLock;
	AppCondition* mpCond;
};

#define CL_THREAD_SAFE_BLOCK(pLock) AppAutoLock __block((pLock))
#define CL_THREAD_UNSAFE_BLOCK(pLock) AppAutoUnlock __block((pLock))
#define CL_THREAD_SAFE_BLOCK_WITH_COND(pLock, pCond) AppAutoLock __block((pLock), (pCond))
#define CL_THREAD_UNSAFE_BLOCK_WITH_COND(pLock, pCond) AppAutoUnlock __block((pLock), (pCond))

#endif	//_APP_MUTEX_H_