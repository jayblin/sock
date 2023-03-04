#include "sock/utils.hpp"
#include <cstring>
#include <iostream>
#include <string>
#include <string_view>

#if defined(WIN32) || defined(_WIN32) || \
    defined(__WIN32) && !defined(__CYGWIN__)

std::string sock::error()
{
	return std::to_string(WSAGetLastError());
}

/* void sock::log_error(const std::string_view prepend) */
/* { */
/* 	std::cout << prepend << " [" << std::to_string(WSAGetLastError()) << "]" << std::endl; */
/* } */

#else

/* std::string sock::error() */
/* { */
/* 	return strerror(errno); */
/* } */

/* void sock::log_error(const std::string_view prepend) */
/* { */
/* 	std::cout << prepend <<  " [" << std::string{strerror(errno)} << "]" << std::endl; */
/* } */

#endif
