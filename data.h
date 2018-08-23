#pragma once
#include <cereal/archives/binary.hpp>

#include <string>
struct innerdata {
	int x;
	int y;
	template<class Archive>
	void serialize(Archive & ar) {
		ar(cereal::make_nvp("x", x), cereal::make_nvp("y", y));
	}
	innerdata() :x(0), y(0) {};
};

struct data
{
	std::string msg;
	innerdata in;
public:
	template<class Archive>
	void serialize(Archive &ar)
	{
		ar(cereal::make_nvp("msg", msg), cereal::make_nvp("innderdata", in));
	}
};