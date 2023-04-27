#include "sock/internal/unix_socket.hpp"
#include "sock/utils.hpp"
#include <algorithm>
#include <asm-generic/socket.h>
#include <bits/types/struct_timeval.h>
#include <iostream>
#include <netdb.h>
#include <string>
#include <utility>

static constexpr int get_address_family(sock::Domain d)
{
	switch (d)
	{
		case sock::Domain::INET:
			return AF_INET;
		case sock::Domain::UNSPEC:
			return AF_INET;
	}
}

static constexpr int get_socket_type(sock::Type t)
{
	switch (t)
	{
		case sock::Type::STREAM:
			return SOCK_STREAM;
	}
}

static constexpr int get_protocol(sock::Protocol p)
{
	switch (p)
	{
		case sock::Protocol::TCP:
			return IPPROTO_TCP;
	}
}

static constexpr int get_option_name(sock::Option o)
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
	}
}

static const auto _socket = socket;

sock::internal::UnixSocket::UnixSocket(const sock::CtorArgs args)
{
	m_domain = get_address_family(args.domain);
	m_socket_type = get_socket_type(args.type);
	m_protocol = get_protocol(args.protocol);
	m_flags = args.flags;

	m_fd = _socket(
		m_domain,
		m_socket_type,
		m_protocol
	);

	if (m_fd < 0)
	{
		m_status = sock::Status::SOCKET_CREATE_ERROR;
	}
}

sock::internal::UnixSocket::~UnixSocket()
{
	if (m_fd >= 0)
	{
		shutdown();
		close(m_fd);
	}
}

sock::internal::UnixSocket& sock::internal::UnixSocket::option(
	sock::Option opt,
	int val
)
{
	const auto result = setsockopt(
		m_fd,
		SOL_SOCKET,
		get_option_name(opt),
		&val,
		sizeof(val)
	);

	if (result < 0)
	{
		m_status = sock::Status::OPTION_SET_ERROR;
	}

	return *this;
}

sock::internal::UnixSocket& sock::internal::UnixSocket::option(
	sock::Option opt,
	std::chrono::milliseconds timeout
)
{
	timeval to {
		.tv_sec = static_cast<long>(timeout.count() / 1000),
		.tv_usec = static_cast<long>(
			timeout.count() - static_cast<long>(timeout.count() / 1000)*1000
		)
	};

	const auto result = setsockopt(
		m_fd,
		SOL_SOCKET,
		get_option_name(opt),
		&to,
		sizeof(to)
	);

	if (result < 0)
	{
		m_status = sock::Status::OPTION_SET_ERROR;
	}

	return *this;
}

static const auto _bind = bind;

sock::internal::UnixSocket& sock::internal::UnixSocket::bind(sock::Address address)
{
	addrinfo hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = m_domain;
	hints.ai_socktype = m_socket_type;
	hints.ai_flags = m_flags;
	hints.ai_protocol = m_protocol;
	hints.ai_canonname = nullptr;
	hints.ai_addr = nullptr;
	hints.ai_next = nullptr;

	addrinfo* addr;
	const auto s = getaddrinfo(address.host.empty() ? nullptr : address.host.data(), address.port.data(), &hints, &addr);

	if (s != 0) {
		m_status = sock::Status::GETADDRINFO_ERROR;
	}
	else
	{
		for (auto rp = addr; rp != nullptr; rp = rp->ai_next) {
			if (_bind(m_fd, rp->ai_addr, rp->ai_addrlen) < 0)
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

	return *this;
}

static const auto _listen = listen;

sock::internal::UnixSocket& sock::internal::UnixSocket::listen(size_t max_connections)
{
	if (_listen(m_fd, max_connections) < 0)
	{
		m_status = sock::Status::LISTEN_ERROR;
	}

	return *this;
}

static const auto _connect = connect;

sock::internal::UnixSocket& sock::internal::UnixSocket::connect(sock::Address address)
{
	addrinfo hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = m_domain;
	hints.ai_socktype = m_socket_type;
	hints.ai_flags = m_flags;
	hints.ai_protocol = m_protocol;
	hints.ai_canonname = nullptr;
	hints.ai_addr = nullptr;
	hints.ai_next = nullptr;

	addrinfo* addr;
	const auto s = getaddrinfo(address.host.empty() ? nullptr : address.host.data(), address.port.data(), &hints, &addr);

	if (s != 0) {
		m_status = sock::Status::GETADDRINFO_ERROR;
	}
	else
	{
		for (auto rp = addr; rp != nullptr; rp = rp->ai_next) {

			if (_connect(m_fd, rp->ai_addr, rp->ai_addrlen) < 0)
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

	return *this;
}

static const auto _accept = accept;

sock::internal::UnixSocket sock::internal::UnixSocket::accept()
{
	sockaddr_storage peer_addr;
	socklen_t peer_addr_len = sizeof(peer_addr);

	return sock::internal::UnixSocket {_accept(
		m_fd,
		reinterpret_cast<sockaddr*>(&peer_addr),
		&peer_addr_len
	)};
}

static const auto _receive = recv;

void sock::internal::UnixSocket::receive(sock::Buffer& buff, int flags)
{
	buff.reset();
	auto n = _receive(m_fd, buff.buffer(), buff.max_size() - 1, flags);

	if (n < 0)
	{
		m_status = sock::Status::RECEIVE_ERROR;
		buff.received_size(0);
	}
	else
	{
		buff.received_size(n);
	}
}

static const auto _send = send;

sock::internal::UnixSocket& sock::internal::UnixSocket::send(std::string_view str)
{
	auto send_result = _send(m_fd, str.data(), str.length(), 0);

	if (send_result < 0)
	{
		m_status = sock::Status::SEND_ERROR;
	}

	return *this;
}

static const auto _shutdown = shutdown;

void sock::internal::UnixSocket::shutdown()
{
	const auto shutdown_result = _shutdown(m_fd, SHUT_RDWR);

	if (shutdown_result < 0)
	{
		m_status = sock::Status::SHUTDOWN_ERROR;
	}
}
