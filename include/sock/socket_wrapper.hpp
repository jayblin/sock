#ifndef SOCK_INTERNAL_SOCKET_WRAPPER_H_
#define SOCK_INTERNAL_SOCKET_WRAPPER_H_

#include "sock/socket.hpp"
#include "sock/utils.hpp"
#include <chrono>
#include <functional>
#include <string_view>
#include <utility>

namespace sock
{
	/**
	 * Same as `sock::Socket` but after each member function call a callback
	 * is called.
	 */
	class SocketWrapper
	{
	public:
		SocketWrapper(sock::CtorArgs args) :
		    m_sock {sock::Socket {std::move(args)}} {};

		SocketWrapper(
		    sock::CtorArgs args,
		    std::function<void(sock::Socket&)> callback
		) :
		    m_sock {sock::Socket {std::move(args)}},
		    m_callback {callback}
		{
			if (m_callback)
			{
				m_callback(m_sock);
			}
		}

		SocketWrapper(
		    sock::Socket&& socket,
		    std::function<void(sock::Socket&)> callback
		) :
		    m_sock {std::move(socket)},
		    m_callback {callback}
		{
			if (m_callback)
			{
				m_callback(m_sock);
			}
		}

		SocketWrapper(const SocketWrapper&) = delete;
		SocketWrapper& operator=(const SocketWrapper&) = delete;

		SocketWrapper(SocketWrapper&& other)
		{
			*this = std::move(other);
		};

		SocketWrapper& operator=(SocketWrapper&& other)
		{
			if (this != &other)
			{
				m_callback = other.m_callback;
				m_sock = std::move(other.m_sock);

				other.m_callback = nullptr;
			}

			return *this;
		}

		auto option(sock::Option opt, int val) -> SocketWrapper&
		{
			m_sock.option(opt, val);
			if (m_callback)
			{
				m_callback(m_sock);
			}

			return *this;
		}

		auto option(sock::Option opt, std::chrono::milliseconds duration)
		    -> SocketWrapper&
		{
			m_sock.option(opt, duration);
			if (m_callback)
			{
				m_callback(m_sock);
			}

			return *this;
		}

		auto bind(sock::Address address) -> SocketWrapper&
		{
			m_sock.bind(address);
			if (m_callback)
			{
				m_callback(m_sock);
			}

			return *this;
		}

		auto listen(size_t max_connections) -> SocketWrapper&
		{
			m_sock.listen(max_connections);
			if (m_callback)
			{
				m_callback(m_sock);
			}

			return *this;
		}

		auto connect(sock::Address address) -> SocketWrapper&
		{
			m_sock.connect(address);
			if (m_callback)
			{
				m_callback(m_sock);
			}

			return *this;
		}

		auto accept() -> SocketWrapper
		{
			SocketWrapper result {m_sock.accept(), m_callback};
			if (m_callback)
			{
				m_callback(m_sock);
			}

			return result;
		}

		auto receive(sock::Buffer& buffer, int flags = 0) -> void
		{
			m_sock.receive(buffer, flags);
			if (m_callback)
			{
				m_callback(m_sock);
			}
		}

		auto send(std::string_view payload) -> SocketWrapper&
		{
			m_sock.send(payload);
			if (m_callback)
			{
				m_callback(m_sock);
			}

			return *this;
		}

		auto shutdown() -> void
		{
			m_sock.shutdown();
			if (m_callback)
			{
				m_callback(m_sock);
			}
		}

		auto callback(std::function<void(sock::Socket&)> callback)
		    -> SocketWrapper&
		{
			m_callback = callback;

			return *this;
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
		std::function<void(sock::Socket&)> m_callback {nullptr};
	};
} // namespace sock

#endif // SOCK_SOCKET_WRAPPER_H_
