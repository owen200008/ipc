
set(ipc_test_src
	base/log/IPCLogTest.cpp
    base/time/OnTimerTest.cpp
	main.cpp)

set(ipc_client_src client.cpp tcpdefinetest.cpp tcpdefinetest.h)
set(ipc_server_src server.cpp tcpdefinetest.cpp tcpdefinetest.h)

add_executable(ipc_test ${ipc_test_src})
target_link_libraries(ipc_test ${unittest_libs})

add_executable(ipc_client ${ipc_client_src})
target_link_libraries(ipc_client ${unittest_libs})

add_executable(ipc_server ${ipc_server_src})
target_link_libraries(ipc_server ${unittest_libs})