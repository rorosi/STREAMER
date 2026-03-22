/**********************************************************
* Copyright(C) 2022 CAN-Lab Inc.
* Garnet Kim <garnet@can-lab.co.kr> - 2022.10.31
* FileName : CLApp_NetworkSocket.h
* Auther   : Garnet Kim
* Modify   : 2022-10-31
**********************************************************/
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include "base/CLApp_Thread.h"
#include "base/CLApp_Time.h"
#include "base/CLApp_Command.h"
#include "CLApp_NetworkSocket.h"


const int DEFAULT_TCP_WINDOW_SIZE = (8*1024*1024);

int CLAppNetworkSocket::createSocket(int port)
{
	int sock;
	int result;
	int reuseaddr=1;
	int backlog=5;
	int windowsize=DEFAULT_TCP_WINDOW_SIZE;
	int windowsizelen=sizeof(windowsize);
	struct linger so_linger = {1, 0};

	union 
	{
		struct sockaddr sa;
		struct sockaddr_in addr;
		struct sockaddr_in6 addr6;
	} sockadd;

	memset(&sockadd, 0, sizeof(sockadd));

	sockadd.addr.sin_family=AF_INET;
	sockadd.addr.sin_addr.s_addr=htonl(INADDR_ANY);
	sockadd.addr.sin_port=htons(port);

	sock=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);	

	result=setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr));

	result=setsockopt(sock, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger));
	
	windowsize = 8*1024*1024;
	result=setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &windowsize, (socklen_t)windowsizelen);

	result=setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &windowsize, (socklen_t)windowsizelen);
	//printf("Requested TCP window size: %dB \n", windowsize);

	result=getsockopt(sock, SOL_SOCKET, SO_SNDBUF, &windowsize, (socklen_t*)&windowsizelen);
	// printf("Accepted TCP window size: %dB \n", windowsize);

	result=bind(sock, (struct sockaddr*)&sockadd.addr, sizeof(sockadd.addr));

	result=listen(sock, backlog);

	result=fcntl(sock, F_GETFL, 0);

	result=fcntl(sock, F_SETFL, result|FASYNC|O_NONBLOCK);
	return sock;
}

ssize_t CLAppNetworkSocket::sndData(int s, const unsigned char *msg, size_t len, int flags, int retries)
{
	ssize_t slen, totslen=0;
	fd_set ww_set ;
	struct  timeval wtimeout;	
	int result=0;	
		
	if (len<=0)
		return 0;

	do {
		wtimeout.tv_sec = 0L;    
		wtimeout.tv_usec = 100000L;

		FD_ZERO(&ww_set);
		FD_SET(s, &ww_set);	
		
		slen=send(s, (unsigned char *)msg+totslen, (size_t)(len-totslen), flags);
		if (slen < 0) {
			if (errno==EINTR||errno==EAGAIN) {//EAGAIN ->11, EINTR -> 4
				if(retries == 0) {					
					printf("error, could not send: %d \n", errno);
					return -1; 
				}
				
				if ( (result = select(s+1, NULL, &ww_set, NULL, &wtimeout)) < 0 ) {
					return -2;
				} else if(result > 0)	{
					if (FD_ISSET(s, &ww_set)) { 
						FD_CLR(s, &ww_set); 
					}
				}	
				usleep(1000);
			} else {
				printf("\"sendN\" error: %d \n", errno);
				return -3;
			}			
		} else {
			totslen+=slen;
        }
	} while ((totslen<(ssize_t)len) && ((retries--)>0));
		
	return totslen;
}

ssize_t CLAppNetworkSocket::rcvData(int s, unsigned char *buf, size_t len, int flags, int retries)
{
	ssize_t rlen;
	ssize_t totrlen=0;
	int rFlags = flags;

	if (len<=0)
		return len;

	do
	{
		rlen=recv(s, (unsigned char *)buf+totrlen, (size_t)(len-totrlen), rFlags);
		if (rlen<0)
		{
			if (errno==EINTR||errno==EAGAIN) {
				usleep(10);
			} else {
				return -1;
			}
		}
		else
			totrlen += rlen;
		usleep(1);
	} while ((totrlen<(ssize_t)len) && ((retries--)>0));
	
	return totrlen;
}


