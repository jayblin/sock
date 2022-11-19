#ifndef SOCK_SOCKET_FACTORY_H_
#define SOCK_SOCKET_FACTORY_H_

#include "socket.hpp"

namespace sock
{
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

		auto create(Socket::CtorArgs&&) const -> Socket;

	private:
		SocketFactory();
		SocketFactory(const SocketFactory&);
		SocketFactory& operator=(const SocketFactory&);
	};
} // namespace sock

#endif // SOCK_SOCKET_FACTORY_H_
