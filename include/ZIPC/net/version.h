/***********************************************************************************************
// filename : 	version.h
// describ:		ipc version
// version:   	1.0V
// Copyright:   Copyright (c) 2018 zilliz
************************************************************************************************/
#pragma once

/*
v1.1.0
1.finish server & client
2.add usecase for test

v1.2.0
1.fix future bug, delete all use future.get() wait, it will deadlock in the netthread
2.change lock to std::mutex
3.change some state param to volatile
4.change callback function from function point to std::function
5.change close logic, set the sign to close
6.change shutdown logic, while(IsShutdown()) to check

v1.3.0
1.change usecase to create senddata in memory
2.change usecase after auth, client & server send data sametime

v1.4.0
1.add version support

*/

#define IPC_VERSION     1.4.0



