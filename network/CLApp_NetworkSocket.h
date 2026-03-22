/**********************************************************
* Copyright(C) 2022 CAN-Lab Inc.
* Garnet Kim <garnet@can-lab.co.kr> - 2022.10.31
* FileName : CLApp_NetworkSocket.h
* Auther   : Garnet Kim
* Modify   : 2022-10-31
**********************************************************/
#ifndef _CLAPP_NETWORK_SOCKET_H_
#define _CLAPP_NETWORK_SOCKET_H_

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/poll.h>
#include <time.h>
#include <errno.h>

/*
	Network protocol packet
	----------------------------------------------------------------------------------------------
	| SCODE(4) | PACKET_SIZE(4) | TYPE_SIZE(2) | SEQ_NUMB(2) | TOTAL_DATA_SIZE(4) | DATA | ECODE |
	----------------------------------------------------------------------------------------------

	Control packet (This is the DATA of the Network protocol packet)
	----------------------------------------------------------------------------------------------
	| String Command(4) | param length(2) | param
	  - 1. STRT  : Stream Start
	  - 2. STOP  : Stream Stop
	  - 3. CLSE  : Socket Close
	  - 4. LIVE  : Set Live format (raw, yuv, bmp)
	  - 4. AESP  : AE Set params ( PC -> Board )
	  - 5. AEGP  : AE Get params ( PC <- Board )
	  - 6. FWUP  : FW file download (PC -> Board )
	----------------------------------------------------------------------------------------------
*/
#define	SCODE_SIZE					4
#define	ECODE_SIZE					4
#define	PACKET_SIZE					4
#define	RESOL_SIZE					4	/* width(16bits) + height(16bits) */
#define	MODE_SIZE					2	/* channel(8bits) + mode(8bits) */
#define SEQ_NUMB_SIZE				2
#define DATA_SIZE					4
#define PACKET_PARAMS_SIZE			(SCODE_SIZE + ECODE_SIZE + PACKET_SIZE + RESOL_SIZE + MODE_SIZE + SEQ_NUMB_SIZE + DATA_SIZE)

#define MAX_STREAM_SLICE		(1400 * 32)
#define MAX_PACKET_SLICE		(MAX_STREAM_SLICE + PACKET_PARAMS_SIZE)

#define	SCODE					"STA"
#define	ECODE					"END"


class CLAppNetworkSocket
{
public:
	static int createSocket(int port);
	static ssize_t sndData(int s, const unsigned char *msg, size_t len, int flags, int retries);
	static ssize_t rcvData(int s, unsigned char *buf, size_t len, int flags, int retries);
	static int findFirstPacket(int Start, char *pFromStr, int FromStrSize, char *pObjectStr, int ObjectStrSize);
	static bool convertHexStr2Dec(char *HexStr, unsigned long HexStrCnt, long &Nbr);	
	static ssize_t sendAfterMakePacket(int sock_fd, int resol_type, int mode_type, unsigned char *szData, int data_size);
    static ssize_t analysisAfterRecvPacket(int sock_fd, char *mode_type, unsigned char *szData, int *size);
};

#endif // _CLAPP_NETWORK_SOCKET_H_
