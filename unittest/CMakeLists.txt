
set(ZIPCLIB_UNITTEST_INCLUDE ${PROJECT_SOURCE_DIR}/unittest ${PROJECT_SOURCE_DIR}/../include)

include_directories(${ZIPCLIB_UNITTEST_INCLUDE})

set(unittest_libs
    gtest pthread zipc)

set(unittest_deps zipc)

add_subdirectory(ZIPC)
