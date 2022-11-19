cmake_minimum_required(VERSION 3.23)

project(sock VERSION 0.0.1 LANGUAGES CXX)

list(
	APPEND
	SOURCE_FILES
	src/socket.cpp
	src/socket_factory.cpp
	src/utils.cpp
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
		sql
		${EXTRA_LIBS}
)

# GoogleTest

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

add_custom_target(tests)

add_executable(
	tests_executable
	${SOURCE_FILES}
	tests/socket.cpp
)

target_link_libraries(
	tests_executable
	GTest::gtest_main
	${EXTRA_LIBS}
)

set_property(TARGET tests_executable PROPERTY CXX_STANDARD 20)

target_include_directories(
	tests_executable
	PUBLIC
	${CMAKE_SOURCE_DIR}/include/local
	${CMAKE_SOURCE_DIR}/include
)

add_dependencies(tests tests_executable)

include(GoogleTest)
gtest_discover_tests(tests_executable)

#~GoogleTest
