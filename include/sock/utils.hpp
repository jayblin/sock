#ifndef SOCK_UTILS_H_
#define SOCK_UTILS_H_

#include <string_view>
#if defined(WIN32) || defined(_WIN32) || \
    defined(__WIN32) && !defined(__CYGWIN__)

# include <winsock2.h>
# include <ws2tcpip.h>

#else

# include <netinet/in.h>
# include <sys/socket.h>
# include <unistd.h>

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

	enum class Status
	{
		GOOD,
		SOCKET_CREATE_ERROR,
		BIND_ERROR,
		LISTEN_ERROR,
		GETADDRINFO_ERROR,
		ACCEPT_FAILED,
		SEND_ERROR,
		SHUTDOWN_ERROR,
		CONNECT_ERROR,
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

	struct CtorArgs
	{
		Domain domain;
		Type type;
		Protocol protocol;
		std::string_view host;
		std::string_view port;
		Flags flags {Flags::DEFAULT};
	};

	void log_error(const std::string_view message);
} // namespace sock

#endif // SOCK_UTILS_H_
