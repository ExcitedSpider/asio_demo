#pragma once
struct innerdata {
	int x;
	int y;
	int z;
	template<class Archive>
	void serialize(Archive & ar) {
		ar(cereal::make_nvp("x", x), cereal::make_nvp("y", y), cereal::make_nvp("z", z));
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