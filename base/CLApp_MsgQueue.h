/**********************************************************
* Copyright(C) 2022 CAN-Lab Inc.
* Garnet Kim <garnet@can-lab.co.kr> - 2022.10.04
* FileName : CLApp_MsgQueue.h
* Auther   : Garnet Kim
* Modify   : 2022-10-04
**********************************************************/
#ifndef _CLAPP_MESSAGE_QUEUE_H_
#define _CLAPP_MESSAGE_QUEUE_H_

#include "CLApp_Command.h"

using namespace std;

class CLAppMsgQueue
{
public:
	CLAppMsgQueue();
	CLAppMsgQueue(key_t key);
	~CLAppMsgQueue();

	bool open(key_t key);
	bool send(long nType, CLAppCommand* pCmd);
	bool read(long nType, CLAppCommand* pCmd);
	bool read(long nType, CLAppCommand* pCmd, long nTimeout);	// TIMEOUT : miliseconds
	bool peek(long nType );
	bool peekRead(long nType, CLAppCommand* pCmd );
	void close();
	void display(bool bTransmit, CLAppCommand* pCmd);
	bool valid();

protected:
	int			m_nQueueID;
	int			mnCmdSize;
};

#endif //_CLAPP_MESSAGE_QUEUE_H_
