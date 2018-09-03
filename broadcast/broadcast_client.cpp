#include "broadcast_client.h"

mutex mu_c;

void BroadcastClient::read()
{
	boost::asio::streambuf* buf = new boost::asio::streambuf();
	boost::asio::streambuf::mutable_buffers_type* bufs = &(buf->prepare(CLINET_BUFFER_SIZE));
	sock->async_read_some(*bufs, boost::bind(&BroadcastClient::read_handler, this, _1, _2, buf));
}

BroadcastClient* BroadcastClient::instance = nullptr;

 void BroadcastClient::read_handler(error_code ec, std::size_t bytes_transferred, boost::asio::streambuf * buf)
{
	if (ec) return;
	do_read(buf, bytes_transferred);
	do_write();
}

 void BroadcastClient::do_read(boost::asio::streambuf * buf, std::size_t bytes_transferred) 
 {
	buf->commit(bytes_transferred);
	buf->data();
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

 BroadcastClient * BroadcastClient::getInstance()
 {
	 return instance;
 }

/*
** 构造函数
*/

 BroadcastClient::BroadcastClient(io_service & io_) :io(io_), b(new char[CLINET_BUFFER_SIZE]), sock(new ip::tcp::socket(io)), rng(time(0)), is_callback_setted(false)
{
	memset(b.get(), '\0', CLINET_BUFFER_SIZE);
	instance = this;
}

 void BroadcastClient::start(std::string ipv4)
{
	ip::tcp::endpoint ep(ip::address::from_string(ipv4), 666);
	sock->async_connect(ep, boost::bind(&BroadcastClient::connect_handler, this, _1));
	io.run();
}

/*
** 设置客户端的st_Player状态，在下一帧发送给host
*/

 void BroadcastClient::set_client_status(st_Player st_Player)
{
		p = st_Player;
 }

/*
** 设置回调
*/

 void BroadcastClient::on_client_status_change(boost::function<void(st_CommonSetting cs, BroadcastClient*clientptr)> call_back)
{
	on_status_change_func = call_back;
	if (is_callback_setted == false)
		is_callback_setted = true;
}

/*
** client的启动方法
** 参数1：host的ipv4地址，参数2：回调函数
** 例如 start("127.0.0.1", callback_func)
** 回调的示例在下面
** 建议开一个新线程来执行本方法
*/
 void start_broadcast_client(std::string ipv4, boost::function<void(st_CommonSetting cs, BroadcastClient* clientptr)> on_status_change_func)
 {
	io_service io;
	BroadcastClient client(io);
	client.on_client_status_change(on_status_change_func);
	client.start(ipv4);
	io.run();
}

/*
** 回调的示例
** cs: 广播的st_CommonSetting类; clientptr: 客户端的this指针
** 当每次收到host同步广播就会调用这个方法
** 建议在这个方法中读取st_CommonSetting信息
** 并使用	clientptr->set_client_status()来设置新状态，并在下一帧发给host
*/
void change_listener(st_CommonSetting cs, BroadcastClient * clientptr)
{

}
