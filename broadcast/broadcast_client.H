#pragma once
#include<iostream>
using std::cout;
using std::endl;

#define BOOST_ASIO_DISABLE_STD_CHRONO
#define BOOST_THREAD_VERSION 5
#define MAX_SOCKET_AMOUNT 4
#define BUFFER_SIZE 2048
#include<boost/thread.hpp>
#include<boost/asio.hpp>
#include<boost/smart_ptr.hpp>
#include<boost/bind.hpp>
#include<iostream>
#include<string>
#include<boost/function.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include<vector>
#include<boost/atomic.hpp>
#include <sstream>
#include <boost/random.hpp>
#include <ctime>
#include <boost/ref.hpp>
#include "message.h"

using namespace boost::asio;
using boost::system::error_code;
using ip::tcp;
using std::cout; using std::endl;
using boost::system::error_code;


class BroadcastClient {
	typedef boost::shared_ptr<ip::tcp::socket> sockptr;
	typedef boost::shared_array<char> buffers;
	typedef boost::mt19937 rand;
	typedef boost::system::error_code error_code;

private:
	io_service& io;
	buffers b;
	sockptr sock;
	rand rng;
	Player p;
	boost::function<void(CommonSetting cs, BroadcastClient* clientptr)> on_status_change_func;
	bool is_callback_setted;

	void read();

	void read_handler(error_code ec, std::size_t bytes_transferred, boost::asio::streambuf* buf);

	void do_read(boost::asio::streambuf* buf, std::size_t bytes_transferred);

	void connect_handler(error_code ec);

	void do_write();

	void write_handler(boost::asio::streambuf *buf, error_code ec);

public:
	/*
	** 构造函数
	*/
	BroadcastClient(io_service& io_);
	void start(std::string ipv4);;
	/*
	** 设置客户端的Player状态，在下一帧发送给host
	*/
	void set_client_status(Player player);

	/*
	** 设置回调
	*/
	void on_client_status_change(boost::function<void(CommonSetting cs, BroadcastClient* clientptr)> call_back);
};

/*
** client的启动方法
** 参数1：host的ipv4地址，参数2：回调函数
** 例如 start("127.0.0.1", callback_func)
** 回调的示例在下面
** 建议开一个新线程来执行本方法
*/
void start(std::string ipv4, boost::function<void(CommonSetting cs, BroadcastClient* clientptr)> on_status_change_func);

/*
** 回调的示例
** cs: 广播的CommonSetting类; clientptr: 客户端的this指针
** 当每次收到host同步广播就会调用这个方法
** 建议在这个方法中读取commonsetting信息
** 并使用	clientptr->set_client_status()来设置新状态，并在下一帧发给host
*/
void change_listener(CommonSetting cs, BroadcastClient *clientptr);

