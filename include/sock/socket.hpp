#ifndef SOCKET_H_
#define SOCKET_H_

#include "sock/buffer.hpp"
#include "sock/utils.hpp"
#include <string_view>

namespace sock
{
	class Socket
	{
	public:
		struct CtorArgs
		{
			Domain domain;
			Type type;
			Protocol protocol;
			std::string_view host;
			std::string_view port;
			Flags flags {Flags::DEFAULT};
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

		Socket();
#if defined(WIN32) || defined(_WIN32) || \
    defined(__WIN32) && !defined(__CYGWIN__)
		Socket(SOCKET sock) : m_sock {sock} {};
#else
#endif
		Socket(CtorArgs);
		~Socket();
		auto bind() -> Socket&;
		auto listen(size_t backlog) -> Socket&;
		auto connect() -> Socket&;
		auto accept() -> Socket;
		auto receive(Buffer&, int flags = 0) -> void;
		auto send(std::string&&) -> Socket&;
		auto shutdown() -> void;

		auto is_valid() const -> bool
		{
			return m_status != Status::GOOD;
		};

		auto status() const -> Status
		{
			return m_status;
		};

		auto status(const Status status) -> Socket&
		{
			m_status = status;
			return *this;
		};

	private:
		Status m_status {Status::GOOD};

#if defined(WIN32) || defined(_WIN32) || \
    defined(__WIN32) && !defined(__CYGWIN__)
		SOCKET m_sock {INVALID_SOCKET};
		addrinfo* m_addrinfo {nullptr};

#else
#endif
	};
} // namespace sock

#endif // SOCKET_H_
