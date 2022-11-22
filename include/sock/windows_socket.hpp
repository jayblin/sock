#ifndef SOCK_WINDOWS_SOCKET_H_
#define SOCK_WINDOWS_SOCKET_H_

#include "sock/buffer.hpp"
#include "sock/utils.hpp"

namespace sock
{
	class WindowsSocket
	{
	public:
		WindowsSocket();
		WindowsSocket(SOCKET sock) : m_sock {sock} {};
		WindowsSocket(const CtorArgs);
		~WindowsSocket();
		auto bind() -> WindowsSocket&;
		auto listen(const size_t backlog) -> WindowsSocket&;
		auto connect() -> WindowsSocket&;
		auto accept() -> WindowsSocket;
		auto receive(Buffer&, int flags = 0) -> void;
		auto send(const std::string_view) -> WindowsSocket&;
		auto shutdown() -> void;

		auto is_valid() const -> bool { return m_status != Status::GOOD; };

		auto status() const -> Status { return m_status; };

	private:
		Status m_status {Status::GOOD};

		SOCKET m_sock {INVALID_SOCKET};
		addrinfo* m_addrinfo {nullptr};
	};
} // namespace sock

#endif // SOCK_WINDOWS_SOCKET_H_
