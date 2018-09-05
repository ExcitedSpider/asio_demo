#pragma once
#include"st_Player.h"
#include"st_CommonSetting.h"
struct client_message
{
	st_Player Player;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & Player;
	}
	friend class boost::serialization::access;

	client_message(st_Player p) :Player(p) {};
	client_message() {};
};
struct broadcast_message
{
	st_CommonSetting cs;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & cs;
	}

	broadcast_message(st_CommonSetting cs_) : cs(cs_) {}
	broadcast_message() {};

	friend class boost::serialization::access;
};