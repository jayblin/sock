#include "sock/socket.hpp"
#include <string>

#if defined(WIN32) || defined(_WIN32) || \
    defined(__WIN32) && !defined(__CYGWIN__)

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

sock::Socket::Socket(sock::Socket::CtorArgs args)
{
	addrinfo hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = get_address_family(args.domain);
	hints.ai_socktype = get_socket_type(args.type);
	hints.ai_protocol = get_protocol(args.protocol);
	hints.ai_flags = args.flags;

	// Resolve the local address and port to be used by the server
	auto getaddrinfo_result = getaddrinfo(
	    args.host.length() > 0 ? args.host.data() : NULL,
	    args.port.data(),
	    &hints,
	    &m_addrinfo
	);

	if (getaddrinfo_result != 0)
	{
		status(sock::Socket::Status::GETADDRINFO_ERROR);
		return;
	}

	m_sock = socket(
	    m_addrinfo->ai_family,
	    m_addrinfo->ai_socktype,
	    m_addrinfo->ai_protocol
	);

	if (INVALID_SOCKET == m_sock)
	{
		status(sock::Socket::Status::SOCKET_CREATE_ERROR);
	}
}

sock::Socket::~Socket()
{
	freeaddrinfo(m_addrinfo);
	closesocket(m_sock);
}

static auto _bind = bind;

sock::Socket& sock::Socket::bind()
{
	// bind socket to ip address and port
	const auto bind_result =
	    _bind(m_sock, m_addrinfo->ai_addr, (int) m_addrinfo->ai_addrlen);

	if (SOCKET_ERROR == bind_result)
	{
		status(sock::Socket::Status::BIND_ERROR);
	}

	return *this;
}

static auto _listen = listen;

sock::Socket& sock::Socket::listen(size_t backlog)
{
	if (_listen(m_sock, backlog) == SOCKET_ERROR)
	{
		status(sock::Socket::Status::LISTEN_ERROR);
	}

	return *this;
}

static auto _connect = connect;

sock::Socket& sock::Socket::connect()
{
	// Try to connect to ip until succeded or failed
	for (auto ptr = m_addrinfo; ptr != NULL; ptr = ptr->ai_next)
	{
		if (INVALID_SOCKET == m_sock)
		{
			m_sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		}

		if (_connect(m_sock, ptr->ai_addr, (int) ptr->ai_addrlen) ==
		    SOCKET_ERROR)
		{
			status(sock::Socket::Status::CONNECT_ERROR);
			closesocket(m_sock);
			m_sock = INVALID_SOCKET;
		}
		else
		{
			status(sock::Socket::Status::GOOD);
			break;
		}
	}

	return *this;
}

static auto _accept = accept;

sock::Socket sock::Socket::accept()
{
	return sock::Socket {_accept(m_sock, NULL, NULL)};
}

static auto _receive = recv;

void sock::Socket::receive(sock::Buffer& buff, int flags)
{
	buff.reset();
	auto n = _receive(m_sock, buff.buffer(), buff.max_size() - 1, flags);
	buff.received_size(n);
}

static auto _send = send;

sock::Socket& sock::Socket::send(std::string&& str)
{
	auto send_result = _send(m_sock, str.data(), str.length(), 0);

	if (send_result == SOCKET_ERROR)
	{
		status(sock::Socket::Status::SEND_ERROR);
	}

	return *this;
}

static auto _shutdown = shutdown;

void sock::Socket::shutdown()
{
	const auto shutdown_result = _shutdown(m_sock, SD_SEND);

	if (shutdown_result == SOCKET_ERROR)
	{
		status(sock::Socket::Status::SHUTDOWN_ERROR);
	}
}

#else

#endif
