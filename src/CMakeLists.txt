macro(source_group_by_dir source_files)
    if(MSVC)
        set(sgbd_cur_dir ${PROJECT_SOURCE_DIR})
        foreach(sgbd_file ${${source_files}})
			set(samefilename FALSE)
			while (NOT samefilename)
				string(REGEX REPLACE "/[^/\\.]*/\\.\\./" "/" sgbd_fpath ${sgbd_file})
				string(COMPARE EQUAL ${sgbd_fpath} ${sgbd_file} samefilename)
				set(sgbd_file ${sgbd_fpath})
			endwhile()
            string(REGEX REPLACE "${sgbd_cur_dir}/" "" sgbd_fpath ${sgbd_file})
			string(REGEX REPLACE "(.*)/" "" sgbd_group_name ${sgbd_fpath})
			string(COMPARE EQUAL ${sgbd_fpath} ${sgbd_group_name} sgbd_nogroup)
			if(sgbd_nogroup)
				set(sgbd_real_group_name "\\")
			else()
				string(REGEX REPLACE "/${sgbd_group_name}" "" sgbd_real_group_name ${sgbd_fpath})
				string(REPLACE "/" "\\" sgbd_real_group_name ${sgbd_real_group_name})
			endif()
			source_group(${sgbd_real_group_name} FILES ${sgbd_file})
        endforeach(sgbd_file)
    endif(MSVC)
endmacro(source_group_by_dir)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../3rd/libevent/include)

set(base_src
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/ZIPC/base/inc/IPCDefine.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/ZIPC/base/misc/fastdelegate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/ZIPC/base/misc/fastdelegatebind.h

	${CMAKE_CURRENT_SOURCE_DIR}/ZIPC/base/conf/IPCConfRead.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/ZIPC/base/conf/IPCConfRead.h

	${CMAKE_CURRENT_SOURCE_DIR}/ZIPC/base/log/IPCLog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/ZIPC/base/log/IPCLog.h

    ${CMAKE_CURRENT_SOURCE_DIR}/ZIPC/base/mem/SmartBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/ZIPC/base/mem/SmartBuffer.h

    ${CMAKE_CURRENT_SOURCE_DIR}/ZIPC/base/mt/SpinLock.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/ZIPC/base/mt/SpinLock.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ZIPC/base/mt/Mutex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/ZIPC/base/mt/Mutex.h

    ${CMAKE_CURRENT_SOURCE_DIR}/ZIPC/base/sys/SysInfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/ZIPC/base/sys/SysInfo.h
)

set(net_src
    ${CMAKE_CURRENT_SOURCE_DIR}/ZIPC/net/NetMgr.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ZIPC/net/NetMgr.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ZIPC/net/NetThread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ZIPC/net/NetThread.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ZIPC/net/TcpSocket.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ZIPC/net/TcpSocket.h

    ${CMAKE_CURRENT_SOURCE_DIR}/ZIPC/net/TcpSessionNotify.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/ZIPC/net/TcpSessionNotify.h
        ${CMAKE_CURRENT_SOURCE_DIR}/ZIPC/net/TcpClient.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/ZIPC/net/TcpClient.h
        ${CMAKE_CURRENT_SOURCE_DIR}/ZIPC/net/TcpServer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/ZIPC/net/TcpServer.h

    ${CMAKE_CURRENT_SOURCE_DIR}/../include/ZIPC/net/TcpDefine.h
)

add_compile_options(-fPIC)

set(TotalSourceData ${base_src} ${net_src})
source_group_by_dir(TotalSourceData)
add_library(zipc STATIC
	${TotalSourceData}
)

set_target_properties(zipc PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(zipc)

