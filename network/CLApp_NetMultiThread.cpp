/**********************************************************
* Copyright(C) 2023 CAN-Lab Inc.
* Garnet Kim <garnet@can-lab.co.kr> - 2023.01.27
* FileName : CLApp_NetMultiThread.cpp
* Auther   : Garnet Kim
* Modify   : 2023-01-27
**********************************************************/
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sched.h>
#include <errno.h>

#include "base/CLApp_Mutex.h"
#include "base/CLApp_Condition.h"

#include "CLApp_NetMultiThread.h"


#define CL_MULTI_THREAD_SAFE_BLOCK(pLock) CLAppAutoLock __block((pLock))


bool CLAppNetMultiThread::m_bExit = false;

CLAppNetMultiThread::CLAppNetMultiThread(int index)
{
	CL_MULTI_THREAD_SAFE_BLOCK(&m_Lock);
	m_State = INITIALIZED;
	m_Priority = MIDDLE;
	m_Policy = REALTIME;
	m_Tid = 0;
	m_nIndex = index;
}

CLAppNetMultiThread::~CLAppNetMultiThread()
{
	CL_THREAD_UNSAFE_BLOCK(&m_Lock);
	if (m_State!=INITIALIZED && m_State!=DONE) { }
}

void CLAppNetMultiThread::multiStart()
{
	CL_MULTI_THREAD_SAFE_BLOCK(&m_Lock);

	struct sched_param schedParam;
	
	if (m_Policy == REALTIME) {
		switch(m_Priority) {
		case HIGH :
			schedParam.sched_priority = sched_get_priority_max(SCHED_FIFO);
			break;
		case LOW :
			schedParam.sched_priority = sched_get_priority_max(SCHED_FIFO) - 2;
			break;
		case MIDDLE :
		default :
			schedParam.sched_priority = sched_get_priority_max(SCHED_FIFO) - 1;
			break;
		}
	}
	else {
		schedParam.sched_priority = 0;
	}
	
	{
		pthread_attr_t attr[1];
		pthread_attr_init(attr);
		pthread_attr_setdetachstate(attr, PTHREAD_CREATE_DETACHED);
		pthread_attr_setstacksize(attr, 1024);		
		pthread_attr_setschedparam(attr, &schedParam);
		pthread_create(&m_Tid, attr, CLAppNetMultiThread::mutliEntry, this);
		pthread_attr_destroy(attr);
	}
}

void CLAppNetMultiThread::multiStop()
{
	CL_MULTI_THREAD_SAFE_BLOCK(&m_Lock);

	if (m_State != DONE) {
		m_State = STOPPED;
	}
}

void CLAppNetMultiThread::multiJoin()
{
	CL_MULTI_THREAD_SAFE_BLOCK(&m_Lock);
	
	while (m_State!=DONE) {
		m_Cond.wait(&m_Lock);
	}
}

bool CLAppNetMultiThread::multiRun()
{
	CL_MULTI_THREAD_SAFE_BLOCK(&m_Lock);
	return (m_State == RUNNING && m_bExit == false);
}

void* CLAppNetMultiThread::mutliEntry(void* arg)
{
	CLAppNetMultiThread* pCamThread = static_cast<CLAppNetMultiThread*>(arg);
	
	//CL_MULTI_THREAD_SAFE_BLOCK(&pCamThread->m_Lock);
	pCamThread->m_State = RUNNING;

	pCamThread->multiLoop();
	
	//CL_MULTI_THREAD_SAFE_BLOCK(&pCamThread->m_Lock);
	pCamThread->m_State = DONE;

	pCamThread->m_Cond.signal();	

	return NULL;
}
