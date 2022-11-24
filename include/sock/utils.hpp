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
		GOOD = 0,
		SOCKET_CREATE_ERROR = 1,
		BIND_ERROR = 2,
		LISTEN_ERROR = 3,
		GETADDRINFO_ERROR = 4,
		ACCEPT_FAILED = 5,
		SEND_ERROR = 6,
		SHUTDOWN_ERROR = 7,
		CONNECT_ERROR = 8,
		OPTION_SET_ERROR = 9,
	};

	enum Flags
	{
		DEFAULT = 0,
#if defined(WIN32) || defined(_WIN32) || \
    defined(__WIN32) && !defined(__CYGWIN__)
		PASSIVE = AI_PASSIVE,
#else
		PASSIVE = 0,
#endif
	};

	/*
	 * @see https://learn.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-setsockopt
	 * @see https://man7.org/linux/man-pages/man7/socket.7.html
	 *
	 * @TODO: Add all options maybe?
	 */
	enum class Option
	{
		REUSEADDR,
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

	constexpr std::string_view str_status(sock::Status status)
	{
		switch (status)
		{
			case sock::Status::ACCEPT_FAILED:
				return "ACCEPT";
			case sock::Status::SOCKET_CREATE_ERROR:
				return "SOCKET_CREATE_ERROR";
			case sock::Status::BIND_ERROR:
				return "BIND_ERROR";
			case sock::Status::CONNECT_ERROR:
				return "CONNECT_ERROR";
			case sock::Status::GETADDRINFO_ERROR:
				return "GETADDRINFO_ERROR";
			case sock::Status::LISTEN_ERROR:
				return "LISTEN_ERROR";
			case sock::Status::SEND_ERROR:
				return "SEND_ERROR";
			case sock::Status::SHUTDOWN_ERROR:
				return "SHUTDOWN_ERROR";
			case sock::Status::GOOD:
				return "GOOD";
			case sock::Status::OPTION_SET_ERROR:
				return "OPTION_SET_ERROR";
		}

		return "UNKNOWN STATUS";
	}

	/*
	 * Outputs an error related to sockets with prepended `prepend`.
	 *
	 * @see https://learn.microsoft.com/en-us/windows/win32/winsock/windows-sockets-error-codes-2 for windows error codes.
	 */
	void log_error(const std::string_view prepend);

} // namespace sock

#endif // SOCK_UTILS_H_
