/**********************************************************
* Copyright(C) 2022 CAN-Lab Inc.
* Garnet Kim <garnet@can-lab.co.kr> - 2022.10.04
* FileName : CLApp_MsgQueue.cpp
* Auther   : Garnet Kim
* Modify   : 2022-10-04
**********************************************************/
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

#include "CLApp_MsgQueue.h"

CLAppMsgQueue::CLAppMsgQueue()
{
	mnCmdSize = sizeof(CLAppCommand::command_t) - sizeof(long);
	m_nQueueID = -1;
}


CLAppMsgQueue::~CLAppMsgQueue()
{
}


CLAppMsgQueue::CLAppMsgQueue(key_t key)
{
	mnCmdSize = sizeof(CLAppCommand::command_t) - sizeof(long);
	m_nQueueID = -1;
	open(key);
}


bool CLAppMsgQueue::open(key_t key)
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


bool CLAppMsgQueue::send(long nType, CLAppCommand* pCmd)
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


bool CLAppMsgQueue::read(long nType, CLAppCommand* pCmd)
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


bool CLAppMsgQueue::read(long nType, CLAppCommand* pCmd, long nTimeout)
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


bool CLAppMsgQueue::peek(long nType )
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

bool CLAppMsgQueue::peekRead(long nType, CLAppCommand* pCmd )
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

void CLAppMsgQueue::close()
{
	if(valid() == true) {
		msgctl(m_nQueueID, IPC_RMID, 0);
	}

	m_nQueueID = -1;
}


void CLAppMsgQueue::display(bool bTransmit, CLAppCommand* pCmd)
{
	unsigned char* pBuffer = (unsigned char*)pCmd->getCommand();
	string szData;
	char szTemp[50];

	for(int i=0; i<(int)sizeof(CLAppCommand::command_t); i++)
	{
		sprintf(szTemp, "%02x ", pBuffer[i]);
		szData += szTemp;
		if((i != 0) && (i%8)==0)
			printf("\n");
	}

	szData += "\n";
	printf(szData.c_str());
}

bool CLAppMsgQueue::valid()
{
	if(m_nQueueID >= 0){
		return true;
	}
	return false;
}
