
set(ipc_test_src base/mem/SmartBufferTest.cpp
	base/log/IPCLogTest.cpp
	main.cpp)

#set(ipc_client_src client.cpp)
#set(ipc_server_src server.cpp)

add_executable(ipc_test ${ipc_test_src})
target_link_libraries(ipc_test zipc)

#add_executable(ipc_client ${ipc_client_src})
#target_link_libraries(ipc_client ${unittest_libs})

#add_executable(ipc_server ${ipc_server_src})
#target_link_libraries(ipc_server ${unittest_libs})