cmake_minimum_required(VERSION 3.23)

project(sock VERSION 0.0.1 LANGUAGES CXX)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
	set(SOCK_STANDALONE TRUE)
endif()

option(SOCK_BUILD_TESTS "Build test programs" ${SOCK_STANDALONE})

list(
	APPEND
	SOURCE_FILES
	${CMAKE_SOURCE_DIR}/src/socket.cpp
	${CMAKE_SOURCE_DIR}/src/socket_factory.cpp
	${CMAKE_SOURCE_DIR}/src/utils.cpp
)

add_library(sock SHARED ${SOURCE_FILES})

if (WIN32)
	list(APPEND EXTRA_LIBS wsock32)
	list(APPEND EXTRA_LIBS ws2_32)
endif()

set_property(TARGET sock PROPERTY CXX_STANDARD 20)

target_include_directories(
	sock
	PUBLIC
		${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(
	sock
	PUBLIC
		${EXTRA_LIBS}
)

if (SOCK_BUILD_TESTS)
	message('aaaa')
    add_subdirectory(tests)
endif()
