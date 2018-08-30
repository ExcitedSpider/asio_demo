#pragma once
#include"chatroom_client.h"
#include <Windows.h>

int main() 
{
	ChatroomClient * client =  client_start("127.0.0.1", client_message_listener);
	
	while (true)
	{
		Sleep(3000);
		client->post_helloworld();
	}

	exit(0);
}