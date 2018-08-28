#pragma once
/*
Description : ��Ϸ�����趨
Author : �Ž���
Date : 2018/8/20
*/

using namespace std;

struct Player {
	int networkID, Level, score;//����ID,�ȼ�,����
	float speed, HP, damage;//�ٶȡ�Ѫ�����˺�
	bool animDone;//�����ж�
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