#pragma once
#include"chatroom_server.h"
#include<boost/asio/steady_timer.hpp>
#include<boost/asio/system_timer.hpp>
#include<boost/asio/high_resolution_timer.hpp>

#include <Windows.h>

int main()
{
	ChatroomServer* server = server_start(server_message_listener);

	while (true)
	{
		Sleep(1000);
		server->post_helloworld();
	}
}