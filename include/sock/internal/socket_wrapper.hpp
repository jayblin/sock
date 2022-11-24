#ifndef SOCK_INTERNAL_SOCKET_WRAPPER_H_
#define SOCK_INTERNAL_SOCKET_WRAPPER_H_

#include "sock/internal/socket.hpp"
#include "sock/socket.hpp"
#include "sock/utils.hpp"
#include <string>
#include <string_view>
#include <utility>

namespace sock::internal
{
	class SocketWrapper
	{
	public:
		SocketWrapper(const sock::CtorArgs args)
			: m_sock {std::move(sock::internal::Socket{std::move(args)})}
		{};

		SocketWrapper(
			const sock::CtorArgs args,
			std::function<void(sock::internal::Socket&)> callback
		) :
			m_sock {std::move(sock::internal::Socket{std::move(args)})},
			m_callback {callback}
		{
			m_callback(m_sock);
		}

		SocketWrapper(
			sock::internal::Socket&& socket,
			std::function<void(sock::internal::Socket&)> callback
		) :
			m_sock {std::move(socket)},
			m_callback {callback}
		{
			m_callback(m_sock);
		}

		auto option(const sock::Option opt, const bool val) -> SocketWrapper&
		{
			m_sock.option(opt, val);
			m_callback(m_sock);

			return *this;
		}

		auto bind() -> SocketWrapper&
		{
			m_sock.bind();
			m_callback(m_sock);

			return *this;
		}

		auto listen(size_t max_connections) -> SocketWrapper&
		{
			m_sock.listen(max_connections);
			m_callback(m_sock);

			return *this;
		}

		auto connect() -> SocketWrapper&
		{
			m_sock.connect();
			m_callback(m_sock);

			return *this;
		}

		auto accept() -> SocketWrapper
		{
			SocketWrapper result {m_sock.accept(), m_callback};
			m_callback(m_sock);

			return result;
		}

		auto receive(Buffer& buffer, int flags = 0) -> void
		{
			m_sock.receive(buffer, flags);
			m_callback(m_sock);
		}

		auto send(const std::string_view payload) -> SocketWrapper&
		{
			m_sock.send(payload);
			m_callback(m_sock);

			return *this;
		}

		auto shutdown() -> void
		{
			m_sock.shutdown();
			m_callback(m_sock);
		}

		auto is_valid() const
		{
			return m_sock.is_valid();
		}

		auto status() const
		{
			return m_sock.status();
		}

	private:
		sock::internal::Socket m_sock;
		std::function<void(sock::internal::Socket&)> m_callback;
	};
}

#endif // SOCK_INTERNAL_SOCKET_WRAPPER_H_
