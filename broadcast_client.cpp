#pragma once
#include<iostream>
using std::cout;
using std::endl;

#define BOOST_ASIO_DISABLE_STD_CHRONO
#define BOOST_THREAD_VERSION 5
#define MAX_SOCKET_AMOUNT 4
#define BUFFER_SIZE 4096
#include<boost/thread.hpp>
#include<boost/asio.hpp>
#include<boost/smart_ptr.hpp>
#include<boost/bind.hpp>
#include<iostream>
#include<string>
#include<boost/function.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/string.hpp>
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

mutex mu;

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

	void read()
	{
		sock->async_read_some(buffer(b.get(), BUFFER_SIZE * sizeof(char)), boost::bind(&BroadcastClient::read_handler, this, _1));
	}

	void read_handler(error_code ec)
	{
		if (ec) return;
		do_read();
		do_write();
	}

	void do_read() {
		std::string content = b.get();
		std::stringstream ss(content);

		{
			cereal::JSONInputArchive ia(ss);
			broadcast_message msg;
			ia(msg);
			if (is_callback_setted)
			{
				on_status_change_func(msg.cs, this);
			}
			else
			{
				cout << "Callback func hasn't been setted, use BroadcastClient::on_client_status_change() to set."<<endl;
			}
			
		}
		read();
	}

	void connect_handler(error_code ec)
	{
		if (ec) return;
		read();
	}

	void do_write()
	{
		int r = rng();
		std::stringstream ss;
		{
			cereal::JSONOutputArchive oa(ss);
			client_message msg(p);
			std::stringstream ss_;
			ss_ << "random_name" << rng() % 100;
			p.name = ss_.str();
			oa(msg);
		}
		std::string* content = new std::string(ss.str());
		sock->async_write_some(buffer(*content), boost::bind(&BroadcastClient::write_handler, this, content , _1));
	}

	void write_handler(std::string* content, error_code ec)
	{
		delete content;
		if (ec) return;
	}

public:
	/*
	** 构造函数
	*/
	BroadcastClient(io_service& io_) :io(io_), b(new char[BUFFER_SIZE]), sock(new ip::tcp::socket(io)),rng(time(0)), is_callback_setted(false)
	{
		memset(b.get(), '\0', BUFFER_SIZE);
	};
	void start(std::string ipv4)
	{
		ip::tcp::endpoint ep(ip::address::from_string(ipv4), 666);
		sock->async_connect(ep, boost::bind(&BroadcastClient::connect_handler, this, _1 ));
		io.run();
	};
	/*
	** 设置客户端的Player状态，在下一帧发送给host
	*/
	void set_client_status(Player player)
	{
		try
		{
			mu.lock();
			p = player;
			mu.unlock();
		}
		catch (exception e) {
			cout << e.what();
		}
	}

	/*
	** 设置回调
	*/
	void on_client_status_change(boost::function<void(CommonSetting cs, BroadcastClient* clientptr)> call_back)
	{
		on_status_change_func = call_back;
		if(is_callback_setted == false)
			is_callback_setted = true;
	}
};

/*
** client的启动方法
** 参数1：host的ipv4地址，参数2：回调函数
** 例如 start("127.0.0.1", callback_func)
** 回调的示例在下面
** 建议开一个新线程来执行本方法
*/
void start(std::string ipv4, boost::function<void(CommonSetting cs, BroadcastClient* clientptr)> on_status_change_func)
{
	io_service io;
	BroadcastClient client(io);
	client.on_client_status_change(on_status_change_func);
	client.start(ipv4);
	io.run();
}

/*
** 回调的示例
** cs: 广播的CommonSetting类; clientptr: 客户端的this指针
** 当每次收到host同步广播就会调用这个方法
** 建议在这个方法中读取commonsetting信息
** 并使用	clientptr->set_client_status()来设置新状态，并在下一帧发给host
*/
void change_listener(CommonSetting cs, BroadcastClient *clientptr)
{
	cout << "broadcast msg: [networkdID] " << cs.networkID << endl;

}

int main()
{
	start("127.0.0.1",change_listener);
}