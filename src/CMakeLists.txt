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

#build event
set(EVENT__DISABLE_SAMPLES ON CACHE BOOL "flags")
set(EVENT__DISABLE_REGRESS ON CACHE BOOL "flags")
set(EVENT__DISABLE_TESTS ON CACHE BOOL "flags")
set(EVENT__DISABLE_THREAD_SUPPORT ON CACHE BOOL "flags")
set(EVENT__DISABLE_OPENSSL ON CACHE BOOL "flags")
set(EVENT__DISABLE_BENCHMARK ON CACHE BOOL "flags")
set(EVENT__DISABLE_CLOCK_GETTIME ON CACHE BOOL "flags")
set(EVENT__DISABLE_DEBUG_MODE ON CACHE BOOL "flags")
set(EVENT__BUILD_SHARED_LIBRARIES ${BASICLIB_BUILD_SHARED})
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../3rd/libevent libevent)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../3rd/libevent/include)
include_directories(${PROJECT_BINARY_DIR}/src/libevent/include)

set(base_src
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/ZIPC/base/inc/IPCDefine.h

	${CMAKE_CURRENT_SOURCE_DIR}/ZIPC/base/conf/IPCConfRead.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/ZIPC/base/conf/IPCConfRead.h

	${CMAKE_CURRENT_SOURCE_DIR}/ZIPC/base/log/IPCLog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/ZIPC/base/log/IPCLog.h

    ${CMAKE_CURRENT_SOURCE_DIR}/ZIPC/base/mem/SmartBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/ZIPC/base/mem/SmartBuffer.h

    ${CMAKE_CURRENT_SOURCE_DIR}/ZIPC/base/mem/BitStream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/ZIPC/base/mem/BitStream.h

    ${CMAKE_CURRENT_SOURCE_DIR}/ZIPC/base/time/OnTimer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/ZIPC/base/time/OnTimer.h
)

set(net_src
    ${CMAKE_CURRENT_SOURCE_DIR}/ZIPC/net/NetThread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ZIPC/net/NetThread.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ZIPC/net/TcpThreadSocket.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ZIPC/net/TcpThreadSocket.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ZIPC/net/TcpThreadSocketSession.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ZIPC/net/TcpThreadSocketSession.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ZIPC/net/TcpThreadSocketClient.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ZIPC/net/TcpThreadSocketClient.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ZIPC/net/TcpThreadAccept.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ZIPC/net/TcpThreadAccept.h

    ${CMAKE_CURRENT_SOURCE_DIR}/ZIPC/net/TcpAfx.h

    ${CMAKE_CURRENT_SOURCE_DIR}/ZIPC/net/NetMgr.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/ZIPC/net/NetMgr.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ZIPC/net/NetBaseObject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/ZIPC/net/NetBaseObject.h

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

