#include "http_service.h"
#define SERVER_IP "127.0.0.1"

HttpService* HttpService::instance = nullptr;

HttpService::HttpService():is_gc_setted(false), is_jr_setted(false)
{
}

HttpService * HttpService::getInstance()
{
	if (HttpService::instance == nullptr)
	{
		HttpService::instance = new HttpService();
	}
	return instance;

}

void HttpService::joinRoom(std::string ipv4, std::string playerName)
{
	HttpRequest* request = new HttpRequest();
	request->setRequestType(HttpRequest::Type::POST);
	std::stringstream ss;
	ss << "http://"<<SERVER_IP<<":8080/join_room?ipv4=" << ipv4<<"&playername="<< playerName;
	request->setUrl(ss.str());
	request->setResponseCallback(boost::bind(&HttpService::joinroom_callback, this, _1,_2));
	request->setTag("Join Room");
	HttpClient::getInstance()->send(request);
	request->release();
}

void HttpService::openRoom(std::string ipv4, std::string playername)
{
	HttpRequest* request = new HttpRequest();
	request->setRequestType(HttpRequest::Type::POST);
	std::stringstream ss;
	ss << "http://"<< SERVER_IP <<":8080/open_room?ipv4=" << ipv4<<"&playername="<<playername;
	request->setUrl(ss.str());
	request->setResponseCallback(boost::bind(&HttpService::joinroom_callback, this, _1, _2));
	request->setTag("Open Rooms");
	HttpClient::getInstance()->send(request);
	request->release();

}

void HttpService::leaveRoom(std::string ipv4)
{
	HttpRequest* request = new HttpRequest();
	request->setRequestType(HttpRequest::Type::GET);
	std::stringstream ss;
	ss << "http://" << SERVER_IP << ":8080/leave_room?ipv4=" << ipv4;
	request->setUrl(ss.str());
	request->setResponseCallback(boost::bind(&HttpService::leaveroom_callback, this, _1, _2));
	request->setTag("Open Rooms");
	HttpClient::getInstance()->send(request);
	request->release();
}

void HttpService::getRoomsViaHttp()
{
	HttpRequest* request = new HttpRequest();
	request->setRequestType(HttpRequest::Type::GET);
	std::stringstream ss;
	ss << "http://" << SERVER_IP << ":8080/get_rooms";
	request->setUrl(ss.str());
	request->setResponseCallback(boost::bind(&HttpService::getroom_callback,this,_1,_2));
	request->setTag("Get Rooms");
	HttpClient::getInstance()->send(request);
	request->release();
}

void HttpService::set_getrooms_cb(boost::function<void(room_list_type*)> callback)
{
	is_gc_setted = true;
	m_getrooms_callback = callback;
}

void HttpService::set_joinroom_cb(boost::function<void(bool)> callback)
{
	is_jr_setted = true;
	m_getrooms_callback = callback;
}

room_list_type * HttpService::parseRoomListJson(const char * json)
{
	rapidjson::Document d;
	d.Parse<0>(json);
	if (d.HasParseError())
	{
		cocos2d::log("parse Json error");
		return new room_list_type();
	}
	if (d.IsObject() && d.HasMember("roomAmount") && 
		d.HasMember("errorCode") && 
		d.HasMember("roomlist") &&
		d.HasMember("nameList"))
	{
		if (d["errorCode"].GetInt())
		{
			cocos2d::log("server error");
			return new room_list_type();
		}
		else
		{
			auto rl = new room_list_type();
			int roomAmount = d["roomAmount"].GetInt();
			const rapidjson::Value& roomArray = d["roomlist"];
			assert(roomArray.IsArray());
			for (auto i = 0; i != roomAmount; i++)
			{
				auto object = roomArray[i].GetObjectW();
				auto room = new st_room;
				room->ipv4 = object["ipv4"].GetString();
				auto namelist = object["nameList"].GetArray();
				for (int i = 0; i < namelist.Size(); i++)
				{
					room->nameList.push_back(namelist[i].GetString());
				}
				room->playernum = object["playerNum"].GetInt();
				rl->push_back(room_ptr_type(room));
			}
			return rl;
		}
	}
}

const room_ptr_type HttpService::parseRoomJson(const char * json)
{
	rapidjson::Document d;
	d.Parse<0>(json);
	if (d.HasParseError())
	{
		cocos2d::log("parse Json error");
		return room_ptr_type();
	}
	if (d.IsObject() && d.HasMember("ipv4") && d.HasMember("playerNum"))
	{
		auto room = new st_room();
		room->ipv4 = d["ipv4"].GetString();
		room->playernum = d["playerNum"].GetInt();
		return room_ptr_type(room);
	}
	cocos2d::log("parse Json error");
	return room_ptr_type();

}

void HttpService::getroom_callback(HttpClient * sender, HttpResponse * response)
{
	std::stringstream ss;
	auto charvec = response->getResponseData();
	for (int i = 0; i < charvec->size(); i++)
	{
		ss << charvec->at(i);
	}
	room_list_type *rl = parseRoomListJson(ss.str().c_str());
	m_cached_list = rl;
	if (is_gc_setted)
	{
		m_getrooms_callback(m_cached_list);
	}
	else
	{
		cocos2d::log("have not set get rooms callback in httpservice!");
	}
}

void HttpService::joinroom_callback(HttpClient * sender, HttpResponse * response)
{
	std::stringstream ss;
	auto charvec = response->getResponseData();
	for (int i = 0; i < charvec->size(); i++)
	{
		ss << charvec->at(i);
	}
	bool join_succeed = false;
	if (ss.str() == "true")
		join_succeed = true;

	cocos2d::log(ss.str().data());
	if(is_jr_setted)
		m_join_room_callback(join_succeed);
	else
	{
		cocos2d::log("have not set join room callback in httpservice!");
	}
}

void HttpService::leaveroom_callback(HttpClient * sender, HttpResponse * response)
{
}
