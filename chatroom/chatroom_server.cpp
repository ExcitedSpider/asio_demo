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
** 之后需要io_serce.run()
*/

 void ChatroomServer::start_accept()
{
	sockptr s(new ip::tcp::socket(io));
	socks[current_sock_amount] = s;
	ac.async_accept(*socks[current_sock_amount], boost::bind(&ChatroomServer::accept_handler, this, socks[current_sock_amount], _1));
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
	boost::archive::binary_oarchive oa(buf);
	oa << *msg;
	auto cb = buf.data();
	for (int i = 0; i < current_sock_amount; ++i)
	{
		socks[i]->async_write_some(buffer(cb), boost::bind(&ChatroomServer::write_handler, this, _1));
	}
	io.poll();
	cout << "broadcast one msg, left : " << ml.size() << " msg(s). " << endl;
	if (is_on_recieve_setted)
		on_recieve(msg);
	else
		cout << "on_recieve_func hasn't been setted, call ChatroomServer::set_on_recieve() to set" << endl;
	if (ml.size())
		broadcast();

}

 void ChatroomServer::write_handler(error_code ec)
{
	if (ec)
		return;

}

 void ChatroomServer::accept_handler(sockptr sp, error_code ec)
{
	cout << "a new player from " << sp->remote_endpoint().address() << "  join chatroom" << endl;
	if (ec)
		return;
	++current_sock_amount;

	do_read(sp);
	start_accept();
}

 void ChatroomServer::do_read(sockptr sp)
{
	sp->async_read_some(buf.prepare(BUFFER_SIZE), boost::bind(&ChatroomServer::read_handler, this, sp, _1, _2));
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

 void ChatroomServer::read_handler(sockptr sp, error_code ec, size_t bites_trans)
{
	buf.commit(bites_trans);
	boost::archive::binary_iarchive ia(buf);
	msg_ptr mp(new ChatMessage);
	ia >> *mp;
	buf.consume(bites_trans);
	ml.push_back(mp);
	broadcast();

	do_read(sp);
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
void server_start_qe(boost::function<void(boost::shared_ptr<ChatMessage>)> on_recieve, ChatroomServer *ptr)
{
	boost::asio::io_service io;
	ChatroomServer* server = new ChatroomServer(io);
	server->set_on_recieve(on_recieve);
	server->start_accept();
	io.run();
	ptr = server;
}
