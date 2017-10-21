
include_directories(${MOD_ECS_INCLUDE_DIRS})

find_package(Boost REQUIRED unit_test_framework)

set(TESTS
#	num_components.cpp
	metafunctions.cpp
	manager_metafunctions.cpp
)

foreach(TEST ${TESTS})

	get_filename_component(TEST_NAME ${TEST} NAME_WE)

	add_executable(${TEST_NAME} ${TEST})

	target_link_libraries(${TEST_NAME}
		${Boost_FILESYSTEM_LIBRARY}
		${Boost_SYSTEM_LIBRARY}
		${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
		ModularECS
	)

	add_test(${TEST_NAME} ${TEST_NAME}
		COMMAND ${TEST_NAME}
	)

	target_compile_definitions(${TEST_NAME} PUBLIC -DBOOST_TEST_MAIN -DBOOST_TEST_DYN_LINK -DBOOST_TEST_MODULE=${TEST_NAME})

	add_test(${TEST_NAME} ${TEST_NAME}
		COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME}
	)

endforeach()

