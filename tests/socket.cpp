#include "sock/socket_factory.hpp"
#include "sock/utils.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

class Status
{
public:
	auto status() const -> sock::Status
	{
		return m_status;
	}

	auto status(sock::Status s) -> void
	{
		m_status = s;

		if (s != sock::Status::GOOD)
		{
			m_error = sock::error();
		}
	}

	auto error() const -> const std::string&
	{
		return m_error;
	}

	Status() {}

	Status(sock::Status s)
	{
		this->status(s);
	}

	auto operator()(sock::Status s) -> void
	{
		this->status(s);
	}

private:
	sock::Status m_status {sock::Status::GOOD};
	std::string m_error;
};

struct State
{
	Status create;
	std::vector<Status> option;
	Status bind;
	Status listen;
	std::vector<Status> accept;
	Status connect;
	std::vector<Status> receive;
	std::vector<Status> send;
	Status shutdown;
};

void assert_state(const State& actual, State&& expected = State {})
{
	ASSERT_EQ(expected.bind.status(), actual.create.status())
	    << actual.create.error();
	ASSERT_EQ(expected.connect.status(), actual.connect.status())
	    << actual.connect.error();
	ASSERT_EQ(expected.bind.status(), actual.bind.status())
	    << actual.bind.error();
	ASSERT_EQ(expected.listen.status(), actual.listen.status())
	    << actual.listen.error();
	ASSERT_EQ(expected.shutdown.status(), actual.shutdown.status())
	    << actual.shutdown.error();

	ASSERT_EQ(expected.send.size(), actual.send.size());
	size_t idx = 0;
	for (const auto& i : actual.send)
	{
		ASSERT_EQ(expected.send[idx].status(), i.status())
		    << "send [" << idx << "] " << i.error();
		idx++;
	}

	ASSERT_EQ(expected.receive.size(), actual.receive.size());
	idx = 0;
	for (const auto& i : actual.receive)
	{
		ASSERT_EQ(expected.receive[idx].status(), i.status())
		    << "receive [" << idx << "] " << i.error();
		idx++;
	}

	ASSERT_EQ(expected.accept.size(), actual.accept.size());
	idx = 0;
	for (const auto& i : actual.accept)
	{
		ASSERT_EQ(expected.accept[idx].status(), i.status())
		    << "accept [" << idx << "] " << i.error();
		idx++;
	}

	ASSERT_EQ(expected.option.size(), actual.option.size());
	idx = 0;
	for (const auto& i : actual.option)
	{
		ASSERT_EQ(expected.option[idx].status(), i.status())
		    << "option [" << idx << "] " << i.error();
		idx++;
	}
};

