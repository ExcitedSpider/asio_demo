#pragma once
#include<iostream>
using std::cout;
using std::endl;

#define BOOST_ASIO_DISABLE_STD_CHRONO
#define BOOST_THREAD_VERSION 5
#define MAX_SOCKET_AMOUNT 3
#define BUFFER_SIZE 1024
#define CLOCK_TIMER boost::posix_time::milliseconds(50)
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
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <sstream>
#include <boost/function.hpp>
#include "message.h"

using namespace boost::asio;
using boost::system::error_code;
using ip::tcp;
using std::cout; using std::endl;
using boost::system::error_code;

class BroadcastServer 
{
	typedef std::vector<boost::shared_ptr<ip::tcp::socket>> sptrvec;
	typedef boost::shared_ptr<ip::tcp::socket> sockptr;
	typedef boost::system::error_code error_code;
	typedef boost::shared_ptr<boost::asio::streambuf> buffer;
private:
	sptrvec socks;
	io_service& io;
	boost::atomic<int> current_sock_amount;
	ip::tcp::acceptor ac;
	boost::atomic<int> current_send_time;
	CommonSetting cs;
	Player players[4];
	boost::function<void(CommonSetting cs, BroadcastServer* clientptr)> on_status_change_func;
	bool is_callback_setted;

	void accept_handler(sockptr sp, error_code ec);

	void write_handler(error_code ec);
	void do_broadcast(boost::asio::deadline_timer *t);

	void do_write();

	
	void read_handler(int sockid, error_code ec, std::size_t bytes_transferred, buffer buf);

	void read(int i);

	void merge();

public:
	BroadcastServer(io_service &io_);;

	/*
	** Main method, to start service. Use io_service::run when service start
	*/
	void start_accept();

	/*
	** start broadcast
	*/
	void start_broadcast(boost::asio::deadline_timer *t);


	/*
	** 设置host server的commonsetting状态
	*/
	void set_server_status(CommonSetting cs_);

	void on_server_status_change(boost::function<void(CommonSetting cs, BroadcastServer* clientptr)> call_back);
};

/*
** host server的启动方法
** 参数1：回调函数
** 例如 start( callback_func)
** 回调的示例在下面
** 建议开一个新线程来执行本方法
*/
/*
** host server的启动方法
** 参数1：回调函数
** 例如 start( callback_func)
** 回调的示例在下面
** 建议开一个新线程来执行本方法
*/
void start(boost::function<void(CommonSetting cs, BroadcastServer* clientptr)> call_back_func);

/*
** 回调的示例
** cs: 广播的CommonSetting类; serverptr: host的this指针
** 当每次收到host同步广播就会调用这个方法
** 建议在这个方法中读取commonsetting信息
** 并使用	serverptr->set_server_status()来设置新全局状态，并在下一帧广播
*/
void change_listener(CommonSetting cs, BroadcastServer *serverptr);

