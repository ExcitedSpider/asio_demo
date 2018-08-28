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
	/*
	** 构造函数。
	** 参数1：io_servce
	*/
	ChatroomClient(io_service &io_) :io(io_), is_on_recieve_setted(false), sock(new ip::tcp::socket(io)) {};

	/*
	** 开启tcp连接
	** 之后需要io_serce.run()
	*/
	void start(std::string ipv4)
	{
		ip::tcp::endpoint ep(ip::address::from_string(ipv4), 667);
		sock->async_connect(ep, boost::bind(&ChatroomClient::connect_handler, this, _1));
		io.run();
	};
	
	/*
	** 发送一条消息
	*/
	void post(ChatMessage msg)
	{
		boost::archive::binary_oarchive oa(buf);
		oa << msg;
		sock->async_write_some(buf.prepare(buf.size()), boost::bind(&ChatroomClient::write_handler, this, _1));
	}


	/*
	** 设置回调函数
	*/
	void set_on_recieve(boost::function<void(msg_ptr)> call_back_func)
	{
		if (!is_on_recieve_setted)
			is_on_recieve_setted = true;
		on_recieve = call_back_func;
	}

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
		buf.size();
		boost::archive::binary_iarchive ia(buf);
		msg_ptr mp(new ChatMessage);
		ia >> *mp;
		buf.consume(bites_trans);
		if (is_on_recieve_setted)
			on_recieve(mp);
		else
			cout << "on_recieve_func hasn't been setted, call ChatroomServer::set_on_recieve() to set" << endl;
		read();
	}

	//use only for debug
	//you are not supposed to use this
	void post_helloworld()
	{
		ChatMessage msg;
		msg.message = "Hello Client";
		msg.playerName = "Autumn";
		post(msg);
	}

	void write_handler(error_code ec)
	{
		if (ec)
			return;
	}
};

/*
** 这个是回调函数的示例
** 每次有新消息到达时就会调用这个函数
** mp：消息类ChatMessage的指针，
*/
void client_message_listener(boost::shared_ptr<ChatMessage> mp)
{
	cout << "[player]\t\t[message]" << endl;
	cout << mp->playerName << "\t\t" << mp->message << endl;
}

/*
** 这个是启动方法
** 参数1：回调函数
** 建议开一个新线程来执行本方法
*/
ChatroomClient* client_start(std::string ipv4, boost::function<void(boost::shared_ptr<ChatMessage>)> on_recieve)
{
	io_service io;
	ChatroomClient* client = new ChatroomClient(io);
	client->set_on_recieve(on_recieve);
	client->start(ipv4);
	return client;
}

int main() {
	client_start("127.0.0.1", client_message_listener);
}