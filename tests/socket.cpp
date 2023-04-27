#include "sock/socket_factory.hpp"
#include "sock/utils.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <thread>

GTEST_TEST(Socket, server_and_client_communication_test)
{
	bool server_closed = false;
	bool client_closed = false;

	auto& factory = sock::SocketFactory::instance();
	auto server = factory.create({
	    .domain = sock::Domain::INET,
	    .type = sock::Type::STREAM,
	    .protocol = sock::Protocol::TCP,
	    .flags = sock::Flags::PASSIVE,
	});
	ASSERT_EQ(sock::Status::GOOD, server.status()) << sock::error();

	server.bind({.host = "localhost", .port = "8843"});
	ASSERT_EQ(sock::Status::GOOD, server.status()) << sock::error();

	// SO_REUSEADDR must be set so tests can reuse socket ports.
	server.option(sock::Option::REUSEADDR, 1);
	ASSERT_EQ(sock::Status::GOOD, server.status()) << sock::error();

	std::thread server_thread {
	    [&server, &server_closed]()
	    {
		    server.listen(1);

		    auto connection = server.accept();
		    ASSERT_EQ(sock::Status::GOOD, server.status()) << sock::error();
		    ASSERT_EQ(sock::Status::GOOD, connection.status())
		        << sock::error();

		    sock::Buffer buff;
		    ASSERT_EQ(0, buff.received_size());

		    connection.receive(buff);
		    ASSERT_EQ(sock::Status::GOOD, connection.status())
		        << sock::error();
		    ASSERT_EQ(11, buff.received_size());
		    ASSERT_STREQ("Hello there", buff.buffer());
		    ASSERT_STREQ("Hello there", buff.view().data());

		    connection.send("General Kenobi!");
		    ASSERT_EQ(sock::Status::GOOD, connection.status())
		        << sock::error();

		    connection.receive(buff);
		    ASSERT_EQ(sock::Status::GOOD, connection.status())
		        << sock::error();
		    ASSERT_EQ(8, buff.received_size());
		    ASSERT_STREQ("Bye now!", buff.buffer());
		    ASSERT_STREQ("Bye now!", buff.view().data());

		    connection.send("");
		    ASSERT_EQ(sock::Status::GOOD, connection.status())
		        << sock::error();

		    server_closed = true;
	    }};

	server_thread.detach();

	auto client = factory.create({
	    .domain = sock::Domain::INET,
	    .type = sock::Type::STREAM,
	    .protocol = sock::Protocol::TCP,
	});
	ASSERT_EQ(sock::Status::GOOD, client.status());

	// SO_REUSEADDR must be set so tests can reuse socket ports.
	client.option(sock::Option::REUSEADDR, 1);
	ASSERT_EQ(sock::Status::GOOD, client.status());

	std::thread client_thread {
	    [&client, &client_closed]()
	    {
		    client.connect({.host = "localhost", .port = "8843"});
		    ASSERT_EQ(sock::Status::GOOD, client.status()) << sock::error();

		    client.send("Hello there");
		    ASSERT_EQ(sock::Status::GOOD, client.status()) << sock::error();

		    sock::Buffer buff;

		    client.receive(buff);
		    ASSERT_EQ(sock::Status::GOOD, client.status()) << sock::error();
		    ASSERT_EQ(15, buff.received_size());
		    ASSERT_STREQ("General Kenobi!", buff.buffer());
		    ASSERT_STREQ("General Kenobi!", buff.view().data());

		    client.send("Bye now!");
		    ASSERT_EQ(sock::Status::GOOD, client.status()) << sock::error();

		    client.receive(buff);
		    ASSERT_EQ(sock::Status::GOOD, client.status()) << sock::error();
		    ASSERT_EQ(0, buff.received_size());

		    client_closed = true;
	    }};

	client_thread.join();

	ASSERT_EQ(true, server_closed);
	ASSERT_EQ(true, client_closed);
}

