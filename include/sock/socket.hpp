#ifndef SOCKET_H_
#define SOCKET_H_

#include "sock/buffer.hpp"
#include "sock/utils.hpp"
#include <concepts>
#include <type_traits>

#if defined(WIN32) || defined(_WIN32) || \
    defined(__WIN32) && !defined(__CYGWIN__)

# include "sock/windows_socket.hpp"

namespace sock
{
	typedef sock::WindowsSocket Socket;
} // namespace sock

#else
#endif

namespace sock
{
	template<class T>
	concept is_socket = requires(
	    T t,
	    const CtorArgs args,
	    const size_t max_connections,
	    Buffer& buffer,
	    int flags,
	    const std::string_view payload
	)
	{
		{T(args)};
		{
			t.bind()
			} -> std::same_as<T&>;
		{
			t.listen(max_connections)
			} -> std::same_as<T&>;
		{
			t.connect()
			} -> std::same_as<T&>;
		{
			t.accept()
			} -> std::same_as<T>;
		{
			t.receive(buffer, flags)
			} -> std::same_as<void>;
		{
			t.send(payload)
			} -> std::same_as<T&>;
		{
			t.shutdown()
			} -> std::same_as<void>;
	};

	template<class T, class S>
	concept has_status = requires(T const t)
	{
		{
			t.is_valid()
			} -> std::same_as<bool>;
		{
			t.status()
			} -> std::same_as<S>;
	};
} // namespace sock

static_assert(sock::is_socket<sock::Socket>);
static_assert(sock::has_status<sock::Socket, sock::Status>);

#endif // SOCKET_H_
