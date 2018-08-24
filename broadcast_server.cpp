#pragma once
#include<iostream>
using std::cout;
using std::endl;

#define BOOST_ASIO_DISABLE_STD_CHRONO
#define BOOST_THREAD_VERSION 5
#define MAX_SOCKET_AMOUNT 4
#define BUFFER_SIZE 256
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
#include "message.h"
using namespace boost::asio;
using boost::system::error_code;
using ip::tcp;
using std::cout; using std::endl;

class BroadcastServer 
{
	typedef std::vector<boost::shared_ptr<ip::tcp::socket>> sptrvec;
	typedef boost::shared_ptr<ip::tcp::socket> sockptr;
	typedef boost::shared_array<char> buffers;
private:
	sptrvec socks;
	io_service& io;
	boost::atomic<int> current_sock_amount;
	ip::tcp::acceptor ac;
	buffers buf[3];
	boost::atomic<int> current_send_time;

	void accept_handler(sockptr sp,error_code ec) 
	{
		if (ec)
			return;
		++current_sock_amount;
		cout << "a tcp connect from " << sp->remote_endpoint().address() << " established" << endl;
		read(current_sock_amount.load()-1);
		start_accept();
	}

	void write_handler(std::string* msg, error_code ec)
	{
		++current_send_time;
		if (current_send_time == current_sock_amount)
		{
			current_send_time.store(0);
			delete msg;
		}
		if (ec)
			cout << "send failed" << endl;
		else
			cout << "send success" << endl;
	}
	void do_broadcast(boost::asio::deadline_timer *t)
	{
		cout << "start one broadcast" << endl;
		
		do_write();
		
		t->expires_at(t->expires_at() + boost::posix_time::seconds(1));
		t->async_wait(boost::bind(&BroadcastServer::do_broadcast, this, t));
	}

	void do_write() 
	{
		std::stringstream ss;
		{
			cereal::JSONOutputArchive oa(ss);
			data d;
			d.msg = "hello world msg!";
			d.in.x = 2;
			d.in.y = 3;
			d.in.z = 4;
			oa(d);
		}

		std::string *msg = new std::string(ss.str());

		for (int i = 0; i < current_sock_amount; ++i)
		{
			socks[i]->async_write_some(buffer(*msg), boost::bind(&BroadcastServer::write_handler, this, msg, _1));
		}
	}

	
	void read_handler(int sockid, error_code ec) 
	{
		if (ec) return;
		{
			std::stringstream ss(buf[sockid].get());
			buf[sockid].reset(new char[256]);
			memset(buf[sockid].get(), '\0', BUFFER_SIZE);
			cereal::JSONInputArchive ia(ss);
			message msg;
			ia(msg);
			cout << "client message: " << msg.x << endl;
		}
		read(sockid);
	}

	void read(int i)
	{
		socks[i]->async_read_some(buffer(buf[i].get(), BUFFER_SIZE * sizeof(char)), boost::bind(&BroadcastServer::read_handler, this, i, _1));
	}

public:
	BroadcastServer(io_service &io_) : io(io_), socks(MAX_SOCKET_AMOUNT), current_sock_amount(0), ac(io, tcp::endpoint(tcp::v4(), 666)), current_send_time(0)
	{
		for (int i = 0; i < 3; i++) {
			buf[i].reset(new char[BUFFER_SIZE]);
			memset(buf[i].get(), '\0', BUFFER_SIZE);
		}
	};

	/*
	** Main method, to start service. Use io_service::run when service start
	*/
	void start_accept()
	{
		sockptr s (new ip::tcp::socket(io));
		socks[current_sock_amount] = s;
		ac.async_accept(*socks[current_sock_amount], boost::bind(&BroadcastServer::accept_handler, this, socks[current_sock_amount], _1));
	}

	/*
	** start broadcast
	*/
	void start_broadcast(boost::asio::deadline_timer *t) {
		t->async_wait(boost::bind(&BroadcastServer::do_broadcast, this, t));
		io.run();
	}
};

int main()
{
	io_service io;
	BroadcastServer server(io);
	server.start_accept();
	boost::asio::deadline_timer t(io, boost::posix_time::seconds(1));
	server.start_broadcast(&t);
	io.run();
}