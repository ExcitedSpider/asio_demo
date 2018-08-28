#pragma once
#include"Player.h"
#include"CommonSetting.h"
struct client_message
{
	Player player;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & player;
	}
	friend class boost::serialization::access;

	client_message(Player p) :player(p) {};
	client_message() {};
};
struct broadcast_message
{
	CommonSetting cs;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & cs;
	}

	broadcast_message(CommonSetting cs_) : cs(cs_) {}
	broadcast_message() {};

	friend class boost::serialization::access;
};