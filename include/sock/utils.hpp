#ifndef SOCK_UTILS_H_
#define SOCK_UTILS_H_

#include <string_view>
#if defined(WIN32) || defined(_WIN32) || \
    defined(__WIN32) && !defined(__CYGWIN__)

# include <winsock2.h>
# include <ws2tcpip.h>

#else

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#endif

namespace sock
{
	enum class Domain
	{
		INET,
		UNSPEC,
	};

	enum class Type
	{
		STREAM,
	};

	enum class Protocol
	{
		TCP,
	};

	enum Flags
	{
		DEFAULT,
#if defined(WIN32) || defined(_WIN32) || \
    defined(__WIN32) && !defined(__CYGWIN__)
		PASSIVE = AI_PASSIVE,
#else
#endif
	};

	void log_error(const std::string_view message);
} // namespace sock

#endif // SOCK_UTILS_H_
