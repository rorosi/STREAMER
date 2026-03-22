/**********************************************************
* Copyright(C) 2022 CAN-Lab Inc.
* Garnet Kim <garnet@can-lab.co.kr> - 2022.10.31
* FileName : CLApp_NetworkServer.cpp
* Auther   : Garnet Kim
* Modify   : 2022-10-31
**********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ether.h>
#include <arpa/inet.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/sockios.h>

#include "CLApp_Define.h"
#include "base/CLApp_Thread.h"
#include "base/CLApp_Time.h"
#include "base/CLApp_Command.h"
#include "base/CLApp_MsgQueue.h"
#include "CLApp_NetworkSocket.h"
#include "CLApp_NetworkServer.h"

#define CL_TCP_DEV          "enp0s3" // "eth0"
#define CL_TCP_PORT            9000


CLAppNetworkServer* CLAppNetworkServer::m_pInstance = NULL;

CLAppNetworkServer::CLAppNetworkServer()
{
    m_nClientCount = 0;
	m_bIsGetIpaddr = false;
	m_szIpAddr = "";

	m_pNetThreadControl = NULL;

	for(int idx=0;idx<(int)MAX_VIDEO_CONNECT_SIZE ;idx++) {
		m_pNetThreadVideo[idx] = NULL;
	}
}

CLAppNetworkServer::~CLAppNetworkServer()
{

}

void CLAppNetworkServer::initInstance()
{
	bool    bIsIpAddr = false;

	CLAppThread::initInstance();

    init_object();

	bIsIpAddr = get_network_info(CL_TCP_DEV);
	if(bIsIpAddr == false){ }
}
	
void CLAppNetworkServer::exitInstance()
{
	CLAppThread::exitInstance();
}	

void CLAppNetworkServer::mainLoop()
{
    int result = 0;

	while (isRunning())
	{
        msleep(10);

        if(m_bSocketEnable == false) {
            try_connect_socket();
            msleep(500);
            continue;
        } else {
            struct pollfd fdset;

            result = poll_object();
            if(result == 0)
                continue;

            start_agent_objects();
			
            while (isRunning())
            {
                struct sockaddr_in fdaddr;                
                int fdidx;

                fdidx = get_agent_objects(&fdset, &fdaddr);
                if (fdidx < 0) {
                    break;
                }

                if (fdset.revents & ~POLLIN) {
                    del_object(fdset.fd);
                } else if (fdset.revents & POLLIN) {
                    int sock = -1;
                    socklen_t addrlen;
                    
                    union 
                    {
                        struct sockaddr sa;
                        struct sockaddr_in addr;
                    } sockadd;

                    memset(&sockadd, 0, sizeof(sockadd));
                    
                    addrlen = sizeof(sockadd.addr);

                    if(fdidx >= CLSOCK_CONTROL && fdidx < CLSOCK_MAX)
                    { // accept socket control and video                        
                        
						sock = accept(fdset.fd, (struct sockaddr*)&sockadd.sa, &addrlen);
						if(sock <= 0){
							printf("\"accept\" error: %d, %s\n", errno, inet_ntoa(sockadd.addr.sin_addr));
							break;
						} else {

							printf("accept from %s for %s \n", 
									inet_ntoa(sockadd.addr.sin_addr),
									(fdidx == CLSOCK_CONTROL)?"control":"streaming");

							result = fcntl(sock, F_GETFL, 0);
							result = fcntl(sock, F_SETFL, result|FASYNC|O_NONBLOCK);

							 if(fdidx == CLSOCK_CONTROL) 
							 {
								client_ctrlThreadStop(0);

								client_ctrlThreadStart(0, sock);
							}
							else
							{
								client_videoThreadStop((fdidx - CLSOCK_VIDEO_01));
								
								client_videoThreadStart( (fdidx - CLSOCK_VIDEO_01), sock);
							}
						}                    
                    } // switch   
					else
					{
						break;
					}     
                } // else if
            } // while

	        finish_agent_objects();
        }
    }
}

int CLAppNetworkServer::try_connect_socket()
{
    int result = 0, idx;
    
    for (idx=0; idx<(int)CLSOCK_MAX; idx++)
    {
        if(m_netSocketFd[idx] > 0) {
            close(m_netSocketFd[idx]);  

            m_netSocketFd[idx] = -1;     
        }

        m_netSocketFd[idx] = CLAppNetworkSocket::createSocket(CL_TCP_PORT + idx);
        if(m_netSocketFd[idx] < 0) {
            printf("Could not create socket for network %s (%d) \n",
                    (idx == 0)?"control":"stream", idx);
            return -1;
        }

		add_object(m_netSocketFd[idx], POLLIN);
    }
        
    m_bSocketEnable = true;

    printf("Creating sockets for control and streaming \n");
    return result;
}

void CLAppNetworkServer::init_object()
{
    int idx = 0;

    for (idx=0; idx<(int)MAX_NET_OBJECT_SIZE; idx++)
	{
		m_pNetObjHndl.fds[idx].fd = -1;
		m_pNetObjHndl.fds[idx].events = 0;
		m_pNetObjHndl.fds[idx].revents = 0;
		m_pNetObjHndl.sock_type[idx] = CLFD_TYPE_NONE;

		memset(&m_pNetObjHndl.sock_addr[idx], 0, sizeof(struct sockaddr_in));
	}

    for (idx=0; idx<(int)CLSOCK_MAX; idx++)
	{	
        m_netSocketFd[idx] = -1;
    }

    m_netObjCount = 0;

    m_agentObjUsingEnable = false;
	m_agentObjIdx = -1;

    m_bSocketEnable = false;
}

int CLAppNetworkServer::set_object_type(int fd, int type)
{
    int idx = 0;

	for (idx=0; idx<m_netObjCount; idx++)
	{
		if (m_pNetObjHndl.fds[idx].fd == fd)
		{
			m_pNetObjHndl.sock_type[idx] = type;
			return 0;
		}
	}
	return -1;
}

int CLAppNetworkServer::get_object_type(int fd)
{
    int idx = 0;

	for (idx=0; idx<m_netObjCount; idx++)
	{
		if (m_pNetObjHndl.fds[idx].fd == fd)
		{
			return m_pNetObjHndl.sock_type[idx];
		}
	}
	return CLFD_TYPE_NONE;
}

int CLAppNetworkServer::add_object(int fd, int events, int type, void* _addr)
{
	if (m_netObjCount >= (int)MAX_NET_OBJECT_SIZE) {
		printf("No More Object to Poll \n");
		return -1;
	}

	m_pNetObjHndl.fds[m_netObjCount].fd = fd;
	m_pNetObjHndl.fds[m_netObjCount].events = events;
	m_pNetObjHndl.fds[m_netObjCount].revents = 0;
	m_pNetObjHndl.sock_type[m_netObjCount] = type;

	if (_addr)
	{
		memcpy(&m_pNetObjHndl.sock_addr[m_netObjCount], 
                    (struct sockaddr_in*)_addr, sizeof(struct sockaddr_in));
	}

	printf("Add Network Object #%d - %d, %d \n", m_netObjCount, fd, type);
	return ++m_netObjCount;
}

int CLAppNetworkServer::del_object(int fd)
{
	int idx = 0;
    bool updateAgentObjIdx = true;

	for (idx=0; idx<m_netObjCount; idx++)
	{
		if (m_pNetObjHndl.fds[idx].fd == fd)
		{
			printf("Delete Network Object #%d - %d \n", idx, fd);
		
            if (m_agentObjIdx >= 0 && updateAgentObjIdx == true) {
				m_agentObjIdx--;
			}

			if (idx < (m_netObjCount-1)) 
			{
				memcpy(&m_pNetObjHndl.fds[idx],
                        &m_pNetObjHndl.fds[m_netObjCount-1],
                            sizeof(struct pollfd));
				memcpy(&m_pNetObjHndl.sock_type[idx],
                        &m_pNetObjHndl.sock_type[m_netObjCount-1],
                            sizeof(int));

				memcpy(&m_pNetObjHndl.sock_addr[idx],
					    &m_pNetObjHndl.sock_addr[m_netObjCount-1],
                            sizeof(struct sockaddr_in));
			}
			
			m_pNetObjHndl.fds[m_netObjCount-1].fd = -1;
			m_pNetObjHndl.fds[m_netObjCount-1].events = 0;
			m_pNetObjHndl.fds[m_netObjCount-1].revents = 0;
			m_pNetObjHndl.sock_type[m_netObjCount-1] = CLFD_TYPE_NONE;

			memset(&m_pNetObjHndl.sock_addr[m_netObjCount-1], 0, sizeof(struct sockaddr_in));
	
			return --m_netObjCount;
		}

        if (m_agentObjIdx >= 0 && m_pNetObjHndl.fds[m_agentObjIdx].fd == fd)
		{
			updateAgentObjIdx = false;
		}
	}

	return -1;
}

int CLAppNetworkServer::poll_object(int timeo)
{
	int result = 0;
	
	while(1)
	{
		result = poll(m_pNetObjHndl.fds, (nfds_t)m_netObjCount, timeo); 

		if (result < 0)
		{
			if(errno == EINTR || errno == EAGAIN)
			{
				printf("\"poll\" EINTR | EAGAIN\n");
				continue;
			}
			printf("\"poll\" Error: %d\n", errno);
		}
		break;
	}

	return result;
}

int CLAppNetworkServer::start_agent_objects()
{
	if (!m_agentObjUsingEnable) {
		m_agentObjUsingEnable = true;
		m_agentObjIdx = -1;
		return 0;
	}
	return -1;
}

int CLAppNetworkServer::get_agent_objects(struct pollfd* fdset, void* _addr)
{
	if (!m_agentObjUsingEnable) {
		return -1;
	}
	
	m_agentObjIdx++;
	if (m_agentObjIdx >= m_netObjCount) {
		return -1;
	}

	if (fdset) {
		memcpy(fdset, &m_pNetObjHndl.fds[m_agentObjIdx], sizeof(struct pollfd));
	}

	if (_addr)
	{
		struct sockaddr_in* addr = (struct sockaddr_in*)_addr;
		memcpy(addr, &m_pNetObjHndl.sock_addr[m_agentObjIdx], sizeof(struct sockaddr_in));
	}
	return m_agentObjIdx;
}

void CLAppNetworkServer::finish_agent_objects()
{
	m_agentObjUsingEnable = false;
	m_agentObjIdx=-1;
}


void CLAppNetworkServer::check_ip_addr()
{
    int iresult = 0;
    char	szBufs[128];
    bool    bIsIpAddr = false;

    bIsIpAddr = get_network_info(CL_TCP_DEV);
    if(bIsIpAddr == true) {
        return;
    }
    
    memset(szBufs, 0, 128);
#if 0	
    if (0 == access("/var/run/udhcpc.pid", F_OK)) {
        CLAppFile f;

        if(f.open((char *)"/var/run/udhcpc.pid", CLAppFile::modeRead) == true)
        {
            f.read(szBufs, 128);
            f.close();

            if(strlen(szBufs) > 0){
                char	szSys[256];

                memset(szSys, 0, 256);
                sprintf(szSys, "/bin/kill -9 %s \n", szBufs);		
                iresult = system(szSys);	
                msleep(1500);
            }
        }			
        unlink((char *)"/var/run/udhcpc.pid");
    }
#endif

    memset(szBufs, 0, 128);

	printf(" >< DHCP START!!! >< \n");    
    sprintf(szBufs, "/sbin/udhcpc -i %s -s /etc/udhcpc.d/50default -p /var/run/udhcpc.pid", CL_TCP_DEV);		
    iresult = system(szBufs);
    if(iresult == 0){

    }
    msleep(1500);
    
    bIsIpAddr = get_network_info(CL_TCP_DEV);
}


bool CLAppNetworkServer::get_network_info(const char *szNetDev)
{
	int 				sock_fd = -1;	
	struct ifconf		sock_ifconf; 
	struct ifreq 		*sock_ifreq; 
	struct sockaddr_in	*sock_addr; 

	string szDevice = ""; 
	string szIP = "";
	string szBroadcast = "";
	string szNetmask = "";
	string szMac = "";
	unsigned char aucMac[6] = { 0, 0, 0, 0, 0, 0};
	const int IFCLEN = sizeof(struct ifreq)*3;
	char ifcbuf[IFCLEN];
	bool bIsEnable = false;
	
	sock_fd = socket(AF_INET, SOCK_DGRAM, 0); 
	if(sock_fd < 0) {
		printf("Could not get open for %s \n", CL_TCP_DEV);
		return bIsEnable;
	}	
		 
	memset((void *)&sock_ifconf, 0, sizeof(struct ifconf)); 

	sock_ifconf.ifc_len = sizeof(struct ifreq) * 3; 
	sock_ifconf.ifc_buf = ifcbuf; 
			
	if(ioctl(sock_fd, SIOCGIFCONF, &sock_ifconf) != 0) {
		close(sock_fd); 
		return bIsEnable;
	}
		 			 
	sock_ifreq = sock_ifconf.ifc_req; 

    if(!(sock_ifreq->ifr_flags & IFF_UP)){
        strncpy(sock_ifreq->ifr_name, CL_TCP_DEV, IFNAMSIZ);

        sock_ifreq->ifr_flags |= IFF_UP;
        ioctl(sock_fd, SIOCSIFFLAGS, sock_ifreq);
    }

	for (int i=0; i<3; i++)			
	{	
		if (!strncmp(sock_ifreq[i].ifr_name, szNetDev, (strlen(szNetDev) -1 ) ))	
		{ 
			szDevice = sock_ifreq[i].ifr_name; 

			sock_addr = (struct sockaddr_in *)(&sock_ifreq[i].ifr_addr); 
			szIP = inet_ntoa(sock_addr->sin_addr); 

			if(sock_addr->sin_addr.s_addr == htonl(0x7F000001))
				continue;
			
			ioctl(sock_fd, SIOCGIFBRDADDR, &sock_ifreq[i]);		
			sock_addr = (struct sockaddr_in *)&sock_ifreq[i].ifr_broadaddr; 
			szBroadcast = inet_ntoa(sock_addr->sin_addr);   

			ioctl(sock_fd, SIOCGIFNETMASK, &sock_ifreq[i]);		
			sock_addr = (struct sockaddr_in *)&sock_ifreq[i].ifr_netmask; 
			szNetmask = strdup(inet_ntoa(sock_addr->sin_addr));	  

			if(ioctl(sock_fd, SIOCGIFHWADDR, &sock_ifreq[i]) == 0) 
			{ 
				char szTemp[256];

				memcpy(aucMac, &sock_ifreq[i].ifr_hwaddr.sa_data, 6); 
				sprintf(szTemp, "%02X:%02X:%02X:%02X:%02X:%02X", 
					aucMac[0], aucMac[1], aucMac[2],
					aucMac[3], aucMac[4], aucMac[5]);

				szMac = szTemp;
				bIsEnable = true;
			} 
			else 
			{
				memset(aucMac, 0, 6);
				szMac = "00:00:00:00:00:00";
			}
			
			printf("----------------------------------------- \n");
			printf("  %s \n", szDevice.c_str());
			printf("  ---- \n");
			printf("	 H/W address : %s \n", szMac.c_str());
			printf("	 IP address  : %s \n", szIP.c_str());			
			printf("	 broadcast	 : %s \n", szBroadcast.c_str());
			printf("	 netmask	 : %s \n", szNetmask.c_str());					
			printf("-----------------------------------------\n");	
			
			if(m_bIsGetIpaddr == false) {
				m_szIpAddr = szIP;
				m_bIsGetIpaddr = true;
			}
			break;				
		} 
	}				

	close(sock_fd); 		

	return bIsEnable;
}


int CLAppNetworkServer::client_videoThreadStart(int index, int iSockFd)
{	
	if(m_pNetThreadVideo[index]) {
		m_pNetThreadVideo[index]->multiStop();
		m_pNetThreadVideo[index]->multiJoin();
		delete m_pNetThreadVideo[index];
	}

	m_pNetThreadVideo[index] = new CLAppSocketThreadVideo(index, iSockFd);
	if(m_pNetThreadVideo[index]) {
		m_pNetThreadVideo[index]->multiStart();
	}

	return 0;
}

void CLAppNetworkServer::client_videoThreadStop(int idx)
{
	if(m_pNetThreadVideo[idx]) {
		m_pNetThreadVideo[idx]->multiStop();
		m_pNetThreadVideo[idx]->multiJoin();
		delete m_pNetThreadVideo[idx];
	}
	m_pNetThreadVideo[idx] = NULL;
}

int CLAppNetworkServer::client_ctrlThreadStart(int index, int iSockFd)
{
	if(m_pNetThreadControl) {
		m_pNetThreadControl->multiStop();
		m_pNetThreadControl->multiJoin();
		delete m_pNetThreadControl;
	}

	m_pNetThreadControl = new CLAppSocketThreadControl(index, iSockFd);
	if(m_pNetThreadControl) {
		m_pNetThreadControl->multiStart();
	}
	return 0;
}

void CLAppNetworkServer::client_ctrlThreadStop(int idx)
{	
	if(m_pNetThreadControl) {
		m_pNetThreadControl->multiStop();
		m_pNetThreadControl->multiJoin();

		delete m_pNetThreadControl;
	}
	m_pNetThreadControl = NULL;
}

void CLAppNetworkServer::clientThreadClose()
{
	for(int idx=0;idx<(int)MAX_VIDEO_CONNECT_SIZE ;idx++) {
		client_videoThreadStop(idx);
	}
	
	client_ctrlThreadStop(0);
}

int CLAppNetworkServer::get_command_after_analysis(unsigned char *szData, int size)
{
	CLAppNetCommandPacket *pCmdPkt = (CLAppNetCommandPacket *)&szData[0];
	CLAppMsgQueue qModuleMain(KEY_MQ_MAIN_TASK);
        
	if(!strncmp((char *)pCmdPkt->command, "STRT", 4)) {
		CLAppCommand cmd(M_MODULE_MAIN_STREAM_START, 0, 0, 0, 0);
    	qModuleMain.send(1,	&cmd );

		printf("Get Message : Start... \n");
	} else if(!strncmp((char *)pCmdPkt->command, "STOP", 4)) {
		CLAppCommand cmd(M_MODULE_MAIN_STREAM_STOP, 0, 0, 0, 0);
    	qModuleMain.send(1,	&cmd );	
		printf("Get Message : Stop... \n");
	} else if(!strncmp((char *)pCmdPkt->command, "CLSE", 4)) {
		CLAppCommand cmd(M_MODULE_MAIN_STREAM_CLOSE, 0, 0, 0, 0);
    	qModuleMain.send(1,	&cmd );	
		printf("Get Message : Close... \n");
	} else if(!strncmp((char *)pCmdPkt->command, "LIVE", 4)) {
		CLAppCommand cmd(M_MODULE_MAIN_CTRL_PARAMS, 0, 0, 0, 0);
    	qModuleMain.send(1,	&cmd );	
	} else if(!strncmp((char *)pCmdPkt->command, "RECS", 4)) {
		unsigned int status = 0;

		if(pCmdPkt->length > 0)
			memcpy((char *)&status, &pCmdPkt->params[0], pCmdPkt->length);

		printf("Get Message : RECS (%016x)... \n", status);
		CLAppCommand cmd(M_MODULE_MAIN_REC_STATUS, status, 0, 0, 0);
    	qModuleMain.send(1,	&cmd );	
	} 
	return 0;
}

int CLAppNetworkServer::get_ipaddr_status(char *szIpaddr, int *port)
{
	if(m_bIsGetIpaddr == false) {
		return -1;
	}

	if(m_szIpAddr.length() > 0 && szIpaddr != NULL){
		strncpy(szIpaddr, m_szIpAddr.c_str(), m_szIpAddr.length());
	} else {
		return -1;
	}
	*port = CL_TCP_PORT;
	return 0; 
}

void CLAppSocketThreadControl::multiLoop(int index)
{
	int check_read_size = 0, ret;
	char szCtrlBufs[MAX_STREAM_SLICE];
	int resol_type = 0, mode_type = 0;
	
    CLAppCtrlPrmsBuffer *pCtrlPrmsQueue = new CLAppCtrlPrmsBuffer();	
	if(pCtrlPrmsQueue) {
		pCtrlPrmsQueue->deleteAll();
	}

	while (multiRun())
	{
		if(pCtrlPrmsQueue != NULL && pCtrlPrmsQueue->isEmpty() != true) {
			CLAppBufferFrame pCtrlPrmFrame;

			pCtrlPrmsQueue->getFirstData(&pCtrlPrmFrame);
			
			if(pCtrlPrmFrame.getSize() > 0){
				CLAppNetCommandPacket	*pNetCmdPacket = (CLAppNetCommandPacket *)&szCtrlBufs[0];
				int isize = sizeof(CLAppNetCommandPacket);


				memset((char*)pNetCmdPacket, 0, sizeof(CLAppNetCommandPacket) );

				if(1 == pCtrlPrmFrame.getFlag())
				{
					memcpy((char*)pNetCmdPacket->command, "LIVE", 4);

					pNetCmdPacket->length = pCtrlPrmFrame.getSize();
					memcpy((char*)&pNetCmdPacket->params[0], pCtrlPrmFrame.getFrame(), pNetCmdPacket->length);

					isize = sizeof(CLAppNetCommandPacket) + pNetCmdPacket->length;
				}
			
				ret = CLAppNetworkSocket::sendAfterMakePacket(m_nSocketFd, 
							resol_type, mode_type, 
							(unsigned char *)pNetCmdPacket, 
							(int)isize);
			}
		}

		if(ioctl(m_nSocketFd, FIONREAD, &check_read_size) < 0) {
			break;
		}

		if(check_read_size <= 0) {
			usleep(5000);
			continue;
		} else {			
			int mode_type = 0, ctrl_size = 0;

			ret = CLAppNetworkSocket::analysisAfterRecvPacket(m_nSocketFd, 
										(char *)&mode_type, 
										(unsigned char *)&szCtrlBufs[0], 
										&ctrl_size);
				
			if(ret < 0) {
				usleep(5000);
				continue;
			} else if(ret > 0) {
				CLAppNetworkServer::get_command_after_analysis((unsigned char *)&szCtrlBufs[0], ctrl_size);
			}
		}
	}
}

void CLAppSocketThreadVideo::multiLoop(int index)
{
	CLAppDateTime 	tOldTime, tNewTime;
	CLAppTimeSpan 	span;
	int frame_count = 0;


    CLAppNetworkBuffer *pNetworkVideoQueue = new CLAppNetworkBuffer(m_nIndex);	
	if(pNetworkVideoQueue) {
		pNetworkVideoQueue->deleteAll();
	}

	printf("Video(#-%d) Thread Start... \n", m_nIndex);
	while (multiRun())
	{
		tNewTime = CLAppDateTime::getSystemTime();
		span	 = tNewTime - tOldTime;
		
		if ( abs( span.getTotalSeconds() ) >= 1) 
		{
			tOldTime = tNewTime;
			
			if(frame_count > 0) {
				printf("Video CH-#%d, Frame %dfps \n", m_nIndex, frame_count);	
				frame_count = 0;
			}			
		}
		
		if(pNetworkVideoQueue != NULL && pNetworkVideoQueue->isEmpty() != true) {

			CLAppBufferFrame pVideoFrame;

			pNetworkVideoQueue->getFirstData(&pVideoFrame);
			
			if(pVideoFrame.getSize() > 0){
				int ret = 0;
				int resol_type = ((pVideoFrame.getWidth() & 0xffff) << 16) + (pVideoFrame.getHeight() & 0xffff);
				int mode_type = ((pVideoFrame.getChannel() & 0xff) << 8) + (pVideoFrame.getType() & 0xff);

				ret = CLAppNetworkSocket::sendAfterMakePacket(m_nSocketFd, 
							resol_type, mode_type, 
							(unsigned char *)pVideoFrame.getFrame(), 
							(int)pVideoFrame.getSize());

				frame_count++;

				if(ret < 0) {
					printf("Error, video socket %d... \n", ret);
					break;
				}
			}
		} else {
			usleep(1);
		}
    }
	printf("Video(#-%d) Thread Stop... \n", m_nIndex);

	if(m_nSocketFd > 0){
		close(m_nSocketFd);
	}

	m_nSocketFd = -1;
}
