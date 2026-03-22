#include <App_Condition.h>
#include <App_Mutex.h>

AppMutex::AppMutex()
{
	int result;
	pthread_mutexattr_t mutexAttr;

	result = pthread_mutexattr_init(&mutexAttr);

#ifndef NDEBUG
	result = pthread_mutexattr_settype(&mutexAttr, PTHREAD_MUTEX_ERRORCHECK_NP);
#else
	result = pthread_mutexattr_settype(&mutexAttr, PTHREAD_MUTEX_ADAPTIVE_NP);
#endif

	result = pthread_mutex_init(&mLock, &mutexAttr);

	result = pthread_mutexattr_destroy(&mutexAttr);

	if(result == 0){

	}	
}

AppMutex::~AppMutex()
{
	pthread_mutex_destroy(&mLock);
}


int AppMutex::lock() 
{
	return pthread_mutex_lock(&mLock);
}

int AppMutex::tryLock() 
{
	return pthread_mutex_trylock(&mLock);
}

int AppMutex::unlock() 
{
	return pthread_mutex_unlock(&mLock);
}


AppAutoLock::AppAutoLock(AppMutex* pLock, AppCondition* pCond) :
mpLock(pLock),
mpCond(pCond)
{
	if (mpCond)
	{
		if (mpLock->tryLock()!=0)
		{
			mpCond->wait(mpLock);
		}
	}
	else
	{
		mpLock->lock();
	}
}


AppAutoLock::~AppAutoLock()
{
	if (mpCond)
	{
		mpCond->signal();
	}
	mpLock->unlock();
}


AppAutoUnlock::AppAutoUnlock(AppMutex* pLock, AppCondition* pCond) :
mpLock(pLock),
mpCond(pCond)
{
	if (mpCond)
	{
		mpCond->signal();
	}
	mpLock->unlock();
}


AppAutoUnlock::~AppAutoUnlock()
{
	if (mpCond)
	{
		if (mpLock->tryLock()!=0)
		{
			mpCond->wait(mpLock);
		}
	}
	else
	{
		mpLock->lock();
	}
}
