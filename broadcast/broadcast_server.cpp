#include "broadcast_server.h"

mutex mu;

BroadcastServer* BroadcastServer::getInstance() {
	return instance;
}

BroadcastServer* BroadcastServer::instance = nullptr;


 void BroadcastServer::accept_handler(sockptr sp, error_code ec)
{
	if (ec)
		return;
	++current_sock_amount;
	cout << "a tcp connect from " << sp->remote_endpoint().address() << " established" << endl;
	read(current_sock_amount.load() - 1);
	if (current_sock_amount != MAX_SOCKET_AMOUNT)
		start_accept();
}

 void BroadcastServer::write_handler(error_code ec)
{
	++current_send_time;
	if (current_send_time == current_sock_amount)
	{
		current_send_time.store(0);
		//delete buf;
	}
}

 void BroadcastServer::do_broadcast(boost::asio::deadline_timer * t)
{
	 t->expires_at(t->expires_at() + CLOCK_TIMER);
	 t->async_wait(boost::bind(&BroadcastServer::do_broadcast, this, t));

	cout << "start one broadcast" << endl;

	do_write();

}

 void BroadcastServer::do_write()
{
	 merge_player_status();
	 for (int i = 0; i < current_sock_amount; ++i)
	{
		buffer buf(new boost::asio::streambuf());
		{
			boost::archive::binary_oarchive oa(*buf);
			broadcast_message msg(cs);
			oa << msg;
		}
		socks[i]->async_write_some(buf->data(), boost::bind(&BroadcastServer::write_handler, this, _1));
	}
}

 void BroadcastServer::read_handler(int sockid, error_code ec, std::size_t bytes_transferred, buffer buf)
{
	if (ec) return;
	buf->commit(bytes_transferred);
	boost::archive::binary_iarchive ia(*buf);
	client_message msg;
	ia >> msg;
	st_Players[sockid + 1] = msg.Player;
	
	buf->consume(bytes_transferred);

	read(sockid);
}

 void BroadcastServer::read(int i)
{
	buffer buf(new boost::asio::streambuf());
	socks[i]->async_read_some(buf->prepare(BUFFER_SIZE), boost::bind(&BroadcastServer::read_handler, this, i, _1, _2, buf));
}

 void BroadcastServer::merge_player_status()
{
	for (int i = 0; i < 4; i++)
	{
		cs.st_Player[i + 1] = st_Players[i];
		cocos2d::log(st_Players[i].name.data());
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

 BroadcastServer::BroadcastServer(io_service & io_) : io(io_), socks(MAX_SOCKET_AMOUNT), current_sock_amount(0), ac(io, tcp::endpoint(tcp::v4(), 666)), current_send_time(0)
{
	 BroadcastServer::instance = this;
	/*for (int i = 0; i < MAX_SOCKET_AMOUNT; i++) {
	buf[i].reset(new char[BUFFER_SIZE]);
	memset(buf[i].get(), '\0', BUFFER_SIZE);
	}*/
 }

/*
** Main method, to start service. Use io_service::run when service start
*/

 void BroadcastServer::start_accept()
{
	sockptr s(new ip::tcp::socket(io));
	socks[current_sock_amount] = s;
	ac.async_accept(*socks[current_sock_amount], boost::bind(&BroadcastServer::accept_handler, this, socks[current_sock_amount], _1));
}

/*
** start broadcast
*/

 void BroadcastServer::start_broadcast(boost::asio::deadline_timer * t) {
	t->async_wait(boost::bind(&BroadcastServer::do_broadcast, this, t));
	io.run();
}

/*
** 设置host server的st_CommonSetting状态
*/

 void BroadcastServer::set_server_status(st_CommonSetting cs_)
{
		cs = cs_;
}

 void BroadcastServer::on_server_status_change(boost::function<void(st_CommonSetting cs, BroadcastServer*clientptr)> call_back)
{
	on_status_change_func = call_back;
	if (is_callback_setted == false)
		is_callback_setted = true;
}

 void BroadcastServer::set_host_player_status(st_Player p)
 {
	 st_Players[0] = p;//0号玩家固定为host
 }

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
void start_broadcast_server(boost::function<void(st_CommonSetting cs, BroadcastServer*clientptr)> call_back_func)
{
	io_service io;
	BroadcastServer server(io);
	server.on_server_status_change(call_back_func);
	server.start_accept();
	boost::asio::deadline_timer t(io, CLOCK_TIMER);
	server.start_broadcast(&t);
	io.run();
}

/*
** 回调的示例
** cs: 广播的st_CommonSetting类; serverptr: host的this指针
** 当每次收到host同步广播就会调用这个方法
** 建议在这个方法中读取st_CommonSetting信息
** 并使用	serverptr->set_server_status()来设置新全局状态，并在下一帧广播
*/
void change_listener(st_CommonSetting cs, BroadcastServer * serverptr)
{

}
