/**********************************************************
* Copyright(C) 2022 CAN-Lab Inc.
* Garnet Kim <garnet@can-lab.co.kr> - 2022.10.31
* FileName : CLApp_NetworkServer.h
* Auther   : Garnet Kim
* Modify   : 2022-10-31
**********************************************************/
#ifndef _CLAPP_NETWORK_SERVER_H_
#define _CLAPP_NETWORK_SERVER_H_

#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/poll.h>
#include <time.h>
#include <semaphore.h>

#include "base/CLApp_Thread.h"
#include "base/CLApp_BufferQueue.h"
#include "CLApp_NetMultiThread.h"

#define MAX_VIDEO_CONNECT_SIZE		(7U)
#define	MAX_NET_OBJECT_SIZE			(10U)
#define	MAX_CLIENT_HNDL_SIZE		(10U)

enum {
	CLFD_TYPE_NONE = 0,
	CLFD_TYPE_STREAM,
	CLFD_TYPE_MAX,
};

enum {
	CLSOCK_CONTROL = 0,
	CLSOCK_VIDEO_01,
	CLSOCK_VIDEO_02,
	CLSOCK_VIDEO_03,
	CLSOCK_VIDEO_04,
	CLSOCK_VIDEO_05,
	CLSOCK_VIDEO_06,
	CLSOCK_VIDEO_07,
	CLSOCK_VIDEO_08,	
	CLSOCK_MAX,
};

typedef struct CLAppNetClientHndlTag {
	int status;	// 0: stop, 1: running
	int	sock_fd;
	int	sock_type;	
	pthread_t		pid;
	void	*task;	
} CLAppNetClientHndl;

typedef struct CLAppNetServerObjectHndlTag {
	struct pollfd		fds[MAX_NET_OBJECT_SIZE];	
	struct sockaddr_in 	sock_addr[MAX_NET_OBJECT_SIZE];
	int					sock_type[MAX_NET_OBJECT_SIZE];
} CLAppNetServerObjectHndl;

typedef struct CLAppNetCommandPacketTag {
	unsigned char command[4];
	unsigned char prm_type;				// params type - 0: End, 1~255 : continue
	unsigned short length;				// params length
	unsigned char params[1];			// Data Max.slice(1400-4) * prm_type count
}CLAppNetCommandPacket;


class CLAppSocketThreadControl : public CLAppNetMultiThread
{
public:
	CLAppSocketThreadControl(int idex, int sockfd) : CLAppNetMultiThread(idex) { m_nSocketFd = sockfd; }
	virtual void multiLoop(int index = 0);

public:
	int	m_nSocketFd;
};

class CLAppSocketThreadVideo : public CLAppNetMultiThread
{
public:
	CLAppSocketThreadVideo(int idex, int sockfd) : CLAppNetMultiThread(idex) { m_nSocketFd = sockfd; }
	virtual void multiLoop(int index = 0);

public:
	int	m_nSocketFd;
};

class CLAppNetworkServer : public CLAppThread
{
public:
    CLAppNetworkServer();
    virtual ~CLAppNetworkServer();   	
	virtual void mainLoop();
	virtual void initInstance();
	virtual void exitInstance();
	int create_socket(int port);
	int try_connect_socket();
	int get_ipaddr_status (char *szIpaddr, int *port);
	static int get_command_after_analysis(unsigned char *szData, int size);
	
	void clientThreadClose();

private:
	void init_object();
	int set_object_type(int fd, int type);
	int get_object_type(int fd);
	int add_object(int fd, int events, int type = CLFD_TYPE_STREAM, void* _addr = NULL);
	int del_object(int fd);
	int poll_object(int timeo = 1000);
	int start_agent_objects();
	int get_agent_objects(struct pollfd* fdset = NULL, void* _addr = NULL);
	void finish_agent_objects();
	void check_ip_addr();
	bool get_network_info(const char *szNetDev);

	int client_videoThreadStart(int index, int iSockFd);
	void client_videoThreadStop(int idx);
//	static void *client_videoThreadEntry(void *arg);
	int client_ctrlThreadStart(int index, int iSockFd);
	void client_ctrlThreadStop(int idx);
//	static void *client_ctrlThreadEntry(void *arg);

private:  
	CLAppSocketThreadControl			*m_pNetThreadControl;
	CLAppSocketThreadVideo			*m_pNetThreadVideo[MAX_VIDEO_CONNECT_SIZE];
    static CLAppNetworkServer 	*m_pInstance;
	//CLAppNetworkBuffer				*m_pNetworkVideoQueue;

	CLAppNetServerObjectHndl	m_pNetObjHndl;
	int							m_netObjCount;	
	int				m_agentObjIdx;
	bool			m_agentObjUsingEnable;

	int			m_netSocketFd[CLSOCK_MAX];
	bool		m_bSocketEnable;

public:
	CLAppNetClientHndl		mpClientHndl[MAX_CLIENT_HNDL_SIZE];
	int				m_nClientCount;
	bool 			m_bIsGetIpaddr;
	string			m_szIpAddr;

public:
	static CLAppNetworkServer& instance(){
		if(m_pInstance == NULL){
			m_pInstance = new CLAppNetworkServer();
		}
		return *m_pInstance;
	}
	static CLAppNetworkServer& release(){
		if(m_pInstance ){
			delete m_pInstance;
			m_pInstance = NULL;
		}
		return *m_pInstance;
	}
};

#endif // _CLAPP_NETWORK_SERVER_H_
