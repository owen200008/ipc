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

#define BASIC_NET_CLOSE_WAITCLOSE               (NET_ERROR | 0x0010)            //wait close

#define BASIC_NET_SOCKET_ERROR					(NET_ERROR | 0x0020)			//socket error


//
#define BASIC_NETCODE_SUCC					0x00000001				//
#define BASIC_NETCODE_CLOSE_REMOTE			0x00000002				//
#define BASIC_NETCODE_FILTER_HANDLE			0x00000004				//
#define BASIC_NETCODE_FILTER_ERROR			0x00000008				//

#define BASIC_NETCODE_CONNET_GENERR         0x00000010
#define BASIC_NETCODE_CONNET_FAIL			0x00000020				//

//handle shake
#define BASIC_NET_HC_RET_HANDSHAKE			0x00000001		//

//shake success
#define BASIC_NET_HR_RET_HANDSHAKE			0x00000001		//

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define TIL_RESET_MASK					0xFF000000		//

#define TIL_SS_LINK						0x0000000F		//
#define TIL_SS_IDLE						0x00000000		//
#define TIL_SS_CONNECTING				0x00000001		//
#define TIL_SS_CONNECTED				0x00000002		//
#define TIL_SS_LISTENING				0x00000003		//


#define TIL_SS_SHAKEHANDLE_MASK			0x0000F000		//
#define TIL_SS_SHAKEHANDLE_TRANSMIT		0x00001000		//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define ADDRESS_MAX_LENGTH		64