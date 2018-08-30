#pragma once
#include "chatroom_client.h"

/*
** 构造函数。
** 参数1：io_servce
*/

ChatroomClient::ChatroomClient(io_service & io_) :io(io_), is_on_recieve_setted(false), sock(new ip::tcp::socket(io)), is_connected(false){}

/*
** 开启tcp连接
** 之后需要io_serce.poll()
*/

void ChatroomClient::start(std::string ipv4)
{
	ip::tcp::endpoint ep(ip::address::from_string(ipv4), 667);
	sock->async_connect(ep, boost::bind(&ChatroomClient::connect_handler, this, _1));

}

void ChatroomClient::timing_thread_func()
{
	auto timer = new deadline_timer(io, CLOCK_TIME);	 timer->async_wait(boost::bind(&ChatroomClient::timer_handler, this, _1, timer));
	io.run();
}

void ChatroomClient::timer_handler(error_code ec, deadline_timer* timer)
{
	//cout << "start one timer" << endl;
	timer->expires_at(timer->expires_at() + CLOCK_TIME);
	timer->async_wait(boost::bind(&ChatroomClient::timer_handler, this, _1, timer));
	//io.run();
}

/*
** 发送一条消息
*/

void ChatroomClient::post(ChatMessage msg)
{
	if (is_connected)
	{
		cout << "post one msg" << endl;
		boost::shared_ptr<std::stringstream> ss(new std::stringstream);
		boost::archive::text_oarchive oa(*ss);
		oa << msg;
		string * str = new string(ss->str());
		sock->async_write_some(buffer(*str), boost::bind(&ChatroomClient::write_handler, this, _1, str));
		io.poll();
	}
	else
	{
		call_callback(msg_ptr(new ChatMessage(msg)));
	}
}

/*
** 设置回调函数
*/

void ChatroomClient::set_on_recieve(boost::function<void(msg_ptr)> call_back_func)
{
	if (!is_on_recieve_setted)
		is_on_recieve_setted = true;
	on_recieve = call_back_func;
}

void ChatroomClient::connect_handler(error_code ec)
{
	if (ec)
		return;
	is_connected = true;
	read();
	boost::thread timingThread(boost::bind(&ChatroomClient::timing_thread_func, this));
	
}

void ChatroomClient::read()
{
	sock->async_read_some(buf.prepare(BUFFER_SIZE), boost::bind(&ChatroomClient::read_handler, this, _1, _2));
}

void ChatroomClient::call_callback(msg_ptr mp)
{
	if (is_on_recieve_setted)
		on_recieve(mp);
	else
		cout << "on_recieve_func hasn't been setted, call ChatroomServer::set_on_recieve() to set" << endl;
}

void ChatroomClient::read_handler(error_code ec, size_t bites_trans)
{
	try {
		buf.commit(bites_trans);
		boost::archive::binary_iarchive ia(buf);
		msg_ptr mp(new ChatMessage);
		ia >> *mp;
		buf.consume(bites_trans);
		call_callback(mp);
	}
	catch (boost::archive::archive_exception e)
	{
		cout << "archive_exception occured: " << e.what() << endl;
	}
	read();
}

//use only for debug
//you are not supposed to use this

void ChatroomClient::post_helloworld()
{
	ChatMessage msg;
	msg.message = "Hello Client";
	msg.playerName = "Autumn";
	post(msg);
}

void ChatroomClient::write_handler(error_code ec, string* str)
{
	delete str;
	cout << "write handler" << endl;
	if (ec)
		return;
}

/*
** 这个是回调函数的示例
** 每次有新消息到达时就会调用这个函数
** mp：消息类ChatMessage的指针，
*/
void client_message_listener(boost::shared_ptr<ChatMessage> mp)
{
	cout << "[player]\t\t[message]" << endl;
	cout << mp->playerName << "\t\t" << mp->message << endl;
}

/*
** 这个是启动方法
** 参数1：回调函数
** 建议开一个新线程来执行本方法
*/
ChatroomClient* client_start(std::string ipv4, boost::function<void(boost::shared_ptr<ChatMessage>)> on_recieve)
{
	io_service* io = new io_service;
	ChatroomClient* client = new ChatroomClient(*io);
	client->set_on_recieve(on_recieve);
	client->start(ipv4);
	io->poll();
	return client;
}

