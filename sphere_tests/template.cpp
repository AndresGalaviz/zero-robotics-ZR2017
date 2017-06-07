#include "ZRGame.h"
#include "ZR_API.h"
#include "ZRUser.hpp"
#include "ZR_API.h"

static ZeroRoboticsGame &game = ZeroRoboticsGame::instance();
static ZeroRoboticsAPI &api = ZeroRoboticsAPI::instance();

//Implement your simulation code in init() and loop()
class ZRUser01 : public ZRUser{
void init(){
}

void loop(){
}

};



ZRUser *zruser01 = new ZRUser01;
