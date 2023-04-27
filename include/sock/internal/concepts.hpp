#ifndef SOCKET_INTERNAL_CONCEPTS_H_
#define SOCKET_INTERNAL_CONCEPTS_H_

#include "sock/buffer.hpp"
#include "sock/utils.hpp"
#include <chrono>
#include <concepts>
#include <functional>
#include <string_view>

namespace sock::internal
{
	// clang-format off
	template<class T>
	concept has_socket_interface = requires(
	    T t,
	    size_t max_connections,
	    Buffer& buffer,
	    int flags
	)
	{
		{ T((CtorArgs){}) };
		{ t.option((sock::Option){}, (int){}) } -> std::same_as<T&>;
		{ t.option(
			(sock::Option){},
			(std::chrono::milliseconds){}
		) } -> std::same_as<T&>;
		{ t.bind((sock::Address){}) } -> std::same_as<T&>;
		{ t.listen(max_connections) } -> std::same_as<T&>;
		{ t.connect((sock::Address){}) } -> std::same_as<T&>;
		{ t.accept() } -> std::same_as<T>;
		{ t.receive(buffer, flags) } -> std::same_as<void>;
		{ t.send((std::string_view){}) } -> std::same_as<T&>;
		{ t.shutdown() } -> std::same_as<void>;
	};
	// clang-format on

	// clang-format off
	template<class T, class S>
	concept has_status = requires(T const t)
	{
		{ t.is_valid() } -> std::same_as<bool>;
		{ t.status() } -> std::same_as<S>;
	};

	template<class T, class S>
	concept is_socket = has_socket_interface<T> && has_status<T, S>;
	// clang-format on

} // namespace sock::internal

#endif // SOCKET_INTERNAL_CONCEPTS_H_
