#ifndef SOCK_WINDOWS_SOCKET_H_
#define SOCK_WINDOWS_SOCKET_H_

#include "sock/buffer.hpp"
#include "sock/utils.hpp"

namespace sock::internal
{
	class WindowsSocket
	{
	public:
		WindowsSocket();
		WindowsSocket(SOCKET sock) :
		    m_sock {sock} {};
		WindowsSocket(CtorArgs);
		WindowsSocket(const WindowsSocket&) = delete;

		WindowsSocket(WindowsSocket&& other)
		{
			*this = std::move(other);
		};

		~WindowsSocket();

		WindowsSocket& operator=(const WindowsSocket&) = delete;

		WindowsSocket& operator=(WindowsSocket&& other)
		{
			if (this != &other)
			{
				m_status = other.m_status;
				m_sock = other.m_sock;
				m_hints = other.m_hints;

				other.m_sock = -1;
			}

			return *this;
		}

		auto option(sock::Option, int value) -> WindowsSocket&;
		auto bind(sock::Address) -> WindowsSocket&;
		auto listen(size_t backlog) -> WindowsSocket&;
		auto connect(sock::Address) -> WindowsSocket&;
		auto accept() -> WindowsSocket;
		auto receive(sock::Buffer&, int flags = 0) -> void;
		auto send(std::string_view) -> WindowsSocket&;
		auto shutdown() -> void;

		auto is_valid() const -> bool
		{
			return m_status != Status::GOOD;
		};

		auto status() const -> Status
		{
			return m_status;
		};

	private:
		Status m_status {Status::GOOD};
		SOCKET m_sock {INVALID_SOCKET};
		addrinfo m_hints;
	};
} // namespace sock::internal

#endif // SOCK_WINDOWS_SOCKET_H_