GTEST_TEST(Socket, socket_can_handle_callbacks)
{
	bool server_closed = false;
	bool client_closed = false;
	std::string sock_error {""};
	std::string sock_str {""};

	auto& factory = sock::SocketFactory::instance();
	auto server = factory
	                  .wrap({
	                      .domain = sock::Domain::UNSPEC,
	                      .type = sock::Type::STREAM,
	                      .protocol = sock::Protocol::TCP,
	                  })
	                  .with(
	                      [&sock_error, &sock_str](sock::Socket& socket)
	                      {
		                      sock_str += "a";

		                      if (socket.status() != sock::Status::GOOD)
		                      {
			                      sock_error = sock::error();
		                      }
	                      }
	                  )
	                  .create();
	ASSERT_EQ(sock::Status::GOOD, server.status()) << sock::error();

	server.bind({.host = "localhost", .port = "9843"});
	ASSERT_EQ(sock::Status::GOOD, server.status()) << sock::error();

	// SO_REUSEADDR must be set so tests can reuse socket ports.
	server.option(sock::Option::REUSEADDR, 1);
	ASSERT_EQ(sock::Status::GOOD, server.status()) << sock::error();

	std::thread server_thread {
	    [&server, &server_closed, &sock_error, &sock_str]()
	    {
		    server.listen(1);

		    auto connection = server.accept();
		    ASSERT_EQ(sock::Status::GOOD, server.status()) << sock::error();
		    ASSERT_EQ(sock::Status::GOOD, connection.status())
		        << sock::error();

		    sock::Buffer buff;
		    ASSERT_EQ(0, buff.received_size());

		    connection.receive(buff);
		    ASSERT_EQ(sock::Status::GOOD, connection.status())
		        << sock::error();
		    ASSERT_EQ(11, buff.received_size());
		    ASSERT_STREQ("Hello there", buff.buffer());
		    ASSERT_STREQ("Hello there", buff.view().data());

		    connection.send("General Kenobi!");
		    ASSERT_EQ(sock::Status::GOOD, connection.status())
		        << sock::error();

		    connection.receive(buff);
		    ASSERT_EQ(sock::Status::GOOD, connection.status())
		        << sock::error();
		    ASSERT_EQ(8, buff.received_size());
		    ASSERT_STREQ("Bye now!", buff.buffer());
		    ASSERT_STREQ("Bye now!", buff.view().data());

		    connection.send("");
		    ASSERT_EQ(sock::Status::GOOD, connection.status())
		        << sock::error();

		    ASSERT_LT(0, sock_str.length());
		    ASSERT_STREQ("", sock_error.c_str());

		    server_closed = true;
	    }};

	server_thread.detach();

	auto client = factory
	                  .wrap({
	                      .domain = sock::Domain::UNSPEC,
	                      .type = sock::Type::STREAM,
	                      .protocol = sock::Protocol::TCP,
	                  })
	                  .with(
	                      [](auto& socket)
	                      {
		                      if (socket.status() != sock::Status::GOOD)
		                      {}
	                      }
	                  )
	                  .create();
	ASSERT_EQ(sock::Status::GOOD, client.status()) << sock::error();

	// SO_REUSEADDR must be set so tests can reuse socket ports.
	client.option(sock::Option::REUSEADDR, 1);
	ASSERT_EQ(sock::Status::GOOD, client.status()) << sock::error();

	std::thread client_thread {
	    [&client, &client_closed]()
	    {
		    client.connect({"localhost", "9843"});
		    ASSERT_EQ(sock::Status::GOOD, client.status()) << sock::error();

		    client.send("Hello there");
		    ASSERT_EQ(sock::Status::GOOD, client.status()) << sock::error();

		    sock::Buffer buff;

		    client.receive(buff);
		    ASSERT_EQ(sock::Status::GOOD, client.status()) << sock::error();
		    ASSERT_EQ(15, buff.received_size());
		    ASSERT_STREQ("General Kenobi!", buff.buffer());
		    ASSERT_STREQ("General Kenobi!", buff.view().data());

		    client.send("Bye now!");
		    ASSERT_EQ(sock::Status::GOOD, client.status()) << sock::error();

		    client.receive(buff);
		    ASSERT_EQ(sock::Status::GOOD, client.status()) << sock::error();
		    ASSERT_EQ(0, buff.received_size());

		    client_closed = true;
	    }};

	client_thread.join();

	ASSERT_EQ(true, server_closed);
	ASSERT_EQ(true, client_closed);
}

