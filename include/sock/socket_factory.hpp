#ifndef SOCK_SOCKET_FACTORY_H_
#define SOCK_SOCKET_FACTORY_H_

#include "sock/internal/concepts.hpp"
#include "sock/internal/socket_wrapper.hpp"
#include "sock/internal/socket.hpp"
#include "sock/utils.hpp"
#include <functional>
#include <utility>

namespace sock
{
	class WrapperBuilder
	{
	public:
		WrapperBuilder(const CtorArgs args) : m_args {std::move(args)} {};

		auto with(
			std::function<void(sock::internal::Socket&)>&& callback
		) -> WrapperBuilder&
		{
			m_callback = std::move(callback);

			return *this;
		}

		auto create() -> sock::internal::SocketWrapper
		{
			static_assert(
				sock::internal::is_socket<sock::internal::SocketWrapper, sock::Status>
			);

			return sock::internal::SocketWrapper(
				std::move(m_args),
				std::move(m_callback)
			);
;
		}
	
	private:
		CtorArgs m_args;
		std::function<void(internal::Socket&)> m_callback;
	};

	/**
	 * This class contains OS specific logic for initializing socket library.
	 * All `sock::Socket` objects should be created by means of this class.
	 */
	class SocketFactory
	{
	public:
		~SocketFactory();

		static SocketFactory& instance()
		{
			static SocketFactory _instance;
			return _instance;
		}

		auto wrap(const CtorArgs args) -> WrapperBuilder
		{
			return WrapperBuilder {std::move(args)};
		}

		auto create(const CtorArgs args) const -> internal::Socket
		{
			static_assert(
				sock::internal::is_socket<sock::internal::Socket, sock::Status>
			);

			return sock::internal::Socket(std::move(args));
		}

	private:
		SocketFactory();
		SocketFactory(const SocketFactory&);
		SocketFactory& operator=(const SocketFactory&);
	};

} // namespace sock

#endif // SOCK_SOCKET_FACTORY_H_
