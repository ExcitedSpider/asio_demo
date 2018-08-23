#pragma once
#include<iostream>
using std::cout;
using std::endl;

#define BOOST_ASIO_DISABLE_STD_CHRONO
#define BOOST_THREAD_VERSION 5
#define MAX_SOCKET_AMOUNT 4
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
#include<vector>
#include<boost/atomic.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/string.hpp>
#include <sstream>
#include "data.h"
using namespace boost::asio;
using boost::system::error_code;
using ip::tcp;
using std::cout; using std::endl;

class BroadcastServer 
{
	typedef std::vector<boost::shared_ptr<ip::tcp::socket>> sptrvec;
	typedef boost::shared_ptr<ip::tcp::socket> sockptr;
private:
	sptrvec socks;
	io_service& io;
	boost::atomic<int> current_sock_amount;
	ip::tcp::acceptor ac;



	void accept_handler(sockptr sp,error_code ec) 
	{
		if (ec)
			return;
		++current_sock_amount;
		cout << "a tcp connect from " << sp->remote_endpoint().address() << " established" << endl;
		start_accept();
	}

	void write_handler(error_code ec)
	{
		if (ec)
			cout << "send failed" << endl;
		else
			cout << "send success" << endl;
	}
	void do_broadcast() 
	{
		//cout << "start one broadcast" << endl;
		std::stringstream ss;
		{
			cereal::JSONOutputArchive oa(ss);
			data d;
			d.msg = "hello world msg!";
			d.in.x = 2;
			d.in.y = 3;
			oa(d);
		}
		std::string content =  ss.str();
		
		for (int i = 0; i < current_sock_amount; ++i)
		{
			socks[i]->async_write_some(buffer(content), boost::bind(&BroadcastServer::write_handler, this, _1));
		}
		start_broadcast();
	}

public:
	BroadcastServer(io_service &io_) : io(io_), socks(MAX_SOCKET_AMOUNT), current_sock_amount(0), ac(io, tcp::endpoint(tcp::v4(), 666)) {};

	void start_accept()
	{
		sockptr s (new ip::tcp::socket(io));
		socks[current_sock_amount] = s;
		ac.async_accept(*socks[current_sock_amount], boost::bind(&BroadcastServer::accept_handler, this, socks[current_sock_amount], _1));
	}

	void start_broadcast() {
		boost::asio::deadline_timer t(io, boost::posix_time::seconds(1));
		t.async_wait(boost::bind(&BroadcastServer::do_broadcast, this));
		io.run();
	}
};

int main()
{
	io_service io;
	BroadcastServer server(io);
	server.start_accept();
	server.start_broadcast();
	io.run();
}