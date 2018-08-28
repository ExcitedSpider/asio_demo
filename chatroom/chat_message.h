#pragma once
#include<string>
using std::string;
struct ChatMessage
{
	string message;
	string playerName;

	template<class Archive>
	void serialize(Archive &ar, const unsigned int version)
	{
		ar & message;
		ar & playerName;
	}
};