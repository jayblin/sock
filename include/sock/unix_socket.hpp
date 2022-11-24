#ifndef SOCK_UNIX_SOCKET_H_
#define SOCK_UNIX_SOCKET_H_

#include "sock/buffer.hpp"
#include "sock/utils.hpp"
#include <functional>
#include <netinet/in.h>
#include <sys/socket.h>

namespace sock
{
	class UnixSocket
	{
	public:
		UnixSocket();
		UnixSocket(int fd) : m_fd {fd} {};
		UnixSocket(const CtorArgs);
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
				m_addr = other.m_addr;
				m_client_addr = other.m_addr;
				m_client_len = other.m_client_len;
				/* m_args = std::move(other.m_args); */
			}

			return *this;
		}

		auto option(const sock::Option, const int) -> UnixSocket&;
		auto bind() -> UnixSocket&;
		auto listen(const size_t backlog) -> UnixSocket&;
		auto connect() -> UnixSocket&;
		auto accept() -> UnixSocket;
		auto receive(Buffer&, int flags = 0) -> void;
		auto send(const std::string_view) -> UnixSocket&;
		auto shutdown() -> void;

		constexpr auto is_valid() const -> bool { return m_status != Status::GOOD; }
		constexpr auto status() const -> Status { return m_status; }

	private:
		Status m_status {Status::GOOD};

		int m_fd;
		sockaddr_in m_addr;
		sockaddr_in m_client_addr;
		socklen_t m_client_len;
		/* sock::CtorArgs m_args; */
	};
} // namespace sock

#endif // SOCK_UNIX_SOCKET_H_
