#include <string.h>
#include <math.h>
#include "ZRGame.h"
#include "ZRGameInternal.h"
#include <stdlib.h>
#include "Constants.h"


/*****************HIDDEN FUNCTIONS*****************/
// ex. void ZeroRoboticsGameImpl::initZones() -> Impl instead


/***********USER FUNCTIONS*************/

void ZeroRoboticsGame::getAnalyzerPos(float pos[3], int analyzerID){  //analyzerID should be 1 or 2
	if(analyzerID==1)
		memcpy(pos, pimpl.challInfo.world.analyzer1Coords, sizeof(pos));
	else
		memcpy(pos, pimpl.challInfo.world.analyzer2Coords, sizeof(pos));
}


int ZeroRoboticsGame::hasAnalyzer(){
	return pimpl.challInfo.me.hasAnalyzer;
}

void ZeroRoboticsGame::obtainAnalyzer(int analyzerID){
	float analyzerPos[3];
	getAnalyzerPos(analyzerPos,2);

	if(pimpl.challInfo.me.zrState[0]-analyzerPos[0]<=5 &&pimpl.challInfo.me.zrState[1]-analyzerPos[1]<=5 &&pimpl.challInfo.me.zrState[2]-analyzerPos[2]<=5){//5 is temporary value for how close you have to be to center of analyzer to get it
		pimpl.challInfo.me.hasAnalyzer+=analyzerID;
	}
}
