/*
    Description : 玩家类实现
    Author : 张建东
    Date : 2018-8-21
*/

#include "Player.h"
#include "CommonSetting.h"
#include "cocos2d.h"
using namespace std;

void Player::UpdateAnim() {
    string prename = "player_move_";
    if (getNextDirection() == "Stop") {
        setAnimDone(true);
        player->stopAllActions();
        return;
    }
    if (getNextDirection() == getNowDirection() && getAnimDone()) {
        string name = prename + nextDirection;
        Animate* animate = Animate::create(AnimationCache::getInstance()->getAnimation(name));
        auto setDoneToTrue = CallFunc::create([this]() {
            setAnimDone(true);
            log("test ok");
        });
        auto sequence = Sequence::create(animate,setDoneToTrue,NULL);
        player->runAction(sequence);        
        setAnimDone(false);
    }
    if(getNextDirection()!=getNowDirection()){
        string name = prename + nextDirection;
        player->stopAllActions();
        Animate* animate = Animate::create(AnimationCache::getInstance()->getAnimation(name));
        auto setDoneToTrue = CallFunc::create([this]() {
            setAnimDone(true);
        });
        auto sequence = Sequence::create(animate, setDoneToTrue, NULL);

        player->runAction(sequence);
        setNowDirection(getNextDirection());        
        setAnimDone(false);
    }
}
pair<int, int> directionScale[8] = { {-10,0} ,{-7,7},{0,10},{7,7},{10,0},{7,-7},{0,-7},{-7,-7} };
void Player::Move() {
    setSpeed(0.5);
    for(int i=0;i<8;i++)
        if (nextDirection == CommonSetting::getInstance()->Direction[i]) {
            auto pointDestination = Vec2(directionScale[i].first*speed, directionScale[i].second*speed) + player->getPosition();
            
            auto moveby = MoveBy::create(1, Vec2(directionScale[i].first*speed,directionScale[i].second*speed));
            player->runAction(moveby);
        }
}

void Player::setPlayer(Sprite* x) {player = x;}

Sprite* Player::getPlayer() { return player; }

void Player::CreatePlayer(int id) {
    auto player_texture = Director::getInstance()->getTextureCache()->addImage("player_move.png");
    CommonSetting::getInstance()->player[id].setPlayer(Sprite::createWithTexture(player_texture, CC_RECT_PIXELS_TO_POINTS(Rect(0, 0, 128, 128))));
    CommonSetting::getInstance()->setNetworkID(id);
}

Player::Player() {
    damage = 60.0f;
    HP = 100.0f;
    name = "SEU" + to_string(networkID);
    nextDirection = "Stop";
    nowDirection = "West";
    setAnimDone(true);
}

//速度
float Player::getSpeed() const { return speed; }
void Player::setSpeed(const float& x) { speed = x; }
//血量
float Player::getHP() const { return HP; }
void Player::setHP(const float& x) { HP = x; }
//伤害
float Player::getDamage() const { return damage; }
void Player::setDamage(const float& x) { damage = x; }
//名称
string Player::getName() const { return name; }
void Player::setName(const string& x) { name = x; }
//下一步方向
string Player::getNextDirection() const { return nextDirection; }
void Player::setNextDirection(const string& x) { nextDirection = x; }
//当前方向
string Player::getNowDirection() const { return nowDirection; }
void Player::setNowDirection(const string& x) { nowDirection = x; }
bool Player::getAnimDone()const { return animDone; }
void Player::setAnimDone(const bool& x) { animDone = x; }