#pragma once
#include "st_Player.h"

/*
Description : ����һЩȫ�ֱ�������
Author : �Ž���
Date : 2018/8/20
*/

struct st_CommonSetting {
	st_Player st_Player[5];

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & st_Player;
	}

	friend class boost::serialization::access;

};
