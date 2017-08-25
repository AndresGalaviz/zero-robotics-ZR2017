#include <string.h>
#include <math.h>
#include "ZRGame.h"
#include "ZRGameInternal.h"
#include <stdlib.h>
#include "Constants.h"

//analyzers are always initialized to the same location
#ifdef ZR2D
	float analyzerPos[2][3] = { { 0.30f, -0.48f,  0.00f},
															{-0.30f,  0.48f,  0.00f} };
#else
	float analyzerPos[2][3] = { { 0.30f, -0.48f, -0.36f},
															{-0.30f,  0.48f, -0.36f} };
#endif

/*****************HIDDEN FUNCTIONS*****************/
// ex. void ZeroRoboticsGameImpl::initZones() -> Impl instead


/***********USER FUNCTIONS*************/

int ZeroRoboticsGame::hasAnalyzer()
{
	int result = 0;
	if (pimpl.challInfo.me.hasAnalyzer[0]) result += 1;
	if (pimpl.challInfo.me.hasAnalyzer[1]) result += 2;
	return result;
}

void ZeroRoboticsGame::getAnalyzer(bool pickedUp[2])
{
	pickedUp[0] = (pimpl.challInfo.me.hasAnalyzer[0] || pimpl.challInfo.other.hasAnalyzer[0]);
	pickedUp[1] = (pimpl.challInfo.me.hasAnalyzer[1] || pimpl.challInfo.other.hasAnalyzer[1]);
}

void ZeroRoboticsGameImpl::updateAnalyzer()
{
	// the analyzer is picked-up by standing over its location for 3 consecutive cycles with no translational velocity

	// the satellite must be "stoppped", otherwise no point in checking anything else
	float speed = mathVecMagnitude(&challInfo.me.sphState[3],3);
	if (speed > MAX_ANALYZER_SPEED)
	{
		challInfo.me.analyzerTime[0] = 0;
		challInfo.me.analyzerTime[1] = 0;
		return;
	}

	// now check distance and status of each analyzer
	for (int i=0; i < NUM_ANALYZERS; i++)
	{
		// only check if it has NOT been picked up already
		if ( (!challInfo.me.hasAnalyzer[i]) && (!challInfo.other.hasAnalyzer[i]))
		{
			// check distance to analyzer
			float tmpVec[3];
			mathVecSubtract(tmpVec,&challInfo.me.sphState[0],analyzerPos[i],3);
			float dist = mathVecMagnitude(tmpVec,3);
			
			#ifdef SHOW_DEBUG
				GAME_TRACE(("Update analyzer %d: dist = %4.3f, time = %d", i, dist, challInfo.me.analyzerTime[i]));
			#endif
			
			if (dist > SPHERE_RADIUS)
			{
				challInfo.me.analyzerTime[i] = 0;
			}
			else
			{
				// if stopped and at analyzer, increase time there
				challInfo.me.analyzerTime[i] ++;
				
				// if sat has been at analyzer for 3 seconds, pick it up
				if (challInfo.me.analyzerTime[i] > 3)
				{
					challInfo.me.hasAnalyzer[i] = true;
					GAME_TRACE(("Analyzer %d successfully picked up!", i));
				}
			}
		}
	}
}
