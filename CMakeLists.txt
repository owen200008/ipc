cmake_minimum_required(VERSION 3.8)

project(zipc_library LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -fPIC")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -fPIC")
endif()

set(ZIPC_LIBRARY_INCLUDE ${PROJECT_SOURCE_DIR}/include)
set(ZIPC_LIBRARY_SRC ${PROJECT_SOURCE_DIR}/src)


include_directories(${ZIPC_LIBRARY_INCLUDE})
include_directories(${ZIPC_LIBRARY_SRC})

add_subdirectory(${PROJECT_SOURCE_DIR}/src src)

if (BUILD_UNIT_TEST)
	add_subdirectory(unittest)
endif(BUILD_UNIT_TEST)
unset(BUILD_UNIT_TEST CACHE)

if (BUILD_BENCHMARK)
	add_subdirectory(benchmark)
endif(BUILD_BENCHMARK)
unset(BUILD_BENCHMARK CACHE)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/zipc DESTINATION include)

install(FILES
        ${PROJECT_BINARY_DIR}/src/libzipc.a
        DESTINATION lib)

# uninstall target
if(NOT TARGET uninstall)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY)

    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()