GTEST_TEST(Socket, server_and_client_communication_test)
{
	bool server_closed = false;
	bool client_closed = false;

	State server_state;
	State connection_state;
	State client_state;

	auto& factory = sock::SocketFactory::instance();
	auto server = factory.create({
	    .domain = sock::Domain::INET,
	    .type = sock::Type::STREAM,
	    .protocol = sock::Protocol::TCP,
	    .flags = sock::Flags::PASSIVE,
	});
	server_state.create(server.status());

	server.option(sock::Option::REUSEADDR, 1);
	server_state.option.push_back(server.status());

	server.bind({.host = "localhost", .port = "8843"});
	server_state.bind = server.status();

	std::thread server_thread {
	    [&server, &server_closed, &server_state, &connection_state]()
	    {
		    server.listen(1);

		    auto connection = server.accept();
		    server_state.accept.push_back(server.status());
		    connection_state.create(connection.status());

		    sock::Buffer buff;
		    ASSERT_EQ(0, buff.received_size());

		    connection.receive(buff);
		    connection_state.receive.push_back(connection.status());
		    ASSERT_EQ(11, buff.received_size());
		    ASSERT_STREQ("Hello there", buff.buffer());
		    ASSERT_STREQ("Hello there", buff.view().data());

		    connection.send("General Kenobi!");
		    connection_state.send.push_back(connection.status());

		    connection.receive(buff);
		    connection_state.receive.push_back(connection.status());
		    ASSERT_EQ(8, buff.received_size());
		    ASSERT_STREQ("Bye now!", buff.buffer());
		    ASSERT_STREQ("Bye now!", buff.view().data());

		    connection.send("");
		    connection_state.send.push_back(connection.status());

		    connection.shutdown();
		    connection_state.shutdown(connection.status());

		    server_closed = true;
	    }};

	server_thread.detach();

	auto client = factory.create({
	    .domain = sock::Domain::INET,
	    .type = sock::Type::STREAM,
	    .protocol = sock::Protocol::TCP,
	});
	client_state.create(client.status());

	std::thread client_thread {
	    [&client, &client_closed, &client_state]()
	    {
		    client.connect({.host = "localhost", .port = "8843"});
		    client_state.connect(client.status());

		    client.send("Hello there");
		    client_state.send.push_back(client.status());

		    sock::Buffer buff;

		    client.receive(buff);
		    client_state.receive.push_back(client.status());
		    ASSERT_EQ(15, buff.received_size());
		    ASSERT_STREQ("General Kenobi!", buff.buffer());
		    ASSERT_STREQ("General Kenobi!", buff.view().data());

		    client.send("Bye now!");
		    client_state.send.push_back(client.status());

		    client.receive(buff);
		    client_state.receive.push_back(client.status());
		    ASSERT_EQ(0, buff.received_size());

		    client_closed = true;
	    }};

	client_thread.join();

	ASSERT_EQ(true, server_closed);
	ASSERT_EQ(true, client_closed);

	assert_state(
	    server_state,
	    State {
	        .create {sock::Status::GOOD},
	        .option {{sock::Status::GOOD}},
	        .bind {sock::Status::GOOD},
	        .listen {sock::Status::GOOD},
	        .accept {{sock::Status::GOOD}},
	        .connect {},
	        .receive {},
	        .send {},
	        .shutdown {sock::Status::GOOD},
	    }
	);
	assert_state(
	    connection_state,
	    State {
	        .create {sock::Status::GOOD},
	        .option {},
	        .bind {},
	        .listen {},
	        .accept {},
	        .connect {},
	        .receive {{sock::Status::GOOD}, {sock::Status::GOOD}},
	        .send {{sock::Status::GOOD}, {sock::Status::GOOD}},
	        .shutdown {sock::Status::GOOD},
    }
	);
	assert_state(
	    client_state,
	    State {
	        .create {sock::Status::GOOD},
	        .option {},
	        .bind {sock::Status::GOOD},
	        .listen {},
	        .accept {},
	        .connect {sock::Status::GOOD},
	        .receive {{sock::Status::GOOD}, {sock::Status::GOOD}},
	        .send {{sock::Status::GOOD}, {sock::Status::GOOD}},
	        .shutdown {sock::Status::GOOD},
    }
	);
}

