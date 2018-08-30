#pragma once
#include "chatroom_server.h"

/*
** 构造函数。
** 参数1：io_servce
*/

ChatroomServer::ChatroomServer(io_service & io_) : io(io_), ac(io, tcp::endpoint(tcp::v4(), 667)), socks(MAX_SOCKET_AMOUNT), is_on_recieve_setted(false)
{
	current_sock_amount.store(0);
	current_send_time.store(0);
}

/*
** 开启接收tcp连接
** 之后需要io_serce.poll()
*/

void ChatroomServer::start_accept()
{
	sockptr s(new ip::tcp::socket(io));
	socks[current_sock_amount] = s;
	cout << current_sock_amount.load() << endl;
	ac.async_accept(*socks[current_sock_amount], boost::bind(&ChatroomServer::accept_handler, this, this->current_sock_amount.load(), _1));
	boost::thread timingThread(boost::bind(&ChatroomServer::timing_thread_func, this));
	io.poll();
}

void ChatroomServer::timing_thread_func()
{
	auto timer = new deadline_timer(io, CLOCK_TIME);	 timer->async_wait(boost::bind(&ChatroomServer::timer_handler, this, _1, timer));
	io.run();
}

void ChatroomServer::timer_handler(error_code ec, deadline_timer* timer)
{
	// cout << "start one timer" << endl;
	timer->expires_at(timer->expires_at() + CLOCK_TIME);
	timer->async_wait(boost::bind(&ChatroomServer::timer_handler, this, _1, timer));
	//io.run();
}

void ChatroomServer::post(ChatMessage msg)
{
	msg_ptr mp(new ChatMessage(msg));
	ml.push_back(mp);
	error_code ec;
	broadcast();
}

/*
** 设置回调函数
*/

void ChatroomServer::set_on_recieve(boost::function<void(msg_ptr)> call_back_func)
{
	if (!is_on_recieve_setted)
		is_on_recieve_setted = true;
	on_recieve = call_back_func;
}

void ChatroomServer::broadcast()
{
	msg_ptr msg = ml.front();
	ml.pop_front();
	if (is_on_recieve_setted)
		on_recieve(msg);
	else
		cout << "on_recieve_func hasn't been setted, call ChatroomServer::set_on_recieve() to set" << endl;

	for (int i = 0; i < current_sock_amount; ++i)
	{
		buffer nbuf(new streambuf());
		boost::archive::binary_oarchive oa(*nbuf);
		oa << *msg;
		socks[i]->async_write_some(nbuf->data(), boost::bind(&ChatroomServer::write_handler, this, _1, nbuf));
	}
	io.poll();
	cout << "broadcast one msg, left : " << ml.size() << " msg(s). " << endl;
}

void ChatroomServer::write_handler(error_code ec, buffer nbuf)
{
	//delete nbuf;
	if (ec)
		return;
}

void ChatroomServer::accept_handler(int sockid, error_code ec)
{

	cout << "a new player from " << socks[sockid]->remote_endpoint().address() << "  join chatroom" << endl;
	if (ec)
		return;
	++current_sock_amount;

	do_read(sockid);
	if (current_sock_amount != 3)
		start_accept();
}


void ChatroomServer::do_read(int sockid)
{
	boost::shared_array<char> charbuf(new char[BUFFER_SIZE]);
	socks[sockid]->async_read_some(boost::asio::buffer(charbuf.get(), BUFFER_SIZE), boost::bind(&ChatroomServer::read_handler, this, sockid, charbuf, _1, _2));
	io.poll();
}

//use only for debug
//you are not supposed to use this

void ChatroomServer::post_helloworld()
{
	ChatMessage msg;
	msg.message = "Hello Server";
	msg.playerName = "QE";
	post(msg);
}


void ChatroomServer::read_handler(int sockid, boost::shared_array<char> charbuf, error_code ec, std::size_t bytes_transferred)
{
	if (ec)
		return;

	try {
		std::stringstream ss;
		ss << charbuf.get();
		boost::archive::text_iarchive ia(ss); // an exception here
		msg_ptr mp(new ChatMessage);
		ia >> *mp;
		//delete nbuf;
		ml.push_back(mp);
		broadcast();
		do_read(sockid);
	}
	catch (boost::archive::archive_exception e)
	{
		cout << "read message failed" << endl;
		do_read(sockid);
	}


}

/*
** 这个是回调函数的示例
** 每次有新消息到达时就会调用这个函数
** mp：消息类ChatMessage的指针，
*/
void server_message_listener(boost::shared_ptr<ChatMessage> mp)
{
	cout << "[player]\t\t[message]" << endl;
	cout << mp->playerName << "\t\t" << mp->message << endl;
}

/*
** 这个是启动方法
** 参数1：ip地址，参数2：回调函数
** 建议开一个新线程来执行本方法
*/
ChatroomServer* server_start(boost::function<void(boost::shared_ptr<ChatMessage>)> on_recieve)
{
	boost::asio::io_service *io = new boost::asio::io_service;
	ChatroomServer* server = new ChatroomServer(*io);
	server->set_on_recieve(on_recieve);
	server->start_accept();
	io->poll();
	return server;
}
