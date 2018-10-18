/***********************************************************************************************
// filename : 	TcpDefine.h
// describ:		tcp define
// version:   	1.0V
************************************************************************************************/
#pragma once

/////////////////////////////////////////////////////////////////////////////////////////////
#define NET_ERROR								(0xE0000000 | 0x00070000)	    //net error

#define BASIC_NET_OK							0								//success

#define BASIC_NET_GENERIC_ERROR					(NET_ERROR | 0x0001)			//generic error
#define BASIC_NET_INVALID_ADDRESS				(NET_ERROR | 0x0002)			//connect address error
#define BASIC_NET_ALREADY_CONNECT				(NET_ERROR | 0x0003)			//
#define BASIC_NET_CONNECTING_ERROR				(NET_ERROR | 0x0004)			//
#define BASIC_NET_NO_CONNECT					(NET_ERROR | 0x0005)			//
#define BASIC_NET_CLOSE_WAITCLOSE               (NET_ERROR | 0x0010)            //wait close
#define BASIC_NET_SOCKET_ERROR					(NET_ERROR | 0x0020)			//socket error
#define BASIC_NET_SOCKET_NOEXIST				(NET_ERROR | 0x0021)			//socket no exist

#define BASIC_NET_BIND_ERROR					(NET_ERROR | 0x0032)			//bind error
#define BASIC_NET_LISTEN_ERROR					(NET_ERROR | 0x0033)			//listen error

//
#define BASIC_NETCODE_SUCC					0x00000001				//
#define BASIC_NETCODE_CLOSE_REMOTE			0x00000002				//

//handle shake
#define BASIC_NET_HC_RET_HANDSHAKE			0x00000001		//

//shake success
#define BASIC_NET_HR_RET_HANDSHAKE			0x00000001		//

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


