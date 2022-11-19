#include "sock/utils.hpp"
#include <iostream>
#include <string>
#include <string_view>

void sock::log_error(const std::string_view message)
{
	std::cout << message << " [" << std::to_string(WSAGetLastError()) << "]"
	          << "\n";
}
