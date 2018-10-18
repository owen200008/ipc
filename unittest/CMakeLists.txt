
set(ZIPCLIB_UNITTEST_INCLUDE ${PROJECT_SOURCE_DIR}/unittest ${PROJECT_SOURCE_DIR}/../include)

include_directories(${ZIPCLIB_UNITTEST_INCLUDE})
if(MSVC)
	set(unittest_libs zipc event ws2_32.lib)
else()
	set(unittest_libs pthread zipc event)
endif()
add_subdirectory(ZIPC)
