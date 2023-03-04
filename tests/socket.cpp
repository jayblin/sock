#include "sock/socket_factory.hpp"
#include "sock/utils.hpp"
#include <gtest/gtest.h>
#include <iostream>

TEST(Socket, server_and_client_communication_test)
{
	bool server_closed = false;
	bool client_closed = false;

	auto& factory = sock::SocketFactory::instance();

	std::thread server_thread {
	    [&factory, &server_closed]() {
		    auto server = factory.create({
				.domain = sock::Domain::INET,
				.type = sock::Type::STREAM,
				.protocol = sock::Protocol::TCP,
				.port = "8843",
				.flags = sock::Flags::PASSIVE,
			});
		    ASSERT_EQ(sock::Status::GOOD, server.status());

			// SO_REUSEADDR must be set so tests can reuse socket ports.
			server.option(sock::Option::REUSEADDR, 1);
		    ASSERT_EQ(sock::Status::GOOD, server.status());

		    server.bind();
		    ASSERT_EQ(sock::Status::GOOD, server.status());

		    server.listen(10);

		    auto connection = server.accept();
		    ASSERT_EQ(sock::Status::GOOD, server.status());
		    ASSERT_EQ(sock::Status::GOOD, connection.status());

		    sock::Buffer buff;
		    ASSERT_EQ(0, buff.received_size());

		    connection.receive(buff);
		    ASSERT_EQ(sock::Status::GOOD, connection.status());
		    ASSERT_EQ(11, buff.received_size());
		    ASSERT_STREQ("Hello there", buff.buffer());

		    connection.send("General Kenobi!");
		    ASSERT_EQ(sock::Status::GOOD, connection.status());

		    connection.receive(buff);
		    ASSERT_EQ(sock::Status::GOOD, connection.status());
		    ASSERT_EQ(8, buff.received_size());
		    ASSERT_STREQ("Bye now!", buff.buffer());

		    connection.send("");
		    ASSERT_EQ(sock::Status::GOOD, connection.status());

			server_closed = true;
	    }
	};

	server_thread.detach();

	std::thread client_thread {
	    [&factory, &client_closed]() {
		    auto client = factory.create({
				.domain = sock::Domain::UNSPEC,
				.type = sock::Type::STREAM,
				.protocol = sock::Protocol::TCP,
				.host = "localhost",
				.port = "8843",
			});
		    ASSERT_EQ(sock::Status::GOOD, client.status());

			// SO_REUSEADDR must be set so tests can reuse socket ports.
			client.option(sock::Option::REUSEADDR, 1);
		    ASSERT_EQ(sock::Status::GOOD, client.status());

		    client.connect();
		    ASSERT_EQ(sock::Status::GOOD, client.status());

		    client.send("Hello there");
		    ASSERT_EQ(sock::Status::GOOD, client.status());

		    sock::Buffer buff;

		    client.receive(buff);
		    ASSERT_EQ(sock::Status::GOOD, client.status());
		    ASSERT_EQ(15, buff.received_size());
		    ASSERT_STREQ("General Kenobi!", buff.buffer());

		    client.send("Bye now!");
		    ASSERT_EQ(sock::Status::GOOD, client.status());

		    client.receive(buff);
		    ASSERT_EQ(sock::Status::GOOD, client.status());
		    ASSERT_EQ(0, buff.received_size());

			client_closed = true;
	    }
	};

	client_thread.join();

	ASSERT_TRUE(server_closed);
	ASSERT_TRUE(client_closed);
}

TEST(Socket, socket_can_handle_callbacks)
{
	bool server_closed = false;
	bool client_closed = false;
	std::string sock_error {""};
	std::string sock_str {""};

	auto& factory = sock::SocketFactory::instance();

	std::thread server_thread {
	    [&factory, &server_closed, &sock_error, &sock_str] () {
		    auto server = factory
				.wrap({
					.domain = sock::Domain::INET,
					.type = sock::Type::STREAM,
					.protocol = sock::Protocol::TCP,
					.port = "9843",
					.flags = sock::Flags::PASSIVE,
				})
				.with([&sock_error, &sock_str] (sock::Socket& socket) {
					sock_str += "a";

					if (socket.status() != sock::Status::GOOD)
					{
						sock_error = sock::error();
					}
				})
				.create()
			;
		    ASSERT_EQ(sock::Status::GOOD, server.status());

			// SO_REUSEADDR must be set so tests can reuse socket ports.
			server.option(sock::Option::REUSEADDR, 1);
		    ASSERT_EQ(sock::Status::GOOD, server.status());

		    server.bind();
		    ASSERT_EQ(sock::Status::GOOD, server.status());

		    server.listen(10);

		    auto connection = server.accept();
		    ASSERT_EQ(sock::Status::GOOD, server.status());
		    ASSERT_EQ(sock::Status::GOOD, connection.status());

		    sock::Buffer buff;
		    ASSERT_EQ(0, buff.received_size());

		    connection.receive(buff);
		    ASSERT_EQ(sock::Status::GOOD, connection.status());
		    ASSERT_EQ(11, buff.received_size());
		    ASSERT_STREQ("Hello there", buff.buffer());

		    connection.send("General Kenobi!");
		    ASSERT_EQ(sock::Status::GOOD, connection.status());

		    connection.receive(buff);
		    ASSERT_EQ(sock::Status::GOOD, connection.status());
		    ASSERT_EQ(8, buff.received_size());
		    ASSERT_STREQ("Bye now!", buff.buffer());

		    connection.send("");
		    ASSERT_EQ(sock::Status::GOOD, connection.status());

			ASSERT_LT(0, sock_str.length());
			ASSERT_STREQ("", sock_error.c_str());

			server_closed = true;
	    }
	};

	server_thread.detach();

	std::thread client_thread {
	    [&factory, &client_closed]()
	    {
		    auto client = factory
				.wrap({
					.domain = sock::Domain::UNSPEC,
					.type = sock::Type::STREAM,
					.protocol = sock::Protocol::TCP,
					.host = "localhost",
					.port = "9843",
				})
				.with([](auto& socket) {
					if (socket.status() != sock::Status::GOOD)
					{
					}
				})
				.create()
			;
		    ASSERT_EQ(sock::Status::GOOD, client.status());

			// SO_REUSEADDR must be set so tests can reuse socket ports.
			client.option(sock::Option::REUSEADDR, 1);
		    ASSERT_EQ(sock::Status::GOOD, client.status());

		    client.connect();
		    ASSERT_EQ(sock::Status::GOOD, client.status());

		    client.send("Hello there");
		    ASSERT_EQ(sock::Status::GOOD, client.status());

		    sock::Buffer buff;

		    client.receive(buff);
		    ASSERT_EQ(sock::Status::GOOD, client.status());
		    ASSERT_EQ(15, buff.received_size());
		    ASSERT_STREQ("General Kenobi!", buff.buffer());

		    client.send("Bye now!");
		    ASSERT_EQ(sock::Status::GOOD, client.status());

		    client.receive(buff);
		    ASSERT_EQ(sock::Status::GOOD, client.status());
		    ASSERT_EQ(0, buff.received_size());

			client_closed = true;
	    }
	};

	client_thread.join();

	ASSERT_TRUE(server_closed);
	ASSERT_TRUE(client_closed);
}
