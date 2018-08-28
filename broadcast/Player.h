#pragma once
/*
Description : 游戏人物设定
Author : 张建东
Date : 2018/8/20
*/

using namespace std;

struct Player {
	int networkID, Level, score;//网络ID,等级,分数
	float speed, HP, damage;//速度、血量、伤害
	bool animDone;//动画判断
	string name, nowDirection, nextDirection;

	template<class Archive>
	void serialize(Archive &ar, const unsigned int version)
	{
		ar & networkID;
		ar & Level;
		ar & score;
		ar & speed;
		ar & HP;
		ar & damage;
		ar & animDone;
		ar & name;
		ar & nowDirection;
		ar & nextDirection;
	}

	friend class boost::serialization::access;
};