int CLAppNetworkSocket::findFirstPacket(int Start, char *pFromStr, int FromStrSize, char *pObjectStr, int ObjectStrSize)
{
	int i = 0, j = 0;

	if (Start > FromStrSize) 
		return -1;

	if (ObjectStrSize > FromStrSize)
		return -1;

	for (i = Start; i < FromStrSize; i++) {

		if (pFromStr[i] == pObjectStr[0]) {
			int isFind = 65535;

			if (i > FromStrSize - ObjectStrSize) {
				isFind = -1;
				return -1;
			}

			for (j = 0; j < ObjectStrSize; j++) {

				if (pFromStr[i + j] != pObjectStr[j]) {
					isFind = -1;
					break;
				}
			}
			
			if (isFind == 65535)
				return i;
		}
	}
	return -1;
}

bool CLAppNetworkSocket::convertHexStr2Dec(char *HexStr, unsigned long HexStrCnt, long &Nbr)
{
	unsigned long i = 0;

	for (i = 0; i < HexStrCnt; i++)
	{
		if (*(HexStr + i) < '0' || *(HexStr + i) > '9')
		{
			if (*(HexStr + i) < 'A' || *(HexStr + i) > 'F')
			{
				if (*(HexStr + i) < 'a' || *(HexStr + i) > 'f')
				{
					if (*(HexStr + i) != '-' && *(HexStr + i) != '\0')
					{
						return false;
					}
				}
			}
		}
	}

	char *stopstring;
	Nbr = strtol(HexStr, &stopstring, 16);
	return true;
}

ssize_t CLAppNetworkSocket::sendAfterMakePacket(int sock_fd, int resol_type, int mode_type, unsigned char *szData, int data_size)
{
	char	pktBufs[MAX_PACKET_SLICE + 2];	
	char	strBufs[5];
	long 	seq_sz = data_size;
	int 	r, w = 0, packet_size = 0;
	int		iflag = (MSG_NOSIGNAL | MSG_DONTWAIT);
	int		iretry = 20;
	short 	seq_nr = 0, seq_nr_pre = 0;

	if( data_size == 0 || szData == NULL) {
		return -1;
	}

	while( seq_sz > 0)	
	{		
		w  = (seq_sz >= MAX_STREAM_SLICE ) ? MAX_STREAM_SLICE : seq_sz;
		packet_size = 0;

		// Start Code
		memcpy(pktBufs + packet_size, SCODE, SCODE_SIZE);
		packet_size += SCODE_SIZE;

		// Packet Code
		sprintf(strBufs, "%03x", (PACKET_PARAMS_SIZE + w));
		memcpy(pktBufs + packet_size, strBufs, PACKET_SIZE);
		packet_size += PACKET_SIZE;

		// resol = width(2bype) + height(2bype) 
		memcpy(pktBufs + packet_size, (char *)&resol_type, RESOL_SIZE);
		packet_size += RESOL_SIZE;

		// channel + mode 
		memcpy(pktBufs + packet_size, (char *)&mode_type, MODE_SIZE);
		packet_size += MODE_SIZE;

		// Packet Sequence Number
		memcpy(pktBufs + packet_size, &seq_nr, SEQ_NUMB_SIZE);
		seq_nr = (seq_nr + 1);
		packet_size += SEQ_NUMB_SIZE;

		// Total Data Size
		memcpy(pktBufs + packet_size, &data_size, DATA_SIZE);
		packet_size += DATA_SIZE;

		// Data 
		memcpy(pktBufs + packet_size , szData , w);
		packet_size += w;

		// End Code
		memcpy(pktBufs + packet_size, ECODE, ECODE_SIZE);
		packet_size += ECODE_SIZE;
		
		if((seq_nr_pre+1) != seq_nr) {	}		
		seq_nr_pre = seq_nr;
		
		if ((r = sndData(sock_fd, (unsigned char*)pktBufs, packet_size, 
							iflag /* 0 */, iretry/*2*/)) < 0){
			return r;
		}

		seq_sz -= w ;	
		szData += w ;	
	} // End of while
	
	return 0;
}

