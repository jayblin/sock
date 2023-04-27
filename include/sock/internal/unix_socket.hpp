#ifndef SOCK_UNIX_SOCKET_H_
#define SOCK_UNIX_SOCKET_H_

#include "sock/buffer.hpp"
#include "sock/utils.hpp"
#include <chrono>
#include <functional>

namespace sock::internal
{
	class UnixSocket
	{
	public:
		UnixSocket();
		UnixSocket(int fd) : m_fd {fd} {};
		UnixSocket(CtorArgs);
		UnixSocket(const UnixSocket&) = delete;
		UnixSocket(UnixSocket&& other)
		{
			*this = std::move(other);

			other.m_fd = -1;
		};

		~UnixSocket();

		UnixSocket& operator=(const UnixSocket&) = delete;
		UnixSocket& operator=(UnixSocket&& other)
		{
			if (this != &other)
			{
				m_status = other.m_status;
				m_fd = other.m_fd;
			}

			return *this;
		}

		auto option(sock::Option, std::chrono::milliseconds) -> UnixSocket&;
		auto option(sock::Option, int) -> UnixSocket&;
		auto bind(sock::Address) -> UnixSocket&;
		auto listen(size_t backlog) -> UnixSocket&;
		auto connect(sock::Address) -> UnixSocket&;
		auto accept() -> UnixSocket;
		auto receive(sock::Buffer&, int flags = 0) -> void;
		auto send(std::string_view) -> UnixSocket&;
		auto shutdown() -> void;

		constexpr auto is_valid() const -> bool { return m_status != Status::GOOD; }
		constexpr auto status() const -> Status { return m_status; }

	private:
		Status m_status {Status::GOOD};

		int m_fd;
		int m_domain;
		int m_socket_type;
		int m_protocol;
		int m_flags;
	};
} // namespace sock

#endif // SOCK_UNIX_SOCKET_H_
