/**********************************************************
* Copyright(C) 2022 CAN-Lab Inc.
* Garnet Kim <garnet@can-lab.co.kr> - 2022.10.04
* FileName : CLApp_Define.h
* Auther   : Garnet Kim
* Modify   : 2022-10-04
**********************************************************/
#ifndef _CLAPP_TYPES_H__
#define _CLAPP_TYPES_H__

// Main Task
const int M_MODULE_MAIN_STREAM_START			= 1001;
const int M_MODULE_MAIN_STREAM_STOP 			= 1002;
const int M_MODULE_MAIN_STREAM_CLOSE 			= 1003;
const int M_MODULE_MAIN_CTRL_PARAMS 			= 1004;
const int M_MODULE_MAIN_REC_STATUS  			= 1005;

// Network Task
#define KEY_MQ_MAIN_TASK    			0x20000001
#define KEY_MQ_NETWORK_TASK 			0x20000002


#endif /* _CLAPP_TYPES_H__ */
