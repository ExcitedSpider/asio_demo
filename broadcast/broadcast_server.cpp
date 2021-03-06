#include "broadcast_server.h"

mutex mu;

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
	cout << "start one broadcast" << endl;

	do_write();

	t->expires_at(t->expires_at() + CLOCK_TIMER);
	t->async_wait(boost::bind(&BroadcastServer::do_broadcast, this, t));
}

 void BroadcastServer::do_write()
{
	merge();
	cs.networkCnt = current_sock_amount;
	for (int i = 0; i < current_sock_amount; ++i)
	{
		buffer buf(new boost::asio::streambuf());
		{
			boost::archive::binary_oarchive oa(*buf);
			broadcast_message msg(cs);
			msg.cs.networkID = i;
			oa << msg;
		}
		cout << buf->size() << endl;
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
	players[sockid + 1] = msg.player;
	buf->consume(bytes_transferred);
	//delete buf;    //换用shared_ptr解决内存管理问题

	read(sockid);
}

 void BroadcastServer::read(int i)
{
	buffer buf(new boost::asio::streambuf());
	socks[i]->async_read_some(buf->prepare(BUFFER_SIZE), boost::bind(&BroadcastServer::read_handler, this, i, _1, _2, buf));
}

 void BroadcastServer::merge()
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

 BroadcastServer::BroadcastServer(io_service & io_) : io(io_), socks(MAX_SOCKET_AMOUNT), current_sock_amount(0), ac(io, tcp::endpoint(tcp::v4(), 666)), current_send_time(0)
{
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
** 设置host server的commonsetting状态
*/

 void BroadcastServer::set_server_status(CommonSetting cs_)
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

 void BroadcastServer::on_server_status_change(boost::function<void(CommonSetting cs, BroadcastServer*clientptr)> call_back)
{
	on_status_change_func = call_back;
	if (is_callback_setted == false)
		is_callback_setted = true;
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
 void start(boost::function<void(CommonSetting cs, BroadcastServer*clientptr)> call_back_func)
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
** cs: 广播的CommonSetting类; serverptr: host的this指针
** 当每次收到host同步广播就会调用这个方法
** 建议在这个方法中读取commonsetting信息
** 并使用	serverptr->set_server_status()来设置新全局状态，并在下一帧广播
*/
void change_listener(CommonSetting cs, BroadcastServer * serverptr)
{
	cout << "current players: " << cs.networkCnt + 1 << endl;

}
