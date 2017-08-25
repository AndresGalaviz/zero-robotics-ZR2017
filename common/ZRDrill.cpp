#include <string.h>
#include <math.h>
#include "ZRGame.h"
#include "ZRGameInternal.h"
#include <stdlib.h>
#include "Constants.h"

/*****************HIDDEN FUNCTIONS*****************/

/***********USER FUNCTIONS*************/
// TODO: What happens when you run into de ground? 
// TODO: Tank has to be pointing down, position +4cm from surface, speed < 1cm/s. Penalyzed if conditions not met
bool ZeroRoboticsGame::startDrill()
{
	// if the drill has already been started, don't do anything - it must be stopped before a new operation starts
	if (pimpl.challInfo.me.drillEnabled)
	{
		GAME_TRACE(("Drill already enabled, startDrill aborted."));
		return false;
	}

  int playerX = GRID_X(pimpl.challInfo.me.zrState[0]);
  int playerY = GRID_Y(pimpl.challInfo.me.zrState[1]);
  #ifndef ZR2D
	  int playerZ = GRID_Z(pimpl.challInfo.me.zrState[2]);
	#endif
	
	// store where the drill operation started
	pimpl.challInfo.me.drillSquare[0] = playerX;
	pimpl.challInfo.me.drillSquare[1] = playerY;
	
	// store current satellite orientation at start of drill process, but only X & Y, ignore Z
	pimpl.challInfo.me.drillInitAtt[0] = pimpl.challInfo.me.zrState[ZR_ATT_X];
	pimpl.challInfo.me.drillInitAtt[1] = pimpl.challInfo.me.zrState[ZR_ATT_Y];
	mathVecNormalize(pimpl.challInfo.me.drillInitAtt, 2);												// normalize 2D pointing vector
	pimpl.challInfo.me.drillInitAtt[2] = 0.0f;																	// tank always points to surface, so this should always be 0
	
	// the start of the drill operation must happen when its not rotating
	float rot_rate = mathVecMagnitude(&pimpl.challInfo.me.sphState[RATE_X],3);
	if (rot_rate > MAX_DRILL_INIT_RATE)
	{
		pimpl.challInfo.me.drillError = true;
		GAME_TRACE(("Drill ERROR : drill started when rotating too fast (%4.3f)!",rot_rate));
	}

	// start drill
	pimpl.challInfo.me.drillEnabled = true;
	GAME_TRACE(("Drill enabled at sample square [%d, %d] with initial attitude [%4.3f, %4.3f]", playerX, playerY, pimpl.challInfo.me.drillInitAtt[0], pimpl.challInfo.me.drillInitAtt[1]));
	
	pimpl.updateDrill();

	// return the value of the error after updating once upon start
	return pimpl.challInfo.me.drillError;
}

bool ZeroRoboticsGame::getDrillError()
{
	return pimpl.challInfo.me.drillError;
}

bool ZeroRoboticsGame::getDrillEnabled()
{
	return pimpl.challInfo.me.drillEnabled;
}

void ZeroRoboticsGameImpl::updateDrill()
{
	// if the drill is enabled, checks to see if the drilling motion is complete and if a drill error takes place
	if (!challInfo.me.drillEnabled) return;			// if the drill is off, there's nothing to do
	if (challInfo.me.drillError) return;				// if there already is a drill error, don't do anything until its corrected
	
	// the satellite must be moving less than MIN_DRILL_SPEED at all times when the drill is on
	float	speed = mathVecMagnitude(&challInfo.me.zrState[VEL_X], 3);
	if (speed > MAX_DRILL_SPEED)
	{
		GAME_TRACE(("Drill ERROR : moving too fast (%4.3f m/s).", speed));
		challInfo.me.drillError = true;
		return;
	}
	
	// the satellite should remain within the original square it started drilling at
	if (!checkAtSquare(challInfo.me.drillSquare[0], challInfo.me.drillSquare[1]))
	{
		GAME_TRACE(("Drill ERROR : satellite moved out of original sample square!"));
		challInfo.me.drillError = true;
		return;
	}
	
	// the tank should always be pointing down, which means that the ZR Z poiting vector should be always about zero
	if (fabs(challInfo.me.zrState[ZR_ATT_Z]) > MAX_DRILL_POINT_ERROR)
	{
		GAME_TRACE(("Drill ERROR : tank not pointing at surface (%4.3f).", fabs(challInfo.me.zrState[ZR_ATT_Z])));
		challInfo.me.drillError = true;
		return;
	}
	
	// check if the rotation is complete
	float attInner = mathVecInner(challInfo.me.drillInitAtt, &challInfo.me.zrState[ZR_ATT_X], 3);
	bool rot_complete =  attInner < (-MIN_DRILL_ROTATION);
	if (rot_complete && !challInfo.me.sampleReady)
	{		
		// update number of drills at this grid square
		challInfo.world.grid[challInfo.me.drillSquare[0]][challInfo.me.drillSquare[1]].numDrills++;
		challInfo.me.lastDrill[0] = challInfo.me.drillSquare[0];
		challInfo.me.lastDrill[1] = challInfo.me.drillSquare[1];
		challInfo.me.lastDrillTime = apiImpl.api->getTime();
		
		// set ready variable
		challInfo.me.sampleReady = true;
		GAME_TRACE(("Sample ready for pickup (number of drills at this sample square = %d).", challInfo.world.grid[challInfo.me.drillSquare[0]][challInfo.me.drillSquare[1]].numDrills));
	}
}

bool ZeroRoboticsGame::stopDrill() {

	GAME_TRACE(("Drill Disabled"));

	pimpl.challInfo.me.drillEnabled = false;			// disable drill
	pimpl.challInfo.me.drillError   = false;			// reset errors

	pimpl.challInfo.me.drillSquare[0] = OFF_GRID;		// reset current square
	pimpl.challInfo.me.drillSquare[1] = OFF_GRID;

	return true;
}
