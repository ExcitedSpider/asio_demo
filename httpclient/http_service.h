#pragma once

#include "network/HttpClient.h"
#include "cocos2d.h"
#include <sstream>
#include "json/rapidjson.h"
#include "json/document.h"
#include "boost/function.hpp"
#include "st_Room.h"
#include "boost/smart_ptr.hpp"
#include <vector>
#include "boost/bind.hpp"

using namespace cocos2d::network;

typedef std::vector<boost::shared_ptr<st_room>> room_list_type;
typedef boost::shared_ptr<st_room> room_ptr_type;

class HttpService
{
private:
	room_list_type *m_cached_list;
	boost::function<void(room_list_type *)> m_getrooms_callback;
	bool is_gc_setted;
	boost::function<void(bool)> m_join_room_callback;
	bool is_jr_setted;
	static HttpService* instance;
public:
	static HttpService* getInstance();
	void joinRoom(std::string ipv4, std::string playerName);
	void openRoom(std::string ipv4, std::string playername);
	void leaveRoom(std::string ipv4);
	void getRoomsViaHttp();
	void set_getrooms_cb(boost::function<void(room_list_type *)> callback);
	void set_joinroom_cb(boost::function<void(bool)> callback);
private:
	HttpService();
	room_list_type * parseRoomListJson(const char * json);
	const room_ptr_type parseRoomJson(const char * json);
	void getroom_callback(HttpClient *sender, HttpResponse *response);
	void joinroom_callback(HttpClient *sender, HttpResponse *response);
	void leaveroom_callback(HttpClient *sender, HttpResponse *response);

};