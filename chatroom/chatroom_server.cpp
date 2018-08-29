#pragma once
#include "chatroom_server.h"

/*
** ���캯����
** ����1��io_servce
*/

 ChatroomServer::ChatroomServer(io_service & io_) : io(io_), ac(io, tcp::endpoint(tcp::v4(), 667)), socks(MAX_SOCKET_AMOUNT), is_on_recieve_setted(false)
{
	current_sock_amount.store(0);
	current_send_time.store(0);
}

/*
** ��������tcp����
** ֮����Ҫio_serce.run()
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
** ���ûص�����
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
** ����ǻص�������ʾ��
** ÿ��������Ϣ����ʱ�ͻ�����������
** mp����Ϣ��ChatMessage��ָ�룬
*/
void server_message_listener(boost::shared_ptr<ChatMessage> mp)
{
	cout << "[player]\t\t[message]" << endl;
	cout << mp->playerName << "\t\t" << mp->message << endl;
}

/*
** �������������
** ����1��ip��ַ������2���ص�����
** ���鿪һ�����߳���ִ�б�����
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
