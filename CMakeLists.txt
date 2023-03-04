cmake_minimum_required(VERSION 3.23)

project(sock VERSION 1.0.0 LANGUAGES CXX)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
	set(SOCK_STANDALONE TRUE)
endif()

option(SOCK_BUILD_TESTS "Build test programs" ${SOCK_STANDALONE})

list(
	APPEND
	SOURCE_FILES
	${PROJECT_SOURCE_DIR}/src/socket.cpp
	${PROJECT_SOURCE_DIR}/src/socket_factory.cpp
	${PROJECT_SOURCE_DIR}/src/utils.cpp
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

# TESTS
if (SOCK_BUILD_TESTS)
	# https://google.github.io/googletest/quickstart-cmake.html
	include(FetchContent)
	FetchContent_Declare(
		googletest
		GIT_REPOSITORY https://github.com/google/googletest.git
		GIT_TAG release-1.12.1
	)

	# For Windows: Prevent overriding the parent project's compiler/linker settings
	set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
	FetchContent_MakeAvailable(googletest)

	enable_testing()

	add_custom_target(sock_tests)

	add_executable(
		sock_tests_executable
		tests/socket.cpp
	)

	set_property(TARGET sock_tests_executable PROPERTY CXX_STANDARD 20)

	target_link_libraries(
		sock_tests_executable
		sock
		GTest::gtest_main
		${EXTRA_LIBS}
	)

	add_dependencies(sock_tests sock_tests_executable)

	include(GoogleTest)
	gtest_discover_tests(sock_tests_executable)

endif()
#~TESTS
