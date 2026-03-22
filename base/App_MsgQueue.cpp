#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include <stdarg.h>
#include <string.h> 
#include <unistd.h> 
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <string>

#include "App_MsgQueue.h"

AppMsgQueue::AppMsgQueue()
{
	mnCmdSize = sizeof(AppCommand::command_t) - sizeof(long);
	m_nQueueID = -1;
}


AppMsgQueue::~AppMsgQueue()
{
}


AppMsgQueue::AppMsgQueue(key_t key)
{
	mnCmdSize = sizeof(AppCommand::command_t) - sizeof(long);
	m_nQueueID = -1;
	open(key);
}


bool AppMsgQueue::open(key_t key)
{
	if(valid() == true)
	{
		return false;
	}

	if((m_nQueueID = msgget(key, IPC_CREAT | 0666)) == -1) 
	{
		switch(errno)
		{		
			case EEXIST :	
				break;

			case EIDRM :	
				break;

			case ENOENT :	
				break;

			case ENOMEM :	
				break;;

			case ENOSPC :	
				break;
		}
		return false;
	}
			
	return true;
}


bool AppMsgQueue::send(long nType, AppCommand* pCmd)
{
	bool result = false;

	if ( valid() == true )
	{
		pCmd->getCommand()->type = nType;

		if ( msgsnd( m_nQueueID, pCmd->getCommand(), mnCmdSize, 0 ) != -1 )
		{
			result = true;
		}
	}
	return result;
}


bool AppMsgQueue::read(long nType, AppCommand* pCmd)
{
	bool result = false;

	if(valid() == true) {
		pCmd->getCommand()->type = nType;

		if ( msgrcv(m_nQueueID, pCmd->getCommand(), mnCmdSize, nType, 0) != -1 )
		{
			result = true;
		}
	}
	return result;
}


bool AppMsgQueue::read(long nType, AppCommand* pCmd, long nTimeout)
{
	bool result = false;

	if(nTimeout > 0)	{
		struct timeval tCurrent;
		struct timeval tEnd;

		gettimeofday(&tCurrent, NULL);

		tEnd.tv_sec = tCurrent.tv_sec + nTimeout / 1000;
		tEnd.tv_usec = tCurrent.tv_usec + (nTimeout % 1000 * 1000);

		while((tCurrent.tv_sec < tEnd.tv_sec) || 
			(tCurrent.tv_sec == tEnd.tv_sec && tCurrent.tv_usec <= tEnd.tv_usec))
		{
			if(peek(nType))
			{
				result = read(nType, pCmd);
				break;
			}

			usleep(10000);

			gettimeofday(&tCurrent, NULL);
		}
	} else {
		result = read(nType, pCmd);
	}	
	return result;
}


bool AppMsgQueue::peek(long nType )
{
 	bool result = false;

	if ( valid() == true )
	{
		if ( msgrcv(m_nQueueID, NULL, 0, nType,  IPC_NOWAIT) == -1 )
		{
			if ( errno == E2BIG )
				result = true;

			if ( errno != ENOMSG && errno != E2BIG )
			{
				printf( "Failed to receive, Q_Id:%d err=%s(%d) \n",
					  m_nQueueID, strerror( errno ), errno );
			}
		}
	}
    
	return result;
}

bool AppMsgQueue::peekRead(long nType, AppCommand* pCmd )
{
 	bool result = false;

	if ( valid() == true )
	{
		if ( msgrcv(m_nQueueID, NULL, 0, nType,  IPC_NOWAIT) == -1 )
		{
			if ( errno == E2BIG ){
				result = true;

				pCmd->getCommand()->type = nType;

				if ( msgrcv(m_nQueueID, pCmd->getCommand(), mnCmdSize, nType, 0) != -1 )
				{
					result = true;
				}
		
			}
		}
	}
    
	return result;
}

void AppMsgQueue::close()
{
	if(valid() == true) {
		msgctl(m_nQueueID, IPC_RMID, 0);
	}

	m_nQueueID = -1;
}


void AppMsgQueue::display(bool bTransmit, AppCommand* pCmd)
{
	unsigned char* pBuffer = (unsigned char*)pCmd->getCommand();
	string szData;
	char szTemp[50];

	for(int i=0; i<(int)sizeof(AppCommand::command_t); i++)
	{
		sprintf(szTemp, "%02x ", pBuffer[i]);
		szData += szTemp;
		if((i != 0) && (i%8)==0)
			printf("\n");
	}

	szData += "\n";
	printf(szData.c_str());
}

bool AppMsgQueue::valid()
{
	if(m_nQueueID >= 0){
		return true;
	}
	return false;
}
