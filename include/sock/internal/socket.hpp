#ifndef SOCK_INTERNAL_SOCKET_H_
#define SOCK_INTERNAL_SOCKET_H_

#if defined(WIN32) || defined(_WIN32) || \
    defined(__WIN32) && !defined(__CYGWIN__)

#include "sock/internal/windows_socket.hpp"

namespace sock::internal
{
	typedef WindowsSocket Socket;
} // namespace sock

#else

#include "sock/internal/unix_socket.hpp"

namespace sock::internal
{
	typedef UnixSocket Socket;
} // namespace sock

#endif

#endif // SOCK_INTERNAL_SOCKET_H_
