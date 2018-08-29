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
	/*
	** 构造函数。
	** 参数1：io_servce
	*/
	ChatroomServer(io_service &io_);
	/*
	** 开启接收tcp连接
	** 之后需要io_serce.run()
	*/
	void start_accept();

	/*
	** 发送一条消息
	*/
	void post(ChatMessage msg);

	/*
	** 设置回调函数
	*/
	void set_on_recieve(boost::function<void(msg_ptr)> call_back_func);

private:

	void broadcast();
	void write_handler(error_code ec);
	void accept_handler(sockptr sp, error_code ec);
	void do_read(sockptr sp);
	//use only for debug
	//you are not supposed to use this
	void post_helloworld();
	void read_handler(sockptr sp, error_code ec, size_t bites_trans);
};
/*
** 这个是回调函数的示例
** 每次有新消息到达时就会调用这个函数
** mp：消息类ChatMessage的指针，
*/
void server_message_listener(boost::shared_ptr<ChatMessage> mp);

/*
** 这个是启动方法
** 参数1：ip地址，参数2：回调函数
** 建议开一个新线程来执行本方法
*/
ChatroomServer* server_start_qe(boost::function<void(boost::shared_ptr<ChatMessage>)> on_recieve);

