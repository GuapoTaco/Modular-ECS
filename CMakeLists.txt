cmake_minimum_required(VERSION 3.0)

project(ModularECS VERSION 0.1.0)

# Options
option (MOD_ECS_TEST "Compile the tests?" ON)

find_package(Boost REQUIRED)

set(MOD_ECS_HEADERS
	include/ecs/entity.h
	include/ecs/manager.h
	include/ecs/misc_metafunctions.h
	include/ecs/segmented_map.h
)

add_library(ModularECS INTERFACE)
target_include_directories(ModularECS INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(ModularECS INTERFACE Boost::boost)
target_compile_options(ModularECS INTERFACE -std=c++17)

if(${MOD_ECS_TEST})
	enable_testing()
	add_subdirectory(test)
endif(${MOD_ECS_TEST})


find_package(Doxygen)

if(${DOXYGEN_FOUND})

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
