#include "sock/internal/unix_socket.hpp"
#include "sock/utils.hpp"
#include <algorithm>
#include <iostream>
#include <string>
#include <utility>

static constexpr auto get_address_family(const sock::Domain d)
{
	switch (d)
	{
		case sock::Domain::INET:
			return AF_INET;
		case sock::Domain::UNSPEC:
			return AF_INET;
	}
}

static constexpr auto get_socket_type(const sock::Type t)
{
	switch (t)
	{
		case sock::Type::STREAM:
			return SOCK_STREAM;
	}
}

static constexpr auto get_protocol(const sock::Protocol p)
{
	switch (p)
	{
		case sock::Protocol::TCP:
			return IPPROTO_TCP;
	}
}

static constexpr auto get_option_name(const sock::Option o)
{
	switch (o)
	{
		case sock::Option::REUSEADDR:
			return SO_REUSEADDR;
	}
}

sock::internal::UnixSocket::UnixSocket(const sock::CtorArgs args)
{
	m_fd = socket(
		get_address_family(args.domain),
		get_socket_type(args.type),
		get_protocol(args.protocol)
	);

	if (m_fd < 0)
	{
		m_status = sock::Status::SOCKET_CREATE_ERROR;
	}

	m_addr.sin_family = get_address_family(args.domain);
	// @TODO: Should this be in `args`?
	m_addr.sin_addr.s_addr = INADDR_ANY;
	m_addr.sin_port = htons(std::stoi(args.port.data()));

	/* m_args = std::move(args); */
}

sock::internal::UnixSocket::~UnixSocket()
{
	if (m_fd >= 0)
	{
		shutdown();
		close(m_fd);
	}
}

sock::internal::UnixSocket& sock::internal::UnixSocket::option(const sock::Option opt, const int val)
{
	const auto result = setsockopt(
		m_fd,
		/* get_protocol(m_args.protocol), */
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

static auto _bind = bind;

sock::internal::UnixSocket& sock::internal::UnixSocket::bind()
{
	// bind socket to ip address and port
	const auto bind_result = _bind(
		m_fd,
		reinterpret_cast<sockaddr*>(&m_addr),
		sizeof(m_addr)
	);

	if (bind_result < 0)
	{
		m_status = sock::Status::BIND_ERROR;
	}

	return *this;
}

static auto _listen = listen;

sock::internal::UnixSocket& sock::internal::UnixSocket::listen(size_t max_connections)
{
	if (_listen(m_fd, max_connections) < 0)
	{
		m_status = sock::Status::LISTEN_ERROR;
	}

	return *this;
}

static auto _connect = connect;

sock::internal::UnixSocket& sock::internal::UnixSocket::connect()
{
	const auto result = _connect(
		m_fd,
		reinterpret_cast<sockaddr*>(&m_addr),
		sizeof(m_addr)
	);

	if (result < 0)
	{
		m_status = sock::Status::CONNECT_ERROR;
	}

	return *this;
}

static auto _accept = accept;

sock::internal::UnixSocket sock::internal::UnixSocket::accept()
{
	m_client_len = sizeof(m_client_addr);

	return sock::internal::UnixSocket {_accept(
		m_fd,
		reinterpret_cast<sockaddr*>(&m_client_addr),
		&m_client_len
	)};
}

static auto _receive = recv;

void sock::internal::UnixSocket::receive(sock::Buffer& buff, int flags)
{
	buff.reset();
	auto n = _receive(m_fd, buff.buffer(), buff.max_size() - 1, flags);
	buff.received_size(n);
}

static auto _send = send;

sock::internal::UnixSocket& sock::internal::UnixSocket::send(const std::string_view& str)
{
	auto send_result = _send(m_fd, str.data(), str.length(), 0);

	if (send_result < 0)
	{
		m_status = sock::Status::SEND_ERROR;
	}

	return *this;
}

static auto _shutdown = shutdown;

void sock::internal::UnixSocket::shutdown()
{
	const auto shutdown_result = _shutdown(m_fd, SHUT_RDWR);

	if (shutdown_result < 0)
	{
		m_status = sock::Status::SHUTDOWN_ERROR;
	}
}
