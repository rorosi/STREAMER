/**********************************************************
* Copyright(C) 2022 CAN-Lab Inc.
* Garnet Kim <garnet@can-lab.co.kr> - 2022.10.04
* FileName : CLApp_Mutex.cpp
* Auther   : Garnet Kim
* Modify   : 2022-10-04
**********************************************************/
#include <CLApp_Condition.h>
#include <CLApp_Mutex.h>

CLAppMutex::CLAppMutex()
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

CLAppMutex::~CLAppMutex()
{
	pthread_mutex_destroy(&mLock);
}


int CLAppMutex::lock() 
{
	return pthread_mutex_lock(&mLock);
}

int CLAppMutex::tryLock() 
{
	return pthread_mutex_trylock(&mLock);
}

int CLAppMutex::unlock() 
{
	return pthread_mutex_unlock(&mLock);
}


CLAppAutoLock::CLAppAutoLock(CLAppMutex* pLock, CLAppCondition* pCond) :
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


CLAppAutoLock::~CLAppAutoLock()
{
	if (mpCond)
	{
		mpCond->signal();
	}
	mpLock->unlock();
}


CLAppAutoUnlock::CLAppAutoUnlock(CLAppMutex* pLock, CLAppCondition* pCond) :
mpLock(pLock),
mpCond(pCond)
{
	if (mpCond)
	{
		mpCond->signal();
	}
	mpLock->unlock();
}


CLAppAutoUnlock::~CLAppAutoUnlock()
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
