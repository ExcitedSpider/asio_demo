#pragma once
#include"Player.h"
#include"CommonSetting.h"
#include <cereal/types/string.hpp>
struct client_message
{
	Player player;

	template<class Archive>
	void serialize(Archive & ar)
	{
		ar(player);
	}

	client_message(Player p) : player(p) {};
};
struct broadcast_message
{
	CommonSetting cs;

	template<class Archive>
	void serialize(Archive & ar)
	{
		ar(cs);
	}

	broadcast_message(CommonSetting cs_) : cs(cs_) {}
	broadcast_message() {};
};