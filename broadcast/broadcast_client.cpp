#include "broadcast_client.h"

mutex mu;

void BroadcastClient::read()
{
	boost::asio::streambuf* buf = new boost::asio::streambuf();
	boost::asio::streambuf::mutable_buffers_type* bufs = &(buf->prepare(BUFFER_SIZE));
	sock->async_read_some(*bufs, boost::bind(&BroadcastClient::read_handler, this, _1, _2, buf));
}

 void BroadcastClient::read_handler(error_code ec, std::size_t bytes_transferred, boost::asio::streambuf * buf)
{
	if (ec) return;
	do_read(buf, bytes_transferred);
	do_write();
}

 void BroadcastClient::do_read(boost::asio::streambuf * buf, std::size_t bytes_transferred) {
	buf->commit(bytes_transferred);
	{
		boost::archive::binary_iarchive ia(*buf);
		broadcast_message msg;
		ia >> msg;
		if (is_callback_setted)
		{
			on_status_change_func(msg.cs, this);
		}
		else
		{
			cout << "Callback func hasn't been setted, use BroadcastClient::on_client_status_change() to set." << endl;
		}

	}

	delete buf;

	read();
}

 void BroadcastClient::connect_handler(error_code ec)
{
	if (ec) return;
	read();
}

 void BroadcastClient::do_write()
{
	int r = rng();
	boost::asio::streambuf *buf = new boost::asio::streambuf();
	{
		boost::archive::binary_oarchive oa(*buf);
		client_message msg(p);
		std::stringstream ss_;
		ss_ << "random_name" << rng() % 100;
		p.name = ss_.str();
		oa << msg;
	}
	sock->async_write_some(buf->data(), boost::bind(&BroadcastClient::write_handler, this, buf, _1));
}

 void BroadcastClient::write_handler(boost::asio::streambuf * buf, error_code ec)
{
	delete buf;
	if (ec) return;
}

/*
** ���캯��
*/

 BroadcastClient::BroadcastClient(io_service & io_) :io(io_), b(new char[BUFFER_SIZE]), sock(new ip::tcp::socket(io)), rng(time(0)), is_callback_setted(false)
{
	memset(b.get(), '\0', BUFFER_SIZE);
}

 void BroadcastClient::start(std::string ipv4)
{
	ip::tcp::endpoint ep(ip::address::from_string(ipv4), 666);
	sock->async_connect(ep, boost::bind(&BroadcastClient::connect_handler, this, _1));
	io.run();
}

/*
** ���ÿͻ��˵�Player״̬������һ֡���͸�host
*/

 void BroadcastClient::set_client_status(Player player)
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
** ���ûص�
*/

 void BroadcastClient::on_client_status_change(boost::function<void(CommonSetting cs, BroadcastClient*clientptr)> call_back)
{
	on_status_change_func = call_back;
	if (is_callback_setted == false)
		is_callback_setted = true;
}

/*
** client����������
** ����1��host��ipv4��ַ������2���ص�����
** ���� start("127.0.0.1", callback_func)
** �ص���ʾ��������
** ���鿪һ�����߳���ִ�б�����
*/
void start(std::string ipv4, boost::function<void(CommonSetting cs, BroadcastClient*clientptr)> on_status_change_func)
{
	io_service io;
	BroadcastClient client(io);
	client.on_client_status_change(on_status_change_func);
	client.start(ipv4);
	io.run();
}

/*
** �ص���ʾ��
** cs: �㲥��CommonSetting��; clientptr: �ͻ��˵�thisָ��
** ��ÿ���յ�hostͬ���㲥�ͻ�����������
** ��������������ж�ȡcommonsetting��Ϣ
** ��ʹ��	clientptr->set_client_status()��������״̬��������һ֡����host
*/
void change_listener(CommonSetting cs, BroadcastClient * clientptr)
{
	cout << "broadcast msg: [networkdID] " << cs.networkID << endl;

}