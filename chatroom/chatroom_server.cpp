#pragma once
#include<iostream>
using std::cout;
using std::endl;

#define BOOST_ASIO_DISABLE_STD_CHRONO
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

class ChatroomServer
{
	typedef std::vector<boost::shared_ptr<ip::tcp::socket>> sptrvec;
	typedef boost::shared_ptr<ip::tcp::socket> sockptr;
	typedef boost::shared_array<char> buffers;
	typedef list<boost::shared_ptr<ChatMessage>> msglist;
	typedef boost::shared_ptr<ChatMessage> msg_ptr;

private:
	sptrvec socks;
	io_service& io;
	boost::atomic<int> current_sock_amount;
	ip::tcp::acceptor ac;
	boost::atomic<int> current_send_time;
	msglist ml;
	streambuf buf;
	boost::function<void(msg_ptr)> on_recieve;
	bool is_on_recieve_setted;

public:
	ChatroomServer(io_service &io_) : io(io_), ac(io, tcp::endpoint(tcp::v4(), 667)), socks(MAX_SOCKET_AMOUNT), is_on_recieve_setted(false)
	{
		current_sock_amount.store(0);
		current_send_time.store(0);
	}
	void start_accept()
	{
		sockptr s(new ip::tcp::socket(io));
		socks[current_sock_amount] = s;
		ac.async_accept(*socks[current_sock_amount], boost::bind(&ChatroomServer::accept_handler, this, socks[current_sock_amount], _1));
	}

	void post(ChatMessage msg)
	{
		msg_ptr mp(new ChatMessage(msg));
		ml.push_back(mp);
		error_code ec;
		broadcast();
	}

	void set_on_recieve(boost::function<void(msg_ptr)> call_back_func)
	{
		if (!is_on_recieve_setted)
			is_on_recieve_setted = true;
		on_recieve = call_back_func;
	}

private:

	void broadcast()
	{
		msg_ptr msg = ml.front();
		ml.pop_front();
		boost::archive::binary_oarchive oa(buf);
		oa << *msg;
		auto cb = buf.data();
		for (int i = 0; i < current_sock_amount; ++i)
		{
			socks[i]->async_write_some(buffer(cb), boost::bind(&ChatroomServer::write_handler, this, _1));
		}
		io.poll();
		cout << "broadcast one msg, left : " << ml.size()<<" msg(s). " << endl;
		if (is_on_recieve_setted)
			on_recieve(msg);
		else
			cout << "on_recieve_func hasn't been setted, call ChatroomServer::set_on_recieve() to set" << endl;
		if (ml.size())
			broadcast();

	}

	void write_handler( error_code ec)
	{
		if (ec)
			return;
		
	}

	void accept_handler(sockptr sp, error_code ec)
	{
		cout << "a new player from "<<sp->remote_endpoint().address() <<"  join chatroom" << endl;
		if (ec)
			return;
		++current_sock_amount;

		do_read(sp);
		start_accept();
	}
	void do_read(sockptr sp)
	{
		sp->async_read_some(buf.prepare(BUFFER_SIZE),boost::bind(&ChatroomServer::read_handler, this, sp,  _1, _2));
	}

	//use only for debug
	//you are not supposed to use this
	void post_helloworld()
	{
		ChatMessage msg;
		msg.message = "Hello Server";
		msg.playerName = "QE";
		post(msg);
	}

	void read_handler(sockptr sp, error_code ec, size_t bites_trans)
	{
		buf.commit(bites_trans);
		boost::archive::binary_iarchive ia(buf);
		msg_ptr mp(new ChatMessage);
		ia >> *mp;
		buf.consume(bites_trans);
		ml.push_back(mp);
		broadcast();

		do_read(sp);
	}
};

void message_listener(boost::shared_ptr<ChatMessage> mp)
{
	cout << "[player]\t\t[message]" << endl;
	cout << mp->playerName<<"\t\t"<< mp->message << endl;
}

ChatroomServer* server_start()
{
	boost::asio::io_service io;
	ChatroomServer* server = new ChatroomServer(io);
	server->set_on_recieve(message_listener);
	server->start_accept();
	io.run();
	return server;
}
