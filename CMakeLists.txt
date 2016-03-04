cmake_minimum_required(VERSION 3.0)

find_package(Boost REQUIRED)

find_package(Hana REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftemplate-backtrace-limit=0")

set(MOD_ECS_INCLUDE_DIRS "${Boost_INCLUDE_DIR};${Hana_INCLUDE_DIR};${CMAKE_CURRENT_SOURCE_DIR}/include/")

set (MOD_ECS_TEST ON CACHE BOOL "Compile the tests?")

if(${MOD_ECS_TEST})
	add_subdirectory(test)
endif(${MOD_ECS_TEST})

