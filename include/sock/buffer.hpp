#ifndef SOCK_BUFFER_H_
#define SOCK_BUFFER_H_

#include <cstring>

namespace sock
{
	/**
	 * A wrapper around `const char*`.
	 * Should be used with `sock::Socket::receive()`.
	 */
	class Buffer
	{
	public:
		static const size_t MAX = 1024 * 10;

		Buffer() { reset(); };

		/**
		 * Fills inner `const char*` with 0's.
		 */
		auto reset() -> Buffer&
		{
			std::memset(m_buff, 0, MAX);

			return *this;
		}

		/**
		 * Returns inner `const char*`.
		 */
		auto buffer() -> char* { return m_buff; }

		/**
		 * Returns the maximum allowed size of inner `const char*`.
		 */
		constexpr auto max_size() const { return Buffer::MAX; }

		/**
		 * Returns the current size of a string that can be obtained from
		 * inner `const char*`.
		 * Should be set by `sock::Socket::receive()`.
		 */
		constexpr auto received_size() const { return m_received_size; }

		/**
		 * Sets the current size of a string that can be obtained from
		 * inner `const char*`.
		 * Should be used by `sock::Socket::receive()`.
		 */
		auto received_size(int rs) -> Buffer&
		{
			m_received_size = rs;

			return *this;
		}

	private:
		char m_buff[MAX];
		int m_received_size = 0;
	};
} // namespace sock

#endif // SOCK_BUFFER_H_
