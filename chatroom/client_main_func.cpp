#pragma once
#include"chatroom_client.h"

int main() {
	client_start("127.0.0.1", client_message_listener);
}