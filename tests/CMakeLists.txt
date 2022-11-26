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

add_executable(
	sock_tests
	${SOURCE_FILES}
	socket.cpp
)

target_link_libraries(
	sock_tests
	GTest::gtest_main
	${EXTRA_LIBS}
)

set_property(TARGET sock_tests PROPERTY CXX_STANDARD 20)

target_include_directories(
	sock_tests
	PUBLIC
		${CMAKE_SOURCE_DIR}/include
)

include(GoogleTest)
gtest_discover_tests(sock_tests)

#~GoogleTest
