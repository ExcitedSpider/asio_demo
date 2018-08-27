#pragma once
#include <cereal/archives/json.hpp>
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
	void serialize(Archive &ar)
	{
		ar(networkID, Level, score, speed, HP, damage, animDone, name, nowDirection, nextDirection);
	}
};