GTEST_TEST(Socket, does_wait_for_specified_time)
{
	using namespace std::chrono_literals;

	bool server_closed = false;
	bool client_closed = false;

	auto& factory = sock::SocketFactory::instance();
	auto server = factory.create({
	    .domain = sock::Domain::UNSPEC,
	    .type = sock::Type::STREAM,
	    .protocol = sock::Protocol::TCP,
	});
	ASSERT_EQ(sock::Status::GOOD, server.status()) << sock::error();

	server.bind({.host = "localhost", .port = "9843"});
	ASSERT_EQ(sock::Status::GOOD, server.status()) << sock::error();

	server.option(sock::Option::REUSEADDR, 1);
	ASSERT_EQ(sock::Status::GOOD, server.status()) << sock::error();

	server.option(sock::Option::RCVTIMEO, 1500ms);
	ASSERT_EQ(sock::Status::GOOD, server.status()) << sock::error();

	std::thread server_thread {
	    [&server, &server_closed]()
	    {
		    server.listen(1);

		    auto connection = server.accept();
		    ASSERT_EQ(sock::Status::GOOD, server.status()) << sock::error();
		    ASSERT_EQ(sock::Status::GOOD, connection.status())
		        << sock::error();

		    sock::Buffer buff;
		    ASSERT_EQ(0, buff.received_size());

		    connection.receive(buff);
		    ASSERT_EQ(sock::Status::GOOD, connection.status())
		        << sock::error();
		    ASSERT_EQ(11, buff.received_size());
		    ASSERT_STREQ("Hello there", buff.buffer());

		    connection.send("General Kenobi!");
		    ASSERT_EQ(sock::Status::GOOD, connection.status())
		        << sock::error();

		    connection.receive(buff);
		    ASSERT_EQ(sock::Status::GOOD, connection.status())
		        << sock::error();
		    ASSERT_EQ(8, buff.received_size());
		    ASSERT_STREQ("Bye now!", buff.buffer());

		    connection.send("Goodbye");
		    ASSERT_EQ(sock::Status::GOOD, connection.status())
		        << sock::error();

		    server_closed = true;
	    }};

	server_thread.detach();

	auto client = factory.create({
	    .domain = sock::Domain::UNSPEC,
	    .type = sock::Type::STREAM,
	    .protocol = sock::Protocol::TCP,
	});
	ASSERT_EQ(sock::Status::GOOD, client.status()) << sock::error();

	// SO_REUSEADDR must be set so tests can reuse socket ports.
	client.option(sock::Option::REUSEADDR, 1);
	ASSERT_EQ(sock::Status::GOOD, client.status()) << sock::error();

	std::thread client_thread {
	    [&client, &client_closed]()
	    {
		    client.connect({"localhost", "9843"});
		    ASSERT_EQ(sock::Status::GOOD, client.status()) << sock::error();

		    client.send("Hello there");
		    ASSERT_EQ(sock::Status::GOOD, client.status()) << sock::error();

		    sock::Buffer buff;

		    client.receive(buff);
		    ASSERT_EQ(sock::Status::GOOD, client.status()) << sock::error();
		    ASSERT_EQ(15, buff.received_size());
		    ASSERT_STREQ("General Kenobi!", buff.buffer());

		    std::this_thread::sleep_for(1000ms);

		    client.send("Bye now!");
		    ASSERT_EQ(sock::Status::GOOD, client.status()) << sock::error();

		    client.receive(buff);
		    ASSERT_EQ(sock::Status::GOOD, client.status()) << sock::error();
		    ASSERT_STREQ("Goodbye", buff.buffer());
		    ASSERT_EQ(7, buff.received_size());

		    client_closed = true;
	    }};

	client_thread.join();

	ASSERT_EQ(true, server_closed);
	ASSERT_EQ(true, client_closed);
}

