#include <string.h>
#include <math.h>
#include "ZRGame.h"
#include "ZRGameInternal.h"
#include <stdlib.h>
#include "Constants.h"


/*****************HIDDEN FUNCTIONS*****************/
// ex. void ZeroRoboticsGameImpl::initZones() -> Impl instead



/***********USER FUNCTIONS*************/

void ZeroRoboticsGame::getAnalyzerPos(float pos[3], int analyzerID)
{
	memcpy(pos, pimpl.challInfo.world.analyzerCoords, sizeof(pos));
}

int ZeroRoboticsGame::hasAnalyzer()
{
	return pimpl.challInfo.me.hasAnalyzer;
}
