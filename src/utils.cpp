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

#else

std::string sock::error()
{
	return strerror(errno);
}

#endif