GTEST_TEST(Socket, server_does_disconnect_when_client_hangs)
{
	using namespace std::chrono_literals;

	bool server_closed = false;
	bool client_closed = false;

	auto& factory = sock::SocketFactory::instance();
	auto server = factory.create({
	    .domain = sock::Domain::UNSPEC,
	    .type = sock::Type::STREAM,
	    .protocol = sock::Protocol::TCP,
	});
	ASSERT_EQ(sock::Status::GOOD, server.status()) << sock::error();

	server.bind({.host = "localhost", .port = "9843"});
	ASSERT_EQ(sock::Status::GOOD, server.status()) << sock::error();

	server.option(sock::Option::REUSEADDR, 1);
	ASSERT_EQ(sock::Status::GOOD, server.status()) << sock::error();

	server.option(sock::Option::RCVTIMEO, 100ms);
	ASSERT_EQ(sock::Status::GOOD, server.status()) << sock::error();

	std::thread server_thread {
	    [&server, &server_closed]()
	    {
		    server.listen(1);

		    auto connection = server.accept();
		    ASSERT_EQ(sock::Status::GOOD, server.status()) << sock::error();
		    ASSERT_EQ(sock::Status::GOOD, connection.status())
		        << sock::error();

		    sock::Buffer buff;
		    ASSERT_EQ(0, buff.received_size());

		    connection.receive(buff);
		    ASSERT_EQ(sock::Status::GOOD, connection.status())
		        << sock::error();
		    ASSERT_EQ(11, buff.received_size());
		    ASSERT_STREQ("Hello there", buff.buffer());

		    connection.send("General Kenobi!");
		    ASSERT_EQ(sock::Status::GOOD, connection.status())
		        << sock::error();

		    connection.receive(buff);

		    ASSERT_EQ(
		        sock::Status::RECEIVE_ERROR,
		        connection.status()
		    ) << "Server should not receive data if client didnt send anything for 100ms.";
		    ASSERT_STREQ("", buff.buffer());
		    ASSERT_EQ(0, buff.received_size());

		    server_closed = true;
	    }};

	server_thread.detach();

	auto client = factory.create({
	    .domain = sock::Domain::UNSPEC,
	    .type = sock::Type::STREAM,
	    .protocol = sock::Protocol::TCP,
	});
	ASSERT_EQ(sock::Status::GOOD, client.status()) << sock::error();

	// SO_REUSEADDR must be set so tests can reuse socket ports.
	client.option(sock::Option::REUSEADDR, 1);
	ASSERT_EQ(sock::Status::GOOD, client.status()) << sock::error();

	std::thread client_thread {
	    [&client, &client_closed]()
	    {
		    client.connect({"localhost", "9843"});
		    ASSERT_EQ(sock::Status::GOOD, client.status()) << sock::error();

		    client.send("Hello there");
		    ASSERT_EQ(sock::Status::GOOD, client.status()) << sock::error();

		    sock::Buffer buff;

		    client.receive(buff);
		    ASSERT_EQ(sock::Status::GOOD, client.status()) << sock::error();
		    ASSERT_EQ(15, buff.received_size());
		    ASSERT_STREQ("General Kenobi!", buff.buffer());

		    std::this_thread::sleep_for(200ms);

		    client.send("Bye now!");
		    ASSERT_EQ(sock::Status::GOOD, client.status()) << sock::error();

		    client.receive(buff);
		    ASSERT_EQ(
		        sock::Status::RECEIVE_ERROR,
		        client.status()
		    ) << "Client should not receive data because server closed connection after 100ms of inactivity.";
		    ASSERT_STREQ("", buff.buffer());
		    ASSERT_EQ(0, buff.received_size());

		    client_closed = true;
	    }};

	client_thread.join();

	ASSERT_EQ(true, server_closed);
	ASSERT_EQ(true, client_closed);
}
