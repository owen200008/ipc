#!/bin/bash


if [[ ! -d cmake_build ]]; then
        mkdir cmake_build
fi

cd cmake_build

INSTALL_PREFIX=`pwd`/../../zdb_common

echo installing zipc to $INSTALL_PREFIX

BUILD_UNIT_TEST_OPT=""
BUILD_BENCHMARK_OPT=""
THIRD_PARTY_HOME=${INSTALL_PREFIX}
BUILD_TYPE="Debug"


while true ; do
        case "$1" in
               	-u|--unittest) echo "building unit tests" ; BUILD_UNIT_TEST_OPT="-DBUILD_UNIT_TEST=ON"; shift ;;
               	-b|--benchmark) echo "building benchmark" ; BUILD_BENCHMARK_OPT="-DBUILD_BENCHMARK=ON"; shift ;;
		-t|--build_type) BUILD_TYPE=$2 ; shift 2 ;;
                -h|--3rd_home) THIRD_PARTY_HOME=$2 ; shift 2 ;;
                --) shift ; break ;;
		*) break ;;
        esac
done


CMAKE_CMD="cmake -DTHIRD_PARTY_HOME=${THIRD_PARTY_HOME} \
-DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} \
-DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
${BUILD_UNIT_TEST_OPT} \
${BUILD_BENCHMARK_OPT} \
../"

echo ${CMAKE_CMD}

${CMAKE_CMD}
make -j8 && make uninstall && make install

