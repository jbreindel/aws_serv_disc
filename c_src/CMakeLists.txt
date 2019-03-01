cmake_minimum_required(VERSION 3.2)
project(aws_serv_disc)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(BUILD_SHARED_LIBS ON)

execute_process(COMMAND erl -noshell -s init stop -eval "io:format(\"~ts/erts-~ts/include/\", [code:root_dir(), erlang:system_info(version)])." OUTPUT_VARIABLE ERTS_INCLUDE_DIR)
execute_process(COMMAND erl -noshell -s init stop -eval "io:format(\"~ts\", [code:lib_dir(erl_interface, include)])." OUTPUT_VARIABLE ERL_INTERFACE_INCLUDE_DIR)
execute_process(COMMAND erl -noshell -s init stop -eval "io:format(\"~ts\", [code:lib_dir(erl_interface, lib)])." OUTPUT_VARIABLE ERL_INTERFACE_LIB_DIR)

find_package(AWSSDK REQUIRED COMPONENTS servicediscovery)

if (WIN32)
	add_definitions(-DUSE_IMPORT_EXPORT)
endif()

if (APPLE)
	add_definitions(-DUNDER_MACOS)
endif()

include_directories(${ERTS_INCLUDE_DIR} ${ERL_INTERFACE_INCLUDE_DIR})
link_directories(${ERL_INTERFACE_LIB_DIR})

add_library(aws_serv_disc aws_serv_disc.cpp)
set_target_properties(aws_serv_disc PROPERTIES
	LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../priv
)
target_link_libraries(aws_serv_disc
	${AWSSDK_LINK_LIBRARIES}
	erl_interface
	ei
)
