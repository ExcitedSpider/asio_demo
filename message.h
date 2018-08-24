#pragma once
struct message
{
	int x;
	template<class Archive>
	void serialize(Archive & ar)
	{
		ar(cereal::make_nvp("x", x));
	}
	message() :x(0) {};
};