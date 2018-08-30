#pragma once
#include"chatroom_server.h"
#include <boost\random.hpp>

#include <Windows.h>

int main()
{
	ChatroomServer* server = server_start(server_message_listener);
	boost::random::mt19937 rng;
	while (true) {
		Sleep(2500);
		server->post_helloworld();
	}
}