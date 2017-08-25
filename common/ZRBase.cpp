#include "ZRGame.h"
#include <string.h>
#include "ZRGameInternal.h"
#include <math.h>
#define zr_rand() rand()
// TODO: Ports might take processing time?
// TODO: Implement a SPHERES-like base with the characteristics discussed in the past meeting (Fri 19)

bool ZeroRoboticsGame::atBaseStation()
{
	// check that all requirements to be at the base station are met
	
	// must be within 0.15cm of center of work area
	float dist = mathVecMagnitude(&pimpl.challInfo.me.sphState[0],3);
	if (dist > BASE_RADIUS)
	{
		GAME_TRACE(("Base Station: too far from Base Station."));
		return false;
	}
	
	// must have a speed under 0.01m/s
	float speed = mathVecMagnitude(&pimpl.challInfo.me.sphState[3],3);
	if (speed > MAX_BASE_SPEED)
	{
		GAME_TRACE(("Base Station: moving too fast."));
		return false;
	}
	
	float rot_rate = mathVecMagnitude(&pimpl.challInfo.me.sphState[RATE_X],3);
	if (rot_rate > MAX_BASE_RATE)
	{
		GAME_TRACE(("Base Station: rotating too fast."));
		return false;
	}
	
	#ifndef ZR2D
		// must be pointing correctly
	#endif
	
	return true;
}
