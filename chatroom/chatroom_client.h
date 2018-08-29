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
	ChatroomClient(io_service &io_);;

	/*
	** 开启tcp连接
	** 之后需要io_serce.run()
	*/
	void start(std::string ipv4);;
	
	/*
	** 发送一条消息
	*/
	void post(ChatMessage msg);


	/*
	** 设置回调函数
	*/
	void set_on_recieve(boost::function<void(msg_ptr)> call_back_func);

private:

	void connect_handler(error_code ec);
	void read();
	void read_handler(error_code ec, size_t bites_trans);
	//use only for debug
	//you are not supposed to use this
	void post_helloworld();

	void write_handler(error_code ec);
};

/*
** 这个是回调函数的示例
** 每次有新消息到达时就会调用这个函数
** mp：消息类ChatMessage的指针，
*/
void client_message_listener(boost::shared_ptr<ChatMessage> mp);

/*
** 这个是启动方法
** 参数1：回调函数
** 建议开一个新线程来执行本方法
*/
void client_start(std::string ipv4, boost::function<void(boost::shared_ptr<ChatMessage>)> on_recieve, ChatroomClient* ptr);

