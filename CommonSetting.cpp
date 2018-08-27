#include "cocos2d.h"
#include "CommonSetting.h"
/*
    Description : ����һЩȫ�ֱ�������
    Author : �Ž���
    Date : 2018/8/20
*/

CommonSetting* CommonSetting::instance = nullptr;

const std::string CommonSetting::Direction[8] = { "West", "NorthWest", "North", "NorthEast",
                                                  "East", "SouthEast", "South", "SouthWest" };

CommonSetting::CommonSetting() {

}

CommonSetting::~CommonSetting() {

}

int CommonSetting::getNetworkID() { return networkID; }
int CommonSetting::getNetworkCnt() { return networkCnt; }
void CommonSetting::setNetworkID(const int & x) { networkCnt = x; }

CommonSetting* CommonSetting::getInstance() {
    if (nullptr == instance) instance = new CommonSetting();
    return instance;
}