ssize_t CLAppNetworkSocket::analysisAfterRecvPacket(int sock_fd, 
					char *mode_type, unsigned char *szData, int *size)
{	
	char	pktBufs[MAX_PACKET_SLICE + 2];
	long 	slice_size = 0;
	int		recv_len = 0, packet_size = 0;
	int 	total_data_size = 0;
	int 	read_buf_size = 0, data_size = 0;
	int 	timeout_cnt = 0;

	// Read '_STA' code
	recv_len = rcvData(sock_fd, (unsigned char*)&pktBufs[0], SCODE_SIZE, 0,  1);
	if( strncmp((char*) &pktBufs[0], SCODE, SCODE_SIZE))
	{
		printf("Failed, could not find start code \n");
		return -1;
	}
	packet_size = SCODE_SIZE;
	
	// Read Packet Size
	recv_len = rcvData(sock_fd, (unsigned char*)&pktBufs[packet_size], PACKET_SIZE, 0, 100);
	if( !convertHexStr2Dec(&pktBufs[packet_size], PACKET_SIZE, slice_size) )
	{
		printf("Failed, could not find packet size \n");
		return -1;
	}
	packet_size += recv_len;

	// Read Resolution 
	recv_len = rcvData(sock_fd, (unsigned char*)&pktBufs[packet_size], RESOL_SIZE, 0, 100);	
	if( recv_len != RESOL_SIZE)
	{
		printf("Failed, could not get resolution \n");
		return -1;
	}
	packet_size += recv_len;

	// Read Mode 
	recv_len = rcvData(sock_fd, (unsigned char*)&pktBufs[packet_size], MODE_SIZE, 0, 100);	
	if( recv_len != MODE_SIZE)
	{
		printf("Failed, could not get mode \n");
		return -1;
	}
	if(mode_type != NULL) {
		memcpy(mode_type, &pktBufs[packet_size], MODE_SIZE);
	}
	packet_size += recv_len;

	// Read Packet Sequence Number
	recv_len = rcvData(sock_fd, (unsigned char*)&pktBufs[packet_size], SEQ_NUMB_SIZE, 0, 100);	
	if( recv_len != SEQ_NUMB_SIZE)
	{
		printf("Failed, could not get sequence number \n");
		return -1;
	}
	packet_size += recv_len;

	// Read Total Data size
	recv_len = rcvData(sock_fd, (unsigned char *)&pktBufs[packet_size], DATA_SIZE, 0, 100);
	if( recv_len != DATA_SIZE )
	{
		printf("Failed, could not get data size \n");
		return -1;
	}
	memcpy(&total_data_size, &pktBufs[packet_size], DATA_SIZE);
	packet_size += recv_len;

	// Read Data
	data_size = (slice_size - PACKET_PARAMS_SIZE);

	data_size = (data_size >= MAX_PACKET_SLICE)?MAX_PACKET_SLICE:data_size;
	do {
		ioctl(sock_fd, FIONREAD, &read_buf_size);

		if ((int)read_buf_size >= data_size) {
			break;
		} else {
			usleep(5000);
		}
	} while (timeout_cnt++ < 20);

	if (timeout_cnt >= 20) {
		printf("Failed, could not received to data \n");
		return -1;
	}

	// Read Data
	recv_len = rcvData(sock_fd, (unsigned char *)&pktBufs[packet_size], 
							data_size, 0, 100);
	if( recv_len != data_size )
	{
		printf("Failed, could not get data \n");
		return -1;
	}

	if(szData != NULL && size != NULL){
		memcpy(szData, &pktBufs[packet_size], (slice_size - PACKET_PARAMS_SIZE));
		*size = (slice_size - PACKET_PARAMS_SIZE);
	}
	packet_size += recv_len;

	// Read End Code
	recv_len = rcvData(sock_fd, (unsigned char*)&pktBufs[packet_size], ECODE_SIZE, 0, 100);
	if( strncmp((char*)&pktBufs[packet_size], ECODE, ECODE_SIZE))
	{
		printf("Failed, could not find end code \n");
		return -1;
	}	
	packet_size += recv_len;

	return total_data_size;
}
