#pragma once
#include "Player.h"

/*
Description : ����һЩȫ�ֱ�������
Author : �Ž���
Date : 2018/8/20
*/

struct CommonSetting {
	int networkID = 1;
	int networkCnt = 1;
	Player player[5];

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & player;
		ar & networkID;
		ar & networkCnt;
	}

	friend class boost::serialization::access;

};
