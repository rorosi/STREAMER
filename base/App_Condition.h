#ifndef _APP_CONDITION_H_
#define _APP_CONDITION_H_

#include <pthread.h>

class AppMutex;

class AppCondition
{
	friend class AppMutex;

public:
	AppCondition();
	virtual ~AppCondition();

	void wait(AppMutex* pLock);
	int timedWait(AppMutex* pLock, unsigned int usecs);
	void signal();
	void broadcast();

private:
	pthread_cond_t mCond;
};

#endif	// _APP_CONDITION_H_

