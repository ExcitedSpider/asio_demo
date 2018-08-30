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
	** ���캯��
	*/
	BroadcastClient(io_service& io_);
	void start(std::string ipv4);;
	/*
	** ���ÿͻ��˵�Player״̬������һ֡���͸�host
	*/
	void set_client_status(Player player);

	/*
	** ���ûص�
	*/
	void on_client_status_change(boost::function<void(CommonSetting cs, BroadcastClient* clientptr)> call_back);
};

/*
** client����������
** ����1��host��ipv4��ַ������2���ص�����
** ���� start("127.0.0.1", callback_func)
** �ص���ʾ��������
** ���鿪һ�����߳���ִ�б�����
*/
void start(std::string ipv4, boost::function<void(CommonSetting cs, BroadcastClient* clientptr)> on_status_change_func);

/*
** �ص���ʾ��
** cs: �㲥��CommonSetting��; clientptr: �ͻ��˵�thisָ��
** ��ÿ���յ�hostͬ���㲥�ͻ�����������
** ��������������ж�ȡcommonsetting��Ϣ
** ��ʹ��	clientptr->set_client_status()��������״̬��������һ֡����host
*/
void change_listener(CommonSetting cs, BroadcastClient *clientptr);
