#include <gtest/gtest.h>
#include <iostream>
#include "sock/socket_factory.hpp"

TEST(Socket, ServerAndClient)
{
	auto& factory = sock::SocketFactory::instance();

	std::thread server_thread{[&factory]() {
		auto server = factory.create({
			.domain = sock::Domain::INET,
			.type = sock::Type::STREAM,
			.protocol = sock::Protocol::TCP,
			.port = "8843",
			.flags = sock::Flags::PASSIVE,
		});
		ASSERT_EQ(sock::Socket::Status::GOOD, server.status());

		server.bind();
		ASSERT_EQ(sock::Socket::Status::GOOD, server.status());

		server.listen(10);

		auto connection = server.accept();
		ASSERT_EQ(sock::Socket::Status::GOOD, server.status());
		ASSERT_EQ(sock::Socket::Status::GOOD, connection.status());

		sock::Buffer buff;
		ASSERT_EQ(0, buff.received_size());

		connection.receive(buff);
		ASSERT_EQ(sock::Socket::Status::GOOD, connection.status());
		ASSERT_EQ(11, buff.received_size());
		ASSERT_STREQ("Hello there", buff.buffer());

		connection.send("General Kenobi!");
		ASSERT_EQ(sock::Socket::Status::GOOD, connection.status());

		connection.receive(buff);
		ASSERT_EQ(sock::Socket::Status::GOOD, connection.status());
		ASSERT_EQ(8, buff.received_size());
		ASSERT_STREQ("Bye now!", buff.buffer());

		connection.send("");
		ASSERT_EQ(sock::Socket::Status::GOOD, connection.status());
	}};

	server_thread.detach();

	std::thread client_thread{[&factory]() {
		auto client = factory.create({
			.domain = sock::Domain::UNSPEC,
			.type = sock::Type::STREAM,
			.protocol = sock::Protocol::TCP,
			.host = "localhost",
			.port = "8843",
		});
		ASSERT_EQ(sock::Socket::Status::GOOD, client.status());

		client.connect();
		ASSERT_EQ(sock::Socket::Status::GOOD, client.status());

		client.send("Hello there");
		ASSERT_EQ(sock::Socket::Status::GOOD, client.status());

		sock::Buffer buff;

		client.receive(buff);
		ASSERT_EQ(sock::Socket::Status::GOOD, client.status());
		ASSERT_EQ(15, buff.received_size());
		ASSERT_STREQ("General Kenobi!", buff.buffer());

		client.send("Bye now!");
		ASSERT_EQ(sock::Socket::Status::GOOD, client.status());

		client.receive(buff);
		ASSERT_EQ(sock::Socket::Status::GOOD, client.status());
		ASSERT_EQ(0, buff.received_size());
	}};

	client_thread.join();
}
