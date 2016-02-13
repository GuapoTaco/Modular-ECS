
include_directories(${MOD_ECS_INCLUDE_DIRS})

add_executable(mod_ecs_test 
	num_components.cpp
)

target_compile_features(mod_ecs_test PUBLIC
	cxx_variable_templates
)