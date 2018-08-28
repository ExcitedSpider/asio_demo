#pragma once
#include<iostream>
using std::cout;
using std::endl;

#define BUFFER_SIZE 256
#define MAX_SOCKET_AMOUNT 4
#include<boost/thread.hpp>
#include<boost/asio.hpp>
#include<boost/smart_ptr.hpp>
#include<boost/ref.hpp>
#include<boost/bind.hpp>
#include<iostream>
#include<boost/asio/steady_timer.hpp>
#include<boost/asio/system_timer.hpp>
#include<boost/asio/high_resolution_timer.hpp>
#include<string>
#include<boost/function.hpp>
#include<vector>
#include<boost/atomic.hpp>
#include <sstream>
#include <boost/function.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include "chat_message.h"
#include <list>

using std::list;
using namespace boost::asio;
using boost::system::error_code;
using ip::tcp;

class ChatroomClient
{
	typedef boost::shared_ptr<ip::tcp::socket> sockptr;
	typedef boost::shared_array<char> buffers;
	typedef boost::shared_ptr<ChatMessage> msg_ptr;

private:
	io_service& io;
	sockptr sock;
	streambuf buf;
	boost::function<void(msg_ptr)> on_recieve;
	bool is_on_recieve_setted;

public:
	ChatroomClient(io_service &io_) :io(io_), is_on_recieve_setted(false), sock(new ip::tcp::socket(io)) {};

	void start(std::string ipv4)
	{
		ip::tcp::endpoint ep(ip::address::from_string(ipv4), 667);
		sock->async_connect(ep, boost::bind(&ChatroomClient::connect_handler, this, _1));
		io.run();
	};

private:

	void connect_handler(error_code ec)
	{
		if (ec)
			return;
		read();
	}

	void read()
	{
		sock->async_read_some(buf.prepare(BUFFER_SIZE), boost::bind(&ChatroomClient::read_handler, this,_1, _2));
	}

	void read_handler(error_code ec, size_t bites_trans)
	{
		buf.commit(bites_trans);
		boost::archive::binary_iarchive ia(buf);
		ChatMessage msg;
		ia >> msg;
		cout << "[player]\t\t[message]" << endl;
		cout << msg.playerName << "\t\t" << msg.message << endl;
		read();
	}
};

int main()
{
	io_service io;
	ChatroomClient client(io);
	client.start("127.0.0.1");
}