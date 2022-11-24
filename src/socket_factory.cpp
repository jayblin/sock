#include "sock/socket_factory.hpp"

sock::SocketFactory::SocketFactory()
{
#if defined(WIN32) || defined(_WIN32) || \
    defined(__WIN32) && !defined(__CYGWIN__)
	WSADATA wsa_data;

	auto wsa_started = WSAStartup(
	    MAKEWORD(2, 2), // use version 2.2 of Winsock
	    &wsa_data
	);

	if (wsa_started != 0)
	{
		sock::log_error("couldnt start WSA");
	}
#else
	// UNIX SOCKET LIB INIT HERE
#endif
}

sock::SocketFactory::~SocketFactory()
{
#if defined(WIN32) || defined(_WIN32) || \
    defined(__WIN32) && !defined(__CYGWIN__)
	WSACleanup();
#else
	// UNIX SOCKET LIB CLEANUP HERE
#endif
}
