#pragma once
#include<iostream>
using std::cout;
using std::endl;

#define BOOST_ASIO_DISABLE_STD_CHRONO
#define BOOST_THREAD_VERSION 5
#define MAX_SOCKET_AMOUNT 4
#define BUFFER_SIZE 1024
#include<boost/thread.hpp>
#include<boost/asio.hpp>
#include<boost/smart_ptr.hpp>
#include<boost/bind.hpp>
#include<iostream>
#include<boost/asio/steady_timer.hpp>
#include<boost/asio/system_timer.hpp>
#include<boost/asio/high_resolution_timer.hpp>
#include<string>
#include<boost/function.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/string.hpp>
#include<vector>
#include<boost/atomic.hpp>
#include <sstream>
#include <boost/random.hpp>
#include <ctime>
#include "data.h"
#include "message.h"
using namespace boost::asio;
using boost::system::error_code;
using ip::tcp;
using std::cout; using std::endl;

class BroadcastClient {
	typedef boost::shared_ptr<ip::tcp::socket> sockptr;
	typedef boost::shared_array<char> buffers;
	typedef boost::mt19937 rand;
private:
	io_service& io;
	buffers b;
	sockptr sock;
	rand rng;
	boost::asio::deadline_timer *timer;

	void read()
	{
		sock->async_read_some(buffer(b.get(), 1024 * sizeof(char)), boost::bind(&BroadcastClient::read_handler, this, _1));
	}

	void read_handler(error_code ec)
	{
		if (ec) return;
		do_read();
	}

	void do_read() {
		std::string content = b.get();
		std::stringstream ss(content);

		{
			cereal::JSONInputArchive ia(ss);
			data d;
			ia(d);
			cout << d.msg<<'\t'<<d.in.x<<" "<<d.in.y<<" "<<d.in.z<<endl;
		}
		read();
	}

	void connect_handler(error_code ec)
	{
		if (ec) return;
		read();
		start_write();
	}

	void write()
	{
		int r = rng();
		std::stringstream ss;
		{
			cereal::JSONOutputArchive oa(ss);
			message msg;
			msg.x = r%100;
			oa(msg);
		}
		std::string* content = new std::string(ss.str());
		sock->async_write_some(buffer(*content), boost::bind(&BroadcastClient::write_handler, this, content , _1));
		timer->expires_at(timer->expires_at() + boost::posix_time::seconds(1));
		timer->async_wait(boost::bind(&BroadcastClient::write,this));
	}

	void write_handler(std::string* content, error_code ec)
	{
		delete content;
		if (ec) return;
	}

	
	

public:
	BroadcastClient(io_service& io_, boost::asio::deadline_timer *t) :io(io_), b(new char[1024]), sock(new ip::tcp::socket(io)),rng(time(0)),timer(t)
	{
		memset(b.get(), '\0', 1024);
	};
	void start()
	{
		ip::tcp::endpoint ep(ip::address::from_string("127.0.0.1"), 666);
		sock->async_connect(ep, boost::bind(&BroadcastClient::connect_handler, this, _1 ));
		io.run();
	};
	void start_write()
	{
		timer->async_wait(boost::bind(&BroadcastClient::write, this));
		io.run();
	}
};

int main() {
	io_service io;
	boost::asio::deadline_timer t(io, boost::posix_time::seconds(1));
	BroadcastClient client(io,&t);
	
	client.start();
	io.run();
}