#include "sock/internal/windows_socket.hpp"
#include "sock/utils.hpp"
#include <algorithm>
#include <iostream>
#include <string>
#include <winsock2.h>

static constexpr auto get_address_family(sock::Domain d)
{
	switch (d)
	{
		case sock::Domain::INET:
			return AF_INET;
		case sock::Domain::UNSPEC:
			return AF_UNSPEC;
	}
}

static constexpr auto get_socket_type(sock::Type t)
{
	switch (t)
	{
		case sock::Type::STREAM:
			return SOCK_STREAM;
	}
}

static constexpr auto get_protocol(sock::Protocol p)
{
	switch (p)
	{
		case sock::Protocol::TCP:
			return IPPROTO_TCP;
	}
}

static constexpr auto get_option_name(sock::Option o)
{
	switch (o)
	{
		case sock::Option::REUSEADDR:
			return SO_REUSEADDR;
		case sock::Option::ACCEPTCONN:
			return SO_ACCEPTCONN;
		case sock::Option::BROADCAST:
			return SO_BROADCAST;
		case sock::Option::DEBUG:
			return SO_DEBUG;
		case sock::Option::DONTROUTE:
			return SO_DONTROUTE;
		case sock::Option::OPT_ERROR:
			return SO_ERROR;
		case sock::Option::KEEPALIVE:
			return SO_KEEPALIVE;
		case sock::Option::LINGER:
			return SO_LINGER;
		case sock::Option::OOBINLINE:
			return SO_OOBINLINE;
		case sock::Option::RCVBUF:
			return SO_RCVBUF;
		case sock::Option::RCVLOWAT:
			return SO_RCVLOWAT;
		case sock::Option::RCVTIMEO:
			return SO_RCVTIMEO;
		case sock::Option::SNDBUF:
			return SO_SNDBUF;
		case sock::Option::SNDLOWAT:
			return SO_SNDLOWAT;
		case sock::Option::SNDTIMEO:
			return SO_SNDTIMEO;
		case sock::Option::TYPE:
			return SO_TYPE;
			break;
	}
}

const auto _socket = socket;

sock::internal::WindowsSocket::WindowsSocket(sock::CtorArgs args)
{
	addrinfo hints;

	ZeroMemory(&hints, sizeof(hints));

	hints.ai_family = get_address_family(args.domain);
	hints.ai_socktype = get_socket_type(args.type);
	hints.ai_protocol = get_protocol(args.protocol);
	hints.ai_flags = args.flags;

	m_sock = _socket(hints.ai_family, hints.ai_socktype, hints.ai_protocol);

	m_hints = hints;

	if (INVALID_SOCKET == m_sock)
	{
		m_status = sock::Status::SOCKET_CREATE_ERROR;
	}
}

sock::internal::WindowsSocket::~WindowsSocket()
{
	closesocket(m_sock);
}

sock::internal::WindowsSocket& sock::internal::WindowsSocket::option(
    sock::Option option,
    const char* value,
    size_t value_size
)
{
	const auto result = setsockopt(
	    m_sock,
	    SOL_SOCKET,
	    get_option_name(option),
	    value,
	    value_size
	);

	if (result == SOCKET_ERROR)
	{
		m_status = sock::Status::OPTION_SET_ERROR;
	}

	return *this;
}

sock::internal::WindowsSocket&
    sock::internal::WindowsSocket::option(sock::Option opt, int value)
{
	return option(opt, (char*) (&value), sizeof(value));
}

sock::internal::WindowsSocket& sock::internal::WindowsSocket::option(
    sock::Option opt,
    std::chrono::milliseconds timeout
)
{
	const auto ms = timeout.count();

	return option(opt, (char*) (&ms), sizeof(ms));
}

const auto _bind = bind;

sock::internal::WindowsSocket&
    sock::internal::WindowsSocket::bind(sock::Address address)
{
	addrinfo* info {nullptr};

	// Resolve the local address and port to be used by the server
	const auto getaddrinfo_result = getaddrinfo(
	    address.host.length() > 0 ? address.host.data() : NULL,
	    address.port.data(),
	    &m_hints,
	    &info
	);

	if (getaddrinfo_result != 0)
	{
		m_status = sock::Status::GETADDRINFO_ERROR;
	}
	else
	{
		for (auto rp = info; rp != nullptr; rp = rp->ai_next) {
			if (_bind(m_sock, rp->ai_addr, rp->ai_addrlen) == SOCKET_ERROR)
			{
				m_status = sock::Status::BIND_ERROR;
			}
			else
			{
				m_status = sock::Status::GOOD;
				break;
			}
		}
	}

	freeaddrinfo(info);

	return *this;
}

const auto _listen = listen;

sock::internal::WindowsSocket& sock::internal::WindowsSocket::listen(
    size_t backlog
)
{
	if (_listen(m_sock, backlog) == SOCKET_ERROR)
	{
		m_status = sock::Status::LISTEN_ERROR;
	}

	return *this;
}

const auto _connect = connect;

sock::internal::WindowsSocket& sock::internal::WindowsSocket::connect(
    sock::Address address
)
{
	addrinfo* info {nullptr};

	// Resolve the local address and port to be used by the server
	const auto getaddrinfo_result = getaddrinfo(
	    address.host.length() > 0 ? address.host.data() : NULL,
	    address.port.data(),
	    &m_hints,
	    &info
	);

	if (getaddrinfo_result != 0)
	{
		m_status = sock::Status::GETADDRINFO_ERROR;
	}
	else
	{
		// Try to connect to ip until succeded or failed
		for (auto ptr = info; ptr != nullptr; ptr = ptr->ai_next)
		{
			if (_connect(m_sock, ptr->ai_addr, (int) ptr->ai_addrlen)
			    == SOCKET_ERROR)
			{
				m_status = sock::Status::CONNECT_ERROR;
			}
			else
			{
				m_status = sock::Status::GOOD;
				break;
			}
		}
	}

	freeaddrinfo(info);

	return *this;
}

const auto _accept = accept;

sock::internal::WindowsSocket sock::internal::WindowsSocket::accept()
{
	return sock::internal::WindowsSocket {_accept(m_sock, NULL, NULL)};
}

const auto _receive = recv;

void sock::internal::WindowsSocket::receive(sock::Buffer& buff, int flags)
{
	buff.reset();
	const auto n = _receive(m_sock, buff.buffer(), buff.max_size() - 1, flags);

	if (n == SOCKET_ERROR)
	{
		m_status = sock::Status::RECEIVE_ERROR;
		buff.received_size(0);
	}
	else
	{
		buff.received_size(n);
	}
}

const auto _send = send;

sock::internal::WindowsSocket& sock::internal::WindowsSocket::send(
    std::string_view str
)
{
	auto send_result = _send(m_sock, str.data(), str.length(), 0);

	if (send_result == SOCKET_ERROR)
	{
		m_status = sock::Status::SEND_ERROR;
	}

	return *this;
}

const auto _shutdown = shutdown;

void sock::internal::WindowsSocket::shutdown()
{
	const auto shutdown_result = _shutdown(m_sock, SD_SEND);

	if (shutdown_result == SOCKET_ERROR)
	{
		m_status = sock::Status::SHUTDOWN_ERROR;
	}
}
