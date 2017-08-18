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
	getAnalyzerPos(analyzerPos,analyzerID);
	//GAME_TRACE(("epsilon calculating things %f %f %f",pimpl.challInfo.me.zrState[0]-analyzerPos[0], pimpl.challInfo.me.zrState[1]-analyzerPos[1],pimpl.challInfo.me.zrState[2]-analyzerPos[2]));
	if(pimpl.challInfo.me.zrState[0]-analyzerPos[0]<=.05 && pimpl.challInfo.me.zrState[1]-analyzerPos[1]<=.05 &&pimpl.challInfo.me.zrState[2]-analyzerPos[2]<=.05
		&&pimpl.challInfo.me.zrState[3]<.01&&pimpl.challInfo.me.zrState[4]<.01&&pimpl.challInfo.me.zrState[5]<.01){//within .05 meters with a velocity of <.01 m/s
		pimpl.challInfo.me.hasAnalyzer+=analyzerID;
		GAME_TRACE(("obtained analyzer"));
	}
}