GTEST_TEST(Socket, socket_can_handle_callbacks)
{
	bool server_closed = false;
	bool client_closed = false;
	std::string sock_error {""};
	std::string sock_str {""};

	State server_state;
	State connection_state;
	State client_state;

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
	server_state.create(server.status());

	server.option(sock::Option::REUSEADDR, 1);
	server_state.option.push_back(server.status());

	server.bind({.host = "localhost", .port = "9843"});
	server_state.bind = server.status();

	std::thread server_thread {
	    [&server,
	     &server_closed,
	     &sock_error,
	     &sock_str,
	     &server_state,
	     &connection_state]()
	    {
		    server.listen(1);

		    auto conn = server.accept();

		    // Testing that socket-wrapper can be moved without errors.
		    {
			    auto connection = std::move(conn);

			    server_state.accept.push_back(server.status());
			    connection_state.create(connection.status());

			    sock::Buffer buff;
			    ASSERT_EQ(0, buff.received_size());

			    connection.receive(buff);
			    connection_state.receive.push_back(connection.status());
			    ASSERT_EQ(11, buff.received_size());
			    ASSERT_STREQ("Hello there", buff.buffer());
			    ASSERT_STREQ("Hello there", buff.view().data());

			    connection.send("General Kenobi!");
			    connection_state.send.push_back(connection.status());

			    connection.receive(buff);
			    connection_state.receive.push_back(connection.status());
			    ASSERT_EQ(8, buff.received_size());
			    ASSERT_STREQ("Bye now!", buff.buffer());
			    ASSERT_STREQ("Bye now!", buff.view().data());

			    connection.send("");
			    connection_state.send.push_back(connection.status());

			    ASSERT_LT(0, sock_str.length());
			    ASSERT_STREQ("", sock_error.c_str());

			    connection.shutdown();
			    connection_state.shutdown(connection.status());
		    }

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
	client_state.create(client.status());

	std::thread client_thread {
	    [&client, &client_closed, &client_state]()
	    {
		    client.connect({"localhost", "9843"});
		    client_state.connect(client.status());

		    client.send("Hello there");
		    client_state.send.push_back(client.status());

		    sock::Buffer buff;

		    client.receive(buff);
		    client_state.receive.push_back(client.status());
		    ASSERT_EQ(15, buff.received_size());
		    ASSERT_STREQ("General Kenobi!", buff.buffer());
		    ASSERT_STREQ("General Kenobi!", buff.view().data());

		    client.send("Bye now!");
		    client_state.send.push_back(client.status());

		    client.receive(buff);
		    client_state.receive.push_back(client.status());
		    ASSERT_EQ(0, buff.received_size());

		    client_closed = true;
	    }};

	client_thread.join();

	ASSERT_EQ(true, server_closed);
	ASSERT_EQ(true, client_closed);

	assert_state(
	    server_state,
	    State {
	        .create {sock::Status::GOOD},
	        .option {{sock::Status::GOOD}},
	        .bind {sock::Status::GOOD},
	        .listen {sock::Status::GOOD},
	        .accept {{sock::Status::GOOD}},
	        .connect {},
	        .receive {},
	        .send {},
	        .shutdown {sock::Status::GOOD},
	    }
	);
	assert_state(
	    connection_state,
	    State {
	        .create {sock::Status::GOOD},
	        .option {},
	        .bind {},
	        .listen {},
	        .accept {},
	        .connect {},
	        .receive {{sock::Status::GOOD}, {sock::Status::GOOD}},
	        .send {{sock::Status::GOOD}, {sock::Status::GOOD}},
	        .shutdown {sock::Status::GOOD},
    }
	);
	assert_state(
	    client_state,
	    State {
	        .create {sock::Status::GOOD},
	        .option {},
	        .bind {sock::Status::GOOD},
	        .listen {},
	        .accept {},
	        .connect {sock::Status::GOOD},
	        .receive {{sock::Status::GOOD}, {sock::Status::GOOD}},
	        .send {{sock::Status::GOOD}, {sock::Status::GOOD}},
	        .shutdown {sock::Status::GOOD},
    }
	);
}

GTEST_TEST(Socket, does_wait_for_specified_time)
{
	using namespace std::chrono_literals;

	bool server_closed = false;
	bool client_closed = false;

	State server_state;
	State connection_state;
	State client_state;

	auto& factory = sock::SocketFactory::instance();
	auto server = factory.create({
	    .domain = sock::Domain::UNSPEC,
	    .type = sock::Type::STREAM,
	    .protocol = sock::Protocol::TCP,
	});
	server_state.create(server.status());

	// Setting REUSEADDR before binding alows tests to reuse sockets.
	// Run `cat /proc/net/tcp` to view active sockets.
	server.option(sock::Option::REUSEADDR, 1);
	server_state.option.push_back(server.status());

	server.bind({.host = "localhost", .port = "10843"});
	server_state.bind = server.status();

	server.option(sock::Option::RCVTIMEO, 1500ms);
	server_state.option.push_back(server.status());

	std::thread server_thread {
	    [&server, &server_closed, &server_state, &connection_state]()
	    {
		    server.listen(1);

		    auto connection = server.accept();
		    server_state.accept.push_back(server.status());
		    connection_state.create(connection.status());

		    sock::Buffer buff;
		    ASSERT_EQ(0, buff.received_size());

		    connection.receive(buff);
		    connection_state.receive.push_back(connection.status());
		    ASSERT_EQ(11, buff.received_size());
		    ASSERT_STREQ("Hello there", buff.buffer());

		    connection.send("General Kenobi!");
		    connection_state.send.push_back(connection.status());

		    connection.receive(buff);
		    connection_state.receive.push_back(connection.status());
		    ASSERT_EQ(8, buff.received_size());
		    ASSERT_STREQ("Bye now!", buff.buffer());

		    connection.send("Goodbye");
		    connection_state.send.push_back(connection.status());

		    connection.shutdown();
		    connection_state.shutdown(connection.status());

		    server_closed = true;
	    }};

	server_thread.detach();

	auto client = factory.create({
	    .domain = sock::Domain::UNSPEC,
	    .type = sock::Type::STREAM,
	    .protocol = sock::Protocol::TCP,
	});
	client_state.create(client.status());

	std::thread client_thread {
	    [&client, &client_closed, &client_state]()
	    {
		    client.connect({"localhost", "10843"});
		    client_state.connect(client.status());

		    client.send("Hello there");
		    client_state.send.push_back(client.status());

		    sock::Buffer buff;

		    client.receive(buff);
		    client_state.receive.push_back(client.status());
		    ASSERT_EQ(15, buff.received_size());
		    ASSERT_STREQ("General Kenobi!", buff.buffer());

		    std::this_thread::sleep_for(1000ms);

		    client.send("Bye now!");
		    client_state.send.push_back(client.status());

		    client.receive(buff);
		    client_state.receive.push_back(client.status());
		    ASSERT_STREQ("Goodbye", buff.buffer());
		    ASSERT_EQ(7, buff.received_size());

		    client_closed = true;
	    }};

	client_thread.join();

	ASSERT_EQ(true, server_closed);
	ASSERT_EQ(true, client_closed);

	assert_state(
	    server_state,
	    State {
	        .create {sock::Status::GOOD},
	        .option {{sock::Status::GOOD}, {sock::Status::GOOD}},
	        .bind {sock::Status::GOOD},
	        .listen {sock::Status::GOOD},
	        .accept {{sock::Status::GOOD}},
	        .connect {},
	        .receive {},
	        .send {},
	        .shutdown {sock::Status::GOOD},
    }
	);
	assert_state(
	    connection_state,
	    State {
	        .create {sock::Status::GOOD},
	        .option {},
	        .bind {},
	        .listen {},
	        .accept {},
	        .connect {},
	        .receive {{sock::Status::GOOD}, {sock::Status::GOOD}},
	        .send {{sock::Status::GOOD}, {sock::Status::GOOD}},
	        .shutdown {sock::Status::GOOD},
    }
	);
	assert_state(
	    client_state,
	    State {
	        .create {sock::Status::GOOD},
	        .option {},
	        .bind {sock::Status::GOOD},
	        .listen {},
	        .accept {},
	        .connect {sock::Status::GOOD},
	        .receive {{sock::Status::GOOD}, {sock::Status::GOOD}},
	        .send {{sock::Status::GOOD}, {sock::Status::GOOD}},
	        .shutdown {sock::Status::GOOD},
    }
	);
}

GTEST_TEST(Socket, server_does_disconnect_when_client_hangs)
{
	using namespace std::chrono_literals;

	bool server_closed = false;
	bool client_closed = false;

	State server_state;
	State connection_state;
	State client_state;

	auto& factory = sock::SocketFactory::instance();
	auto server = factory.create({
	    .domain = sock::Domain::UNSPEC,
	    .type = sock::Type::STREAM,
	    .protocol = sock::Protocol::TCP,
	});
	server_state.create(server.status());

	server.option(sock::Option::REUSEADDR, 1);
	server_state.option.push_back(server.status());

	server.bind({.host = "localhost", .port = "11843"});
	server_state.bind = server.status();

	server.option(sock::Option::RCVTIMEO, 100ms);
	server_state.option.push_back(server.status());

	std::thread server_thread {
	    [&server, &server_closed, &server_state, &connection_state]()
	    {
		    server.listen(1);

		    auto conn = server.accept();

		    // Testing that socket can be moved without errors.
		    {
			    auto connection = std::move(conn);

			    server_state.accept.push_back(server.status());
			    connection_state.create(connection.status());

			    sock::Buffer buff;
			    ASSERT_EQ(0, buff.received_size());

			    connection.receive(buff);
			    connection_state.receive.push_back(connection.status());
			    ASSERT_EQ(11, buff.received_size());
			    ASSERT_STREQ("Hello there", buff.buffer());

			    connection.send("General Kenobi!");
			    connection_state.send.push_back(connection.status());

			    // Server should not receive data if client didnt send anything
			    // for 100ms.
			    connection.receive(buff);
			    connection_state.receive.push_back(connection.status());
			    ASSERT_STREQ("", buff.buffer());
			    ASSERT_EQ(0, buff.received_size());
			    connection_state.shutdown(connection.status());

			    connection.shutdown();
		    }

		    server_closed = true;
	    }};

	server_thread.detach();

	auto client = factory.create({
	    .domain = sock::Domain::UNSPEC,
	    .type = sock::Type::STREAM,
	    .protocol = sock::Protocol::TCP,
	});
	client_state.create(client.status());

	std::thread client_thread {
	    [&client, &client_closed, &client_state]()
	    {
		    client.connect({"localhost", "11843"});
		    client_state.connect(client.status());

		    client.send("Hello there");
		    client_state.send.push_back(client.status());

		    sock::Buffer buff;

		    client.receive(buff);
		    client_state.receive.push_back(client.status());
		    ASSERT_EQ(15, buff.received_size());
		    ASSERT_STREQ("General Kenobi!", buff.buffer());

		    std::this_thread::sleep_for(200ms);

		    client.send("Bye now!");
		    client_state.send.push_back(client.status());

		    client.receive(buff);
		    client_state.receive.push_back(client.status());
		    ASSERT_STREQ("", buff.buffer());
		    ASSERT_EQ(0, buff.received_size());

		    client_closed = true;
	    }};

	client_thread.join();

	ASSERT_EQ(true, server_closed);
	ASSERT_EQ(true, client_closed);

	assert_state(
	    server_state,
	    State {
	        .create {sock::Status::GOOD},
	        .option {{sock::Status::GOOD}, {sock::Status::GOOD}},
	        .bind {sock::Status::GOOD},
	        .listen {sock::Status::GOOD},
	        .accept {{sock::Status::GOOD}},
	        .connect {},
	        .receive {},
	        .send {},
	        .shutdown {sock::Status::GOOD},
    }
	);
	/* last error was recorded after receive(), shutdown will report
	 * RECEIVE_ERROR. */
	assert_state(
	    connection_state,
	    State {
	        .create {sock::Status::GOOD},
	        .option {},
	        .bind {},
	        .listen {},
	        .accept {},
	        .connect {},
	        .receive {{sock::Status::GOOD}, {sock::Status::RECEIVE_ERROR}},
	        .send {{sock::Status::GOOD}},
	        .shutdown {sock::Status::RECEIVE_ERROR},
    }
	);
	assert_state(
	    client_state,
	    State {
	        .create {sock::Status::GOOD},
	        .option {},
	        .bind {sock::Status::GOOD},
	        .listen {},
	        .accept {},
	        .connect {sock::Status::GOOD},
	        .receive {{sock::Status::GOOD}, {sock::Status::RECEIVE_ERROR}},
	        .send {{sock::Status::GOOD}, {sock::Status::GOOD}},
	        .shutdown {},
    }
	);
}
