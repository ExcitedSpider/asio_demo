#pragma once
#include"chatroom_client.h"
#include <Windows.h>
#include <boost\random.hpp>

int main() 
{
	ChatroomClient * client =  client_start("127.0.0.1", client_message_listener);
	
	boost::random::mt19937 rng;
	while (true)
	{
		Sleep(1500);
		client->post_helloworld();
	}
}