#include"chatroom_server.h"

int main()
{
	server_start_qe(server_message_listener);
}