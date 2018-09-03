#pragma once
#include<iostream>
using std::cout;
using std::endl;

#define BOOST_ASIO_DISABLE_STD_CHRONO
#include "cocos2d.h"
#define BOOST_THREAD_VERSION 5
#define MAX_SOCKET_AMOUNT 3
#define BUFFER_SIZE 4096
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
#include <boost/date_time/local_time/local_time.hpp>  

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
	st_CommonSetting cs;
	boost::function<void(st_CommonSetting cs, BroadcastServer* clientptr)> on_status_change_func;
	bool is_callback_setted;

	void accept_handler(sockptr sp, error_code ec);

	void write_handler(error_code ec);
	void do_broadcast(boost::asio::deadline_timer *t);

	void do_write();

	
	void read_handler(int sockid, error_code ec, std::size_t bytes_transferred, buffer buf);

	void read(int i);

	void merge_player_status();


public:
	static BroadcastServer * instance;

	static BroadcastServer* getInstance();


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
	** ����host server��st_CommonSetting״̬
	*/
	void set_server_status(st_CommonSetting cs_);

	void on_server_status_change(boost::function<void(st_CommonSetting cs, BroadcastServer* clientptr)> call_back);

	void set_host_player_status(st_Player p);

	st_Player st_Players[4];
};

/*
** host server����������
** ����1���ص�����
** ���� start( callback_func)
** �ص���ʾ��������
** ���鿪һ�����߳���ִ�б�����
*/
/*
** host server����������
** ����1���ص�����
** ���� start( callback_func)
** �ص���ʾ��������
** ���鿪һ�����߳���ִ�б�����
*/
void start_broadcast_server(boost::function<void(st_CommonSetting cs, BroadcastServer* clientptr)> call_back_func);

/*
** �ص���ʾ��
** cs: �㲥��st_CommonSetting��; serverptr: host��thisָ��
** ��ÿ���յ�hostͬ���㲥�ͻ�����������
** ��������������ж�ȡst_CommonSetting��Ϣ
** ��ʹ��	serverptr->set_server_status()��������ȫ��״̬��������һ֡�㲥
*/
void change_listener(st_CommonSetting cs, BroadcastServer *serverptr);

