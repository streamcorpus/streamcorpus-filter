#
SET (CMAKE_BUILD_TYPE Debug)
SET (CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

cmake_minimum_required(VERSION 2.6.4)
add_definitions(-DHAVE_NETINET_IN_H)
add_definitions(-DHAVE_INTTYPES_H)

set(CMAKE_CXX_FLAGS "-std=c++0x")

###  BOOST
find_package (Boost REQUIRED COMPONENTS filesystem program_options system regex)
include_directories(${Boost_INCLUDE_DIR})

###  THIFT
find_library (THRIFT_LIB thrift)
find_path (
	THRIFT_INCLUDE_DIR protocol/TBinaryProtocol.h
	/usr/include/thrift
	/opt/local/include
	/usr/local/include
	/usr/include
	/usr/local/include/thrift
)
#message(STATUS "Include file detected: [${THRIFT_INCLUDE_DIR}].")
include_directories(${THRIFT_INCLUDE_DIR})


###  THRIFT STREAMCORPUS
# SC - defined in Makefile
include_directories	(${SC})
link_directories	(${SC})

###  THRIFT STREAMCORPUS-FILTER
include_directories	(build)
add_library (
	filternames
	filternames_constants.cpp 
	filternames_constants.h
	filternames_types.cpp 
	filternames_types.h
)

###  TARGET
add_executable(filter filter.cc multisearch.h multisearch_naive.cc)
target_link_libraries(filter  ${Boost_LIBRARIES} ${THRIFT_LIB} streamcorpus filternames)
