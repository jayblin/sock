#ifndef SOCKET_H_
#define SOCKET_H_

#include "sock/buffer.hpp"
#include "sock/utils.hpp"
#include <concepts>
#include <functional>
#include <string_view>
#include <type_traits>

namespace sock
{
	template<class T>
	concept has_socket_interface = requires(
	    T t,
	    const size_t max_connections,
	    Buffer& buffer,
	    int flags
	)
	{
		{ T((const CtorArgs){}) };
		{ t.option((const sock::Option){}, (const int){}) } -> std::same_as<T&>;
		{ t.bind() } -> std::same_as<T&>;
		{ t.listen(max_connections) } -> std::same_as<T&>;
		{ t.connect() } -> std::same_as<T&>;
		{ t.accept() } -> std::same_as<T>;
		{ t.receive(buffer, flags) } -> std::same_as<void>;
		{ t.send((const std::string_view){}) } -> std::same_as<T&>;
		{ t.shutdown() } -> std::same_as<void>;
	};

	template<class T, class S>
	concept has_status = requires(T const t)
	{
		{ t.is_valid() } -> std::same_as<bool>;
		{ t.status() } -> std::same_as<S>;
	};

	template<class T, class S>
	concept is_socket = has_socket_interface<T> && has_status<T, S>;

} // namespace sock

#endif // SOCKET_H_
