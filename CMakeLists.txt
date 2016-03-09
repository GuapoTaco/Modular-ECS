cmake_minimum_required(VERSION 3.0)

find_package(Boost REQUIRED)

find_package(Hana REQUIRED)

set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(VERSION_PATCH 0)

set(BUILD_DOCS OFF CACHE BOOL "Should the doxygen documentation be built?")
set (MOD_ECS_TEST ON CACHE BOOL "Compile the tests?")

set(MOD_ECS_INCLUDE_DIRS "${Boost_INCLUDE_DIR};${Hana_INCLUDE_DIR};${CMAKE_CURRENT_SOURCE_DIR}/include/")

set(MOD_ECS_HEADERS 
	include/Entity.h
	include/Manager.h
	include/MiscMetafunctions.h
	include/SegmentedMap.h
)

if(${MOD_ECS_TEST})
	add_subdirectory(test)
endif(${MOD_ECS_TEST})

if(${BUILD_DOCS})
	
	find_package(Doxygen REQUIRED)
	
	set(doxy_in Doxyfile.in)
	set(doxy ${CMAKE_BINARY_DIR}/Doxyfile)
	
	configure_file(${doxy_in} ${doxy})
	
	add_custom_target(
		docs
		COMMAND ${DOXYGEN_EXECUTABLE}
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
		COMMENT "Generating Doxygen documentation...."
		DEPENDS ${MOD_ECS_HEADERS}
	)
	
endif()
