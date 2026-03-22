#ifndef _APP_MESSAGE_QUEUE_H_
#define _APP_MESSAGE_QUEUE_H_

#include "App_Command.h"

using namespace std;

class AppMsgQueue
{
public:
	AppMsgQueue();
	AppMsgQueue(key_t key);
	~AppMsgQueue();

	bool open(key_t key);
	bool send(long nType, AppCommand* pCmd);
	bool read(long nType, AppCommand* pCmd);
	bool read(long nType, AppCommand* pCmd, long nTimeout);	// TIMEOUT : miliseconds
	bool peek(long nType );
	bool peekRead(long nType, AppCommand* pCmd );
	void close();
	void display(bool bTransmit, AppCommand* pCmd);
	bool valid();

protected:
	int			m_nQueueID;
	int			mnCmdSize;
};

#endif //_APP_MESSAGE_QUEUE_H_
