#pragma once
#include "chatroom_client.h"

/*
** 构造函数。
** 参数1：io_servce
*/

  ChatroomClient::ChatroomClient(io_service & io_) :io(io_), is_on_recieve_setted(false), sock(new ip::tcp::socket(io)) {}

/*
** 开启tcp连接
** 之后需要io_serce.run()
*/

  void ChatroomClient::start(std::string ipv4)
{
	ip::tcp::endpoint ep(ip::address::from_string(ipv4), 667);
	sock->async_connect(ep, boost::bind(&ChatroomClient::connect_handler, this, _1));
	io.run();
}

/*
** 发送一条消息
*/

  void ChatroomClient::post(ChatMessage msg)
{
	boost::archive::binary_oarchive oa(buf);
	oa << msg;
	sock->async_write_some(buf.prepare(buf.size()), boost::bind(&ChatroomClient::write_handler, this, _1));
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
	read();
}

  void ChatroomClient::read()
{
	sock->async_read_some(buf.prepare(BUFFER_SIZE), boost::bind(&ChatroomClient::read_handler, this, _1, _2));
}

  void ChatroomClient::read_handler(error_code ec, size_t bites_trans)
{
	buf.commit(bites_trans);
	buf.size();
	boost::archive::binary_iarchive ia(buf);
	msg_ptr mp(new ChatMessage);
	ia >> *mp;
	buf.consume(bites_trans);
	if (is_on_recieve_setted)
		on_recieve(mp);
	else
		cout << "on_recieve_func hasn't been setted, call ChatroomServer::set_on_recieve() to set" << endl;
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

  void ChatroomClient::write_handler(error_code ec)
{
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
void client_start(std::string ipv4, boost::function<void(boost::shared_ptr<ChatMessage>)> on_recieve, ChatroomClient* ptr)
{
	io_service io;
	ChatroomClient* client = new ChatroomClient(io);
	client->set_on_recieve(on_recieve);
	client->start(ipv4);
	ptr = client;
}

