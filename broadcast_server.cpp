#pragma once
#include<iostream>
using std::cout;
using std::endl;

#define BOOST_ASIO_DISABLE_STD_CHRONO
#define BOOST_THREAD_VERSION 5
#define MAX_SOCKET_AMOUNT 3
#define BUFFER_SIZE 512
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
#include <boost/function.hpp>
#include "message.h"

using namespace boost::asio;
using boost::system::error_code;
using ip::tcp;
using std::cout; using std::endl;
using boost::system::error_code;

mutex mu;

class BroadcastServer 
{
	typedef std::vector<boost::shared_ptr<ip::tcp::socket>> sptrvec;
	typedef boost::shared_ptr<ip::tcp::socket> sockptr;
	typedef boost::shared_array<char> buffers;
	typedef boost::system::error_code error_code;
private:
	sptrvec socks;
	io_service& io;
	boost::atomic<int> current_sock_amount;
	ip::tcp::acceptor ac;
	buffers buf[3];
	boost::atomic<int> current_send_time;
	CommonSetting cs;
	Player players[4];
	boost::function<void(CommonSetting cs, BroadcastServer* clientptr)> on_status_change_func;
	bool is_callback_setted;

	void accept_handler(sockptr sp,error_code ec) 
	{
		if (ec)
			return;
		++current_sock_amount;
		cout << "a tcp connect from " << sp->remote_endpoint().address() << " established" << endl;
		read(current_sock_amount.load()-1);
		if(current_sock_amount!=MAX_SOCKET_AMOUNT)
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
		merge();
		cs.networkCnt = current_sock_amount;
		for (int i = 0; i < current_sock_amount; ++i)
		{
			std::stringstream ss;
			{
				cereal::JSONOutputArchive oa(ss);
				broadcast_message msg(cs);
				msg.cs.networkID = i;
				oa(msg);
			}
			std::string *msg = new std::string(ss.str());
			socks[i]->async_write_some(buffer(*msg), boost::bind(&BroadcastServer::write_handler, this, msg, _1));
		}
	}

	
	void read_handler(int sockid, error_code ec) 
	{
		if (ec) return;
		{
			std::stringstream ss(buf[sockid].get());
			buf[sockid].reset(new char[BUFFER_SIZE]);
			memset(buf[sockid].get(), '\0', BUFFER_SIZE);
			cereal::JSONInputArchive ia(ss);
			client_message msg;
			ia(msg);
		}
		read(sockid);
	}

	void read(int i)
	{
		socks[i]->async_read_some(buffer(buf[i].get(), BUFFER_SIZE * sizeof(char)), boost::bind(&BroadcastServer::read_handler, this, i, _1));
	}

	void merge()
	{
		for (int i = 0; i < 4; i++)
		{
			cs.player[i + 1] = players[i];
		}

		if (is_callback_setted)
		{
			on_status_change_func(cs, this);
		}
		else
		{
			cout << "Callback func hasn't been setted, use BroadcastServer::on_server_status_change() to set." << endl;
		}
	}

public:
	BroadcastServer(io_service &io_) : io(io_), socks(MAX_SOCKET_AMOUNT), current_sock_amount(0), ac(io, tcp::endpoint(tcp::v4(), 666)), current_send_time(0)
	{
		for (int i = 0; i < MAX_SOCKET_AMOUNT; i++) {
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


	/*
	** 设置host server的commonsetting状态
	*/
	void set_server_status(CommonSetting cs_)
	{
		try
		{
			mu.lock();
			cs = cs_;
			mu.unlock();
		}
		catch (exception e) {
			cout << e.what();
		}
	}

	void on_server_status_change(boost::function<void(CommonSetting cs, BroadcastServer* clientptr)> call_back)
	{
		on_status_change_func = call_back;
		if (is_callback_setted == false)
			is_callback_setted = true;
	}
};

/*
** host server的启动方法
** 参数1：回调函数
** 例如 start( callback_func)
** 回调的示例在下面
** 建议开一个新线程来执行本方法
*/
void start(boost::function<void(CommonSetting cs, BroadcastServer* clientptr)> call_back_func)
{
	io_service io;
	BroadcastServer server(io);
	server.on_server_status_change(call_back_func);
	server.start_accept();
	boost::asio::deadline_timer t(io, boost::posix_time::seconds(1));
	server.start_broadcast(&t);
	io.run();
}

/*
** 回调的示例
** cs: 广播的CommonSetting类; serverptr: host的this指针
** 当每次收到host同步广播就会调用这个方法
** 建议在这个方法中读取commonsetting信息
** 并使用	serverptr->set_server_status()来设置新全局状态，并在下一帧广播
*/
void change_listener(CommonSetting cs, BroadcastServer *serverptr)
{
	cout << "current players: " << cs.networkCnt+1 << endl;

}

int main()
{
	start(change_listener);
}