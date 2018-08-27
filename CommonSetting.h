#pragma once
#include <cereal/archives/json.hpp>
#include "Player.h"

/*
Description : 用于一些全局变量设置
Author : 张建东
Date : 2018/8/20
*/

struct CommonSetting {
	int networkID = 1;
	int networkCnt = 1;
	Player player[5];

	template<class Archive>
	void serialize(Archive& ar)
	{
		ar(player, networkCnt, networkID);
	}
};
